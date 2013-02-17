#pragma once

#include "ofxInteractivePrimitives.h"

namespace ofxInteractivePrimitives
{
	class Element2D;
}

class ofxInteractivePrimitives::Element2D : public Node
{
public:

	Element2D(RootNode &root) : Node()
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