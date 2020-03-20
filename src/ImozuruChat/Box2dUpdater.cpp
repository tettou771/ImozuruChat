#include "Box2dUpdater.h"

Box2dUpdater::Box2dUpdater() {
}

Box2dUpdater::~Box2dUpdater() {
	stopFlag = true;
	stopThread();
}

void Box2dUpdater::setup(ofxBox2d* box2d, ofMutex* mutex) {
	this->box2d = box2d;
	this->mutex = mutex;
	startThread();
}

void Box2dUpdater::threadedFunction() {
	stopFlag = false;
	frameRate = 0;

	float pastUpdateTime = ofGetElapsedTimef();

	while (!stopFlag) {
		float now = ofGetElapsedTimef();

		mutex->lock();
		box2d->update();
		mutex->unlock();
		ofSleepMillis(5);

		frameRate += (1.0 / (now - pastUpdateTime)) * 0.01;
		pastUpdateTime = now;
	}
}

float Box2dUpdater::getFrameRate() {
	return frameRate;
}
