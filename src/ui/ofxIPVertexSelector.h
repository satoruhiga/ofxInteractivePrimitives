#pragma once

#include "ofxInteractivePrimitives.h"

#include "Poco/Util/XMLConfiguration.h"

OFX_INTERACTIVE_PRIMITIVES_START_NAMESPACE

class VertexSelector : public Node
{
public:
	
	VertexSelector(Node &root)
	{
		setParent(&root);
	}
	
	void setup(ofMesh *mesh, int num_control_point)
	{
		this->mesh = mesh;
		
		indexes.resize(num_control_point);
		focus_index = 0;
	}
	
	void save(const string& path_)
	{
		string path = ofToDataPath(path_);
		
		Poco::AutoPtr<Poco::Util::XMLConfiguration> config = new Poco::Util::XMLConfiguration;
		config->loadEmpty("verts");
		
		for (int i = 0; i < indexes.size(); i++)
		{
			string t = "vert[" + ofToString(i) + "]";
			config->setInt(t + "[@index]", i);
			config->setInt(t + "[@vertex]", indexes[i]);
		}
		
		config->save(path);
	}
	
	bool load(const string& path_)
	{
		string path = ofToDataPath(path_);
		if (!ofFile::doesFileExist(path)) return false;
		
		Poco::AutoPtr<Poco::Util::XMLConfiguration> config = new Poco::Util::XMLConfiguration;
		config->loadEmpty("verts");
		config->load(ofToDataPath(path));
		
		vector<string> keys;
		config->keys(keys);
		
		indexes.resize(keys.size());
		
		for (int i = 0; i < keys.size(); i++)
		{
			string t = "vert[" + ofToString(i) + "]";
			int idx = config->getInt(t + "[@index]");
			int vert = config->getInt(t + "[@vertex]");
			
			indexes[idx] = vert;
		}
		
		return true;
	}
	
	// focus
	void changeFocus(int index)
	{
		assert(index >= 0 && index < indexes.size());
		focus_index = index;
	}
	
	void focusNext()
	{
		changeFocus((focus_index + 1) % indexes.size());
	}
	
	void focusPrev()
	{
		int index = focus_index - 1;
		if (index < 0) index = indexes.size() - 1;
		changeFocus(index);
	}
	
	//
	
	size_t getNumIndex() const { return indexes.size(); }
	ofVec3f getVertexPosition(size_t index) const
	{
		assert(mesh);
		assert(indexes[index] < mesh->getNumVertices());
		return mesh->getVertex(indexes[index]);
	}
	
public:
	
	void draw()
	{
		ofPushStyle();
		ofEnableAlphaBlending();
		
		ofNoFill();
		
		ofSetColor(255, 64);
		mesh->drawWireframe();
		
		for (int i = 0; i < indexes.size(); i++)
		{
			if (focus_index == i)
				ofSetColor(255, 0, 0);
			else
				ofSetColor(255);
			
			int idx = indexes[i];
			ofVec3f p = mesh->getVertex(idx);
			ofDrawBox(p, 10);
			
			stringstream ss;
			ss << "  " << i << "->" << idx << "\n  (" << p << ")";
			ofDrawBitmapString(ss.str(), p);
		}
		
		ofSetColor(255);
		
		if (isHover())
		{
			ofVec3f p = mesh->getVertex(current_vertex_index);
			ofPushMatrix();
			ofTranslate(p);
			ofRotateY(ofGetElapsedTimef() * 720);
			ofCircle(0, 0, 10);
			ofPopMatrix();
		}
		
		ofPopStyle();
	}
	
	void hittest()
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPointSize(40);
		
		for (int i = 0; i < mesh->getNumVertices(); i++)
		{
			ofVec3f p = mesh->getVertex(i);
			
			glPushName(i);
			
			glBegin(GL_POINTS);
			glVertex3fv(p.getPtr());
			glEnd();
			
			glPopName();
		}
		
		glPopAttrib();
	}
	
	void mousePressed(int x, int y, int button)
	{
		Node::mousePressed(x, y, button);
		
		const vector<GLuint>& names = getCurrentNameStack();
		if (names.size())
		{
			indexes[focus_index] = names[0];
		}
	}
	
	void mouseMoved(int x, int y)
	{
		Node::mouseMoved(x, y);
		
		const vector<GLuint>& names = getCurrentNameStack();
		if (names.size()) current_vertex_index = names[0];
	}
	
protected:
	
	ofMesh *mesh;
	vector<int> indexes;
	
	int current_vertex_index;
	int focus_index;
};

OFX_INTERACTIVE_PRIMITIVES_END_NAMESPACE