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

	// ���b�Z�[�W�̃e�L�X�g
	string message;

	// ���b�Z�[�W�̌ŗLID
	// �������Ƀ����_���ŐU����
	unsigned long id;

	// ������
	string talker;

	// ���b�Z�[�W�̐F�i�����҂̐F�j
	ofColor color;

	// �e�L�X�g�����O�ɕ`�悷�� fbo
	ofFbo messageFbo;

	// 1�O�̃I�u�W�F�N�g
	vector<MessageObject *> parents;

	// 1��̃I�u�W�F�N�g
	vector<MessageObject *> children;

	// �O�̃��b�Z�[�W�Ƃ̊Ԃ̈��͂̌W���i�{���j
	float parentsForce;

	// �I���̂Ƃ��Ɏg�����\�b�h
	bool inside(float x, float y);
};

