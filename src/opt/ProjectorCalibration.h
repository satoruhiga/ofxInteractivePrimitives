#pragma once

#include "ofxInteractivePrimitives.h"

#include "ofxCameraCalibUtils.h"

#define PROJECTOR_CALIBRATION_BEGIN_NAMESPACE \
namespace ofxInteractivePrimitives { \
namespace ProjectorCalibration {

#define PROJECTOR_CALIBRATION_END_NAMESPACE \
} \
}

PROJECTOR_CALIBRATION_BEGIN_NAMESPACE

class Manager;

class Marker : public ofxInteractivePrimitives::Marker
{
	friend class Manager;
	
public:

	typedef ofPtr<Marker> Ref;
	
	Marker(int id, Node &parent) : id(id), ofxInteractivePrimitives::Marker(parent) {}
	
	void draw();
	void update();
	
	void keyPressed(int key);
	
	ofVec3f& getObjectPoint() { return object_pos; }
	const ofVec3f& getObjectPoint() const { return object_pos; }
	
	
protected:
	
	int id;
	ofVec3f object_pos;
};

class Manager
{
public:
	
	void setup(int num_markers);
	
	void update();
	void draw();
	
	void load(string path);
	void save(string path);
	
	ofMatrix4x4 getHomography();
	bool getEstimatedCameraPose(ofxCameraCalibUtils::CameraParam &params, int width, int height, float initial_fovY = 60);
	
	void setImagePoint(int x, int y);

	Marker* getSelectedMarker();
	
	Marker* operator[](size_t idx) const { return markers[idx]; }
	size_t size() const { return markers.size(); }
	
	Marker* addMarker();
	void removeMarker(Marker* marker);
	void clear();
	
protected:
	
	ofxInteractivePrimitives::RootNode root;
	vector<Marker*> markers;

};

PROJECTOR_CALIBRATION_END_NAMESPACE
