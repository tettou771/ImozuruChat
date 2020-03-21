#include "ImozuruChatBoard.h"

ImozuruChatBoard::ImozuruChatBoard() {
}

ImozuruChatBoard::~ImozuruChatBoard() {
	auto delVector = [](vector<MessageObject*> M) {
		for (auto m : M) {
			delete m;
		}
		M.clear();
	};
	delVector(messages);

	mqtt.disconnect();
}

void ImozuruChatBoard::setup() {
	// box2d の世界を作る
	box2d.init();
	box2d.createBounds(0, 0, ofGetWidth(), ofGetHeight());
	float box2dFps = 60;
	box2d.setFPS(box2dFps);
	box2d.enableEvents();
	box2d.registerGrabbing(); // mouse系のハンドラがこれで有効になる
	box2d.enableGrabbing(); // オブジェクトのマウスドラッグを有効化
	box2d.setGravity(0, 3);

	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);

	ofTrueTypeFontSettings settings("fonts/NotoSansCJKjp-Regular.otf", 10);
	settings.addRanges(ofAlphabet::Latin);
	settings.addRanges(ofAlphabet::Japanese);
	messageFont.load(settings);

	ime.setFont("fonts/NotoSansCJKjp-Regular.otf", 14);

	// 設定値をロード
	loadConfig();
    myHue = ofRandom(255);

	// 接続
	mqttClientID = "ImozuruChatUser_" + ofToString(ofRandom(1000000));
	mqtt.begin(mqttAddress, mqttPort);
	mqttTopic = "/message";
	mqtt.connect(mqttClientID, mqttUser, mqttPassword);
	ofAddListener(mqtt.onOnline, this, &ImozuruChatBoard::onMqttOnline);
	ofAddListener(mqtt.onOffline, this, &ImozuruChatBoard::onMqttOffline);
	ofAddListener(mqtt.onMessage, this, &ImozuruChatBoard::onMqttMessage);

	newMessageOffsetPos = ofPoint(0, 40);
	ime.enable();

	box2dUpdater.setup(&box2d, &box2dMutex);
    
    srand(ofGetUnixTime());
}

void ImozuruChatBoard::update() {
	mqtt.update();
	if (!mqtt.connected()) {
		// もし接続できていなかったら、再接続を試みる
		// 数秒に1度
		if (ofGetFrameNum() % 256 == 0) {
			ofLogNotice() << "retry connect" << endl;
			mqtt.connect(mqttClientID, mqttUser, mqttPassword);
		}
	}

	box2dMutex.lock();
	for (auto m : messages) {
		m->update();
	}
	box2dMutex.unlock();
}

void ImozuruChatBoard::draw() {
    // 説明
    stringstream help;
    help << "使い方" << endl;
    help << "適当なところをクリックして文字入力" << endl;
#ifdef WIN32
    help << "Ctrl + Enter で送信" << endl;
#elif TARGET_OS_MAC
    help << "Command + Enter で送信" << endl;
#endif
    help << "人のコメントに返信するときは、それをクリックして返信" << endl;
    #ifdef WIN32
    help << "文字入力切り替え Ctrl + スペースキー" << endl;
    #elif TARGET_OS_MAC
    help << "文字入力切り替え Shift + スペースキー" << endl;
#endif
    
    ofSetColor(100);
    messageFont.drawStringAlign(help.str(), 10, 10, AlignableFont::Align::LEFT, AlignableFont::Align::TOP);
    
	box2dMutex.lock();
	for (auto m : messages) {
		m->draw();
	}
	for (auto m : selected) {
		m->drawSelected();
	}
	box2dMutex.unlock();

	// 入力中の文字を描画
	ofPoint inputPos;
	if (selected.empty()) {
		inputPos.x = ofGetWidth() / 2;
		inputPos.y = ofGetHeight() / 2;
	}
	else {
		for (auto s : selected) {
			inputPos += s->box2dRect->getPosition();
		}
		inputPos /= selected.size();
		inputPos += newMessageOffsetPos;
	}
	ime.draw(inputPos);

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	if (!mqtt.connected()) {
		ofBackground(100);
		ofSetColor(255);
		messageFont.drawStringAlignCenter("Not connect to MQTT server", ofGetWidth() / 2, ofGetHeight() / 2);
	}
}

