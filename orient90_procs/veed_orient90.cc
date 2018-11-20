/*---------------------------------------------------------------------------------
 * Main program for running orient90 for each file in independent
 * forked processes.
 *---------------------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

#include <util.h>
#include <config.h>
#include "veed_orient90.h"


using namespace std;

//Veed_Config cfg;

pid_t pid[VEED_NUM_PARALLEL_PROCS];

const char * const exec_path = getenv("PWD");
const char *executable = "orient90";

size_t num_procs = VEED_NUM_PARALLEL_PROCS;

size_t num_args = 0;
vector< pair<string,string> > args;

//size_t num_video_dir = cfg.video_subdir.size();
//string veed_video_dir[] = { "90", "270", "180", "0" };


vector< pair<string,string> > undetected;


int main()
{

	Veed_Config cfg;
	size_t num_video_dir = cfg.video_subdir.size();

	size_t num_rounds, num_rem;
	string ex = string( exec_path ) + "/" +  executable;

	// Read all the video files to process.
	vector<string> video_files;

	//cout<< " num_video_dir "<< num_video_dir<< endl;

	for ( size_t i = 0; i < num_video_dir; ++i )
	{
		string f_path = string(VEED_VIDEO_DATA_DIR) + "/"+ cfg.video_subdir[i];
		string subdir = cfg.video_subdir[i];
		video_files.clear();
		cfg.getVideoInSubdir( video_files, subdir );
		//common::listdir_ext( video_files, f_path, ".avi" ); 	// get all files with .avi extension in this directory

		for ( size_t j = 0; j < video_files.size(); ++j )
		{
			args.emplace_back( pair<string,string>( cfg.video_subdir[i], video_files[j]) );
			//cout<< "Running for: "<< cfg.video_subdir[i]<< "/"<< video_files[j]<< endl;
		}
	}
	num_args = args.size();
	num_rounds = (int)(num_args/num_procs);
	num_rem = num_args % num_procs;


	// Now run orienttaion detection algorithms for all the videos.
	for ( size_t i = 0; i < num_rounds; ++i )
	{
		for ( size_t j = 0; j < num_procs; ++j )
		{
			size_t k = num_procs*i + j;
			if ( (pid[j] = fork()) < 0 )
			{
				cerr<< "fork() error for process #"<< num_procs*i + j<< endl;
				abort();
			}
			else if ( pid[j] == 0 )
			{
				//int rc = execl( "/home/jenni/src/veed/veed/src/build/veed", "veed", args[k][2], (char *)NULL  );
				int rc = execl( ex.c_str(), executable, args[k].first.c_str(), args[k].second.c_str(), (char *)NULL );
				exit(rc);
			}
			else
			{
				cout<< "running for: "<< args[k].first<< "/"<< args[k].second<< endl;
			}
		}	// for-loop j

		size_t n = num_procs;
		int stat;
		pid_t p;

		while ( n > 0 )
		{
			p = wait(&stat);
			cout<< "Process with pid "<< (long)p<< " exited with status "<< stat<< endl;
			if ( stat != 0 )
			{
				for ( size_t j = 0; j < num_procs; ++j )
				{
					size_t k = num_procs*i + j;
					if (pid[k] == p )
					{
						undetected.emplace_back( args[k].first, args[k].second );
						break;
					}
				}
			}
			--n;
		}

		cout<< "Round "<< i+1 << " done"<< endl<< endl;
	}// for-loop i

	for ( size_t j = 0; j < num_rem; ++j )
	{
		size_t k = num_procs*num_rounds + j;
		if ( (pid[j] = fork()) < 0 )
		{
			cerr<< "fork() error for process #"<< num_procs*num_rounds + j<< endl;
			abort();
		}
		else if ( pid[j] == 0 )
		{
			//int rc = execl( "/home/jenni/src/veed/veed/src/build/veed", "veed", args[k][2], (char *)NULL  );
			int rc = execl( ex.c_str(), executable, args[k].first.c_str(), args[k].second.c_str(), (char *)NULL );
			exit(rc);
		}
		else
		{
			cout<< "running for: "<< args[k].first<< "/"<< args[k].second<< endl;
		}

	}	// for-loop j

	size_t n = num_rem;
	int stat;
	pid_t p;

	while ( n > 0 )
	{
		p = wait(&stat);
		cout<< "Process with pid "<< (long)p<< " exited with status "<< stat<< endl;
		if ( stat != 0 )
		{
			for ( size_t j = 0; j < num_procs; ++j )
			{
				int k = num_procs*num_rounds + j;
				if (pid[k] == p )
				{
					undetected.emplace_back( args[k].first, args[k].second );
					break;
				}
			}
		}
		--n;
	}

	return 0;
}
