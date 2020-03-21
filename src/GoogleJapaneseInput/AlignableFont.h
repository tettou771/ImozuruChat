#pragma once
#include "ofMain.h"

//#define USE_ofxTrueTypeFontUC

#ifdef USE_ofxTrueTypeFontUC
#include "ofxTrueTypeFontUC.h"
#endif

#ifdef USE_ofxTrueTypeFontUC
class ofxTrueTypeFontUCAlignable : public ofxTrueTypeFontUC {
#else
class AlignableFont : public ofTrueTypeFont {
#endif

public:
	AlignableFont() {
#ifdef USE_ofxTrueTypeFontUC
		ofxTrueTypeFontUC::ofxTrueTypeFontUC();
#else
		ofTrueTypeFont();
#endif
	};
	~AlignableFont() {
		//ofxTrueTypeFontUC::~ofxTrueTypeFontUC();
	}

	enum Align {
		LEFT,
		CENTER,
		RIGHT,
		TOP,
		BOTTOM
	};

	void drawStringAlign(const string &s, float x, float y, Align alignX = LEFT, Align alignY = BOTTOM) {

		ofRectangle boundingBox = getStringBoundingBox(s, 0, 0);
		float w = boundingBox.width;
		float h = boundingBox.height;

		float offsetX = 0, offsetY = 0;

		if (alignX == LEFT) offsetX = 0;
		else if (alignX == CENTER) offsetX = -w / 2;
		else if (alignX == RIGHT) offsetX = -w;

		if (alignY == BOTTOM) offsetY = h - getSize();
		else if (alignY == CENTER) offsetY = - h / 2 + getSize();
		else if (alignY == TOP) offsetY = getSize();

		drawString(s, x + offsetX, y + offsetY);		
	}

	void drawStringAlignCenter(const std::string &s, float x, float y) {
		drawStringAlign(s, x, y, CENTER, CENTER);
	}

	void drawStringAlignCenter(const std::string &s, ofPoint pos) {
		drawStringAlignCenter(s, pos.x, pos.y);
	}

#ifdef USE_ofxTrueTypeFontUC
	float getSize() {
		return getFontSize();
	}
#endif

};
