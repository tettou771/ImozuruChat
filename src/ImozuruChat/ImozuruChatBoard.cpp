#include "ImozuruChatBoard.h"

ImozuruChatBoard::ImozuruChatBoard() {
}

ImozuruChatBoard::~ImozuruChatBoard() {
	auto delVector = [](auto M) {
		for (auto m : M) {
			delete m;
		}
		M.clear();
	};
	delVector(messages);

	mqtt.disconnect();
}

void ImozuruChatBoard::setup() {
	// box2d �̐��E�����
	box2d.init();
	box2d.createBounds(0, 0, ofGetWidth(), ofGetHeight());
	float box2dFps = 60;
	box2d.setFPS(box2dFps);
	box2d.enableEvents();
	box2d.registerGrabbing(); // mouse�n�̃n���h��������ŗL���ɂȂ�
	box2d.enableGrabbing(); // �I�u�W�F�N�g�̃}�E�X�h���b�O��L����
	box2d.setGravity(0, -10);

	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);

	ofTrueTypeFontSettings settings("fonts/NotoSansCJKjp-Regular.otf", 20);
	settings.addRanges(ofAlphabet::Latin);
	settings.addRanges(ofAlphabet::Japanese);
	messageFont.load(settings);

	ime.setFont("fonts/NotoSansCJKjp-Regular.otf", 20);

	// �ݒ�l�����[�h
	loadConfig();

	// �ڑ�
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
}

void ImozuruChatBoard::update() {
	mqtt.update();
	if (!mqtt.connected()) {
		// �����ڑ��ł��Ă��Ȃ�������A�Đڑ������݂�
		// ���b��1�x
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
	box2dMutex.lock();
	for (auto m : messages) {
		m->draw();
	}
	for (auto m : selected) {
		m->drawSelected();
	}
	box2dMutex.unlock();

	// ���͒��̕�����`��
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
	// IME�Ɗ�����̂ł悭���ӂ��Ď������邱��

	switch (key.key) {

	case OF_KEY_RETURN:
		// Ctrl�L�[�������Ă����琶������
		if (ofGetKeyPressed(OF_KEY_CONTROL)) {
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

	// �f�[�^���L�����ǂ�
	if (message.topic == mqttTopic) {
		string receivedMessage = message.payload;
		if (receivedMessage.length() > 0) makeMessageFromReceivedData(receivedMessage);
	}
}

void ImozuruChatBoard::makeMessage(string& message) {
	// �����̌ŗLid
	// �傫�Ȑ��̗����Ȃ̂ŁA����URL���Q�l�ɂ���
	// https://oshiete.goo.ne.jp/qa/2111837.html
	int D = ULONG_MAX / RAND_MAX;
	int M = ULONG_MAX % RAND_MAX;
	unsigned long id = rand() * D + rand() % M;

	// �e�I�u�W�F�N�g��id
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
	// �v�f�̐��������Ȃ�������A��
	if (elements.size() < requireElementNum) {
		ofLog() << "element num " << elements.size() << " less than " << requireElementNum << ".";
		return;
	}

	unsigned long id = ofToInt64(elements[0]);
	string talkerName = elements[1];
	string messageStr = elements[2];
	float hue = ofToFloat(elements[3]);
	unsigned long parentId = ofToInt64(elements[4]);

	// ��̕����񂾂����疳������
	if (messageStr.length() <= 0) return;

	// �e�I�u�W�F�N�g��T��
	vector<MessageObject*> parents = vector<MessageObject*>();
	for (auto m : messages) {
		if (m->id == parentId) {
			parents.push_back(m);
		}
	}

	// ����������W
	ofPoint pos;

	// ��I����ԂȂ�Ƃ肠���������ɏo��
	if (parents.empty()) {
		pos.x = ofGetWidth() / 2;
		pos.y = ofGetHeight() / 2;
	}
	// �I�����Ă���I�u�W�F�N�g������Ȃ�A���̉��ɏo��
	else {
		for (auto s : parents) {
			pos += s->box2dRect->getPosition();
		}
		pos /= parents.size();

		// �I�����Ă��郁�b�Z�[�W�̉��ɏo�������̂ŁA
		// �\�ߌ��߂Ă���I�t�Z�b�g�𑫂�
		pos += newMessageOffsetPos;
	}

	ofRectangle rect;
	messages.push_back(new MessageObject(id, talkerName, messageStr, hue, parents, &box2d, pos, &messageFont));

	// �I�����ꂽ�I�u�W�F�N�g�̎q�ɂ��Ƃ��́A
	// �I��Ώۂ��q�ɐ؂�ւ���B
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
