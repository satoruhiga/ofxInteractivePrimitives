#pragma once

#include "ofMain.h"

namespace ofxInteractivePrimitives
{

class Context;
class RootNode;

class Node : public ofNode
{
	friend class RootNode;
	
public:
	
	friend class Context;
	
	Node();
	virtual ~Node();
	
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
	
	void setParent(Node *o);
	Node* getParent() { return (Node*)ofNode::getParent(); }
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
	ofVec3f localToGlobalPos(const ofVec3f& v);
	ofVec3f globalToLocalPos(const ofVec3f& v);
	
	ofVec3f screenToWorld(const ofVec2f& v);
	ofVec2f worldToScreen(const ofVec3f& v);
	
protected:
	
	struct Internal {};
	void draw(const Internal &);
	void update(const Internal &);
	
	virtual Context* getContext();
	
private:
	
	unsigned int object_id;
	bool hover, down, visible, focus;
	
	ofMatrix4x4 global_matrix, global_matrix_inverse;
	vector<Node*> children;
	
	void clearState();
	
};

class RootNode : public Node
{
public:
	
	RootNode();
	~RootNode();
	
	void draw();
	void update();
	
protected:
	
	Context* getContext();
	
private:
	
	Context *context;
};

	
}
