#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "../GoogleJapaneseInput/AlignableFont.h"

class MessageObject {
public:
	MessageObject();
	MessageObject(
		unsigned int _id,
		string &_talker,
		string &_message,
		float &_talkerHue,
		vector<MessageObject *> _parents,
		ofxBox2d *box2d, ofPoint pos,
		AlignableFont *font
		);

	~MessageObject();

	void update();
	void draw();
	void drawSelected();
	shared_ptr<ofxBox2dRect> box2dRect;

	// メッセージのテキスト
	string message;

	// メッセージの固有ID
	// 生成時にランダムで振られる
	unsigned long id;

	// 発言者
	string talker;

	// メッセージの色（発言者の色）
	ofColor color;

	// テキストを事前に描画する fbo
	ofFbo messageFbo;

	// 1つ前のオブジェクト
	vector<MessageObject *> parents;

	// 1つ後のオブジェクト
	vector<MessageObject *> children;

	// 前のメッセージとの間の引力の係数（倍率）
	float parentsForce;

	// 選択のときに使うメソッド
	bool inside(float x, float y);
};

