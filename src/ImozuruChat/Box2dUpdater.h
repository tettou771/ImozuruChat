#pragma once
#include "ofxBox2d.h"

class Box2dUpdater : public ofThread {
public:
	Box2dUpdater();
	~Box2dUpdater();

	void setup(ofxBox2d* box2d, ofMutex* mutex);
	void threadedFunction() override;
	float getFrameRate();

private:
	ofxBox2d* box2d;
	ofMutex* mutex;
	bool stopFlag;
	float frameRate;
};

