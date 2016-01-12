#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
    root = new ofxInteractivePrimitives::RootNode();
    
	OP.setParent(root);
}

//--------------------------------------------------------------
void testApp::update()
{
	root->update();
	
	if (root->hasFocusObject())
	{
		cam.disableMouseInput();
	}
	else
	{
		cam.enableMouseInput();
	}
}

//--------------------------------------------------------------
void testApp::draw()
{
	cam.begin();
	root->draw();
	cam.end();
}
