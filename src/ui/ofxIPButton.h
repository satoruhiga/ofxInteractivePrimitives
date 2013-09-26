#pragma once

#include "ofMain.h"

#include "ofxIPStringBox.h"

namespace ofxInteractivePrimitives
{
	class Button;
	
	Button* makeButton(Node &parent, const string& label, const ofVec2f& pos = ofVec2f(0, 0), float margin = 2);
}

class ofxInteractivePrimitives::Button : public StringBox
{
public:
	
	ofEvent<ofEventArgs> pressed;

	Button(Node &parent) : StringBox(parent) {}
	Button(Node &parent, const string& label) : StringBox(parent)
	{
		setText(label);
	}

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

		ofDrawBitmapString(text, getMargin(), Font::CHAR_HEIGHT + getMargin());
	}
	
	void mousePressed(int x, int y, int button)
	{
		static ofEventArgs e;
		ofNotifyEvent(pressed, e, this);
		
		StringBox::mousePressed(x, y, button);
	}
};

inline ofxInteractivePrimitives::Button* ofxInteractivePrimitives::makeButton(Node &parent, const string& label, const ofVec2f& pos, float margin)
{
	Button* o = new Button(parent);
	o->setPosition(pos);
	o->setText(label);
	o->setMargin(margin);
	return o;
}

