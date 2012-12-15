#include "ofxInteractivePrimitives.h"

class ofxInteractivePrimitives::Context
{
public:
	
	vector<ofxInteractivePrimitives*> elements;
	
	GLint viewport[4];
	GLdouble projection[16], modelview[16];
	
	unsigned long current_object_id;
	float current_depth;
	
	ofxInteractivePrimitives *current_object;
	ofxInteractivePrimitives *focus_object;

	Context() : current_object_id(0), current_depth(0), current_object(NULL)
	{
		enableAllEvent();
	}
	
	~Context()
	{
		disableAllEvent();
	}
	
	void registerElement(ofxInteractivePrimitives *o)
	{
		// TODO: error check
		elements.push_back(o);
		o->object_id = current_object_id++;
	}
	
	void unregisterElement(ofxInteractivePrimitives *o)
	{
		elements.erase(remove(elements.begin(), elements.end(), o), elements.end());
	}
	
	void enableAllEvent()
	{
//		ofAddListener(ofEvents().update, this, &ofxInteractivePrimitives::Context::update);
//		ofAddListener(ofEvents().draw, this, &ofxInteractivePrimitives::Context::draw);
		
		ofAddListener(ofEvents().mousePressed, this, &ofxInteractivePrimitives::Context::mousePressed);
		ofAddListener(ofEvents().mouseReleased, this, &ofxInteractivePrimitives::Context::mouseReleased);
		ofAddListener(ofEvents().mouseMoved, this, &ofxInteractivePrimitives::Context::mouseMoved);
		ofAddListener(ofEvents().mouseDragged, this, &ofxInteractivePrimitives::Context::mouseDragged);
		
		ofAddListener(ofEvents().keyPressed, this, &ofxInteractivePrimitives::Context::keyPressed);
		ofAddListener(ofEvents().keyReleased, this, &ofxInteractivePrimitives::Context::keyReleased);
	}
	
	void disableAllEvent()
	{
//		ofRemoveListener(ofEvents().update, this, &ofxInteractivePrimitives::Context::update);
//		ofRemoveListener(ofEvents().draw, this, &ofxInteractivePrimitives::Context::draw);
		
		ofRemoveListener(ofEvents().mousePressed, this, &ofxInteractivePrimitives::Context::mousePressed);
		ofRemoveListener(ofEvents().mouseReleased, this, &ofxInteractivePrimitives::Context::mouseReleased);
		ofRemoveListener(ofEvents().mouseMoved, this, &ofxInteractivePrimitives::Context::mouseMoved);
		ofRemoveListener(ofEvents().mouseDragged, this, &ofxInteractivePrimitives::Context::mouseDragged);
		
		ofRemoveListener(ofEvents().keyPressed, this, &ofxInteractivePrimitives::Context::keyPressed);
		ofRemoveListener(ofEvents().keyReleased, this, &ofxInteractivePrimitives::Context::keyReleased);
	}
	
	void prepare()
	{
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetIntegerv(GL_VIEWPORT, viewport);
	}
	
	void hittest()
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			if (!e->getVisible()) continue;
			
