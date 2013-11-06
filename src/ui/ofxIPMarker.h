#pragma once

#include "ofMain.h"

#include "ofxInteractivePrimitives.h"

OFX_INTERACTIVE_PRIMITIVES_START_NAMESPACE

class Marker : public Node
{
public:

	Marker(Node &parent) : Node()
	{
		setParent(&parent);
	}

	void draw()
	{
		ofNoFill();

		if (isDown())
			ofSetColor(255, 0, 0);
		else
			ofSetColor(255, 0, 0);

		ofLine(-10, 0, 10, 0);
		ofLine(0, -10, 0, 10);

		if (isDown())
		{
			ofNoFill();
			ofSetColor(255, 0, 0);
			ofRect(-6, -6, 12, 12);
		}
		else if (isHover())
		{
			ofNoFill();
			ofSetColor(255, 0, 0);
			ofRect(-3, -3, 6, 6);
		}

		ofSetColor(0, 255, 0);
		ofDrawBitmapString(text, 4, 14);
	}

	void hittest()
	{
		ofFill();
		ofRect(-15, -15, 30, 30);
	}

	void mouseDragged(int x, int y, int button)
	{
		move(getMouseDelta());
	}

	void setText(const string& s) { text = s; }
	const string& getText() { return text; }

protected:

	string text;

};

OFX_INTERACTIVE_PRIMITIVES_END_NAMESPACE
