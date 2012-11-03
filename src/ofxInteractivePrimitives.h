#pragma once

#include "ofMain.h"

class ofxInteractivePrimitivesRoot;

class ofxInteractivePrimitives : public ofNode
{
	friend class ofxInteractivePrimitivesRoot;
	
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
	
	void setParent(ofxInteractivePrimitives *o);
	ofxInteractivePrimitives* getParent() { return (ofxInteractivePrimitives*)ofNode::getParent(); }
	void clearParent();
	
	void setVisible(bool v) { visible = v; }
	bool getVisible() { return visible; }
	
	bool isHover() { return hover; }
	bool isDown() { return down; }
	
	// utils
	ofVec2f getMouseDelta();
	
	ofVec3f localToGlobal(const ofVec3f& v);
	ofVec3f globalToLocal(const ofVec3f& v);
	
protected:
	
	struct Internal {};
	void draw(const Internal &);
	void update(const Internal &);
	
private:
	
	unsigned int object_id;
	bool hover, down, visible;
	
	vector<ofxInteractivePrimitives*> children;
	
};

class ofxInteractivePrimitivesRoot : public ofxInteractivePrimitives
{
public:
	
	void draw();
	void update();
	
};