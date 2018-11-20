#ifndef __VEED_ORIENT90_CONFIG_H__
#define __VEED_ORIENT90_CONFIG_H__

#include <iostream>
#include <vector>



#define DISPLAY_VIDEO_OPTICALFLOW

//#define DISPLAY_VIDEO_ORIENT90		// Un-comment this to display video within orient90.
#define ORIENT90_SINGLE_FILE_RUN		// Run orient90 for a single video file. Comment out for testing only.



#define VEED_ORIENT90_CONFIG_FILE			"veed.cfg"

#define VEED_NUM_PARALLEL_PROCS				3

#define VEED_HOME							getenv("HOME")
//#define VEED_BUILD_EXEC_PATH_STR			(std::string(VEED_HOME) + "/src/veed/veed/src/build/")

#define VEED_VIDEO_DATA_DIR					"/home/data/veed/video"
#define VEED_VIDEO_DATA_DIR_STUB			"home/data/veed/video"
#define VEED_DATA_DIR						"/home/data/veed"
#define VEED_DETECT_DIR						"/home/data/veed/detect"
#define VEED_WRITE_DIR						"/home/data/veed/write"

using namespace std;


enum Veed_Orient_T
{
	VEED_ORIENT_0 = 0,			// original frame with no turning
	VEED_ORIENT_90 = 1,			// needs 90 degree clockwise rotation, 1
	VEED_ORIENT_180 = 2,		// needs 180 degree clockwise rotation, 2
	VEED_ORIENT_270	= 3			// needs 270 degree clockwise rotation, 3
};

extern string orient_str[4];

class Veed_Config
{
  public:
	Veed_Config( string cfgFile = VEED_ORIENT90_CONFIG_FILE );
	~Veed_Config() = default;

	int getVideoInSubdir( vector<string> &video_vec, string subdir );

  public:

	//string	video_relative_dir;
	vector<string>	video_subdir;
	vector<string>	video_ext;
};

#endif
