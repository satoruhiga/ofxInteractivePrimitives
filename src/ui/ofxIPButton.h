#pragma once

#include "ofMain.h"

#include "ofxIPStringBox.h"

namespace ofxInteractivePrimitives
{
	class Button;
}

class ofxInteractivePrimitives::Button : public StringBox
{
public:

	Button(Node &parent) : StringBox(parent) {}

	void draw()
	{
		if (isDown())
		{
			ofFill();
			ofSetColor(0, 255, 0);
			ofRect(getContentRect());

			ofSetColor(0, 127, 0);
		}
		else if (isHover())
		{
			ofNoFill();
			ofSetColor(255, 0, 255);
			ofRect(getContentRect());
		}
		else
		{
			ofNoFill();
			ofSetColor(0, 255, 0);
			ofRect(getContentRect());
		}

		ofDrawBitmapString(text, MARGIN, BITMAP_CHAR_HEIGHT + MARGIN);
	}

};
