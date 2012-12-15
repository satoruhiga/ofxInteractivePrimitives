#pragma once

#include "ofMain.h"

class ofxInteractivePrimitivesRootNode;

class ofxInteractivePrimitives : public ofNode
{
	friend class ofxInteractivePrimitivesRootNode;
	
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
	
	virtual void keyPressed(int key) {}
	virtual void keyReleased(int key) {}

	//
	
	void setParent(ofxInteractivePrimitives *o);
	ofxInteractivePrimitives* getParent() { return (ofxInteractivePrimitives*)ofNode::getParent(); }
	void clearParent();
	
	//
	
	inline void setVisible(bool v) { visible = v; }
	inline bool getVisible() const { return visible; }
	inline bool isVisible() const { return visible; }
	
	inline bool isHover() const { return hover; }
	inline bool isDown() const { return down; }
	
	inline bool isFocus() const { return focus; }
	inline bool hasFocus() const { return focus; }
	
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
	bool hover, down, visible, focus;
	
	ofMatrix4x4 global_matrix, global_matrix_inverse;
	vector<ofxInteractivePrimitives*> children;
	
	void clearState();
	
};

class ofxInteractivePrimitivesRootNode : public ofxInteractivePrimitives
{
public:
	
	void draw();
	void update();
	
};