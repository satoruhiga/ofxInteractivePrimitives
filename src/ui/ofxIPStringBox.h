#pragma once

#include "ofMain.h"

#include "ofxInteractivePrimitives.h"

namespace ofxInteractivePrimitives
{

class StringBox : public Node
{
public:

	enum
	{
		MARGIN = 1,
		BITMAP_CHAR_WIDTH = 8,
		BITMAP_CHAR_HEIGHT = 9,
		BITMAP_CHAR_NEWLINE_HEIGHT = 5
	};

	StringBox(RootNode &root) : Node()
	{
		setParent(&root);
	}

	void draw()
	{
		ofNoFill();
		ofSetColor(0, 255, 0);
		ofRect(rect);

		ofDrawBitmapString(text, MARGIN, BITMAP_CHAR_HEIGHT + MARGIN);
	}

	void hittest()
	{
		ofFill();
		ofRect(rect);
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

		rect.y = -1;
		rect.width = max_w * BITMAP_CHAR_WIDTH;
		rect.height = h * BITMAP_CHAR_HEIGHT + (h - 1) * BITMAP_CHAR_NEWLINE_HEIGHT;

		rect.width += MARGIN * 2 + 1;
		rect.height += MARGIN * 2 + 1;
	}

	const string& getText() const { return text; }

	const ofRectangle& getRect() const { return rect; }
	
protected:

	string text;
	ofRectangle rect;

};

}