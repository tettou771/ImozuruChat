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

	// MQTT�̃C�x���g�n���h��
	void onMqttOnline();
	void onMqttOffline();
	void onMqttMessage(ofxMQTTMessage& message);

	// ���b�Z�[�W�̃I�u�W�F�N�g����郁�\�b�h
	// message�̓R�����g���e�B����Ƀ��^�f�[�^�����ăT�[�o�ɑ���
	// ���̃��\�b�h���ł̓I�u�W�F�N�g�̃C���X�^���X�͍�炸�A
	// �����܂ł��T�[�o����̎x���ł������Ȃ�
	void makeMessage(string &message);

	// �T�[�o�Ƀ��b�Z�[�W�̃f�[�^�𑗂�
	// messageData �́Atalker�Ȃǂ̃��^�f�[�^���܂Ƃ߂�string
	void sendMessageData(string &messageData);

	// �T�[�o����󂯎�������b�Z�[�W�𐶐�����
	// _receivedMessageData �́Atalker�Ȃǂ̃��^�f�[�^���܂Ƃ߂�string
	void makeMessageFromReceivedData(string &_receivedMessageData);

	// �����̏��
	string myName; // �����҂̖��O
	float myHue; // �F��

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

	// �����G���W��
	ofxBox2d box2d;
	ofMutex box2dMutex;
	// box2d ��Ɨ������X���b�h�ŉ񂷂��߂̃N���X
	Box2dUpdater box2dUpdater;

	// ���b�Z�[�W�I�u�W�F�N�g�ŕ`�悷��t�H���g
	AlignableFont messageFont;

	// �S�Ẵ��b�Z�[�W�I�u�W�F�N�g
	vector<MessageObject *> messages;

	// ���ݑI�𒆂̃��b�Z�[�W�I�u�W�F�N�g
	vector<MessageObject *> selected;

	// �V�����o�ꂷ��I�u�W�F�N�g�̃I�t�Z�b�g�ʒu
	// �I�����ꂽ���b�Z�[�W����̃I�t�Z�b�g
	// ��I���̂Ƃ��͏���ɒ�������o��̂Ŋ֌W�Ȃ�
	ofPoint newMessageOffsetPos;

	// �ݒ�����[�h
	void loadConfig();
};

