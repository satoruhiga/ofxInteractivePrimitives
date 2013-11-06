#pragma once

#include "ofxInteractivePrimitives.h"

OFX_INTERACTIVE_PRIMITIVES_START_NAMESPACE

class Element2D : public Node
{
public:

	Element2D(Node &root) : Node()
	{
		setParent(&root);
	}
	
	float getContentX() const { return rect.x; }
	float getContentY() const { return rect.y; }
	float getContentWidth() const { return rect.width; }
	float getContentHeight() const { return rect.height; }

	const ofRectangle& getContentRect() const { return rect; }
	void setContentRect(const ofRectangle& o) { rect = o; }
	
private:
	
	ofRectangle rect;
	
};

OFX_INTERACTIVE_PRIMITIVES_END_NAMESPACE