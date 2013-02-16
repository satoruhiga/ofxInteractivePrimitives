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
	
	template <typename T>
	bool in_range(const T& a, const T& b, const T& c)
	{
		return a >= b && a < c;
	}
}

struct ofxInteractivePrimitives::DelayedDeletable
{
public:
	
	DelayedDeletable() : delayed_delete(false) {}
	
	void delayedDelete()
	{
		delayed_delete = true;
		
		// TODO: impl delayed delete
		delete this;
	}
	bool getDelayedDelete() { return delayed_delete; }

private:
	
	bool delayed_delete;
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
	
	PatchCord(Port *port);
	PatchCord(Port *upstream_port, Port *downstream_port);
	~PatchCord() { disconnect(); }
	
	bool isValid() const { return upstream && downstream; }
	
	bool setAnotherPort(Port *port);
	
	Port* getUpstream() const { return upstream; }
	Port* getDownstream() const { return downstream; }
	
	void disconnect();
	
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
	
	ofVec3f getCenter() const { return rect.getCenter(); }
	
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
	
	BasePatcher(RootNode &root) : StringBox(root), patching_port(NULL) {}
	
	void draw()
	{
		StringBox::draw();
		
		ofFill();
		
		for (int i = 0; i < getNumInput(); i++)
		{
			getInputPort(i).draw();
		}

		for (int i = 0; i < getNumOutput(); i++)
		{
			getOutputPort(i).draw();
		}
		
		if (patching_port)
		{
			ofLine(patching_port->getCenter(), globalToLocalPos(ofVec2f(ofGetMouseX(), ofGetMouseY())));
		}
	}
	
	void hittest()
	{
		StringBox::hittest();
		
		ofFill();
		
		pushID(0);
		
		{
			for (int i = 0; i < getNumInput(); i++)
			{
				pushID(i);
				getInputPort(i).draw();
				popID();
			}
		}
		
		popID();
		
		pushID(1);
		
		{
			for (int i = 0; i < getNumOutput(); i++)
			{
				pushID(i);
				getOutputPort(i).draw();
				popID();
			}
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
			if (names[0] == 0)
			{
				patching_port = &getInputPort(names[1]);
			}
			else if (names[0] == 1)
			{
				patching_port = &getOutputPort(names[1]);
			}
			else
			{
				assert(false);
			}
			
			// create new patchcode
		}
		else
		{
			StringBox::mousePressed(x, y, button);
		}
	}
	
	void mouseReleased(int x, int y, int button)
	{
		cout << "rel" << endl;
		
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
	
	Port *patching_port;
	vector<MessageRef> input_data, output_data;
	
	virtual void inputDataUpdated(int index) {}

private:
	
	vector<Port> input_port, output_port;
};


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
		
		T::execute(content, input_data, output_data);
		
		for (int i = 0; i < getNumOutput(); i++)
		{
			Port &output_port = getOutputPort(i);
			output_port.execute(output_data[i]);
		}
	}
	
protected:
	
	void inputDataUpdated(int index)
	{
		// cout << "input updated: " << index << endl;
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
	
//	delayedDelete();
}

PatchCord::PatchCord(Port *port)
{
	if (port->getDirection() == PortIdentifer::INPUT)
	{
		upstream = port;
	}
	else if (port->getDirection() == PortIdentifer::OUTPUT)
	{
		downstream = port;
	}
	else
	{
		assert(false);
	}
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

bool PatchCord::setAnotherPort(Port *port)
{
	if (port->getDirection() == PortIdentifer::INPUT && downstream)
	{
		upstream = port;
		return true;
	}
	else if (port->getDirection() == PortIdentifer::OUTPUT && upstream)
	{
		downstream = port;
		return true;
	}
	
	return false;
}

void Port::execute(MessageRef message)
{
	if (direction == PortIdentifer::INPUT)
	{
		cout << "set input data" << endl;
		data = message;
		
		patcher->inputDataUpdated(index);
	}
	else if (direction == PortIdentifer::OUTPUT)
	{
		cout << "send message" << endl;
		
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
		input[0] = Message<int>::create(42);
		output[0] = Message<int>::create(2000);
		
		return new TestClass;
	}
	
	static void execute(ContextType *context, const vector<MessageRef>& input, vector<MessageRef>& output)
	{
		Message<int> *in0 = input[0]->cast<int>();
		if (in0) context->test(in0->get());
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
		return 1;
	}
	
	static TypeID getOutputType(int index)
	{
		return Type2Int<int>();
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
	
	static void execute(ContextType *context, const vector<MessageRef>& input, vector<MessageRef>& output)
	{
		Message<int> *in0 = input[0]->cast<int>();
		if (in0)
		{
			printf("%i\n", in0->get());
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
	cord = new PatchCord(&node0->getOutputPort(0), &node1->getInputPort(0));
	
	node0->execute();
	
	node0->setPosition(200, 200, 0);
	node1->setPosition(200, 300, 0);
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