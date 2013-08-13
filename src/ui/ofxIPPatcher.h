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
	
	class IPatchObject;
	
	struct NullType {};
	
	template <typename T, typename V>
	class PatchObject;
	
	template <typename T, typename InteractivePrimitiveType>
	struct Wrapper;

	struct DelayedDeletable;
	
	typedef unsigned long TypeID;
	
	// TODO: more better RTTI method
	template <typename T>
	TypeID Type2Int()
	{
		const static unsigned int s = 0;
		return (TypeID)&s;
	};
	
	inline bool in_range(const unsigned int& a, const unsigned int& b, const unsigned int& c)
	{
		return a >= b && a < c;
	}
	
	static Port *patching_port;
	
	struct BaseWrapper;
	
	class Patcher;
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
	}
	
	static void deleteQueue()
	{
		Queue &queue = getQueue();
		Queue::iterator it = queue.begin();
		
		while (it != queue.end())
		{
			DelayedDeletable *o = *it;
			delete o;
			it++;
		}
		
		queue.clear();
	}
	
protected:
	
	static void addToDelayedDeleteQueue(DelayedDeletable *o) { getQueue().insert(o); }
	
protected:
	
	bool getWillDelete() { return will_delete; }
	
private:
	
	typedef std::set<DelayedDeletable*> Queue;
	static Queue& getQueue() { static Queue queue; return queue; }
	
	bool will_delete;
	
};

#pragma mark - BaseMessage

class ofxInteractivePrimitives::BaseMessage : public DelayedDeletable
{
public:
	
	BaseMessage() : type(Type2Int<NullType>()) {}
	
	virtual ~BaseMessage() {}

	inline TypeID getType() const { return type; }
	
	template <typename T>
	inline bool isTypeOf() const { return type == Type2Int<T>(); }
	
	template <typename T>
	inline Message<T>* cast() { return (Message<T>*)this; }
	
	template <typename T>
	inline bool get(T &v)
	{
		if (isTypeOf<T>())
		{
			v = this->cast<T>()->get();
			return true;
		}

		return false;
	}
	
	void execute() {}
	
protected:
	
	TypeID type;
};

#pragma mark - Message

template <typename T>
class ofxInteractivePrimitives::Message : public BaseMessage
{
public:
	
	Message() : value(T())
	{
		type = Type2Int<T>();
	}
	
	Message(const T& v) : value(v)
	{
		type = Type2Int<T>();
	}
	
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
	~PatchCord() {}

	void disconnect();
	
	bool isValid() const { return upstream && downstream; }
	
	Port* getUpstream() const { return upstream; }
	Port* getDownstream() const { return downstream; }
	
	void draw();
	void hittest();
	
	void keyPressed(int key);
	
protected:
	
	Port* upstream;
	Port* downstream;
};

#pragma mark - Port

class ofxInteractivePrimitives::Port
{
	template <typename T, typename V>
	friend class PatchObject;
	
public:
	
	Port(IPatchObject *patcher, size_t index, PortIdentifer::Direction direction, const string &desc = "");
	~Port()
	{
		cords.clear();
	}
	
	MessageRef& requestUpdate();
	
	void draw()
	{
		ofRect(rect);
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
	
	void disconnectAll()
	{
		CordContainerType t = cords;
		CordContainerType::iterator it = t.begin();
		while (it != t.end())
		{
			PatchCord *c = *it;
			c->disconnect();
			it++;
		}
		cords.clear();
	}
	
	PortIdentifer::Direction getDirection() const { return direction; }
	
	void setRect(const ofRectangle& r) { rect = r; }
	const ofRectangle& getRect() { return rect; }
	
	ofVec3f getPos() const { return rect.getCenter(); }
	ofVec3f getGlobalPos() const;
	
	IPatchObject* getPatchObject() const { return patcher; }
	
	TypeID getType() const { return data->getType(); }
	
	const string& getDescription() const { return desc; }
	
	// connection
	int getNumConnected() const { return cords.size(); }
	bool hasConnect() const { return !cords.empty(); }
	
	bool hasConnectTo(Port *port);
	
	//data
	template <typename T>
	inline void set(const T& v)
	{
		setData(Message<T>::create(v));
	}
	
	inline MessageRef& getData() { return data; }
	inline void setData(const MessageRef& d) { data = d; }
	
protected:
	
	typedef std::set<PatchCord*> CordContainerType;
	CordContainerType cords;
	
	size_t index;
	string desc;
	IPatchObject *patcher;
	PortIdentifer::Direction direction;
	
	// data
	MessageRef data;
	
	ofRectangle rect;
};

class ofxInteractivePrimitives::IPatchObject : public ofxInteractivePrimitives::DelayedDeletable
{
	friend class Port;
	
public:
	
	virtual void setupInternal() {}
	
	virtual MessageRef executeUpstream() { return MessageRef(); }
	
	virtual Element2D* getUIElement() = 0;
	
	virtual size_t getNumInput() const { return 0; }
	virtual size_t getNumOutput() const { return 0; }
	
