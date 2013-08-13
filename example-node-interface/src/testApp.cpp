#include "testApp.h"

#include "ofxIPPatcher.h"

ofxInteractivePrimitives::Patcher root;

//

using namespace ofxInteractivePrimitives;

struct TestClassWrapper : public ofxInteractivePrimitives::Wrapper<TestClassWrapper>
{
	int num;
	ofVec3f last_pos;

	static void mousePressed(PatchObject *self, int x, int y, int button)
	{
		self->num = ofRandom(100);
	}

	static void setupPatchObject(PatchObject *self)
	{
		self->setText("hoge");
		self->addOutput("in 1");
		
		self->num = ofRandom(100);
	}
	
	static void updatePatchObject(PatchObject *self)
	{
		if (self->getOutputPort(0).hasConnect())
		{
			int n = self->getPosition().x;
			self->getOutputPort(0).set(self->num);
		}
	}
};

struct DispClassWrapper : public ofxInteractivePrimitives::Wrapper<DispClassWrapper>
{
	static void setupPatchObject(PatchObject *self)
	{
		self->setText("Disp");
		self->addInput("in 0");
	}
	
	static void updatePatchObject(PatchObject *self)
	{
		MessageRef &in = self->getInputPort(0).requestUpdate();
		int n;
		if (in && in->get(n))
		{
			self->setText(ofToString(n));
		}
	}
	
	static bool isOutput() { return true; }
};

struct PowWrapper : public ofxInteractivePrimitives::Wrapper<PowWrapper>
{
	static void setupPatchObject(PatchObject *self)
	{
		self->setText("Pow");
		self->addInput("in 0");
		self->addOutput("out 0");
	}
	
	static void updatePatchObject(PatchObject *self)
	{
		MessageRef &in = self->getInputPort(0).requestUpdate();
		int n;
		if (in && in->get(n))
		{
			n = n * n;
			self->getOutputPort(0).set(n);
		}
	}
};


ofxInteractivePrimitives::IPatchObject *node0;
ofxInteractivePrimitives::IPatchObject *node1;
ofxInteractivePrimitives::IPatchObject *node2;

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);

	root.registerPatchObject<TestClassWrapper>("hoge");
	root.registerPatchObject<DispClassWrapper>("disp");
	root.registerPatchObject<PowWrapper>("pow");
	
	node0 = root.create("hoge");
	node1 = root.create("disp");
	root.create("pow");
	
//	node0 = root.create<TestClassWrapper>();
//	node1 = root.create<DispClassWrapper>();
//	node2 = root.create<PowWrapper>();

	node0->setPosition(200, 200, 0);
	node1->setPosition(200, 300, 0);
}

//--------------------------------------------------------------
void testApp::update()
{
	ofxInteractivePrimitives::DelayedDeletable::deleteQueue();

	root.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
	ofEnableAlphaBlending();
	
	ofSetColor(255);

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