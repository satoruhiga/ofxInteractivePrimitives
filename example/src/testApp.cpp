#include "testApp.h"

#include "ofxInteractivePrimitives.h"

class ControlPoint : public ofxInteractivePrimitives
{
public:
	
	void draw()
	{
		if (isHover())
		{
			ofFill();
		}
		else
		{
			ofNoFill();
		}
		
		if (isDown())
		{
			ofSetColor(255, 0, 0);
		}
		else
		{
			ofSetColor(255);
		}
		
		ofRect(0, 0, 100, 100);
	}
	
	void hittest()
	{
		ofFill();
		ofRect(0, 0, 100, 100);
	}
	
	void mouseMoved(int x, int y)
	{
		printf("%i %i\n", x, y);
	}
};

ControlPoint CP;

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
	CP.setPosition(300, 300, 0);
}

//--------------------------------------------------------------
void testApp::update()
{

}

//--------------------------------------------------------------
void testApp::draw()
{
	
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