	virtual TypeID getInputType(size_t index) const { return Type2Int<void>(); }
	virtual TypeID getOutputType(size_t index) { return Type2Int<void>(); }

	virtual Port& getInputPort(size_t index) = 0;
	virtual Port& getOutputPort(size_t index) = 0;
	
	//
	
	virtual ofVec3f localToGlobalPos(const ofVec3f& v) = 0;
	virtual ofVec3f globalToLocalPos(const ofVec3f& v) = 0;
	
	virtual void setPosition(const ofVec3f &v) = 0;
	void setPosition(float x, float y, float z = 0)
	{
		this->setPosition(ofVec3f(x, y, z));
	}
	virtual ofVec3f getPosition() = 0;
	
protected:
	
	// virtual void inputDataUpdated(size_t index) = 0;
};



#pragma mark - PatchObject

template <
	typename T,
	typename InteractivePrimitiveType = ofxInteractivePrimitives::DraggableStringBox
>
class ofxInteractivePrimitives::PatchObject : public IPatchObject, public InteractivePrimitiveType, public T
{
public:

	PatchObject(Node &parent) : IPatchObject(), InteractivePrimitiveType(parent) {}
	
	void setupInternal()
	{
		disposePatchCords();
		T::setupPatchObject(this);
		alignPort();
	}
	
	~PatchObject()
	{
		this->dispose();
	}
	
	void dispose()
	{
		input_data.clear();
		output_data.clear();

		InteractivePrimitiveType::dispose();
	}
	
	Port& addInput(const string& desc = "")
	{
		input_port.push_back(Port(this, input_port.size(), PortIdentifer::INPUT, desc));
		return input_port.back();
	}
	
	Port& addOutput(const string& desc = "")
	{
		output_port.push_back(Port(this, input_port.size(), PortIdentifer::OUTPUT, desc));
		return output_port.back();
	}
	
	size_t getNumInput() const { return input_port.size(); }
	void setInput(size_t index, MessageRef data) { input_data.at(index) = data; }
	
	size_t getNumOutput() const { return output_port.size(); }
	void setOutput(size_t index, MessageRef data) { output_data.at(index) = data; }
	
	inline Port& getInputPort(size_t index) { return input_port.at(index); }
	inline Port& getOutputPort(size_t index) { return output_port.at(index); }
	
	TypeID getInputType(size_t index) const { return Type2Int<void>(); }
	TypeID getOutputType(size_t index) { return Type2Int<void>(); }

	MessageRef executeUpstream()
	{
//		T::begin(this, input_data, output_data);
		
//		for (int i = 0; i < getNumInput(); i++)
//		{
//			Port &input_port = getInputPort(i);
//			input_port.requestUpdate();
//			input_data[i] = input_port.data;
//		}
		
//		T::end(this, input_data, output_data);
		
		T::updatePatchObject(this);
		
//		for (int i = 0; i < getNumOutput(); i++)
//		{
//			Port &output_port = getOutputPort(i);
//			output_port.data = output_data[i];
//		}
		
		return MessageRef();
	}
	
	// ofxIP
	
	void update()
	{
		InteractivePrimitiveType::update();
		
		if (T::isOutput())
			T::updatePatchObject(this);
	}
	
	void draw()
	{
		ofPushStyle();
		
		InteractivePrimitiveType::draw();
		
		{
			ofPushStyle();
			
			if (this->isHover())
				ofSetLineWidth(2);
			else
				ofSetLineWidth(1);
			
			if (this->isFocus())
				ofSetColor(ofColor::fromHex(0xCCFF77), 127);

			ofNoFill();
			
			ofRectangle r = this->getContentRect();
			r.x -= 2;
			r.y -= 2;
			r.width += 4;
			r.height += 4;
			
			ofRect(r);
			
			ofPopStyle();
		}
		
		const vector<GLuint>& names = this->getCurrentNameStack();
		if (this->isHover() && names.size() == 2)
		{
			size_t index = names[1];
			
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
		
		for (size_t i = 0; i < getNumInput(); i++)
		{
			getInputPort(i).draw();
		}
		
		for (size_t i = 0; i < getNumOutput(); i++)
		{
			getOutputPort(i).draw();
		}
		
		if (patching_port && patching_port->getPatchObject() == this)
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
		
		for (size_t i = 0; i < getNumInput(); i++)
		{
			this->pushID(i);
			getInputPort(i).hittest();
			this->popID();
		}
		
		this->popID();
		
		// output
		this->pushID(PortIdentifer::OUTPUT);
		
		for (size_t i = 0; i < getNumOutput(); i++)
		{
			this->pushID(i);
			getOutputPort(i).hittest();
			this->popID();
		}
		
		this->popID();
	}
	
	void mouseDragged(int x, int y, int button)
	{
		if (patching_port == 0)
		{
			InteractivePrimitiveType::mouseDragged(x, y, button);
		}
		
		T::mouseDragged(this, x, y, button);
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
		else
		{
			InteractivePrimitiveType::mousePressed(x, y, button);
		}
		
		T::mousePressed(this, x, y, button);
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
				else goto __cancel__;
				
				createPatchCord(upstream, downstream);
			}
		}
		else
		{
			InteractivePrimitiveType::mouseReleased(x, y, button);
		}
		
