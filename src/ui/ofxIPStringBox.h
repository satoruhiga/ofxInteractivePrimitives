#pragma once

#include "ofMain.h"

#include "ofxInteractivePrimitives.h"

#include "ofxIPBaseElement.h"

OFX_INTERACTIVE_PRIMITIVES_START_NAMESPACE

class String : public Element2D
{
public:
	
	enum
	{
		CHAR_WIDTH = 8,
		CHAR_HEIGHT = 9,
		NEWLINE_HEIGHT = 5
	};

	String(Node &parent) : Element2D(parent) {}
	
	void draw()
	{
		ofPushStyle();
		ofDrawBitmapString(text, 1, CHAR_HEIGHT + 1);
		ofPopStyle();
	}

	void setText(const string& s) { text = s; onUpdateText(); }
	const string& getText() const { return text; }
	
protected:
	
	string text;
	
	virtual void onUpdateText() {}
};

class StringBox : public String
{
public:

	typedef String Font;

	StringBox(Node &parent) : String(parent), margin(1) {}

	void draw()
	{
		ofPushStyle();
		
		ofNoFill();
		ofDrawRectangle(getContentRect());

		ofDrawBitmapString(text, margin, Font::CHAR_HEIGHT + margin);
		
		ofPopStyle();
	}

	void hittest()
	{
		ofFill();
		ofDrawRectangle(getContentRect());
	}

	float getMargin() const { return margin; }
	void setMargin(float v) { margin = v; updateContentRect(); }
	
protected:

	void onUpdateText()
	{
		updateContentRect();
	}
	
	void updateContentRect()
	{
		int w = 0;
		int h = 1;
		int max_w = 0;
		
		for (int i = 0; i < text.size(); i++)
		{
			char c = text[i];
			if (c == '\n')
			{
				max_w = max(max_w, w);
				
				w = 0;
				h++;
			}
			else
			{
				w++;
			}
		}
		
		max_w = max(max_w, w);
		
		ofRectangle rect;
		rect.y = -1;
		rect.width = max_w * Font::CHAR_WIDTH;
		rect.height = h * Font::CHAR_HEIGHT + (h - 1) * Font::NEWLINE_HEIGHT;
		
		rect.width += margin * 2 + 1;
		rect.height += margin * 2 + 1;
		
		setContentRect(rect);
	}
	
private:
	
	float margin;

};


class DraggableStringBox : public StringBox
{
public:
	
	DraggableStringBox(Node &parent) : StringBox(parent) {}
	
	void mouseDragged(int x, int y, int button)
	{
		move(getMouseDelta());
	}
};

//

inline String* makeString(Node &parent, const string& text, const ofVec2f& pos = ofVec2f(0, 0))
{
	String* o = new String(parent);
	o->setText(text);
	o->setPosition(pos);
	return o;
}

OFX_INTERACTIVE_PRIMITIVES_END_NAMESPACE