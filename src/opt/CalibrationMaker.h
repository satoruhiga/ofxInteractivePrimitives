#pragma once

#include "ofxInteractivePrimitives.h"

class CalibrationMaker : public ofxInteractivePrimitives::Marker
{
public:

	typedef ofPtr<CalibrationMaker> Ref;
	
	class Manager;
	
	CalibrationMaker(int id, Node &parent) : id(id), Marker(parent) {}
	
	void draw();
	void update();
	
	void keyPressed(int key);
	
protected:
	
	int id;
	ofVec3f object_pos;
};

class CalibrationMaker::Manager
{
public:
	
	void setup(int num_markers);
	
	void update();
	void draw();
	
	void load(string path);
	void save(string path);
	
	ofMatrix4x4 getHomography();
	pair<ofMatrix4x4, ofMatrix4x4> getEstimatedCameraPose(int width, int height, float fov);
	
	void setImagePoint(int x, int y, int z = 0);
	
protected:
	
	ofxInteractivePrimitives::RootNode root;
	vector<CalibrationMaker::Ref> markers;

};