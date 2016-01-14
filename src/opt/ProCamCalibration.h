#pragma once

#ifdef USE_PROCAM_CALIBRATION

#include "ofxInteractivePrimitives.h"
#include "ofxCv.h"
#include "Poco/Util/XMLConfiguration.h"

#define PROJECTOR_CALIBRATION_BEGIN_NAMESPACE namespace ofx { namespace InteractivePrimitives { namespace ProCamCalibration {
#define PROJECTOR_CALIBRATION_END_NAMESPACE } } }

PROJECTOR_CALIBRATION_BEGIN_NAMESPACE

class Manager;

class CameraParam
{
public:
	
	CameraParam() {}
	CameraParam(float width, float height, cv::Mat camera_matrix, cv::Mat rvec, cv::Mat tvec, float near_dist = 10, float far_dist = 10000) : camera_matrix(camera_matrix), width(width), height(height), near_dist(near_dist), far_dist(far_dist)
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

	bool load(const string& path)
	{
		ofFile file(path);
		if (!file.exists()) return false;
		
		string tmp;
		
		file >> tmp;
		file >> projection;
		file >> tmp;
		file >> modelview;
		
		return true;
	}

	void save(const string& path)
	{
		ofFile file(path, ofFile::WriteOnly);
		
		file << "#projection" << endl;
		file << projection;
		file << endl;
		
		file << "#modelview" << endl;
		file << modelview;
		file << endl;
		
		file.close();
	}

	
protected:
	
	ofMatrix4x4 modelview;
	ofMatrix4x4 projection;
	
	cv::Mat camera_matrix;
	
	float fovX, fovY;
	float focal_length;
	ofVec2f principal_point;
	float width, height;
	
	float near_dist, far_dist;

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
		m.makeFrustumMatrix(near_dist * (-cx) / fx,
							near_dist * (w - cx) / fx,
							near_dist * (cy - h) / fy,
							near_dist * (cy) / fy,
							near_dist,
							far_dist);
		
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

	Marker(Node &parent)
		: need_update_calib(false)
		, ofxInteractivePrimitives::Marker(parent) {}

	Marker(const string& marker_label, Node &parent)
		: marker_label(marker_label)
		, need_update_calib(false)
		, ofxInteractivePrimitives::Marker(parent) {}
	
	void draw()
	{
		ofxInteractivePrimitives::Marker::draw();
		
		if (hasFocus())
		{
			ofNoFill();
			ofDrawRectangle(-15, -15, 30, 30);
		}
	}

	void update()
	{
		stringstream ss;
		if (!marker_label.empty()) ss << marker_label << endl;
		ss << getX() << ":" << getY() << endl;
		ss << (int)object_pos.x << ":" << (int)object_pos.y << ":"
		<< (int)object_pos.z;
		
		text = ss.str();
		
		if (last_position != getPosition())
		{
			last_position = getPosition();
			need_update_calib = true;
		}
	}

	
	void keyPressed(int key)
	{
		if (key == OF_KEY_LEFT)
			move(-1, 0, 0);
		else if (key == OF_KEY_RIGHT)
			move(1, 0, 0);
		else if (key == OF_KEY_UP)
			move(0, -1, 0);
		else if (key == OF_KEY_DOWN)
			move(0, 1, 0);
	}

	
	ofVec3f getObjectPoint() const { return object_pos; }
	void setObjectPoint(const ofVec3f& v) { object_pos = v; }
	
	ofVec3f getImagePoint() const { return this->getPosition(); }
	void setImagePoint(const ofVec3f& v) { this->setPosition(v); }
	
	void setLabel(const string& label) { marker_label = label; }
	const string& getLabel() const { return marker_label; }
	
protected:
	
	ofVec3f object_pos;
	
	ofVec3f last_position;
	bool need_update_calib;
	
	string marker_label;
};

class Manager : public RootNode
{
public:
	
	void setup(size_t num_markers = 0)
	{
		clearChildren();
		
		markers.resize(num_markers);
		
		for (int i = 0; i < num_markers; i++)
		{
			markers[i] = Marker::Ref(new Marker(*this));
		}
	}