void ImozuruChatBoard::keyPressed(ofKeyEventArgs& key) {
	// IMEと干渉するのでよく注意して実装すること

	switch (key.key) {

	case OF_KEY_RETURN:
#ifdef WIN32
		// Ctrlキーを押していたら生成する
		if (ofGetKeyPressed(OF_KEY_CONTROL)) {
#elif TARGET_OS_MAC
            // Commandキーを押していたら生成する
            if (ofGetKeyPressed(OF_KEY_COMMAND)) {
#else
            // Ctrlキーを押していたら生成する
            if (ofGetKeyPressed(OF_KEY_CONTROL)) {
#endif
			makeMessage(ime.getAll());
			ime.disable();
			ime.clear();
		}
		break;
	}
}

void ImozuruChatBoard::keyReleased(ofKeyEventArgs& key) {
	switch (key.key) {
	case OF_KEY_RETURN:
		if (!ime.isEnabled()) ime.enable();
		break;
	}
}

void ImozuruChatBoard::mousePressed(ofMouseEventArgs& mouse) {
	selected.clear();

	for (auto m : messages) {
		if (m->inside(mouse.x, mouse.y)) {
			selected.push_back(m);
			break;
		}
	}
}

void ImozuruChatBoard::mouseDragged(ofMouseEventArgs& mouse) {
}

void ImozuruChatBoard::mouseReleased(ofMouseEventArgs& mouse) {
}

void ImozuruChatBoard::mouseMoved(ofMouseEventArgs& mouse) {
}

void ImozuruChatBoard::mouseEntered(ofMouseEventArgs& mouse) {
}

void ImozuruChatBoard::mouseExited(ofMouseEventArgs& mouse) {
}

void ImozuruChatBoard::mouseScrolled(ofMouseEventArgs& mouse) {
}

void ImozuruChatBoard::windowResized(int w, int h) {
	box2d.createBounds(0, 0, w, h);
}

void ImozuruChatBoard::onMqttOnline() {
	ofLogNotice() << "MQYY connected";
	mqtt.subscribe(mqttTopic);
}

void ImozuruChatBoard::onMqttOffline() {
	ofLogError() << "MQTT offline";
}

void ImozuruChatBoard::onMqttMessage(ofxMQTTMessage& message) {
	ofLogNotice("MQTT Received") << message.topic << " " << message.payload;

	// データが有る限り読む
	if (message.topic == mqttTopic) {
		string receivedMessage = message.payload;
		if (receivedMessage.length() > 0) makeMessageFromReceivedData(receivedMessage);
	}
}

void ImozuruChatBoard::makeMessage(string message) {
	// 自分の固有id
	// 大きな数の乱数なので、次のURLを参考にした
	// https://oshiete.goo.ne.jp/qa/2111837.html
	int D = ULONG_MAX / RAND_MAX;
	int M = ULONG_MAX % RAND_MAX;
	unsigned long id = rand() * D + rand() % M;
    id = ofRandom(1000000);

	// 親オブジェクトのid
	unsigned long parentId = 0;
	if (!selected.empty()) {
		parentId = selected[0]->id;
	}

	string data = "";
	string d = "|";
	data += ofToString(id) + d;
	data += myName + d;
	data += message + d;
	data += ofToString(myHue) + d;
	data += ofToString(parentId);

	sendMessageData(data);
}

void ImozuruChatBoard::sendMessageData(string& messageData) {
	if (!mqtt.connected()) {
		cout << "MQTT connection is losted." << endl;
		return;
	}

	ofLog() << "Publish " << mqttTopic << "\"" << messageData << "\"";
	mqtt.publish(mqttTopic, messageData);
}

void ImozuruChatBoard::makeMessageFromReceivedData(string& _receivedMessageData) {
	ofLog() << "Received: " << _receivedMessageData;

	int requireElementNum = 5;
	auto elements = ofSplitString(_receivedMessageData, "|");
	// 要素の数が満たなかったら帰る
	if (elements.size() < requireElementNum) {
		ofLog() << "element num " << elements.size() << " less than " << requireElementNum << ".";
		return;
	}

	unsigned long id = ofToInt64(elements[0]);
	string talkerName = elements[1];
	string messageStr = elements[2];
	float hue = ofToFloat(elements[3]);
	unsigned long parentId = ofToInt64(elements[4]);

	// 空の文字列だったら無視する
	if (messageStr.length() <= 0) return;

	// 親オブジェクトを探す
	vector<MessageObject*> parents = vector<MessageObject*>();
	for (auto m : messages) {
		if (m->id == parentId) {
			parents.push_back(m);
		}
	}

	// 生成する座標
	ofPoint pos;

	// 非選択状態ならとりあえず中央に出す
	if (parents.empty()) {
		pos.x = ofGetWidth() / 2;
		pos.y = ofGetHeight() / 2;
	}
	// 選択しているオブジェクトがあるなら、その下に出す
	else {
		for (auto s : parents) {
			pos += s->box2dRect->getPosition();
		}
		pos /= parents.size();

		// 選択しているメッセージの下に出したいので、
		// 予め決めているオフセットを足す
		pos += newMessageOffsetPos;
	}

	ofRectangle rect;
	messages.push_back(new MessageObject(id, talkerName, messageStr, hue, parents, &box2d, pos, &messageFont));

	// 選択されたオブジェクトの子につくときは、
	// 選択対象を子に切り替える。
	bool parentIsSelected = false;
	for (auto p : parents) {
		for (auto s : selected) {
			if (p == s) {
				parentIsSelected = true;
				break;
			}
		}
		if (parentIsSelected) break;
	}

	if (parentIsSelected) {
		selected.clear();
		selected.push_back(messages.back());
	}
}

void ImozuruChatBoard::loadConfig() {
	ofxXmlSettings xml;
	if (xml.load("config.xml")) {
		if (xml.tagExists("config")) {
			xml.pushTag("config");

			if (xml.tagExists("mqtt")) {
				mqttAddress = xml.getAttribute("mqtt", "host", "127.0.0.1", 0);
				mqttPort = xml.getAttribute("mqtt", "port", 1883);
				mqttUser = xml.getAttribute("mqtt", "user", "");
				mqttPassword = xml.getAttribute("mqtt", "password", "");
			}

			if (xml.tagExists("client")) {
				myName = xml.getAttribute("client", "name", "client", 0);
				myHue = xml.getAttribute("client", "hue", 0.0, 0);
			}
		}
	}
}
