#include "testApp.h"

#include "ofxInteractivePrimitives.h"
#include "ofxIPStringBox.h"

#include <set>

ofxInteractivePrimitives::RootNode root;

namespace ofxInteractivePrimitives
{
	struct PortIdentifer;
	
	class Port;
	class PatchCord;
	
	class BaseMessage;
	template <typename T> class Message;
	typedef ofPtr<BaseMessage> MessageRef;
	
	class BasePatcher;
	template <typename T> class Patcher;
	
	struct DelayedDeletable;
	
	typedef unsigned long TypeID;
	
	// TODO: more better RTTI method
	template <typename T>
	TypeID Type2Int()
	{
		const static unsigned int s = 0;
		return (TypeID)&s;
	};
	
	bool in_range(const int& a, const int& b, const int& c)
	{
		return a >= b && a < c;
	}
}

struct ofxInteractivePrimitives::DelayedDeletable
{
public:
	
	DelayedDeletable() : will_delete(false) {}
	
	void delayedDelete()
	{
		will_delete = true;
		addToDelayedDeleteQueue(this);
	}
	
	static void addToDelayedDeleteQueue(DelayedDeletable *o) { getQueue().push(o); }
	static void deleteQueue()
	{
		Queue &queue = getQueue();
		while (queue.size())
		{
			DelayedDeletable *o = queue.front();
			cout << "deleted: " << o << endl;
			delete o;
			queue.pop();
		}
	}
	
protected:
	
	bool getWillDelete() { return will_delete; }

private:
	
	typedef std::queue<DelayedDeletable*> Queue;
	
	bool will_delete;
	
	static Queue& getQueue() { static Queue queue; return queue; }
};


class ofxInteractivePrimitives::BaseMessage : public DelayedDeletable
{
public:
	
	virtual ~BaseMessage() {}
	
	virtual bool isTypeOf() const { return false; }
	
	template <typename T>
	Message<T>* cast() { return (Message<T>*)this; }
	
	void execute() {}
};

template <typename T>
class ofxInteractivePrimitives::Message : public BaseMessage
{
public:
	
	Message() : type(Type2Int<T>()), value(T()) {}
	Message(const T& value) : type(Type2Int<T>()), value(value) {}
	
	bool isTypeOf() const { return type == Type2Int<T>(); }
	const T& get() { return value; }
	bool set(const T& v) { value = v; }
	
	static MessageRef create(const T& v)
	{
		Message<T> *ptr = new Message<T>(v);
		return MessageRef(ptr);
	}
	
	static MessageRef create()
	{
		Message<T> *ptr = new Message<T>(T());
		return MessageRef(ptr);
	}
	
private:
	
	T value;
	TypeID type;
};

struct ofxInteractivePrimitives::PortIdentifer
{
	enum Direction
	{
		INPUT,
		OUTPUT
	};
};

class ofxInteractivePrimitives::PatchCord : public Node, public DelayedDeletable
{
	friend class Port;
	
public:
	
	PatchCord(Port *upstream_port, Port *downstream_port);
	~PatchCord() { disconnect(); }
	
	bool isValid() const { return upstream && downstream; }
	
	Port* getUpstream() const { return upstream; }
	Port* getDownstream() const { return downstream; }
	
	void disconnect();
	
	void draw();
	
protected:
	
	Port* upstream;
	Port* downstream;
	
};

class ofxInteractivePrimitives::Port
{
	template <typename T> friend class Patcher;
	
public:
	
	Port(BasePatcher *patcher, int index, PortIdentifer::Direction direction);
	
	void execute(MessageRef message);
	
	void draw()
	{
		ofRect(rect);
		
		if (direction == PortIdentifer::OUTPUT)
		{
			CordContainerType::iterator it = cords.begin();
			while (it != cords.end())
			{
				PatchCord *o = *it;
				o->draw();
				it++;
			}
		}
	}
	
