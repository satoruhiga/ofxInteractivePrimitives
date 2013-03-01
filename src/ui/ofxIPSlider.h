#pragma once

#include "ofMain.h"

#include "ofxIPStringBox.h"

namespace ofxInteractivePrimitives
{
	class Slider;
}

using namespace ofxInteractivePrimitives;

class ofxInteractivePrimitives::Slider : public Element2D
{
public:
	
	ofEvent<float> valueUpdated;
	
	Slider(Node &root) : Element2D(root), label(*this), value(0), min(0), max(1), slider_width(0)
	{
		setContentRect(ofRectangle(0, 0, 100, 12));
	}
	
	void update()
	{
		label.setText(ofToString(value));
		label.setPosition(getContentWidth(), 1, 0);
	}
	
	void draw()
	{
		ofPushStyle();
		
		ofNoFill();
		ofRect(getContentRect());
		
		ofFill();
		ofRectangle r = getContentRect();
		r.width = slider_width * getContentWidth();
		ofRect(r);
		
		ofPopStyle();
	}
	
	void hittest()
	{
		ofFill();
		
		pushID(0);
		ofRect(getContentRect());
		popID();
	}
	
	void setValue(float v)
	{
		value = v;
		slider_width = ofMap(v, min, max, 0, 1, true);
	}
	
	float getValue() const { return value; }
	
	void setMin(float v) { min = v; }
	float getMin() const { return min; }
	void setMax(float v) { max = v; }
	float getMax() const { return max; }
	
	void mousePressed(int x, int y, int button)
	{
		const vector<GLuint> &names = getCurrentNameStack();
		if (names.size() == 1
			&& names[0] == 0)
		{
			updateValue(x);
		}
	}
	
	void mouseDragged(int x, int y, int button)
	{
		const vector<GLuint> &names = getCurrentNameStack();
		if (names.size() == 0)
		{
			move(getMouseDelta());
		}
		else if (names.size() == 1
				 && names[0] == 0)
		{
			updateValue(x);
		}
	}
	
protected:
	
	float value;
	float min, max;
	float slider_width;
	
	StringBox label;
	
	void updateValue(int x)
	{
		slider_width = ofMap(x, 0, getContentWidth(), 0, 1, true);
		float v = ofMap(slider_width, 0, 1, min, max);
		
		if (value != v)
		{
			value = v;
			ofNotifyEvent(valueUpdated, value);
		}
	}
};
