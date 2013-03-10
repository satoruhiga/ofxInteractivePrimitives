#pragma once

#include "ofMain.h"

#include "ofxInteractivePrimitives.h"

#include "ofxIPBaseElement.h"

namespace ofxInteractivePrimitives
{
	class StringBox;
	class DraggableStringBox;
}

class ofxInteractivePrimitives::StringBox : public Element2D
{
public:

	enum
	{
		MARGIN = 1,
		BITMAP_CHAR_WIDTH = 8,
		BITMAP_CHAR_HEIGHT = 9,
		BITMAP_CHAR_NEWLINE_HEIGHT = 5
	};

	StringBox(Node &parent) : Element2D(parent) {}

	void draw()
	{
		ofPushStyle();
		
		ofNoFill();
		ofRect(getContentRect());

		ofDrawBitmapString(text, MARGIN, BITMAP_CHAR_HEIGHT + MARGIN);
		
		ofPopStyle();
	}

	void hittest()
	{
		ofFill();
		ofRect(getContentRect());
	}

	void setText(const string& s)
	{
		text = s;

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
		rect.width = max_w * BITMAP_CHAR_WIDTH;
		rect.height = h * BITMAP_CHAR_HEIGHT + (h - 1) * BITMAP_CHAR_NEWLINE_HEIGHT;

		rect.width += MARGIN * 2 + 1;
		rect.height += MARGIN * 2 + 1;
		
		setContentRect(rect);
	}

	const string& getText() const { return text; }
	
protected:

	string text;

};


class ofxInteractivePrimitives::DraggableStringBox : public StringBox
{
public:
	
	DraggableStringBox(Node &parent) : StringBox(parent) {}
	
	void mouseDragged(int x, int y, int button)
	{
		move(getMouseDelta());
	}
};