	void hittest()
	{
		ofRect(rect);
	}
	
	void addCord(PatchCord *cord)
	{
		cords.insert(cord);
	}
	
	void removeCord(PatchCord *cord)
	{
		cords.erase(cord);
	}
	
	PortIdentifer::Direction getDirection() const { return direction; }
	
	ofVec3f getPos() const { return rect.getCenter(); }
	ofVec3f getGlobalPos() const;
	
	BasePatcher* getPatcher() const { return patcher; }
	
	bool hasConnectTo(Port *port);
	
protected:
	
	typedef std::set<PatchCord*> CordContainerType;
	CordContainerType cords;
	
	int index;
	BasePatcher *patcher;
	PortIdentifer::Direction direction;
	
	// data
	MessageRef data;
	
	ofRectangle rect;
	
};


class ofxInteractivePrimitives::BasePatcher : public ofxInteractivePrimitives::StringBox, public DelayedDeletable
{
	friend class Port;
	
public:
	
	BasePatcher(RootNode &root) : StringBox(root) {}
	
	void draw()
	{
		ofPushStyle();
		
		StringBox::draw();
		
		ofNoFill();
		
		if (isHover())
		{
			ofRectangle r = getRect();
			r.x -= 3;
			r.y -= 3;
			r.width += 6;
			r.height += 6;
			ofRect(r);
		}

		const vector<GLuint>& names = getCurrentNameStack();
		if (isHover() && names.size() == 2)
		{
			int index = names[1];
			
			ofVec3f p;
			
			if (names[0] == PortIdentifer::INPUT
				&& in_range(names[1], 0, getNumInput()))
			{
				p = getInputPort(index).getPos();
				p.y -= 1;
			}
			else if (names[0] == PortIdentifer::OUTPUT
				&& in_range(names[1], 0, getNumOutput()))
			{
				p = getOutputPort(index).getPos();
			}
			else assert(false);
			
			ofRectangle r;
			r.setFromCenter(p, 14, 8);
			ofRect(r);
		}
		
		ofFill();
		
		for (int i = 0; i < getNumInput(); i++)
		{
			getInputPort(i).draw();
		}

		for (int i = 0; i < getNumOutput(); i++)
		{
			getOutputPort(i).draw();
		}
		
		if (patching_port && patching_port->getPatcher() == this)
		{
			ofLine(patching_port->getPos(), globalToLocalPos(ofVec2f(ofGetMouseX(), ofGetMouseY())));
		}
		
		ofPopStyle();
	}
	
	void hittest()
	{
		StringBox::hittest();
		
		ofFill();
		
		// input
		pushID(PortIdentifer::INPUT);
		
		for (int i = 0; i < getNumInput(); i++)
		{
			pushID(i);
			getInputPort(i).hittest();
			popID();
		}
		
		popID();
		
		// output
		pushID(PortIdentifer::OUTPUT);
		
		for (int i = 0; i < getNumOutput(); i++)
		{
			pushID(i);
			getOutputPort(i).hittest();
			popID();
		}
		
		popID();
	}
	
	void mouseDragged(int x, int y, int button)
	{
		if (!patching_port)
		{
			move(getMouseDelta());
		}
	}

