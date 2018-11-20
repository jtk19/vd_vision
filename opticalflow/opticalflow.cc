#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <opencv/cv.h>
#include <opencv/cxcore.h>

#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio/videoio_c.h>
#include <opencv2/video/background_segm.hpp>

#include <config.h>
#include <util.h>
#include "json_writer.h"


#define H_THRESHOLD 	19
#define V_THRESHOLD		19
#define H_FRAME_SPACE	15
#define V_FRAME_SPACE	10
#define VEED_FRAME_VERT_CUTOFF  	1.0000
#define VEED_FRAME_HORIZ_CUTOFF  	0.2000


using namespace std;
using namespace cv;



Veed_Config cfg;

vector<double> hflow, vflow;
Veed_Orient_T orientation;
cv::Rect face;

bool orient90_detected = false, opflow_orient_detected = false;
bool orient_detected = false;
bool face_detected = false;


bool detectOrient90( Veed_Orient_T *orient, cv::Rect *face, string subdir, string filename );
void turnOrient90( Mat *frame, Veed_Orient_T orient );
void markFace( Mat *frame, Rect *face, int hcentre, int top, json_writer &json );

void doGrabCut( Mat *pframe, int left, int right, int top, int bottom );


int main( int argc, char *argv[] )
{

#ifdef DISPLAY_VIDEO_OPTICALFLOW
	namedWindow("Original", WINDOW_NORMAL | WINDOW_KEEPRATIO );
	namedWindow("Centre Detect", WINDOW_NORMAL | WINDOW_KEEPRATIO );
	cv::moveWindow( "Original", 1260, 0);
	cv::resizeWindow( "Original", 640, 910 );
	cv::moveWindow( "Centre Detect", 600, 0);
	cv::resizeWindow( "Centre Detect", 640, 910 );
#endif

	string subdir = "phase1";
	string filename;

	// Read all the video files to process.
	vector<string> video_files;

	size_t num_video_dir = cfg.video_subdir.size();

	for ( size_t sd = 0; sd < num_video_dir; ++sd )
	{
		subdir = cfg.video_subdir[sd];
		string f_path = string(VEED_VIDEO_DATA_DIR) + "/" + subdir;
		video_files.clear();
		//common::listdir_ext( video_files, f_path, ".avi" ); 	// get all files with .avi extension in this directory
		cfg.getVideoInSubdir( video_files, subdir );

		for ( size_t f = 0; f < video_files.size(); ++f )
		{
			filename = video_files[f];


			stringstream ss;
			ss << VEED_VIDEO_DATA_DIR<< "/"<< subdir<< "/" << filename;
			cout<< "\n------- Processing: "<< ss.str()<< endl;
			cv::VideoCapture cap( ss.str() );
			cv::Mat f1, f0, f2, fgray;

			double max_v_val, max_h_val;
			int max_v, max_h;
			int left, right, top, bottom;
			double v_mean, v_variance, h_mean, h_variance;
			double top_v_mean, btm_v_mean, top_v_variance, btm_v_variance;
			double lft_h_mean, lft_h_variance, rht_h_mean, rht_h_variance;


			orient_detected = face_detected = detectOrient90( &orientation, &face, subdir, filename );
			cout<< "Orientation detected ["<< orient_detected<< "] by orient90: "<< orientation<< endl;
			orient90_detected = orient_detected;

			size_t fcount = 0;
			hflow.clear();
			vflow.clear();

			while ( cap.read(f1) )
			{
				f2 = f1.clone();
				cvtColor( f1, fgray, CV_RGB2GRAY );

				/*------- Optical Flow - Centre Detection --------------------------------------------- */
				if ( fcount++ == 0 )
				{
					hflow.resize( f1.cols, 0 );
					vflow.resize( f1.rows, 0 );
					memset( &(hflow[0]), 0, sizeof(uint64_t) * f1.cols  );
					memset( &(vflow[0]), 0, sizeof(uint64_t) * f1.rows  );
#ifdef DISPLAY_VIDEO_OPTICALFLOW
					imshow( "Centre Detect", f1 );
#endif
				}
				else
				{
					for ( int r = 0; r < f1.rows; ++r )
					{
						for ( int c = 0; c < f1.cols; ++c )
						{
							size_t diff = abs( fgray.at<uchar>(Point(c,r)) - f0.at<uchar>(Point(c,r)) );
							size_t add = ( diff > V_THRESHOLD ? 1 : 0);
							vflow[r] += add;
							add = ( diff > H_THRESHOLD ? 1 : 0);
							hflow[c] += add;
						}
					}

					max_v_val = max_h_val = 0;
					for ( int r = 0; r < f1.rows; ++r )
					{
						if ( max_v_val < vflow[r] )
						{
							max_v_val = vflow[r];
							max_v = r;
						}
					}
					for ( int c = 0; c < f1.cols; ++c )
					{
						if ( max_h_val < hflow[c] )
						{
							max_h_val = hflow[c];
							max_h = c;
						}
					}
					//cout<< "Max (r, c): ( "<< max_v_val<< ","<< max_h_val<< ")"<< endl;
					line( f2, Point( max_h, 1), Point(max_h, f1.rows), Scalar( 0, 255, 0), 1 );
					line( f2, Point( 1, max_v), Point(f2.cols, max_v), Scalar( 0, 255, 0), 1 );
				}
				f0 = fgray.clone();

#ifdef DISPLAY_VIDEO_OPTICALFLOW
				imshow( "Centre Detect", f2 );
				imshow( "Original", fgray );
				waitKey(1);
#endif
			}
			cap.release();


			// Full video processing ------------------------------------------------
			// ------- Do orientation, Centre detection, Cropping -------------------
			// If orient90 failed to detect orientation,
			// estimate orientation using optical-flow variances.

			float v0, v1;

			// Centre point is ( max_h, max_v )
			max_v_val = max_h_val = 0;

			// initialisation for Cropping
			left = 0;
			right = f0.cols;
			top = 0;
			bottom = f0.rows;

			cout<< std::fixed<< std::setprecision(4);

			// Optical Flow vector Max and Means
			v_mean = h_mean = 0.0;
			top_v_mean = btm_v_mean = lft_h_mean = rht_h_mean = 0.0;
			int v_mid = (int)(f0.rows / 2) + 1;
			int h_mid = (int)(f0.cols / 2) + 1;

			for ( int r = 0; r < f0.rows; ++r )
			{
				// Centre detection
				if ( max_v_val < vflow[r] )
				{
					max_v_val = vflow[r];
					max_v = r;
				}

				if ( !orient_detected )
				{
					v_mean += vflow[r];
					if ( r < v_mid )
					{
						top_v_mean += vflow[r];
					}
					else
					{
						btm_v_mean += vflow[r];
					}
				}
			}
			if ( !orient_detected )
			{
				v_mean /= f0.rows;
				top_v_mean /= ( v_mid - 1);
				btm_v_mean /= ( v_mid -1 );
			}

			for ( int c = 0; c < f0.cols; ++c )
			{
				// Centre detection
				if ( max_h_val < hflow[c] )
				{
					max_h_val = hflow[c];
					max_h = c;
				}

				if ( !orient_detected )
				{
					h_mean += hflow[c];
					if ( c < h_mid )
					{
						lft_h_mean += hflow[c];
					}
					else
					{
						rht_h_mean += hflow[c];
					}
				}
			}
			if ( !orient_detected )
			{
				h_mean /= f0.cols;
				lft_h_mean /= ( h_mid -1 );
				rht_h_mean /= ( h_mid -1 );
			}


			// ------- Estimate Optical Flow Vector Variance based Orientation -----------------
			double tmp;
			if ( !orient_detected )
			{
				cout<< "Orientation estimation by optical flow"<< endl;
				v_variance = h_variance = 0;
				lft_h_variance = rht_h_variance = top_v_variance = btm_v_variance = 0;
				for ( int c = 0; c < f0.cols; ++c )
				{
					tmp = hflow[c] - h_mean;
					h_variance += ( tmp * tmp );
					if ( c < h_mid )
					{
						tmp = hflow[c] - lft_h_mean;
						lft_h_variance += (tmp * tmp);
					}
					else
					{
						tmp = hflow[c] - rht_h_mean;
						rht_h_variance += (tmp * tmp);
					}
				}
				for ( int r = 0; r < f0.rows; ++r )
				{
					tmp = vflow[r] - v_mean;
					v_variance += ( tmp * tmp );
					if ( r < v_mid )
					{
						tmp = vflow[r] - top_v_mean;
						top_v_variance += ( tmp * tmp );
					}
					else
					{
						tmp = vflow[r] - btm_v_mean;
						btm_v_variance += ( tmp * tmp );
					}
				}

				cout<< "Horizontal variance : "<< h_variance<< endl;
				cout<< "Vertical variance   : "<< v_variance<< endl;
				cout<< endl;
				cout<< "Top variance   : "<< top_v_variance<< endl;
				cout<< "Bottom variance: "<< btm_v_variance<< endl;
				cout<< "Left variance  : "<< lft_h_variance<< endl;
				cout<< "Right variance : "<< rht_h_variance<< endl;
				cout<< endl;

				// The direction with the largest variance is the horizontal direction.
				if ( h_variance > v_variance )
				{
					// orientation is either 0 or 180.
					// We assume it is 0, so do nothing
					orientation = VEED_ORIENT_0;

					if ( btm_v_variance < top_v_variance )
					{
						cout<< "[Optical-flow Orientation estimation] WARNING: The video might be upside-down."<< endl;
					}
				}
				else
				{
					// orientation is either 90 or 270;

					// Typically the top half has higher variance than the bottom half
					if ( lft_h_variance > rht_h_variance )
					{
						orientation = VEED_ORIENT_90;
					}
					else // rht_h_variance > lft_h_variance
					{
						orientation = VEED_ORIENT_270;
					}
				}
				orient_detected = true; 	// Estimated via optical-flow
				face_detected = false;		// Cannot detect a face by optical-flow

			}

			cout<< "Orientation detected ? "<< orient_detected<< endl;
			cout<< "Orientation: "<< orientation<< endl;

			if ( orient_detected && (orientation != VEED_ORIENT_0) )
			{
				double tmp;
				int tmpi;

				cout<< "Doing the turn."<< endl;

				turnOrient90( &f0, orientation );
				turnOrient90( &f2, orientation );
				turnOrient90( &fgray, orientation );

				if ( orientation == VEED_ORIENT_180 )
				{
					// flip vertical-flow vector
					vector<double> new_vflow( vflow.rbegin(), vflow.rend() );
					vflow = new_vflow;
					max_v = f0.rows - max_v;
				}
				else // 90 or 270
				{
					// Swap max values and max-value-indices also for vertical and horizontal.
					tmpi = max_h;
					max_h = max_v;
					max_v = tmpi;
					tmp = max_h_val;
					max_h_val = max_v_val;
					max_v_val = tmp;

					if ( orientation == VEED_ORIENT_90 )

					{
						// Do a 90 degree clockwise turn
						// New horizontal is the current vertical reversed.
						vector<double> new_hflow( vflow.rbegin(), vflow.rend() );
						vflow = hflow;
						hflow = new_hflow;
						max_h = f0.cols - max_h;
					}
					else 	// VEED_ORIENT_270
					{
						// Do a 270 degree clockwise turn.
						// New vertical is the current horizontal reversed
						vector<double> new_vflow( hflow.rbegin(), hflow.rend() );
						hflow = vflow;
						vflow = new_vflow;
						max_v = f0.rows - max_v;
					}
				}
			}
			//waitKey(0);

			// ------- The Crop ------------------------------------------------------------------------------
			cout<< "Horizontal Optical Flow:\n[ ";
			v0 = v1 = 0;
			left = right = -1;
			for ( int c = 0; c < f0.cols; ++c )
			{
				v1 = (100* hflow[c])/max_h_val;
				//cout<< v1 << " ";
				if ( ( c < f0.cols/3) && (v0 < VEED_FRAME_HORIZ_CUTOFF)  && ( v1 > VEED_FRAME_HORIZ_CUTOFF ) )
				{
					left = c;
				}
				if ( ( c > (2*f0.cols)/3) && (v0 > VEED_FRAME_HORIZ_CUTOFF ) && ( v1 < VEED_FRAME_HORIZ_CUTOFF ) )
				{
					right = c;
					break;
				}
				v0 = v1;
			}
			if ( left == -1 ) left = 0;
			if ( right == -1 ) right = f0.cols;
			cout<< "]"<< endl<< endl;

			cout<< "Vertical Optical Flow:\n[ ";
			v0 = v1 = 0;
			top = bottom = -1;
			for ( int r = 0; r < f0.rows; ++r )
			{
				v1 = ( 100 * vflow[r]) / max_v_val;
				//cout<< v1 << " ";
				if ( ( r < f0.rows/3) && (v0 < VEED_FRAME_VERT_CUTOFF )  && ( v1 > VEED_FRAME_VERT_CUTOFF ) )
				{
					top = r;
				}
				if ( ( r > (2*f0.rows)/3) && (v0 > VEED_FRAME_VERT_CUTOFF ) && ( v1 < VEED_FRAME_VERT_CUTOFF ) )
				{
					bottom = r;
					break;
				}
				v0 = v1;
			}
			if ( top == -1 ) top = 0;
			if ( bottom == -1 ) bottom = f0.rows;
			cout<< "]"<< endl<< endl;

			cout<< "Video size (rows, cols)  : [ "<< f0.rows<< ", "<< f0.cols<< "] "<< endl;
			cout<< "(left, right): [ "<< left << ", "<< right<< "]"<< endl;
			cout<< "(top, bottom): [ "<< top << ", "<< bottom<< "]"<< endl;


			// Adjust the clipping frame with some buffer space.
			int i = 0;
			while ( ((left-1) > 0)  &&  ( i++ < H_FRAME_SPACE) )
				--left;
			i = 0;
			while ( ((right+1) < f0.cols )  &&  ( i++ < H_FRAME_SPACE) )
				++right;
			i = 0;
			while ( ((top-1) > 0)  &&  ( i++ < V_FRAME_SPACE) )
				--top;
			i = 0;
			while ( ((bottom+1) < f0.rows )  &&  ( i++ < V_FRAME_SPACE) )
				++bottom;


			cout<< "Horizontal cutoff columns ( left, right ) : ( "<< left<< ", "<< right<< ")"<< endl;
			cout<< "Vertical cutoff rows ( top, bottom )      : ( "<< top<< ", "<< bottom<< ")"<< endl;
			cout<< endl;

			rectangle( f2, Point( left, top), Point( right, bottom), Scalar( 255, 0, 0) );
			rectangle( fgray, Point( left, top), Point( right, bottom), Scalar( 255, 0, 0) );

#ifdef DISPLAY_VIDEO_OPTICALFLOW
			imshow( "Centre Detect", f2 );
			imshow( "Original", fgray );
			waitKey(1);
#endif

			// ------- Write clipped video ----------------------------------------
			cout<< "Testing..."<< endl;
			cv::VideoCapture cap1( ss.str() );

			size_t idx = filename.find('.');
			string fname = ( idx == string::npos ) ? filename: filename.substr( 0, idx );
			string ext = ( idx == string::npos ) ? ".avi" : filename.substr( idx );

			stringstream ssw, ssw1; // ssw2;
			ssw << VEED_WRITE_DIR<< "/"<< subdir<<"_"<< fname<< ext;
			ssw1<< VEED_WRITE_DIR<< "/"<< subdir<<"_"<< fname<< "_marked"<< ext;
			cv::VideoWriter wr;
			cv::VideoWriter wr1;
			//ssw2<< VEED_WRITE_DIR<< "/"<< f<< "_bgrm.avi";
			//cv::VideoWriter wr2;

			bool fopen = true;

			json_writer json( subdir+ "_" + fname );
			json.setFile( subdir, fname, ext );
			opflow_orient_detected = (orient_detected && (!orient90_detected));
			json.setOrientation( orient90_detected, opflow_orient_detected, orientation);
			json.setCrop( left, right, top, bottom );
			json.setOpflowCentre( max_h, max_v );

			while ( cap1.read(f1) )
			{
				// orientation
				turnOrient90( &f1, orientation );

				// crop
				Mat m( bottom - top + 1, right - left + 1, CV_8UC3, cv::Scalar(255,255,255) );
				Rect r( left, top, right - left, bottom - top);
				f1(r).copyTo( m(Rect( 0, 0, m.cols-1, m.rows-1) ) );

				// Background removal
				//Mat m1 = m.clone();
				//doGrabCut( &m1, 1, m1.cols - 1, 1, m1.rows - 1 );

				if ( fopen )
				{
					cout<< "Video size: ( rows, cols): [ "<< f1.rows<< ", "<< f1.cols<< "] "<< endl;
					fopen = false;
					bool b = wr.open( ssw.str(), CV_FOURCC('X','2','6','4'), 25, m.size() );
					if ( b )
					{
						cout<< "Video file ["<< ssw.str()<< "] opened for writing main video."<< endl;
					}
					else
					{
						cout<< "Video file ["<<  ssw.str()<< "] failed to open for writing main video!"<< endl;
						return -1;
					}
					b = wr1.open( ssw1.str(), CV_FOURCC('X','2','6','4'), 25, m.size() );
					if ( b )
					{
						cout<< "Video file ["<< ssw1.str()<< "] opened for writing marked video."<< endl;
					}
					else
					{
						cout<< "Video file ["<<  ssw1.str()<< "] failed to open for writing marked video!"<< endl;
						return -1;
					}
					/*
					b = wr2.open( ssw2.str(), CV_FOURCC('X','2','6','4'), 25, m.size() );
					if ( b )
					{
						cout<< "Video file ["<< ssw2.str()<< "] opened for writing bgsub video."<< endl;
					}
					else
					{
						cout<< "Video file ["<<  ssw2.str()<< "] failed to open for writing bgsub video!"<< endl;
						return -2;
					}
					*/
				}

				// write before any mark-ups to main output
				wr.write(m);

				// Mark-ups : centres of flow
				line( m, Point( max_h - left, 1), Point( max_h - left, m.rows), Scalar( 0, 255, 0), 1 );
				line( m, Point( 1, max_v - top), Point( m.cols, max_v - top), Scalar( 0, 255, 0), 1 );

				// mark face and body points
				if (face_detected )
				{
					markFace( &m, &face, max_h, top, json );
				}
				else
				{
					json.setFaceDetection( false, 0, 0, 0, 0 );
				}

				wr1.write( m );
				//wr2.write( m1 );


#ifdef DISPLAY_VIDEO_OPTICALFLOW
				//imshow( "Centre Detect", m1);
				imshow( "Original", m);
				waitKey(1);
#endif

			}

			json.write();

			cap1.release();
			wr.release();
			wr1.release();
			//wr2.release();
#ifdef DISPLAY_VIDEO_OPTICALFLOW
			waitKey(0);
#endif
		}

	}

	//char ch;  cout<< "Enter: "<< endl; cin >> ch;

	return 0;
}


