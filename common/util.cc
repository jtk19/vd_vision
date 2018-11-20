#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "string_util.h"
#include "util.h"


int common::listdir( vector<string> &dirEntries, string path )
{
	struct dirent *entry;
	DIR *dp;

	dp = opendir(path.c_str());
	if (dp == NULL)
	{
		cerr<< "[listdir()] Directory ["<< path << "] does not exist."<< endl;
		return -1;
	}

	while((entry = readdir(dp)))
	{
		string d( entry->d_name);
		if ( (d != string("."))  &&  (d != string("..")) )
		{
			dirEntries.push_back( d );
		}
	}

	closedir(dp);
	return 0;
}


int common::listdir_ext( vector<string> &dirEntries, string path, string ext )
{
	struct dirent *entry;
	DIR *dp;

	dp = opendir(path.c_str());
	if (dp == NULL)
	{
		cerr<< "[listdir()] Directory ["<< path << "] does not exist."<< endl;
		return -1;
	}

	ext = trim(ext);
	while((entry = readdir(dp)))
	{
		string d( entry->d_name);
		if ( (d != string("."))  &&  (d != string(".."))
		   && ( d.find(ext) != string::npos) )
		{
			dirEntries.push_back( d );
		}
	}

	closedir(dp);
	return 0;
}


int common::create_dir( string path )
{
	DIR *dr;
	if (  ( ( dr = opendir(path.c_str()) ) == NULL ) )
	{
		cout<< "Creating ["<< path.c_str()<< "]";
		if ( mkdir(path.c_str(), 0775) != 0 )
		{
			cerr << endl<< "Failed to create ["<< path.c_str()<< "]"<< endl;
			return -1;
		}
		cout<< " . . . done"<< endl;
	}
	else
	{
		closedir(dr);
	}
	return 0;
}

int common::create_empty_dir( string path )
{
	DIR *dr;
	if (  ( ( dr = opendir(path.c_str()) ) == NULL ) )
	{
		cout<< "Creating ["<< path.c_str()<< "]";
		if ( mkdir(path.c_str(), 0775) != 0 )
		{
			cerr << endl<< "Failed to create ["<< path.c_str()<< "]"<< endl;
			return -1;
		}
		cout<< " . . . done"<< endl;
	}
	else
	{
		string cmd = string( " rm -rf ") + path + "/*";
		closedir(dr);
		system( cmd.c_str());
	}
	return 0;
}

string date_from_epoch( struct tm dt )
{
	return "hi";
}