	void mousePressed(int x, int y, int button)
	{
		const vector<GLuint>& names = getCurrentNameStack();
		
		if (names.size() == 2)
		{
			if (names[0] == PortIdentifer::INPUT)
			{
				patching_port = &getInputPort(names[1]);
			}
			else if (names[0] == PortIdentifer::OUTPUT)
			{
				patching_port = &getOutputPort(names[1]);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			StringBox::mousePressed(x, y, button);
		}
	}
	
	void mouseReleased(int x, int y, int button)
	{
		if (patching_port)
		{
			const vector<GLuint>& names = getCurrentNameStack();
			
			if (names.size() == 2)
			{
				Port *upstream = NULL;
				Port *downstream = NULL;
				
				if (names[0] == PortIdentifer::OUTPUT
					&& in_range(names[1], 0, getNumInput()))
				{
					upstream = patching_port;
					downstream = &getInputPort(names[1]);
					new PatchCord(patching_port, &getInputPort(names[1]));
				}
				else if (names[0] == PortIdentifer::INPUT
						 && in_range(names[1], 0, getNumOutput()))
				{
					upstream = &getOutputPort(names[1]);
					downstream = patching_port;

					new PatchCord(&getOutputPort(names[1]), patching_port);
				}
				else
				{
					assert(false);
				}
				
				// patching validation
				
				// port is null
				if (upstream == NULL || downstream == NULL) goto __cancel__;
				
				// already connected
				if (upstream->hasConnectTo(downstream)) goto __cancel__;
				
				// patching oneself
				if (upstream->getPatcher() == downstream->getPatcher()) goto __cancel__;
				
				
				// create patchcord
				new PatchCord(upstream, downstream);
				
				__cancel__: {
					ofLogWarning("BasePatcher") << "patching failed";
				}
			}
		}
		
		patching_port = NULL;
	}
	
	virtual const char* getName() const { return ""; }
	
	virtual int getNumInput() const { return 0; }
	virtual int getNumOutput() const { return 0; }
	
	Port& getInputPort(int index) { return input_port.at(index); }
	Port& getOutputPort(int index) { return output_port.at(index); }
	
protected:

	void setupPatcher()
	{
		// NOTICE: do not create Port before setText
		setText(getName());
		
		for (int i = 0; i < getNumInput(); i++)
		{
			input_port.push_back(Port(this, i, PortIdentifer::INPUT));
		}

		for (int i = 0; i < getNumOutput(); i++)
		{
			output_port.push_back(Port(this, i, PortIdentifer::OUTPUT));
		}
	}

protected:
	
	static Port *patching_port;
	
	vector<MessageRef> input_data, output_data;
	
	virtual void inputDataUpdated(int index) {}

private:
	
	vector<Port> input_port, output_port;
};

ofxInteractivePrimitives::Port* ofxInteractivePrimitives::BasePatcher::patching_port = NULL;


template <typename T>
class ofxInteractivePrimitives::Patcher : public BasePatcher
{
public:
	
	Patcher(RootNode &root) : BasePatcher(root)
	{
		input_data.resize(getNumInput());
		output_data.resize(getNumOutput());
		
		setupPatcher();
		
		content = (ContextType*)T::create(input_data, output_data);
	}
	
	const char* getName() const { return T::getName(); }
	
	int getNumInput() const { return T::getNumInput(); }
	TypeID getInputType(int index) const { return T::getInputType(index); }
	void setInput(int index, MessageRef data) {}
	
	
	int getNumOutput() const { return T::getNumOutput(); }
	TypeID getOutputType(int index) const { return T::getOutputType(index); }
	void setOutput(int index, MessageRef data) {}
	
	void execute()
	{
		for (int i = 0; i < getNumInput(); i++)
		{
			Port &input_port = getInputPort(i);
			input_data[i] = input_port.data;
		}
		
		T::execute(this, content, input_data, output_data);
		
		for (int i = 0; i < getNumOutput(); i++)
		{
			Port &output_port = getOutputPort(i);
			output_port.execute(output_data[i]);
		}
	}
	
	void update()
	{
		execute();
		
	}
	
protected:
	
	void inputDataUpdated(int index)
	{
		execute();
	}
	
private:
	
