#pragma once

#include "ofMain.h"

class ofxInteractivePrimitives : public ofNode
{
public:
	
	class Context;
	friend class Context;
	
	static void prepareModelViewMatrix();
	
	ofxInteractivePrimitives();
	virtual ~ofxInteractivePrimitives();
	
	virtual void update() {}
	virtual void draw() {}
	virtual void hittest() {}
	
	virtual void mousePressed(int x, int y, int button) {}
	virtual void mouseReleased(int x, int y, int button) {}
	virtual void mouseMoved(int x, int y) {}
	virtual void mouseDragged(int x, int y, int button) {}
	
	// TODO: key events
	// virtual void keyPressed(int key) {}
	// virtual void keyReleased(int key) {}
	
	void setParent(ofxInteractivePrimitives *o) { ofNode::setParent(*o); }
	ofxInteractivePrimitives* getParent() { return (ofxInteractivePrimitives*)ofNode::getParent(); }
	
	bool isHover() { return hover; }
	bool isDown() { return down; }
	
private:
	
	unsigned int object_id;
	bool hover, down;
	
};
