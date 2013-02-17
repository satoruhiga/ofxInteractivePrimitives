#pragma once

#include "ofMain.h"

#include "ofxInteractivePrimitives.h"
#include "ofxIPStringBox.h"

#include <set>


namespace ofxInteractivePrimitives
{
	struct PortIdentifer;
	
	class Port;
	class PatchCord;
	
	class BaseMessage;
	
	template <typename T>
	class Message;
	
	typedef ofPtr<BaseMessage> MessageRef;
	
	class BasePatcher;
	
	template <typename InteractivePrimitiveType>
	class AbstructPatcher;
	
	template <typename T, typename V>
	class Patcher;
	
	struct DelayedDeletable;
	
	typedef unsigned long TypeID;
	
	// TODO: more better RTTI method
	template <typename T>
	TypeID Type2Int()
	{
		const static unsigned int s = 0;
		return (TypeID) & s;
	};
	
	bool in_range(const int& a, const int& b, const int& c)
	{
		return a >= b && a < c;
	}
	
	static Port *patching_port;
	
	struct BaseWrapper;
}

#pragma mark - DelayedDeletable

struct ofxInteractivePrimitives::DelayedDeletable
{
public:
	
	DelayedDeletable() : will_delete(false) {}
	virtual ~DelayedDeletable() {}
	
	void delayedDelete()
	{
		if (will_delete) return;
		
		will_delete = true;
		
		addToDelayedDeleteQueue(this);
		//		delete this;
	}
	
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
	
	static void addToDelayedDeleteQueue(DelayedDeletable *o) { getQueue().push(o); }
	
protected:
	
	bool getWillDelete() { return will_delete; }
	
private:
	
	typedef std::queue<DelayedDeletable*> Queue;
	static Queue& getQueue() { static Queue queue; return queue; }
	
	bool will_delete;
	
};

#pragma mark - BaseMessage

class ofxInteractivePrimitives::BaseMessage : public DelayedDeletable
{
public:
	
	virtual ~BaseMessage() {}
	
	virtual bool isTypeOf() const { return false; }
	
	template <typename T>
	Message<T>* cast() { return (Message<T>*) this; }
	
	void execute() {}
};

#pragma mark - Message

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

#pragma mark - PortIdentifer

struct ofxInteractivePrimitives::PortIdentifer
{
	enum Direction
	{
		INPUT,
		OUTPUT
	};
};

#pragma mark - PatchCord

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

#pragma mark - Port

class ofxInteractivePrimitives::Port
{
	template <typename T, typename V>
	friend class Patcher;
	
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
	
	void setRect(const ofRectangle& rect) { this->rect = rect; }
	const ofRectangle& getRect() { return rect; }
	
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

class ofxInteractivePrimitives::BasePatcher : public ofxInteractivePrimitives::DelayedDeletable
{
	friend class Port;
	
public:
	
	virtual void execute() {}
	
	virtual int getNumInput() const { return 0; }
	virtual int getNumOutput() const { return 0; }
	
	virtual Port& getInputPort(int index) = 0;
	virtual Port& getOutputPort(int index) = 0;
	
	//
	
	virtual ofVec3f localToGlobalPos(const ofVec3f& v) = 0;
	virtual ofVec3f globalToLocalPos(const ofVec3f& v) = 0;
	virtual ofVec3f getPosition() = 0;
	
protected:
	
	virtual void inputDataUpdated(int index) = 0;
};

#pragma mark - Patcher

template <typename T, typename InteractivePrimitiveType = ofxInteractivePrimitives::StringBox>
class ofxInteractivePrimitives::Patcher : public BasePatcher, public InteractivePrimitiveType
{
public:
	
	Patcher(RootNode &root) : BasePatcher(), InteractivePrimitiveType(root)
	{
		input_data.resize(getNumInput());
		output_data.resize(getNumOutput());
		
		setupPatcher();
		
		content = (ContextType*)T::create(input_data, output_data);
	}
	
	int getNumInput() const { return T::getNumInput(); }
	TypeID getInputType(int index) const { return T::getInputType(index); }
	void setInput(int index, MessageRef data) {}
	
	int getNumOutput() const { return T::getNumOutput(); }
	TypeID getOutputType(int index) const { return T::getOutputType(index); }
	void setOutput(int index, MessageRef data) {}
	
	void update() { T::update(this); }
	
	Port& getInputPort(int index) { return input_port.at(index); }
	Port& getOutputPort(int index) { return output_port.at(index); }
	
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
	
	// ofxIP
	
	void draw()
	{
		ofPushStyle();
		
		InteractivePrimitiveType::draw();
		
		ofNoFill();
		
		if (this->isHover())
		{
			ofRectangle r = this->getContentRect();
			r.x -= 3;
			r.y -= 3;
			r.width += 6;
			r.height += 6;
			ofRect(r);
		}
		
		const vector<GLuint>& names = this->getCurrentNameStack();
		if (this->isHover() && names.size() == 2)
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
			ofLine(patching_port->getPos(), this->globalToLocalPos(ofVec2f(ofGetMouseX(), ofGetMouseY())));
		}
		