	void draw()
	{
		ofPushStyle();
		
		if (getFocusObject())
		{
			ofPushStyle();
			
			ofSetLineWidth(3);
			
			ofSetColor(255, 0, 0);
			
			ofVec2f p = getFocusObject()->getPosition();
			ofNoFill();
			ofDrawCircle(p, 40);
			
			ofDrawCircle(p, 10);
			
			ofDrawLine(-10000, p.y, 10000, p.y);
			ofDrawLine(p.x, -10000, p.x, 10000);
			
			ofPopStyle();
		}
		
		RootNode::draw();
		
		ofPopStyle();
	}

	
	ofMatrix4x4 getHomography()
	{
		assert(markers.size() >= 4);
		
		markUpdated();
		
		using namespace cv;
		using namespace ofxCv;
		
		vector<Point2f> srcPoints, dstPoints;
		for (int i = 0; i < markers.size(); i++)
		{
			Marker* o = markers[i].get();
			
			dstPoints.push_back(Point2f(o->getX(), o->getY()));
			srcPoints.push_back(Point2f(o->object_pos.x, o->object_pos.y));
		}
		
		Mat homography = findHomography(Mat(srcPoints), Mat(dstPoints));
		return homography2glModelViewMatrix(homography);
	}

	
	float getEstimatedCameraPose(cv::Size image_size, cv::Mat& camera_matrix, cv::Mat& rvec, cv::Mat& tvec, float force_fov = 0)
	{
		if (markers.size() <= 6) return -1;
		
		markUpdated();
		
		using namespace ofxCv;
		using namespace cv;
		
		vector<cv::Point3f> object_points;
		vector<cv::Point2f> image_points;
		
		for (int i = 0; i < markers.size(); i++)
		{
			Marker* o = markers[i].get();
			object_points.push_back(toCv(o->object_pos));
			image_points.push_back(toCv((ofVec2f)o->getPosition()));
		}
		
		cv::Mat dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);
		
		float fov = 60;
		if (force_fov != 0) fov = force_fov;
		
		float f = (image_size.height / 2) * tan(ofDegToRad(90 - fov / 2));
		camera_matrix = (cv::Mat_<double>(3, 3) << f, 0, image_size.width / 2, 0, f,
						 image_size.height / 2, 0, 0, 1);
		
		float rms = 0;
		
