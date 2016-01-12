#include "testApp.h"
#include "ofxInteractivePrimitives.h"

//--------------------------------------------------------------
void testApp::setup()
{
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(0);
    
    root = new ofxInteractivePrimitives::RootNode();
    
    addRects();
    
}

//--------------------------------------------------------------
void testApp::update()
{
    root->update();
}

//--------------------------------------------------------------
void testApp::draw()
{
    
    ofSetColor(ofColor::white);
    ofDrawBitmapString("Press spacebar to add more MyRects", 10, 20);
    root->draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
    if (key == ' ')
    {
        addRects();
    }
}

//--------------------------------------------------------------
void testApp::addRects()
{
    for (int i = 0; i < 5; i++)
    {
        rects.push_back(new MyRect);
        rects.back()->setParent(root);
    }
}