		ofPopStyle();
	}
	
	void hittest()
	{
		InteractivePrimitiveType::hittest();
		
		ofFill();
		
		// input
		this->pushID(PortIdentifer::INPUT);
		
		for (int i = 0; i < getNumInput(); i++)
		{
			this->pushID(i);
			getInputPort(i).hittest();
			this->popID();
		}
		
		this->popID();
		
		// output
		this->pushID(PortIdentifer::OUTPUT);
		
		for (int i = 0; i < getNumOutput(); i++)
		{
			this->pushID(i);
			getOutputPort(i).hittest();
			this->popID();
		}
		
		this->popID();
	}
	
	void mouseDragged(int x, int y, int button)
	{
		if (!patching_port)
		{
			this->move(this->getMouseDelta());
		}
	}
	
	void mousePressed(int x, int y, int button)
	{
		const vector<GLuint>& names = this->getCurrentNameStack();
		
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
			else assert(false);
		}
	}
	
	void mouseReleased(int x, int y, int button)
	{
		if (patching_port)
		{
			const vector<GLuint>& names = this->getCurrentNameStack();
			
			if (names.size() == 2)
			{
				Port *upstream = NULL;
				Port *downstream = NULL;
				
				if (names[0] == PortIdentifer::OUTPUT
					&& in_range(names[1], 0, getNumInput()))
				{
					upstream = patching_port;
					downstream = &getInputPort(names[1]);
				}
				else if (names[0] == PortIdentifer::INPUT
						 && in_range(names[1], 0, getNumOutput()))
				{
					upstream = &getOutputPort(names[1]);
					downstream = patching_port;
				}
				else assert(false);
				
				createPatchCord(upstream, downstream);
			}
		}
		
		patching_port = NULL;
	}
	
	//
	
	ofVec3f localToGlobalPos(const ofVec3f& v) { return InteractivePrimitiveType::localToGlobalPos(v); }
	ofVec3f globalToLocalPos(const ofVec3f& v) { return InteractivePrimitiveType::globalToLocalPos(v); }
	ofVec3f getPosition() { return InteractivePrimitiveType::getPosition(); }
	
protected:
	
	void inputDataUpdated(int index)
	{
		execute();
	}
	
	void setupPatcher()
	{
		T::layout(this);
		
		for (int i = 0; i < getNumInput(); i++)
		{
			input_port.push_back(Port(this, i, PortIdentifer::INPUT));
		}
		
		for (int i = 0; i < getNumOutput(); i++)
		{
			output_port.push_back(Port(this, i, PortIdentifer::OUTPUT));
		}
		
		alignPort();
	}
	
	virtual void alignPort()
	{
		for (int i = 0; i < getNumInput(); i++)
		{
			ofRectangle rect;
			rect.x = 14 * i;
			rect.y = -5;
			rect.width = 10;
			rect.height = 4;
			
			getInputPort(i).setRect(rect);
		}
		
		for (int i = 0; i < getNumOutput(); i++)
		{
			ofRectangle rect;
			rect.x = 14 * i;
			rect.y = this->getContentHeight();
			rect.width = 10;
			rect.height = 4;
			
			getOutputPort(i).setRect(rect);
		}
	}
	
	PatchCord* createPatchCord(Port *upstream, Port *downstream)
	{
		// patching validation
		const char *msg = "unknown error";
		
		// port is null
		if (upstream == NULL || downstream == NULL)
		{
			msg = "port is null";
			goto __cancel__;
		}
		
		// already connected
		if (upstream->hasConnectTo(downstream))
		{
			msg = "already connected";
			goto __cancel__;
		}
		
		// patching oneself
		if (upstream->getPatcher() == downstream->getPatcher())
		{
			msg = "patching oneself";
			goto __cancel__;
		}
		
		// TODO: loop detection
		
		// create patchcord
		return new PatchCord(upstream, downstream);
		
	__cancel__:
		
		ofLogWarning("AbstructPatcher") << "patching failed: " << msg;
		return NULL;
	}
	
private:
	
	typedef typename T::ContextType ContextType;
	ContextType *content;
	
	vector<MessageRef> input_data, output_data;
	vector<Port> input_port, output_port;
};

#pragma mark - BaseWrapper

struct ofxInteractivePrimitives::BaseWrapper
{
	static void* create(vector<MessageRef>& input, vector<MessageRef>& output) { return NULL; }
	
	static void execute(BasePatcher *patcher, void *context, const vector<MessageRef>& input, vector<MessageRef>& output) {}
	
	static void layout(BasePatcher *patcher) {}
	static void update(BasePatcher *patcher) {}
	
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
		return 0;
	}
	
	static TypeID getOutputType(int index)
	{
		return Type2Int<void>();
	}
};