bool detectOrient90( Veed_Orient_T *orient, cv::Rect *face, string subdir, string filename )
{
	static const char * const exec_path = getenv("PWD");
	static const char *executable = "orient90";

	bool detected = true;

	cout<< "orient90 detection for: "<< subdir<< "/"<< filename<< endl;

	size_t doti;
	if ( (doti = filename.find('.')) == string::npos )
	{
		cerr<< "[detectOrient90] ERROR. Filename [" << filename
			<<"] must have a .avi/.mp4/.MOV extension."<< endl;
		throw invalid_argument( "[detectOrient90] Filename error." );
	}

	string detectf = string(VEED_DETECT_DIR) + "/" + subdir
						+ "_" + filename.substr( 0, doti ) + ".det";

	ifstream detfs(detectf, ios_base::in);
	if ( !detfs.good() )		// Orient90 has not previously run for this video; run it directly.
	{
		string video = string(VEED_VIDEO_DATA_DIR) + "/" + subdir + "/" + filename;
		string ex = string( exec_path ) + "/" +  executable;
		pid_t pid, p;
		int stat, rtn;

		if ( (pid = fork()) < 0 )
		{
			cerr<< "fork() error calling orient90 for "<< video<< endl;
			abort();
		}
		else if ( pid == 0 )
		{
			//int rc = execl( "/home/jenni/src/veed/veed/src/build/veed", "veed", args[k][2], (char *)NULL  );
			execl( ex.c_str(), executable, subdir.c_str(), filename.c_str(), (char *)NULL );
		}
		else
		{
			cout<< "Running orient90 for: "<< video << endl;
			p = wait(&stat);
			cout<< "Process with pid "<< (long)p<< " exited with status "<< stat<< endl;
			if ( WIFEXITED(stat) )
			{
				rtn = WEXITSTATUS(stat);
				if ( rtn < 0 )
				{
					// oreint90 cannot detect orientation
					cout<< "orient90 detection process failed: "<< rtn<< endl;
					return false;
				}
				else	// try to open the results file again below
				{
					detfs.open( detectf, ios_base::in );
				}
			}
		}
	}


	if ( detfs.good() )			// read orientation from detection log file
	{
		int tmp;

		detfs.exceptions( ifstream::failbit | ifstream::badbit | ifstream::eofbit );

		cout<< "Reading detection results . . ."<< endl;
		try
		{
			detfs >> tmp;
			if ( tmp < 0 )			// orient90 has failed to detect orientation
			{
				return false;			// return failure
			}
			(*orient) = (Veed_Orient_T)tmp;

			detfs >> (face->x);
			detfs >> (face->y);
			detfs >> (face->width);
			detfs >> (face->height);
			cout<< " orientation: "<< (*orient)<< endl;
			cout<< " face at ( "<< face->x<< ", "<< face->y<< ", "
				<< face->width<< ", "<< face->height<< ")"<< endl;
			detected = true;
		}
		catch ( ifstream::failure &e)
		{
			cerr<< "[opticalflow::detectOrient90()] Exception thrown while reading detection file: "
				<< detectf<< endl;
			cerr<< "ERROR: "<< e.what()<< endl;
			detected = false;
		}
	}
	else
	{
		detected = false;		// All attempts at orient90 detection has failed.
		cout<< "orient90 detection file open failed."<< endl;
	}

	return detected;
}


