#pragma once

#include "ofMain.h"

#include "ofxIPStringBox.h"

namespace ofxInteractivePrimitives
{

class Button : public StringBox
{
public:

	Button(RootNode &root) : StringBox(root) {}

	void draw()
	{
		if (isDown())
		{
			ofFill();
			ofSetColor(0, 255, 0);
			ofRect(rect);

			ofSetColor(0, 127, 0);
		}
		else if (isHover())
		{
			ofNoFill();
			ofSetColor(255, 0, 255);
			ofRect(rect);
		}
		else
		{
			ofNoFill();
			ofSetColor(0, 255, 0);
			ofRect(rect);
		}

		ofDrawBitmapString(text, MARGIN, BITMAP_CHAR_HEIGHT + MARGIN);
	}

};

}