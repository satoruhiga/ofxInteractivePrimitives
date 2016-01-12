//
//  NodeOperator.h
//  example-node-operation
//

#include "ofMain.h"
#include "ofxInteractivePrimitives.h"

#ifndef NodeOperator_h
#define NodeOperator_h

class NodeOperator : public ofxInteractivePrimitives::Node
{
public:
    
    enum OperationType
    {
        TRANSLATE,
        ROTATE,
        SCALE
    };
    
    enum OperationAxis
    {
        AXIS_X = 0,
        AXIS_Y = 1,
        AXIS_Z = 2
    };
    
    NodeOperator() : scale(100) {}
    
    void draw()
    {
        ofDrawAxis(scale);
        
        glLineWidth(8);
        
        if (current_axis == AXIS_X)
        {
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(scale, 0, 0);
            glEnd();
        }
        else if (current_axis == AXIS_Y)
        {
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(0, scale, 0);
            glEnd();
        }
        else if (current_axis == AXIS_Z)
        {
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, scale);
            glEnd();
        }
    }
    
    void hittest()
    {
        glLineWidth(1);
        
        glPushName(AXIS_X);
        {
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(scale, 0, 0);
            glEnd();
        }
        glPopName();
        
        glPushName(AXIS_Y);
        {
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(0, scale, 0);
            glEnd();
        }
        glPopName();
        
        glPushName(AXIS_Z);
        {
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, scale);
            glEnd();
        }
        glPopName();
    }
    
    void update()
    {
        if (!isHover())
            current_axis = -1;
    }
    
    void mousePressed(int x, int y, int button)
    {
        //		cout << "pressed: " << ofGetElapsedTimef() << endl;
        //
        //		vector<GLuint> s = getCurrentNameStack();
        //		for (int i = 0; i < s.size(); i++)
        //		{
        //			cout << s[i] << " ";
        //		}
        //		cout << endl;
    }
    
    void mouseReleased(int x, int y, int button)
    {
    }
    
    void mouseDragged(int x, int y, int button)
    {
    }
    
    void mouseMoved(int x, int y)
    {
        const vector<GLuint>& s = getCurrentNameStack();
        
        if (s.size())
        {
            current_axis = s[0];
        }
    }
    
    void setOperationType(OperationType t) { operation_type = t; }
    OperationType getOperationType() { return operation_type; }
    
protected:
    
    float scale;
    int current_axis;
    
    OperationType operation_type;
};

#endif /* NodeOperator_h */