void turnOrient90( Mat *frame, Veed_Orient_T orient )
{
	if ( orient == VEED_ORIENT_0 )
	{
		// nothing to do
		//cout<< "no turn"<< endl;
		return;
	}

	if ( orient == VEED_ORIENT_180 )
	{
		int flip_flag = ( (frame->rows) > (frame->cols) ) ? 0 : 1;
		cv::flip( *frame, *frame, flip_flag );
		//cout<< "turning 180"<< endl; imshow( "Original", *frame ); waitKey(0);
		return;
	}

	// else 90 or 270 degree rotation
	Mat m;
	cv::transpose( *frame, m );
	if ( orient == VEED_ORIENT_90 )
	{
		cv::flip( m, m, 1);
		*frame = m.clone();
		//cout<< "turning 90"<< endl; imshow( "Original", *frame ); waitKey(0);
	}
	else // VEED_ORIENT_270
	{
		int flip_flag = ( m.rows > m.cols) ? 0 : 1;
		cv::flip( m, m, flip_flag );			// for 270 rotation clockwise
		*frame = m.clone();
		//cout<< "turning 270"<< endl; imshow( "Original", *frame ); waitKey(0);
	}
	return;
}


void markFace( Mat *m, Rect *face, int hcentre, int top, json_writer &json )
{
	if ( m == NULL ||  face == NULL )
	{
		return;
	}

	int correction = 0;
	int space = (m->cols) / 4;

	line( *m, Point( space, face->y - top - 30 ),
			  Point( m->cols - space, face->y - top - 30 ), Scalar( 255, 0, 0), 1 );

	line( *m, Point( space, face->y + face->height - top - correction ),
			  Point( m->cols - space, face->y + face->height - top - correction), Scalar( 255, 0, 0), 1 );

	// Shoulders
	int sh_y = face->y - top + ( 1.42 * face->height ) - correction ;
	space = (m->cols) / 6;
	line( *m, Point( space, sh_y ),
			  Point( m->cols - space, sh_y ), Scalar( 255, 0, 0), 1 );

	// Hips
	int hips_y = face->y - top + ( 4.75 * face->height );
	line( *m, Point( space, hips_y ),
				 Point( m->cols - space, hips_y ), Scalar( 255, 0, 0), 1 );

	json.setFaceDetection( true, face->y - 30,
			                     face->y + face->height - correction,
			                     sh_y + top,
			                     hips_y + top );
}