		__cancel__:;
		patching_port = NULL;
		
		T::mouseReleased(this, x, y, button);
	}
	
	void keyPressed(int key)
	{
		if (key == OF_KEY_DEL || key == OF_KEY_BACKSPACE)
		{
			disposePatchCords();
			delayedDelete();
		}
		else
		{
			InteractivePrimitiveType::keyPressed(key);
		}
		
		T::keyPressed(this, key);
	}

	void keyReleased(int key)
	{
		InteractivePrimitiveType::keyPressed(key);
		T::keyReleased(this, key);
	}
	
	//
	
	ofVec3f localToGlobalPos(const ofVec3f& v) { return InteractivePrimitiveType::localToGlobalPos(v); }
	ofVec3f globalToLocalPos(const ofVec3f& v) { return InteractivePrimitiveType::globalToLocalPos(v); }
	ofVec3f getPosition() { return InteractivePrimitiveType::getPosition(); }
	void setPosition(const ofVec3f &v) { InteractivePrimitiveType::setPosition(v); }
	
	Element2D* getUIElement() { return this; }
	
	void updateDisplay()
	{
		alignPort();
	}
	
protected:
	
	void disposePatchCords()
	{
		struct disconnect
		{
			void operator()(Port &o)
			{
				o.disconnectAll();
			}
		};
		
		for_each(input_port.begin(), input_port.end(), disconnect());
		for_each(output_port.begin(), output_port.end(), disconnect());
	}
	
	virtual void alignPort()
	{
		for (size_t i = 0; i < getNumInput(); i++)
		{
			ofRectangle rect;
			rect.x = 14 * i;
			rect.y = -5;
			rect.width = 10;
			rect.height = 4;
			
			getInputPort(i).setRect(rect);
		}
		
		for (size_t i = 0; i < getNumOutput(); i++)
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
		if (upstream->getPatchObject() == downstream->getPatchObject())
		{
			msg = "patching oneself";
			goto __cancel__;
		}
		
		// TODO: loop detection
		
		// create patchcord
		return new PatchCord(upstream, downstream);
		
	__cancel__:
		
		ofLogWarning("AbstructPatchObject") << "patching failed: " << msg;
		return NULL;
	}
	
private:
	
	vector<MessageRef> input_data, output_data;
	vector<Port> input_port, output_port;
};

#pragma mark - Wrapper

template <
	typename T,
	typename InteractivePrimitiveType = ofxInteractivePrimitives::DraggableStringBox
>
struct ofxInteractivePrimitives::Wrapper
{
	typedef T Class;
	typedef ofxInteractivePrimitives::PatchObject<Class, InteractivePrimitiveType> PatchObject;
	
	static PatchObject* Create(ofxInteractivePrimitives::Node &parent)
	{
		PatchObject *o = new PatchObject(parent);
		o->setupInternal();
		return o;
	}
	
//	static void begin(PatchObject *self, const vector<MessageRef>& input, vector<MessageRef>& output) {}
//	static void end(PatchObject *self, const vector<MessageRef>& input, vector<MessageRef>& output) {}

	static void updateRequested(PatchObject *self) {}
	
	static void setupPatchObject(PatchObject *self) {}
	static void updatePatchObject(PatchObject *self) {}
	
	inline static bool isOutput() { return false; }
	
	static void mousePressed(PatchObject *self, int x, int y, int button) {}
	static void mouseReleased(PatchObject *self, int x, int y, int button) {}
	static void mouseMoved(PatchObject *self, int x, int y) {}
	static void mouseDragged(PatchObject *self, int x, int y, int button) {}
	static void keyPressed(PatchObject *self, int key) {}
	static void keyReleased(PatchObject *self, int key) {}

//	static int getNumInput() { return 0; }
//	static int getNumOutput() { return 0; }
};

#pragma mark - Patcher

class ofxInteractivePrimitives::Patcher : public ofxInteractivePrimitives::RootNode
{
public:
	
	inline IPatchObject* create(const string &name)
	{
		if (object_factory.find(name) == object_factory.end()) return NULL;
		else return object_factory[name](this);
	}

	//
	
	template <typename T>
	inline void registerPatchObject(const string &name)
	{
		object_factory[name] = &classFactory<T>;
	}
	
	void save(const string &path) {}
	void load(const string &path) {}
	
	// TODO: undo & redo
	void undo() {}
	void redo() {}
	
protected:
	
	typedef IPatchObject* (*ClassFactory)(Patcher *self);
	map<string, ClassFactory> object_factory;
	
	template <typename T>
	static IPatchObject* classFactory(Patcher *self)
	{
		return T::PatchObject::Create(*self);
	}

};