			e->transformGL();
			glPushName(e->object_id);
			e->hittest();
			glPopName();
			e->restoreTransformGL();
		}
	}
	
	struct Selection
	{
		GLuint min_depth, max_depth;
		vector<GLuint> name_stack;
	};

	static bool sort_by_depth(const Selection &a, const Selection &b)
	{
		return a.min_depth < b.min_depth;
	}

	vector<Selection> pickup(int x, int y)
	{
		const int BUFSIZE = 256;
		GLuint selectBuf[BUFSIZE];
		GLint hits;
		
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		ofPushStyle();
		ofPushView();
		
		glEnable(GL_DEPTH_TEST);
		ofFill();
		
		glSelectBuffer(BUFSIZE, selectBuf);
		glRenderMode(GL_SELECT);
		glMatrixMode(GL_PROJECTION);
		
		glPushMatrix();
		{
			glLoadIdentity();
			gluPickMatrix(x, viewport[3] - y, 1.0, 1.0, viewport);
			glMultMatrixd(projection);
			
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMultMatrixd(modelview);
			
			hittest();
			
			glMatrixMode(GL_PROJECTION);
		}
		glPopMatrix();
		
		glMatrixMode(GL_MODELVIEW);
		
		hits = glRenderMode(GL_RENDER);
		
		ofPopView();
		ofPopStyle();
		glPopAttrib();
		
		if (hits <= 0) return vector<Selection>();
		
		GLuint *ptr = selectBuf;
		
		vector<Selection> picked_stack;
		
		for (int i = 0; i < hits; i++)
		{
			GLuint num_names = ptr[0];
			GLuint min_depth = ptr[1];
			GLuint max_depth = ptr[2];
			
			GLuint *names = &ptr[3];
			
			Selection d;
			
			d.min_depth = min_depth;
			d.max_depth = max_depth;
			d.name_stack.insert(d.name_stack.begin(), names, (names + num_names));
			
			picked_stack.push_back(d);
			
			ptr += (3 + num_names);
		}
		
		sort(picked_stack.begin(), picked_stack.end(), sort_by_depth);
		
		return picked_stack;
	}
	
	ofVec3f getLocalPosition(int x, int y)
	{
		GLdouble ox, oy, oz;
		
		gluUnProject(x, y, current_depth,
					 modelview, projection, viewport,
					 &ox, &oy, &oz);
		
		oy = viewport[3] - oy;
		
		return ofVec2f(ox, oy);
	}

	
	// event callbacks
	
	void draw(ofEventArgs&)
	{
		ofxInteractivePrimitives::prepareModelViewMatrix();
		
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		ofPushStyle();
		
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *w = elements[i];
			if (!w->getVisible()) continue;
			
			w->transformGL();
			w->draw();
			w->restoreTransformGL();
		}
		
		ofPopStyle();
		glPopMatrix();
		glPopAttrib();
	}
	
	void update(ofEventArgs&)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		ofPushStyle();
		
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *w = elements[i];
			if (!w->getVisible()) continue;
			
			elements[i]->update();
		}
		
		ofPopStyle();
		glPopMatrix();
		glPopAttrib();
	}
	
	void mousePressed(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->clearState();
		}

		vector<Selection> p = pickup(e.x, e.y);
		
		if (!p.empty())
		{
			Selection &s = p[0];
			current_depth = (float)s.min_depth / 0xffffffff;
			
			for (int i = 0; i < s.name_stack.size(); i++)
			{
				ofxInteractivePrimitives *w = elements[s.name_stack.at(i)];
				ofVec3f p = getLocalPosition(e.x, e.y);
				p = w->getGlobalTransformMatrix().getInverse().preMult(p);
				
				w->mousePressed(p.x, p.y, e.button);
				
				w->hover = true;
				w->down = true;
				
				current_object = w;
				
				focusWillLost(focus_object);
				focus_object = w;
			}
		}
		else
		{
			current_object = NULL;
			
			focusWillLost(focus_object);
			focus_object = NULL;
		}
		
		if (focus_object)
			focus_object->focus = true;
	}
	
	void mouseReleased(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->clearState();
		}
		
		if (focus_object)
			focus_object->focus = true;

		vector<Selection> p = pickup(e.x, e.y);
		
		if (!p.empty())
		{
			Selection &s = p[0];
			current_depth = (float)s.min_depth / 0xffffffff;
			
			for (int i = 0; i < s.name_stack.size(); i++)
			{
				ofxInteractivePrimitives *w = elements[s.name_stack.at(i)];
				ofVec3f p = getLocalPosition(e.x, e.y);
				p = w->getGlobalTransformMatrix().getInverse().preMult(p);
				
				w->mouseReleased(p.x, p.y, e.button);
				w->hover = true;
			}
		}
		
		if (current_object)
		{
			current_object->down = false;
			current_object = NULL;
		}
	}
	
	void mouseMoved(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->clearState();
		}
		
		if (focus_object)
			focus_object->focus = true;

		vector<Selection> p = pickup(e.x, e.y);
		
		if (!p.empty())
		{
			Selection &s = p[0];
			current_depth = (float)s.min_depth / 0xffffffff;
			
			for (int i = 0; i < s.name_stack.size(); i++)
			{
				ofxInteractivePrimitives *w = elements[s.name_stack.at(i)];
				ofVec3f p = getLocalPosition(e.x, e.y);
				p = w->getGlobalTransformMatrix().getInverse().preMult(p);
				
				w->mouseMoved(p.x, p.y);
				w->hover = true;
			}
		}
		
		if (current_object)
		{
			current_object->down = false;
			current_object = NULL;
		}
	}
	
	void mouseDragged(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->clearState();
		}
		
		if (focus_object)
			focus_object->focus = true;

		if (current_object)
		{
			ofVec3f p = getLocalPosition(e.x, e.y);
			p = current_object->getGlobalTransformMatrix().getInverse().preMult(p);
			
			current_object->mouseDragged(p.x, p.y, e.button);
			current_object->hover = true;
		}
	}
	
	map<int, bool> current_focus_key;
	void keyPressed(ofKeyEventArgs &e)
	{
		if (focus_object)
		{
			current_focus_key[e.key] = true;
			focus_object->keyPressed(e.key);
		}
	}
	
	void keyReleased(ofKeyEventArgs &e)
	{
		if (focus_object)
		{
			current_focus_key[e.key] = false;
			focus_object->keyReleased(e.key);
		}
	}
	
	void focusWillLost(ofxInteractivePrimitives *p)
	{
		// cleanup keys
		
		map<int, bool>::iterator it = current_focus_key.begin();
		while (it != current_focus_key.end())
		{
			if (it->second)
			{
				p->keyReleased(it->first);
			}
			
			it++;
		}
		
		current_focus_key.clear();
	}
};

