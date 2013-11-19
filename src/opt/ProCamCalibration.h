#pragma once

#include "ofxInteractivePrimitives.h"
#include "ofxCv.h"

#define PROJECTOR_CALIBRATION_BEGIN_NAMESPACE namespace ofx { namespace InteractivePrimitives { namespace ProCamCalibration {
#define PROJECTOR_CALIBRATION_END_NAMESPACE } } }

PROJECTOR_CALIBRATION_BEGIN_NAMESPACE

class Manager;

class CameraParam : public ofCamera
{
public:
	
	CameraParam() {}
	CameraParam(float width, float height, cv::Mat camera_matrix, cv::Mat rvec, cv::Mat tvec, float near = 10, float far = 10000) : camera_matrix(camera_matrix), width(width), height(height), near(near), far(far)
	{
		{
			double aspect, focal_length;
			cv::Point2d principal_point, fov;
			cv::Size image_size(width, height);
			
			cv::calibrationMatrixValues(camera_matrix,
										image_size,
										0,
										0,
										fov.x,
										fov.y,
										focal_length,
										principal_point,
										aspect);
			
			this->principal_point.set(principal_point.x, principal_point.y);
			this->focal_length = focal_length;
			this->fovX = fov.x;
			this->fovY = fov.y;
		}
		
		{
			cv::Mat rot3x3;
			if(rvec.rows == 3 && rvec.cols == 3) {
				rot3x3 = rvec;
			} else	{
				cv::Rodrigues(rvec, rot3x3);
			}
			
			const double* rm = rot3x3.ptr<double>(0);
			const double* tm = tvec.ptr<double>(0);
			
			modelview.makeIdentityMatrix();
			modelview.set(rm[0], rm[3], rm[6], 0,
					rm[1], rm[4], rm[7], 0,
					rm[2], rm[5], rm[8], 0,
					tm[0], tm[1], tm[2], 1);
			
			// convert coordinate system opencv to opengl
			modelview.postMultScale(1, -1, -1);
		}
		
		updateFrustum();
	}
	
	void begin(ofRectangle viewport = ofGetCurrentViewport())
	{
		ofPushView();
		
		ofViewport(viewport.x, viewport.y, viewport.width, viewport.height);
		
		ofSetMatrixMode(OF_MATRIX_PROJECTION);
		ofLoadMatrix(projection);
		
		ofSetMatrixMode(OF_MATRIX_MODELVIEW);
		ofLoadMatrix(modelview);
	}
	
	void end()
	{
		ofPopView();
	}
	
	const ofMatrix4x4& getModelview() const { return modelview; }
	const ofMatrix4x4& getProjection() const { return projection; }

protected:
	
	ofMatrix4x4 modelview;
	ofMatrix4x4 projection;
	
	cv::Mat camera_matrix;
	
	float fovX, fovY;
	float focal_length;
	ofVec2f principal_point;
	float width, height;
	
	float near, far;

	void updateFrustum()
	{
		const float w = width;
		const float h = height;
		const float fx = camera_matrix.at<double>(0, 0);
		const float fy = camera_matrix.at<double>(1, 1);
		const float s = camera_matrix.at<double>(0, 1);
		const float cx = camera_matrix.at<double>(0, 2);
		const float cy = camera_matrix.at<double>(1, 2);

		ofMatrix4x4 m;
		m.makeFrustumMatrix(near * (-cx) / fx,
							near * (w - cx) / fx,
							near * (cy - h) / fy,
							near * (cy) / fy,
							near,
							far);
		
		// flip Y-axis
		m.postMultScale(ofVec3f(1, -1, 1));
		
		projection = m;
	}
};

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
	void setObjectPoint(const ofVec3f& v) { object_pos = v; }
	
	ofVec3f getImagePoint() const { return this->getPosition(); }
	void setImagePoint(const ofVec3f& v) { this->setPosition(v); }
	
protected:
	
	int id;
	ofVec3f object_pos;
	
	ofVec3f last_position;
	bool need_update_calib;
	
};

class Manager : public RootNode
{
public:
	
	void setup(size_t num_markers);
	
	void draw();
	
	void load(string path);
	void save(string path);
	
	ofMatrix4x4 getHomography();
	
	float getEstimatedCameraPose(cv::Size image_size, cv::Mat& camera_matrix, cv::Mat& rvec, cv::Mat& tvec);

	float getEstimatedCameraPose(int width, int height, CameraParam &param,  float near = 10, float far = 10000);

	void setSelectedImagePoint(int x, int y);
	Marker* getSelectedMarker();
	
	Marker* operator[](size_t idx) const { return markers[idx].get(); }
	size_t size() const { return markers.size(); }
	
	bool getNeedUpdateCalibration() const;
	
protected:
	
	vector<Marker::Ref> markers;
	
	void markUpdated();

};

PROJECTOR_CALIBRATION_END_NAMESPACE
