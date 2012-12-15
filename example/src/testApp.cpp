#include "testApp.h"

#include "ofxInteractivePrimitives.h"

class MyRect : public ofxInteractivePrimitives
{
public:
	
	string my_string;
	
	MyRect()
	{
		setPosition(ofRandomWidth(), ofRandomHeight(), 0);
		
		my_string = "drag me";
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
		ofNoFill();
		
		if (isFocus())
		{
			ofRect(-10, -10, 120, 120);
			ofDrawBitmapString("I am focused object.\nyou can change my text", 120, 15);
		}

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
		
		ofDrawBitmapString(my_string, 5, 15);
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
	
	void keyPressed(int key)
	{
		if ((key == OF_KEY_BACKSPACE || key == OF_KEY_DEL)
			&& !my_string.empty())
		{
			my_string = my_string.substr(0, my_string.size() - 1);
		}
		
		if (key == OF_KEY_RETURN)
		{
			my_string += "\n";
		}
		else if (isprint(key))
		{
			my_string += key;
		}
	}
};

vector<MyRect*> rects;
ofxInteractivePrimitivesRootNode root;

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