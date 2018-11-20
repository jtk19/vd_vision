#ifndef __VEED_JSON_WRITER_H__
#define __VEED_JSON_WRITER_H__

#include <fstream>
#include <config.h>


using namespace std;

class json_writer
{
public:
	json_writer( string filename );
	~json_writer();

	void write();

	void setFile( string subdir, string filename, string ext )
	{
		_subdir = subdir;
		_filename = filename;
		_extension = ext;
	}

	void setCrop( int left, int right, int top, int bottom )
	{
		_crop_left = left;
		_crop_right = right;
		_crop_top = top;
		_crop_bottom = bottom;
	}

	void setOrientation( bool orient90_detected, bool opflow_detected, Veed_Orient_T orient )
	{
		orient90_orient_detected = orient90_detected;
		opticalflow_orient_detected = opflow_detected;
		_orient = orient;
	}

	void setOpflowCentre( int x, int y)
	{
		_opflow_centre_x = x;
		_opflow_centre_y = y;
	}

	void setFaceDetection( bool detected, int face_top, int chin, int shoulders, int hips )
	{
		_face_detected = detected;
		_face_top = face_top;
		_chin = chin;
		_shoulders = shoulders;
		_hips = hips;
	}

private:

	ofstream jfs;

	string _subdir;
	string _filename, _extension;

	int _crop_left, _crop_right, _crop_top, _crop_bottom;

	bool orient90_orient_detected;
	bool opticalflow_orient_detected;
	Veed_Orient_T _orient;

	int _opflow_centre_x, _opflow_centre_y;

	bool _face_detected;
	int _face_top, _chin, _shoulders, _hips;

	void writeFileInfoNode();
	void writeCropNode();
	void writeOrientationNode();
	void writeFlowCentreNode();
	void writeFaceDetectionNode();

};

#endif
