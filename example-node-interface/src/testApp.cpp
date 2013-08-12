#include "testApp.h"

#include "ofxIPPatcher.h"

ofxInteractivePrimitives::RootNode root;

//

using namespace ofxInteractivePrimitives;

class TestClass
{
public:

	void test(int i)
	{
		cout << "hello: " << i << endl;
	}
};

struct TestClassWrapper : public ofxInteractivePrimitives::AbstructWrapper<TestClassWrapper>
{
	typedef TestClass Context;

	static const char* getName() { return "TestClass"; }

	static void* create(vector<MessageRef>& input, vector<MessageRef>& output)
	{
		output[0] = Message<ofVec3f>::create(ofVec3f(0));

		return new TestClass;
	}
	
	static void setupPatcher(Patcher *self, const vector<MessageRef>& input, vector<MessageRef>& output)
	{
		self->setText(getName());
	}


//	static void execute(Patcher<TestClassWrapper> *patcher, Context *context, const vector<MessageRef>& input, vector<MessageRef>& output)
//	{
//
//		Message<ofVec3f> *out0 = output[0]->cast<ofVec3f>();
//		out0->set(patcher->getPosition());
//	}
	
//	static void update(BasePatcher *patcher, Context *context)
//	{
//		patcher->execute();
//	}

//	static void layout(Patcher<TestClassWrapper> *patcher, Context *context)
//	{
//		patcher->setText(getName());
//	}
	
	static TypeID getInputType(int index)
	{
		return Type2Int<void>();
	}

	static int getNumOutput()
	{
		return 1;
	}

	static TypeID getOutputType(int index)
	{
		return Type2Int<ofVec3f>();
	}
};

struct PrintClassWrapper : public ofxInteractivePrimitives::AbstructWrapper<PrintClassWrapper>
{
	typedef TestClass Context;

	static const char* getName() { return "PrintClass"; }

	static void* create(vector<MessageRef>& input, vector<MessageRef>& output)
	{
		input[0] = Message<int>::create(42);
		return NULL;
	}
	
	static void setupPatcher(Patcher *self, const vector<MessageRef>& input, vector<MessageRef>& output)
	{
		self->setText(getName());
	}


//	static void execute(Patcher<PrintClassWrapper> *patcher, Context *context, const vector<MessageRef>& input, vector<MessageRef>& output)
//	{
//		Message<ofVec3f> *in0 = input[0]->cast<ofVec3f>();
//		if (in0)
//		{ofxInteractivePrimitives
//			stringstream ss;
//			ss << in0->get();
//			patcher->setText(ss.str());
//		}
//	}
//	
//	static void layout(Patcher<PrintClassWrapper> *patcher, Context *context)
//	{
//		patcher->setText(getName());
//	}
	
	static void update(Patcher *self)
	{
	}

	static int getNumInput()
	{
		return 1;
	}

	static TypeID getInputType(int index)
	{
		return Type2Int<int>();
	}
};

TestClassWrapper::Patcher *node0;
PrintClassWrapper::Patcher *node1, *node2;

//--------------------------------------------------------------
void testApp::setup()
{
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);

	node0 = TestClassWrapper::Patcher::Create(root);
	node1 = PrintClassWrapper::Patcher::Create(root);
	node2 = PrintClassWrapper::Patcher::Create(root);

	node0->setPosition(200, 200, 0);
	node1->setPosition(200, 300, 0);
	node2->setPosition(300, 300, 0);
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