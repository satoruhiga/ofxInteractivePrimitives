//
//  MyRect.hpp
//  example
//

#ifndef MyRect_h
#define MyRect_h

#include "ofxInteractivePrimitives.h"

class MyRect : public ofxInteractivePrimitives::Node
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
            ofDrawRectangle(-10, -10, 120, 120);
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
        
        ofDrawRectangle(0, 0, 100, 100);
        
        ofSetColor(0, 255, 0);
        
        ofDrawBitmapString(my_string, 5, 15);
    }
    
    void hittest()
    {
        ofDrawRectangle(0, 0, 100, 100);
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

#endif /* MyRect_hpp */
