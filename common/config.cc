#include <stdexcept>
#include <fstream>
#include <stdlib.h>

#include "string_util.h"
#include "util.h"
#include "config.h"


using namespace std;


string orient_str[4] = {
		"Video oriented correctly, no turning needed.",
		"Video needs to be turned 90 degrees clockwise.",
		"Video needs to be turned 180 degrees clockwise.",
		"Video needs to be turned 270 degrees clockwise."
};


Veed_Config::Veed_Config( string cfgFile )
{

	string var, value;
	char line[1024];
	//DIR *dr;
	size_t i;

	ifstream cfgs( cfgFile.c_str() );
	if (cfgs.fail())
	{
		cerr << "[Orient90_Config()] Failed to open config file '"<< cfgFile.c_str()
			 << "'. Make sure the config file exists with read permissions." << endl;
		string err = string("[Orient90_Config()] Failed to open config file '") + cfgFile.c_str();
		throw invalid_argument( err );
	}

	/*
	// Find the video relative directory with respect to the current executable directory.
	string pwd = getenv("PWD");
	size_t p = 0;
	while ( ( p = pwd.find("/", p) ) != string::npos )
	{
		video_relative_dir += "../";
		++p;
	}
	video_relative_dir += VEED_VIDEO_DATA_DIR_STUB;
*/

	while (cfgs.good())
	{
		cfgs.getline(line, 1023);
		string lnstr( common::trim(line) );

		if  (  ( lnstr[0] == '#' ) )		// skip as comments
		{
			continue;
		}

		i = lnstr.find('=');
		var = lnstr.substr(0, i);
		value = lnstr.substr(i + 1);
		value = common::trim(value);

		// video sub-directories to process
		if (var.find("veed_video_subdir") != string::npos)
		{
			while ( (i = value.find(',')) != string::npos )
			{
				video_subdir.push_back( value.substr(0, i) );
				value = common::trim( value.substr( i + 1 ) );
			}
			video_subdir.push_back( value );
		}

		// Video file types (extensions) to process
		if (var.find("veed_video_extensions") != string::npos)
		{
			while ( (i = value.find(",")) != string::npos )
			{
				video_ext.push_back( value.substr(0, i) );
				value = common::trim( value.substr( i + 1 ) );
			}
			video_ext.push_back( value );
		}

	}
}


int Veed_Config::getVideoInSubdir( vector<string> &video_vec, string subdir )
{
	string f_path = string(VEED_VIDEO_DATA_DIR) + "/" + subdir;
	for ( size_t i = 0; i < video_ext.size(); ++i )
	{
		string ext = string(".") + video_ext[i];
		common::listdir_ext( video_vec, f_path, ext ); 	// get all files with the extension in this directory
	}

	return 0;
}
