#include "testApp.h"

#include "ofxInteractivePrimitives.h"

class MyRect : public ofxInteractivePrimitives
{
public:
	
	MyRect()
	{
		setPosition(ofRandomWidth(), ofRandomHeight(), 0);
	}
	
	void update()
	{
		if (!isDown())
		{
			move(ofRandom(-2, 2), ofRandom(-2, 2), 0);
		}
	}
	
	void draw()
	{
		if (isHover())
			ofFill();
		else
			ofNoFill();
		
		if (isDown())
			ofSetColor(255, 0, 0);
		else
			ofSetColor(255);
		
		ofRect(0, 0, 100, 100);
		
		ofSetColor(0, 255, 0);
		ofDrawBitmapString("drag me", 5, 15);
	}
	
	void hittest()
	{
		ofRect(0, 0, 100, 100);
	}
	
	void mouseMoved(int x, int y)
	{
		printf("%i %i\n", x, y);
	}
	
	void mouseDragged(int x, int y, int button)
	{
		ofVec2f d;
		d.x = ofGetMouseX() - ofGetPreviousMouseX();
		d.y = ofGetMouseY() - ofGetPreviousMouseY();
		
		move(d);
	}
};

vector<MyRect*> rects;
ofxInteractivePrimitivesRoot root;

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
	for (int i = 0; i < 5; i++)
	{
		rects.push_back(new MyRect);
		rects.back()->setParent(&root);
	}
}

//--------------------------------------------------------------
void testApp::update()
{
	root.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
	root.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{

}