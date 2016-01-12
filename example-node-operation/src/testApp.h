#pragma once

#include "ofMain.h"
#include "ofxInteractivePrimitives.h"
#include "NodeOperator.h"

class testApp : public ofBaseApp
{
public:
	void setup();
	void update();
    void draw();
    
    ofxInteractivePrimitives::RootNode *root;
    
    NodeOperator OP;
    ofEasyCam cam;
    
};