static ofxInteractivePrimitives::Context *_context = NULL;
static ofxInteractivePrimitives::Context& getContext()
{
	if (_context == NULL)
		_context = new ofxInteractivePrimitives::Context;
	return *_context;
}

ofxInteractivePrimitives::ofxInteractivePrimitives() : object_id(0), hover(false), down(false), visible(true), focus(false)
{
	getContext().registerElement(this);
}

ofxInteractivePrimitives::~ofxInteractivePrimitives()
{
	getContext().unregisterElement(this);
}

void ofxInteractivePrimitives::prepareModelViewMatrix()
{
	getContext().prepare();
}

ofVec2f ofxInteractivePrimitives::getMouseDelta()
{
	return ofVec2f(ofGetMouseX() - ofGetPreviousMouseX(), ofGetMouseY() - ofGetPreviousMouseY());
}

ofVec3f ofxInteractivePrimitives::localToGlobal(const ofVec3f& v)
{
	return global_matrix.preMult(v);
}

ofVec3f ofxInteractivePrimitives::globalToLocal(const ofVec3f& v)
{
	return global_matrix_inverse.preMult(v);
}

void ofxInteractivePrimitives::setParent(ofxInteractivePrimitives *o)
{
	ofNode::setParent(*o);
	o->children.push_back(this);
}

void ofxInteractivePrimitives::clearParent()
{
	ofxInteractivePrimitives *p = getParent();
	if (p)
	{
		vector<ofxInteractivePrimitives*> &p_children = p->children;
		vector<ofxInteractivePrimitives*>::iterator it;
		
		it = remove(p_children.begin(), p_children.end(), this);
		p_children.erase(it, p_children.end());
	}
	
	ofNode::clearParent();
}

void ofxInteractivePrimitives::clearState()
{
	hover = false;
	focus = false;
}

void ofxInteractivePrimitives::draw(const Internal &)
{
	static Internal intn;
	
	if (getVisible())
	{
		glPushMatrix();
		glMultMatrixf(getLocalTransformMatrix().getPtr());
		
		draw();
		
		for (int i = 0; i < children.size(); i++)
		{
			if (children[i]->getVisible())
				children[i]->draw(intn);
		}
		
		glPopMatrix();
	}
}

void ofxInteractivePrimitives::update(const Internal &)
{
	static Internal intn;
	
	if (getVisible())
	{
		global_matrix = getGlobalTransformMatrix();
		global_matrix_inverse = global_matrix.getInverse();
		
		update();
		
		for (int i = 0; i < children.size(); i++)
		{
			if (children[i]->getVisible())
				children[i]->update(intn);
		}
	}
}

void ofxInteractivePrimitivesRootNode::draw()
{
	ofxInteractivePrimitives::prepareModelViewMatrix();
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	ofPushStyle();
	
	static Internal intn;
	
	if (getVisible())
	{
		glPushMatrix();
		glMultMatrixf(getLocalTransformMatrix().getPtr());
		
		for (int i = 0; i < children.size(); i++)
		{
			if (children[i]->getVisible())
				children[i]->draw(intn);
		}
		
		glPopMatrix();
	}
	
	ofPopStyle();
	glPopMatrix();
	glPopAttrib();
}

void ofxInteractivePrimitivesRootNode::update()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	ofPushStyle();
	
	static Internal intn;
	
	if (getVisible())
	{
		for (int i = 0; i < children.size(); i++)
		{
			if (children[i]->getVisible())
				children[i]->update(intn);
		}
	}
	
	ofPopStyle();
	glPopMatrix();
	glPopAttrib();
}
