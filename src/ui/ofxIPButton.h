#pragma once

#include "ofMain.h"

#include "ofxIPStringBox.h"

OFX_INTERACTIVE_PRIMITIVES_START_NAMESPACE

class Button : public StringBox
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

//

inline Button* makeButton(Node &parent, const string& label, const ofVec2f& pos, float margin)
{
	Button* o = new Button(parent);
	o->setPosition(pos);
	o->setText(label);
	o->setMargin(margin);
	return o;
}

OFX_INTERACTIVE_PRIMITIVES_END_NAMESPACE