	typedef typename T::ContextType ContextType;
	ContextType *content;
	
};


//

using namespace ofxInteractivePrimitives;

// PatchCord

PatchCord::PatchCord(Port *upstream_port, Port *downstream_port)
: upstream(upstream_port), downstream(downstream_port)
{
	getUpstream()->addCord(this);
	getDownstream()->addCord(this);
}

void PatchCord::disconnect()
{
	getUpstream()->removeCord(this);
	getDownstream()->removeCord(this);
	
	delayedDelete();
}

void PatchCord::draw()
{
	if (!isValid()) return;
	
	const ofVec3f p0 = getUpstream()->getPos();
	const ofVec3f p1 = getUpstream()->getPatcher()->globalToLocalPos(getDownstream()->getGlobalPos());
	
	ofLine(p0, p1);
}

// Port

Port::Port(BasePatcher *patcher, int index, PortIdentifer::Direction direction) : patcher(patcher), index(index), direction(direction)
{
	if (direction == PortIdentifer::INPUT)
	{
		rect.x = 14 * index;
		rect.y = -5;
	}
	else if (direction == PortIdentifer::OUTPUT)
	{
		rect.x = 14 * index;
		rect.y = patcher->getRect().height;
	}
	else
	{
		assert(false);
	}
	
	rect.width = 10;
	rect.height = 4;
}

void Port::execute(MessageRef message)
{
	if (direction == PortIdentifer::INPUT)
	{
		data = message;
		patcher->inputDataUpdated(index);
	}
	else if (direction == PortIdentifer::OUTPUT)
	{
		CordContainerType::iterator it = cords.begin();
		while (it != cords.end())
		{
			PatchCord *cord = *it;
			Port *port = cord->getDownstream();
			if (port) port->execute(message);

			it++;
		}
	}
}

ofVec3f Port::getGlobalPos() const
{
	return patcher->localToGlobalPos(getPos());
}

bool Port::hasConnectTo(Port *port)
{
	CordContainerType::iterator it = cords.begin();
	while (it != cords.end())
	{
		PatchCord *cord = *it;
		Port *p = cord->getDownstream();
		if (p == port) return true;
		
		it++;
	}

	return false;
}

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

struct TestClassWrapper
{
	typedef TestClass ContextType;
		
	static const char* getName() { return "TestClass"; }
	
	static void* create(vector<MessageRef>& input, vector<MessageRef>& output)
	{
		output[0] = Message<ofVec3f>::create(ofVec3f(0));
		
		return new TestClass;
	}
	
	static void execute(BasePatcher *patcher, ContextType *context, const vector<MessageRef>& input, vector<MessageRef>& output)
	{
		
		Message<ofVec3f> *out0 = output[0]->cast<ofVec3f>();
		out0->set(patcher->getPosition());
	}
	
	static int getNumInput()
	{
		return 0;
	}
	
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

struct PrintClassWrapper
{
	typedef TestClass ContextType;
	
	static const char* getName() { return "PrintClass"; }
	
	static void* create(vector<MessageRef>& input, vector<MessageRef>& output)
	{
		input[0] = Message<int>::create(42);
		return NULL;
	}
	
	static void execute(BasePatcher *patcher, ContextType *context, const vector<MessageRef>& input, vector<MessageRef>& output)
	{
		Message<ofVec3f> *in0 = input[0]->cast<ofVec3f>();
		if (in0)
		{
			stringstream ss;
			ss << in0->get();
			patcher->setText(ss.str());
		}
	}
	
	static int getNumInput()
	{
		return 1;
	}
	
	static TypeID getInputType(int index)
	{
		return Type2Int<int>();
	}
	
	static int getNumOutput()
	{
		return 0;
	}
	
	static TypeID getOutputType(int index)
	{
		return Type2Int<void>();
	}
};


Patcher<TestClassWrapper> *node0;
Patcher<PrintClassWrapper> *node1;
PatchCord *cord;

//--------------------------------------------------------------
void testApp::setup()
{

	
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground(0);
	
	node0 = new Patcher<TestClassWrapper>(root);
	node1 = new Patcher<PrintClassWrapper>(root);
	
	// node0 -> node1
//	cord = new PatchCord(&node0->getOutputPort(0), &node1->getInputPort(0));
	
	node0->execute();
	
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