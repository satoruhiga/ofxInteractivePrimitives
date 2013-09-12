#pragma once

#include "ofxInteractivePrimitives.h"

#include "ofxCameraCalibUtils.h"

#define PROJECTOR_CALIBRATION_BEGIN_NAMESPACE \
namespace ofxInteractivePrimitives { \
namespace ProCamCalibration {

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
	
	Marker(int id, Node &parent) : id(id), need_update_calib(false), ofxInteractivePrimitives::Marker(parent) {}
	
	void draw();
	void update();
	
	void keyPressed(int key);
	
	ofVec3f getObjectPoint() const { return object_pos; }
	void setObjectPosition(const ofVec3f& v) { object_pos = v; }
	
	ofVec3f getImagePoint() const { return this->getPosition(); }
	void setImagePoint(const ofVec3f& v) { this->setPosition(v); }
	
protected:
	
	int id;
	ofVec3f object_pos;
	
	ofVec3f last_position;
	bool need_update_calib;
	
};

class Manager
{
public:
	
	void setup(size_t num_markers);
	
	void update();
	void draw();
	
	void load(string path);
	void save(string path);
	
	ofMatrix4x4 getHomography();
	void getEstimatedCameraPose(ofxCameraCalibUtils::CameraParam &params, int width, int height, float initial_fovY = 60);

	void setSelectedImagePoint(int x, int y);
	Marker* getSelectedMarker();
	
	Marker* operator[](size_t idx) const { return markers[idx].get(); }
	size_t size() const { return markers.size(); }
	
	bool getNeedUpdateCalibration() const;
	
protected:
	
	ofxInteractivePrimitives::RootNode root;
	vector<Marker::Ref> markers;
	
	void markUpdated();

};

PROJECTOR_CALIBRATION_END_NAMESPACE