		if (force_fov == 0)
		{
			vector<cv::Mat> rvecs;
			vector<cv::Mat> tvecs;
			
			vector<vector<cv::Point3f> > object_points_arr(1);
			vector<vector<cv::Point2f> > image_points_arr(1);
			
			object_points_arr[0] = object_points;
			image_points_arr[0] = image_points;
			
			int flags = 0;
			flags |= CV_CALIB_USE_INTRINSIC_GUESS;
			
			flags |= CV_CALIB_FIX_ASPECT_RATIO;
			flags |= CV_CALIB_ZERO_TANGENT_DIST;
			flags |= (CV_CALIB_FIX_K1 | CV_CALIB_FIX_K2 | CV_CALIB_FIX_K3 |
					  CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5 | CV_CALIB_FIX_K6 |
					  CV_CALIB_RATIONAL_MODEL);
			
			// flags |= CV_CALIB_FIX_PRINCIPAL_POINT;
			
			rms = cv::calibrateCamera(object_points_arr, image_points_arr, image_size,
									  camera_matrix, dist_coeffs, rvecs, tvecs,
									  flags);
			
			rvec = rvecs[0];
			tvec = tvecs[0];
		}
		else
		{
			cv::Mat rvecs;
			cv::Mat tvecs;
			
			cv::solvePnP(object_points, image_points, camera_matrix, dist_coeffs, rvecs, tvecs);
			rms = 1;
			
			rvec = rvecs;
			tvec = tvecs;
		}
		
		
		return rms;
	}

	float getEstimatedCameraPose(int width, int height, CameraParam &param,  float near_dist = 10, float far_dist = 10000, float force_fov = 0)
	{
		cv::Mat camera_matrix, rvec, tvec;
		cv::Size image_size(width, height);
		
		float rms = getEstimatedCameraPose(image_size, camera_matrix, rvec, tvec,
										   force_fov);
		
		if (rms > 0)
		{
			param =
			CameraParam(width, height, camera_matrix, rvec, tvec, near_dist, far_dist);
		}
		
		return rms;
	}

	void setSelectedImagePoint(int x, int y)
	{
		Marker* m = (Marker*)getFocusObject();
		if (m) m->object_pos.set(x, y, 0);
	}

	Marker* getSelectedMarker() { return (Marker*)getFocusObject(); }
	
	Marker::Ref operator[](size_t idx) const { return markers[idx]; }
	size_t size() const { return markers.size(); }
	
	Marker::Ref addMarker(const string& marker_label = "")
	{
		Marker* o = new Marker(marker_label, *this);
		Marker::Ref ref = Marker::Ref(o);
		markers.push_back(ref);
		return ref;
	}

	void removeMarker(Marker::Ref o)
	{
		vector<Marker::Ref>::iterator it = markers.begin();
		while (it != markers.end())
		{
			if (o == *it)
			{
				(*it)->dispose();
				it = markers.erase(it);
			}
			else
				it++;
		}
	}

	void clear()
	{
		clearChildren();
		markers.clear();
	}

	
	bool getNeedUpdateCalibration() const
	{
		for (int i = 0; i < markers.size(); i++)
			if (markers[i]->need_update_calib) return true;
		
		return false;
	}
	
	bool load(const string& path)
	{
		if (!ofFile::doesFileExist(path)) return false;
		
		Poco::AutoPtr<Poco::Util::XMLConfiguration> config =
		new Poco::Util::XMLConfiguration;
		config->loadEmpty("markers");
		config->load(ofToDataPath(path));
		
		vector<string> keys;
		config->keys(keys);
		
		clear();
		
		for (int i = 0; i < keys.size(); i++)
		{
			Marker::Ref o = addMarker();
			string m = keys[i];
			
			o->setPosition(config->getDouble(m + ".image[@x]", 0),
						   config->getDouble(m + ".image[@y]", 0), 0);
			
			o->object_pos.set(config->getDouble(m + ".object[@x]", 0),
							  config->getDouble(m + ".object[@y]", 0),
							  config->getDouble(m + ".object[@z]", 0));
			
			o->setLabel(config->getString(m + ".label", ""));
		}
		
		return true;
	}

	void save(string path)
	{
		Poco::AutoPtr<Poco::Util::XMLConfiguration> config =
		new Poco::Util::XMLConfiguration;
		config->loadEmpty("markers");
		
		for (int i = 0; i < markers.size(); i++)
		{
			Marker::Ref o = (*this)[i];
			
			string m = "marker[" + ofToString(i) + "]";
			
			config->setDouble(m + ".image[@x]", o->getX());
			config->setDouble(m + ".image[@y]", o->getY());
			
			config->setDouble(m + ".object[@x]", o->object_pos.x);
			config->setDouble(m + ".object[@y]", o->object_pos.y);
			config->setDouble(m + ".object[@z]", o->object_pos.z);
			
			config->setString(m + ".label", o->getLabel());
		}
		
		config->save(ofToDataPath(path));
	}

	
protected:
	
	vector<Marker::Ref> markers;
	
	void markUpdated()
	{
		for (int i = 0; i < markers.size(); i++)
		{
			if (markers[i]->need_update_calib)
				markers[i]->need_update_calib = false;
		}
	}

	static ofMatrix4x4 homography2glModelViewMatrix(const cv::Mat& homography)
	{
		ofMatrix4x4 matrix;
		
		matrix(0, 0) = homography.at<double>(0, 0);
		matrix(0, 1) = homography.at<double>(1, 0);
		matrix(0, 2) = 0;
		matrix(0, 3) = homography.at<double>(2, 0);
		
		matrix(1, 0) = homography.at<double>(0, 1);
		matrix(1, 1) = homography.at<double>(1, 1);
		matrix(1, 2) = 0;
		matrix(1, 3) = homography.at<double>(2, 1);
		
		matrix(2, 0) = 0;
		matrix(2, 1) = 0;
		matrix(2, 2) = 1;
		matrix(2, 3) = 0;
		
		matrix(3, 0) = homography.at<double>(0, 2);
		matrix(3, 1) = homography.at<double>(1, 2);
		matrix(3, 2) = 0;
		matrix(3, 3) = 1;
		
		return matrix;
	}

};

PROJECTOR_CALIBRATION_END_NAMESPACE

#endif // USE_PROCAM_CALIBRATION