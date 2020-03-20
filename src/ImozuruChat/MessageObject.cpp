#include "MessageObject.h"



MessageObject::MessageObject() {
}

MessageObject::MessageObject(
	unsigned int _id,
	string &_talker,
	string &_message,
	float &_talkerHue,
	vector<MessageObject *> _parents,
	ofxBox2d *box2d, ofPoint pos,
	AlignableFont *font
	) {

	// id
	id = _id;

	// ������
	talker = _talker;

	// ���b�Z�[�W
	message = _message;

	// �F�ihue�ŗ^������̂ŁAcolor�ɕϊ�����j
	color = ofColor::fromHsb(_talkerHue, 120, 255, 255);

	// ���b�Z�[�W�̐e���w��
	parents.clear();
	for (auto p : _parents) {
		parents.push_back(p);
		p->children.push_back(this);
	}

	// ���b�Z�[�W�̉摜�𐶐�
	float marginX = 16;
	float marginY = 6;
	ofRectangle rect = font->getStringBoundingBox(_message, 0, 0);
	messageFbo.allocate(rect.width + marginX * 2, rect.height + marginY * 2, GL_RGBA);
	int mw = messageFbo.getWidth();
	int mh = messageFbo.getHeight();
	messageFbo.begin();
	ofBackground(0, 0);
	ofSetColor(color);
	ofFill();
	int rectMargin = 1; // �ׂ̃I�u�W�F�N�g�Ƃ�����ƃX�y�[�X���J�����悤�Ɍ����邽�߁A�����߂ɍ��z�c��`��
	ofDrawRectRounded(rectMargin, rectMargin, mw - rectMargin * 2, mh - rectMargin * 2, mw / 2 - rectMargin);
	ofSetColor(30);
	font->drawString(message, marginX, rect.height + marginY);
	messageFbo.end();

	// box2dRect ��ݒ�
	box2dRect = shared_ptr<ofxBox2dRect>(new ofxBox2dRect);
	float mass = 1.0;
	float bound = 0.4;
	float friction = 0.5;
	box2dRect->setPhysics(mass, bound, friction);
	box2dRect->setup(box2d->getWorld(), pos.x, pos.y, mw, mh);
	box2dRect->enableGravity(false);
	box2dRect->setVelocity(ofVec2f(0, 0));

	// �I�u�W�F�N�g���m����������
	parentsForce = 5;

	cout << "Create message \"" << message << "\"" << endl;
}

MessageObject::~MessageObject() {
}

void MessageObject::update() {
	// �e�Ǝ����Ƃ̊Ԃ̈���
	for (auto p : parents) {
		ofPoint parentVec = p->box2dRect->getPosition() - box2dRect->getPosition();

		box2dRect->addForce(parentVec, parentsForce);
		p->box2dRect->addForce(parentVec, -parentsForce);
	}

	// ��ʊO�ɏo�Ă����狭���I�ɖ߂�
	ofPoint pos = box2dRect->getPosition();
	if (pos.x < 0) box2dRect->setPosition(0, pos.y);
	if (ofGetWidth() < pos.x) box2dRect->setPosition(ofGetWidth(), pos.y);
	if (pos.y < 0) box2dRect->setPosition(pos.x, 0);
	if (ofGetHeight() < pos.y) box2dRect->setPosition(pos.x, ofGetHeight());
}

void MessageObject::draw() {
	// �q�Ƃ̊Ԃɐ�������
	// �I�u�W�F�N�g�̕`��̑O�ɂ��邱�ƂŁA�I�u�W�F�N�g�̉��ɏ�����
	for (auto c : children) {
		ofSetLineWidth(6);
		ofSetColor(0, 50);
		ofDrawLine(
			box2dRect->getPosition(),
			c->box2dRect->getPosition()
			);
	}

	ofSetColor(ofColor::white);
	ofFill();

	ofPushMatrix();
	ofTranslate(box2dRect->getPosition());
	ofRotate(box2dRect->getRotation());
	//float w = box2dRect->getWidth();
	//float h = box2dRect->getHeight();
	//ofDrawRectangle(-w / 2, -h / 2, w, h);
	messageFbo.draw(-messageFbo.getWidth() / 2, -messageFbo.getHeight() / 2);

	ofPopMatrix();

}

// �I�𒆂̂��̂͘g�����n�C���C�g����
void MessageObject::drawSelected() {
	ofSetColor(255, 255, 0);
	ofNoFill();
	ofSetLineWidth(2);

	ofPushMatrix();
	ofTranslate(box2dRect->getPosition());
	ofRotate(box2dRect->getRotation());
	float w = box2dRect->getWidth() + 6;
	float h = box2dRect->getHeight() + 6;
	ofDrawRectangle(-w / 2, -h / 2, w, h);
	ofPopMatrix();
}

// �ړ���̍��W�� (x,y) �������Ă��邩�ǂ���
// �N���b�N�̔���Ɏg��
bool MessageObject::inside(float x, float y) {
	ofMatrix4x4 matrix;
	matrix.makeIdentityMatrix();
//	matrix.translate(-ofGetWidth(), -ofGetHeight(), 0);
	matrix.translate(-box2dRect->getPosition());
	matrix.rotate(-box2dRect->getRotation(), 0, 0, 1);

	ofPoint mousePos(x,y);
	ofPoint pos = matrix.preMult(mousePos);
	cout << pos << endl;
	float w = box2dRect->getWidth();
	float h = box2dRect->getHeight();

	return (-w / 2 < pos.x && pos.x < w / 2 && -h / 2 < pos.y && pos.y < h / 2);
}

