#include "ProjectorCalibration.h"

#include "Poco/Util/XMLConfiguration.h"

#include "ofxCv.h"

PROJECTOR_CALIBRATION_BEGIN_NAMESPACE

static ofMatrix4x4 homography2glModelViewMatrix(const cv::Mat &homography);

#pragma mark - CalibrationMarker

void Marker::draw()
{
	ofxInteractivePrimitives::Marker::draw();
	
	if (hasFocus())
	{
		ofNoFill();
		ofRect(-15, -15, 30, 30);
	}
}

void Marker::update()
{
	stringstream ss;
	ss << id << endl;
	ss << getX() << ":" << getY() << endl;
	ss << (int)object_pos.x << ":" << (int)object_pos.y << ":" << (int)object_pos.z;
	
	text = ss.str();
	
	if (last_position != getPosition())
	{
		last_position = getPosition();
		need_update_calib = true;
	}
	
	// cout << need_update_calib << endl;
}

void Marker::keyPressed(int key)
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

#pragma mark - Manager

void Manager::setup(size_t num_markers)
{
	markers.resize(num_markers);
	
	for (int i = 0; i < num_markers; i++)
	{
		markers[i] = Marker::Ref(new Marker(i, root));
	}
}

ofMatrix4x4 Manager::getHomography()
{
	assert(markers.size() >= 4);
	
	markUpdated();
	
	using namespace cv;
	using namespace ofxCv;
	
	vector<Point2f> srcPoints, dstPoints;
	for(int i = 0; i < markers.size(); i++)
	{
		Marker* o = markers[i].get();
		
		dstPoints.push_back(Point2f(o->getX(), o->getY()));
		srcPoints.push_back(Point2f(o->object_pos.x, o->object_pos.y));
	}
	
	Mat homography = findHomography(Mat(srcPoints), Mat(dstPoints));
	return homography2glModelViewMatrix(homography);
}

void Manager::getEstimatedCameraPose(ofxCameraCalibUtils::CameraParam &params, int width, int height, float initial_fovY)
{
	assert(markers.size() >= 6);
	
	markUpdated();
	
	using namespace ofxCv;
	using namespace cv;
	
	vector< vector<cv::Point3f> > object_points(1);
	vector< vector<cv::Point2f> > image_points(1);
	
	for(int i = 0; i < markers.size(); i++)
	{
		Marker* o = markers[i].get();
		object_points[0].push_back(toCv(o->object_pos));
		image_points[0].push_back(toCv((ofVec2f)o->getPosition()));
	}
	
	cv::Mat dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);
	cv::Size image_size(width, height);
	
	float f = (image_size.height / 2) * tan(ofDegToRad(90 - initial_fovY / 2));
	cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) <<
					 f, 0, width / 2,
					 0, f, height / 2,
					 0, 0, 1);
	
	vector<cv::Mat> rvecs;
	vector<cv::Mat> tvecs;
	
	int flags = 0;
	flags |= CV_CALIB_USE_INTRINSIC_GUESS;
	flags |= CV_CALIB_FIX_PRINCIPAL_POINT | CV_CALIB_FIX_ASPECT_RATIO;
	flags |= CV_CALIB_ZERO_TANGENT_DIST;
	flags |= (CV_CALIB_FIX_K1 | CV_CALIB_FIX_K2 | CV_CALIB_FIX_K3 | CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5 | CV_CALIB_FIX_K6 | CV_CALIB_RATIONAL_MODEL);
	
	float rms = cv::calibrateCamera(object_points,
									image_points,
									image_size,
									camera_matrix,
									dist_coeffs,
									rvecs,
									tvecs,
									flags);

	cout << "rms: " << rms << endl;
	
	{
		cv::Point2d principal_point, fov;
		double aspect, focal_length;

		cv::calibrationMatrixValues(camera_matrix,
									image_size,
									0,
									0,
									fov.x,
									fov.y,
									focal_length,
									principal_point,
									aspect);
		
		cout << "fov: " << fov << endl;
	}
	
	params = ofxCameraCalibUtils::CameraParam(ofxCameraCalibUtils::CameraParam::Intrinsics(camera_matrix, image_size),
											  ofxCameraCalibUtils::CameraParam::Extrinsic(rvecs[0], tvecs[0]));
}

void Manager::update()
{
	root.update();
}

void Manager::draw()
{
	ofPushStyle();
	
	if (root.getFocusObject())
	{
		ofPushStyle();
		
		ofSetLineWidth(3);
		
		ofSetColor(255, 0, 0);
		
		ofVec2f p = root.getFocusObject()->getPosition();
		ofNoFill();
		ofCircle(p, 40);
		
		ofCircle(p, 10);
		
		ofLine(-10000, p.y, 10000, p.y);
		ofLine(p.x, -10000, p.x, 10000);
		
		ofPopStyle();
	}

	root.draw();
	
	ofPopStyle();
}

void Manager::setSelectedImagePoint(int x, int y)
{
	Marker *m = (Marker*)root.getFocusObject();
	if (m) m->object_pos.set(x, y, 0);
}

Marker* Manager::getSelectedMarker()
{
	return (Marker*)root.getFocusObject();
}

bool Manager::getNeedUpdateCalibration() const
{
	for (int i = 0; i < markers.size(); i++)
		if (markers[i]->need_update_calib)
			return true;
	
	return false;
}

void Manager::markUpdated()
{
	for (int i = 0; i < markers.size(); i++)
	{
		if (markers[i]->need_update_calib)
			markers[i]->need_update_calib = false;
	}
}

// IO

void Manager::load(string path)
{
	if (!ofFile::doesFileExist(path)) return;
	
	Poco::AutoPtr<Poco::Util::XMLConfiguration> config = new Poco::Util::XMLConfiguration;
	config->loadEmpty("markers");
	config->load(ofToDataPath(path));
	
	vector<string> keys;
	config->keys(keys);

	assert(keys.size() == markers.size());
	
	for (int i = 0; i < keys.size(); i++)
	{
		Marker* o = (*this)[i];
		string m = keys[i];
		
		o->setPosition(config->getDouble(m + ".image[@x]", 0),
					   config->getDouble(m + ".image[@y]", 0),
					   0);
		
		o->object_pos.set(config->getDouble(m + ".object[@x]", 0),
						  config->getDouble(m + ".object[@y]", 0),
						  config->getDouble(m + ".object[@z]", 0));
	}
}

void Manager::save(string path)
{
	Poco::AutoPtr<Poco::Util::XMLConfiguration> config = new Poco::Util::XMLConfiguration;
	config->loadEmpty("markers");
	
	for (int i = 0; i < markers.size(); i++)
	{
		Marker* o = (*this)[i];
		
		string m = "marker[" + ofToString(i) + "]";
		
		config->setDouble(m + ".image[@x]", o->getX());
		config->setDouble(m + ".image[@y]", o->getY());
		
		config->setDouble(m + ".object[@x]", o->object_pos.x);
		config->setDouble(m + ".object[@y]", o->object_pos.y);
		config->setDouble(m + ".object[@z]", o->object_pos.z);
	}
	
	config->save(ofToDataPath(path));
}

// ====

static ofMatrix4x4 homography2glModelViewMatrix(const cv::Mat &homography)
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

PROJECTOR_CALIBRATION_END_NAMESPACE