void doGrabCut( Mat *pframe, int left, int right, int top, int bottom )
{
	cv::Mat result; // segmentation result (4 possible values)
	cv::Mat bgModel,fgModel; // the models (internally used)
	Mat background;

	//imshow("FG Mask MOG 2", *pframe );

	Mat image = (*pframe).clone();
	Mat image2 = image.clone();

	// define bounding rectangle
	cv::Rect rectangle( left, top, right, bottom );

	//cout<< "getting grabcut..."<< endl;
	// GrabCut segmentation
	cv::grabCut(image,    // input image
	            result,   // segmentation result
	            rectangle,// rectangle framing foreground
	            bgModel,fgModel, // models
	            1,        // number of iterations
	            cv::GC_INIT_WITH_RECT); // use rectangle
	//cout<< "done"<< endl;

	// Get the pixels marked as likely foreground
	cv::compare(result,cv::GC_PR_FGD,result,cv::CMP_EQ);
	// Generate output image
	cv::Mat foreground( image.size(), CV_8UC3,cv::Scalar(255,255,255) );
	//cv::Mat background(image.size(),CV_8UC3,cv::Scalar(255,255,255));
	image.copyTo( foreground, result);// bg pixels not copied

	// draw rectangle on original image
	//cv::rectangle( image, rectangle, cv::Scalar(255,255,255),1);

	*pframe = foreground;

}


