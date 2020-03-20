#pragma once

#include "ofxXmlSettings.h"
#include "ofxMQTT.h"
#include "MessageObject.h"
#include "GoogleJapaneseInput.h"
#include "Box2dUpdater.h"

class ImozuruChatBoard {
public:
	ImozuruChatBoard();
	~ImozuruChatBoard();

	void setup();
	void update();
	void draw();
	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);
	void mousePressed(ofMouseEventArgs &mouse);
	void mouseDragged(ofMouseEventArgs &mouse);
	void mouseReleased(ofMouseEventArgs &mouse);
	void mouseMoved(ofMouseEventArgs &mouse);
	void mouseEntered(ofMouseEventArgs &mouse);
	void mouseExited(ofMouseEventArgs &mouse);
	void mouseScrolled(ofMouseEventArgs &mouse);
	void windowResized(int w, int h);

	// MQTTのイベントハンドラ
	void onMqttOnline();
	void onMqttOffline();
	void onMqttMessage(ofxMQTTMessage& message);

	// メッセージのオブジェクトを作るメソッド
	// messageはコメント内容。これにメタデータをつけてサーバに送る
	// このメソッド内ではオブジェクトのインスタンスは作らず、
	// あくまでもサーバからの支持でしか作らない
	void makeMessage(string &message);

	// サーバにメッセージのデータを送る
	// messageData は、talkerなどのメタデータをまとめたstring
	void sendMessageData(string &messageData);

	// サーバから受け取ったメッセージを生成する
	// _receivedMessageData は、talkerなどのメタデータをまとめたstring
	void makeMessageFromReceivedData(string &_receivedMessageData);

	// 自分の情報
	string myName; // 発言者の名前
	float myHue; // 色相

private:
	// tcp
	string mqttAddress;
	int mqttPort;
	string mqttTopic;
	ofxMQTT mqtt;
	string mqttClientID;
	string mqttUser, mqttPassword;

	// ime
	GoogleJapaneseInput ime;

	// 物理エンジン
	ofxBox2d box2d;
	ofMutex box2dMutex;
	// box2d を独立したスレッドで回すためのクラス
	Box2dUpdater box2dUpdater;

	// メッセージオブジェクトで描画するフォント
	AlignableFont messageFont;

	// 全てのメッセージオブジェクト
	vector<MessageObject *> messages;

	// 現在選択中のメッセージオブジェクト
	vector<MessageObject *> selected;

	// 新しく登場するオブジェクトのオフセット位置
	// 選択されたメッセージからのオフセット
	// 非選択のときは勝手に中央から出るので関係ない
	ofPoint newMessageOffsetPos;

	// 設定をロード
	void loadConfig();
};

