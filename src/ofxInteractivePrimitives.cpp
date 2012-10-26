#include "ofxInteractivePrimitives.h"

class ofxInteractivePrimitives::Context
{
public:
	
	vector<ofxInteractivePrimitives*> elements;
	
	GLint viewport[4];
	GLdouble projection[16], modelview[16];
	
	unsigned long current_object_id;
	float current_depth;
	
	ofxInteractivePrimitives *current_responder;

	Context() : current_object_id(0), current_depth(0), current_responder(NULL)
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
		ofAddListener(ofEvents().update, this, &ofxInteractivePrimitives::Context::update);
		ofAddListener(ofEvents().draw, this, &ofxInteractivePrimitives::Context::draw);
		
		ofAddListener(ofEvents().mousePressed, this, &ofxInteractivePrimitives::Context::mousePressed);
		ofAddListener(ofEvents().mouseReleased, this, &ofxInteractivePrimitives::Context::mouseReleased);
		ofAddListener(ofEvents().mouseMoved, this, &ofxInteractivePrimitives::Context::mouseMoved);
		ofAddListener(ofEvents().mouseDragged, this, &ofxInteractivePrimitives::Context::mouseDragged);
	}
	
	void disableAllEvent()
	{
		ofRemoveListener(ofEvents().update, this, &ofxInteractivePrimitives::Context::update);
		ofRemoveListener(ofEvents().draw, this, &ofxInteractivePrimitives::Context::draw);
		
		ofRemoveListener(ofEvents().mousePressed, this, &ofxInteractivePrimitives::Context::mousePressed);
		ofRemoveListener(ofEvents().mouseReleased, this, &ofxInteractivePrimitives::Context::mouseReleased);
		ofRemoveListener(ofEvents().mouseMoved, this, &ofxInteractivePrimitives::Context::mouseMoved);
		ofRemoveListener(ofEvents().mouseDragged, this, &ofxInteractivePrimitives::Context::mouseDragged);
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
			e->hover = false;
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
				
				current_responder = w;
			}
		}
		
		if (p.empty()) current_responder = NULL;
	}
	
	void mouseReleased(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->hover = false;
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
				
				w->mouseReleased(p.x, p.y, e.button);
				w->hover = true;
			}
		}
		
		if (current_responder)
		{
			current_responder->down = false;
			current_responder = NULL;
		}
	}
	
	void mouseMoved(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->hover = false;
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
				
				w->mouseMoved(p.x, p.y);
				w->hover = true;
			}
		}
		
		if (current_responder)
		{
			current_responder->down = false;
			current_responder = NULL;
		}
	}
	
	void mouseDragged(ofMouseEventArgs &e)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			ofxInteractivePrimitives *e = elements[i];
			e->hover = false;
		}
		
		bool forcus_is_out = true;
		
		if (!current_responder)
		{
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
					
					w->mouseDragged(p.x, p.y, e.button);
					w->hover = true;
					
					if (w == current_responder) forcus_is_out = false;
				}
			}
		}
		
		if (forcus_is_out && current_responder)
		{
			ofVec3f p = getLocalPosition(e.x, e.y);
			p = current_responder->getGlobalTransformMatrix().getInverse().preMult(p);
			
			current_responder->mouseDragged(p.x, p.y, e.button);
			current_responder->hover = true;
		}
	}
};

static ofxInteractivePrimitives::Context *_context = NULL;
static ofxInteractivePrimitives::Context& getContext()
{
	if (_context == NULL)
		_context = new ofxInteractivePrimitives::Context;
	return *_context;
}

ofxInteractivePrimitives::ofxInteractivePrimitives() : object_id(0), hover(false), down(false), visible(true)
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