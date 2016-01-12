#pragma once

#include "ofMain.h"
#include "MyRect.h"

class testApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
    void keyPressed(int key);

    void addRects();
    
    vector<MyRect*> rects;
    ofxInteractivePrimitives::RootNode *root;
    
};