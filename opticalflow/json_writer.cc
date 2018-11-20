#include <sstream>
#include <stdexcept>

#include "json_writer.h"


using namespace std;


json_writer::json_writer( string filename )
{
	stringstream ss;
	ss << VEED_WRITE_DIR<< "/" << filename<< ".json";

	jfs.open( ss.str(), ios_base::out );
	if ( !jfs.good() )
	{
		string msg;
		msg = string( "[json_writer] failed to open the file for writing :" ) + ss.str();
		cerr<< msg << endl;
		throw invalid_argument( msg );
	}
}


json_writer::~json_writer()
{
	jfs.close();
}

void json_writer::write()
{
	jfs << "{ \"veed_ml_markup\": {" << endl;

	writeFileInfoNode();
	jfs << ","<< endl;

	writeCropNode();
	jfs << ","<< endl;

	writeOrientationNode();
	jfs << ","<< endl;

	writeFlowCentreNode();
	jfs << ","<< endl;

	writeFaceDetectionNode();
	jfs << endl;

	jfs << "}}"<< endl;
}

void json_writer::writeFileInfoNode()
{
	jfs << "      \"fileinfo\": {"<< endl;
	jfs << "           \"readdir\": \""<< VEED_VIDEO_DATA_DIR<< "/\","<< endl;
	jfs << "           \"subdir\": \""<< _subdir<< "/\","<< endl;
	jfs << "           \"input_video\": \""<< _filename<< _extension<< "\","<< endl;
	jfs << "           \"writedir\": \""<< VEED_WRITE_DIR<< "/\","<< endl;
	jfs << "           \"edited_video\": \""<< _subdir<< "_"
						<< _filename<< _extension<< "\","<< endl;
	jfs << "           \"markup_video\": \""<< _subdir<< "_"
							<< _filename<< "_marked"<< _extension<< "\","<< endl;
	jfs << "           \"markup_json\": \""<< _subdir<< "_"
						<< _filename<< ".json\","<< endl;
	jfs << "           \"detectdir\": \""<< VEED_DETECT_DIR<< "/\","<< endl;
	jfs << "           \"orient90_result\": \""<< _subdir<< "_"
							<< _filename<< ".det\""<< endl;
	jfs << "      }";
}

void json_writer::writeCropNode()
{
	jfs << "      \"crop\": {"<< endl;
	jfs << "          \"left_x\": "<< _crop_left<< ","<< endl;
	jfs << "          \"right_x\": "<< _crop_left<< ","<< endl;
	jfs << "          \"top_y\": "<< _crop_left<< ","<< endl;
	jfs << "          \"bottom_y\": "<< _crop_left<< ""<< endl;
	jfs << "      }";
}

void json_writer::writeOrientationNode()
{

	jfs << "      \"orientation\": {"<< endl;
	jfs << "          \"orient90_detected\": "<< (orient90_orient_detected ? 1 : 0) << ","<< endl;
	jfs << "          \"opticalflow_detected\": "<< (opticalflow_orient_detected ? 1 : 0)<< ","<< endl;
	jfs << "          \"orientation\": "<< (int)_orient<< ""<< endl;
	jfs << "      }";
}

void json_writer::writeFlowCentreNode()
{
	jfs << "      \"opticalflow_centre\": {"<< endl;
	jfs << "          \"centre_x\": "<< _opflow_centre_x<< ","<< endl;
	jfs << "          \"centre_y\": "<< _opflow_centre_y<< ""<< endl;
	jfs << "      }";
}

void json_writer::writeFaceDetectionNode()
{
	jfs << "      \"face_detection\": {"<< endl;
	jfs << "          \"face_detected\": "<< (_face_detected ? 1 : 0 )<< ","<< endl;
	jfs << "          \"face_top_y\": "<< _face_top<< ","<< endl;
	jfs << "          \"chin_y\": "<< _chin<< ","<< endl;
	jfs << "          \"shoulders_y\": "<< _shoulders<< ","<< endl;
	jfs << "          \"hips_y\": "<< _hips<< ""<< endl;
	jfs << "      }";
}
