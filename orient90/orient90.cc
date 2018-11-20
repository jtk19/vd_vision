#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <unistd.h>

//#include <cvd/videofilebuffer.h>
//#include <cvd/videosource.h>
//#include <cvd/videodisplay.h>
//#include <cvd/videobufferflags.h>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio/videoio_c.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>

#include <string_util.h>
#include <util.h>
#include "orient90.h"


using namespace std;
//using namespace CVD;
using namespace cv;


Veed_Config cfg;

string veed_image_data_dir = VEED_IMAGE_DATA_DIR;
string veed_video_data_dir = VEED_VIDEO_DATA_DIR;

string veed_write_dir = VEED_WRITE_DIR;

string veed_original_win = "Original";
string veed_face_detect_win = "Face Detect";

string veed_face_cascade_model = string(VEED_MODEL_HAARCASCADE_DIR) + "haarcascade_frontalface_alt.xml";
string veed_eyes_cascade_model = string(VEED_MODEL_HAARCASCADE_DIR) + "haarcascade_eye_tree_eyeglasses.xml";
string veed_full_body_cascade_model = string(VEED_MODEL_HAARCASCADE_DIR) + "haarcascade_fullbody.xml";
//string veed_upper_body_cascade_model = string(VEED_MODEL_HAARCASCADE_DIR) + "haarcascade_upperbody.xml";

CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
CascadeClassifier full_body_cascade;
//CascadeClassifier upper_body_cascade;

//string veed_video_fnames[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18" };
string veed_video_fnames[] = { "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18" };

string veed_video_dir[] = { "phase1" };

string veed_orient_str[] = { "0", "90", "180", "270"};
int veed_orient_numdir = 4;

char ch;


bool searchVideopersonDetect( Veed_Orient_T *orientation, Rect *roi, string video_avi);
bool personDetectWith90Rotations( Veed_Orient_T *orient, Rect *roi, Mat *m_detect);
bool faceDetect( cv::Mat *frame, cv::Rect *roi );

void rotate90Video_ffmpeg( string video_avi, string write_file, Veed_Orient_T orient );
void rotate90Video( string video_avi, string write_file, Veed_Orient_T orient, Rect *roi = NULL );

void removeOutsideROI( Mat *frame, Rect *roi);

Mat frame_rotate(Mat *src, double angle);
Mat frame_rotate90( Mat *src );
Mat frame_rotate180( Mat *src);

	
int main( int argc, char *argv[] )
{
#ifdef ORIENT90_SINGLE_FILE_RUN

	if ( argc < 3 )
	{
		cout<< "Usage for a single video: orient90 <video sub-directory> <video file>"<< endl;
		cout<< "e.g.: "<< argv[0]<< " client1 model1.avi"<< endl
			<< "     executes the program for /home/data/veed/video/client1/model1.avi"<<endl;
		return -1;
	}
#endif

	cout<< "Welcome to Veed Orient90."<< endl;

#ifdef DISPLAY_VIDEO_ORIENT90
	cv::namedWindow( veed_original_win, WINDOW_NORMAL | WINDOW_KEEPRATIO );
	cv::namedWindow( veed_face_detect_win, WINDOW_NORMAL | WINDOW_KEEPRATIO );
	cv::moveWindow( veed_original_win, 10, 640 );
	cv::moveWindow( veed_face_detect_win, 840, 0);
	cv::resizeWindow( veed_face_detect_win, 910, 910 );
	cout<< "Displays initialised."<< endl;
#endif

	// Load the cascade model for face detection.
	if( !face_cascade.load( veed_face_cascade_model ) )
	{
		cerr<< "--(!)Error loading face model: "<< veed_face_cascade_model<< endl;
		return -1;
	}
	else
	{
		cout<< "Face cascade model loaded: "<< veed_face_cascade_model<< endl;
	}
	// Load the cascade model for eyes detection.
	if ( !eyes_cascade.load( veed_eyes_cascade_model ) )
	{
		cerr<< "--(!)Error loading eyes model: "<< veed_eyes_cascade_model<< endl;
		return -1;
	}
	else
	{
		cout<< "Eyes cascade model loaded: "<< veed_eyes_cascade_model<< endl;
	}
	// Load the cascade model for full-body detection.
	if ( !full_body_cascade.load( veed_full_body_cascade_model ) )
	{
		cerr<< "--(!)Error loading full-body model: "<< veed_full_body_cascade_model<< endl;
		return -1;
	}
	else
	{
		cout<< "Full body cascade model loaded: "<< veed_full_body_cascade_model<< endl;
	}
	/*
	// Load the cascade model for full-body detection.
	if ( !upper_body_cascade.load( veed_upper_body_cascade_model ) )
	{
		cerr<< "--(!)Error loading upper-body model: "<< veed_upper_body_cascade_model<< endl;
		return -1;
	}
	else
	{
		cout<< "Upper body cascade model loaded: "<< veed_upper_body_cascade_model<< endl;
	}
	*/
	// main program
//	string v_rpath = cfg.video_relative_dir;
	string f_path, f_rpath, read_file, write_file, detect_file;
	Veed_Orient_T orientation = VEED_ORIENT_0;
	Rect roi;

	bool detected = false;

#ifdef MULTI_PROC
	int rc;

	cout<< "Running multi-proc."<< endl;

	if ( argc < 3 )
	{
		return 99;
	}

	f_rpath = string("file://") + cfg.video_relative_dir + "/"+ argv[1] + "/" + argv[2];	// relative path
	read_file = string(VEED_VIDEO_DATA_DIR) + "/"+ argv[1] + "/" + argv[2];					// absolute path
	write_file = string(VEED_WRITE_DIR) + "/"+ argv[1] + "_" + argv[2];						// Video write file

	cout<< "Running for :"<< f_rpath;

	detected = searchVideopersonDetect( &orientation, f_rpath );

	if ( detected )
	{
		// Not useing ffmpeg for actual rotations anymore.
		//rotate90Video_ffmpeg( read_file, write_file, orientation );
		rc = 0;
	}
	else
	{
		rc = 9;
	}

	return rc;

#else

#ifdef ORIENT90_SINGLE_FILE_RUN

	cout<< "Running for single file."<< endl;

	int rc;

	if ( argc < 3 )
	{
		cout<< "Usage for a single video: orient90 <video sub-directory> <video file>"<< endl;
		cout<< "e.g.: "<< argv[0]<< " client1 model1.avi"<< endl
			<< "     executes the program for /home/data/veed/video/client1/model1.avi"<<endl;
		return -1;
	}

	string fname(argv[2]);
	size_t doti = fname.find('.');
	if ( doti == string::npos )
	{
		cerr<< "The finename ["<< argv[2]<< "] must be in format: <filename>.<ext>"
			<< "\nwhere ext is '.avi' or '.mp4' or '.MOV' or another video format."<<endl;
		return -2;
	}
	read_file =  string(VEED_VIDEO_DATA_DIR) + "/" + argv[1] + "/" + argv[2];
	detect_file = string(VEED_DATA_DIR) + "/detect/" + argv[1] + "_" + fname.substr( 0, doti ) + ".det";

	detected = searchVideopersonDetect( &orientation, &roi, read_file );

	if (detected)
	{
		cout<< "result: "<< (int)orientation<< "\t"<< orient_str[(size_t)orientation]<< endl;

		// Write results todetection file.
		ofstream ofs( detect_file, ios_base::out );
		if ( ofs.good() )
		{

			ofs << (int)orientation<< " ";
			ofs << roi.x<< " "
				<< roi.y<< " "
				<< roi.width<< " "
				<< roi.height<< " "<< endl;
			ofs.close();
			rc = (int)orientation;
		}
		else
		{
			cout<< "Failed to open the detection file ["<< detect_file.c_str()<< "] for writing results."<< endl;
			rc = -1;
		}
	}
	else
	{
		cout<< "orient90 failed to detect the orientation fo this video."<< endl;
		ofstream ofs( detect_file, ios_base::out );
		if ( ofs.good() )
		{
			ofs << -1<< " ";
			ofs << 0 << " "
				<< 0 << " "
				<< 0 << " "
				<< 0 << " "<< endl;
			ofs.close();
			rc = (int)orientation;
		}
		else
		{
			cout<< "Failed to open the detection file ["<< detect_file.c_str()<< "] for writing negative results."<< endl;
			rc = -1;
		}
		cout<< "Orientation detection failed for: "<< read_file<< endl;
		rc = -19;
	}

	return rc;

#else	// Multi-file run
	vector<string> video_files;

	vector< pair<string,string> > undetected;

	cout<< "Running in a single process for all files."<< endl;

	size_t num_video_dir = cfg.video_subdir.size();

	for ( size_t i = 0; i < num_video_dir; ++i )
	{
		f_path = string(VEED_VIDEO_DATA_DIR) + "/"+ veed_video_dir[i];

		subdir = cfg.video_subdir[i];
		video_files.clear();
		cfg.getVideoInSubdir( video_files, subdir );
		//common::listdir_ext( video_files, f_path, ".avi" ); 	// get all files with .avi extension in this directory
		cout<< "Processing "<< video_files.size()<< " avi video files"<< endl;

		for ( size_t j = 0; j < video_files.size(); ++j )
		{
			f_rpath = "file://" + v_rpath + veed_video_dir[i] + "/" + video_files[j];
			read_file = f_path + "/" + video_files[j];
			write_file = string(VEED_WRITE_DIR) + "/" + veed_video_dir[i] + "_" + video_files[j];

			cout<< "Processing ["<< read_file<< "] writing to ["<< write_file.c_str()<< "]"<< endl;

			detected = searchVideopersonDetect( &orientation, &roi, read_file );
			cout<< "."<< endl;

			if ( detected )
			{
				cout<< "writing to: "<< write_file.c_str()<< endl;
				//rotate90Video_ffmpeg( read_file, write_file, orientation );
				rotate90Video( read_file, write_file, orientation, &roi );
				cout<< "Video written."<< endl;

				//cout<< "Enter a char to continue: ";
				//cin >> ch;
			}
			else
			{
				undetected.push_back( pair<string,string>( f_rpath, write_file) );
			}
			cout<< " ------- end for video search ------- "<< endl<< endl;
		}
	}

	cout<< "Number of videos with no face detection: "<< undetected.size()<< endl;
	cout<< "End of Person Detect."<< endl<< endl;

	return (int)(detected);

#endif

#endif

}


bool searchVideopersonDetect( Veed_Orient_T *orientation, Rect *roi, string video_avi)
{
	cout<< "Opening video ["<< video_avi.c_str()<< "] . . . " << endl;

	/*
	VideoBuffer< Rgb<byte> > *vb = open_video_source<Rgb<byte> >( video_avi.c_str() );
	VideoFileBuffer< Rgb<byte> > *video_buffer = dynamic_cast< VideoFileBuffer<Rgb<byte> >* >(vb);
	video_buffer->on_end_of_buffer( VideoBufferFlags::UnsetPending );

	if ( video_buffer != NULL )
	{
		cout<< "video avi file open succeeded"<< endl;
	}
	else
	{
		cout<< "Video avi file open failed!"<< endl;
		return -1;
	}

	SubImage< Rgb<byte> > *frame;
	*/

	cv::VideoCapture cap( video_avi.c_str() );

	cv::Mat m_bgr, m_detect;
	bool detected = false;
	size_t frame_count = 0, count = 0;
	size_t to_check = VEED_ORIENT_NUM_FRAMES_TO_CHECK * VEED_ORIENT_FRAME_HOP;

	while ( cap.read(m_bgr) && ( frame_count++ < to_check ) )
	{
		if ( frame_count % VEED_ORIENT_FRAME_HOP > 0 )
		{
			continue;
		}

		cout<< "got frame ["<< count++ << "] . . . ( "<< m_bgr.cols<< ", "<< m_bgr.rows<< ") "<< endl;
		m_detect = m_bgr.clone();

#ifdef DISPLAY_VIDEO_ORIENT90
		cv::imshow( veed_original_win, m_bgr );
		cv::imshow( veed_face_detect_win, m_bgr );
		waitKey(1);
#endif

		detected = personDetectWith90Rotations( orientation, roi, &m_detect );
		m_bgr.release();
		m_detect.release();
		if ( detected )
		{
			break;
		}

#ifdef DISPLAY_VIDEO_ORIENT90
		if ( frame_count < 2 )
		{
			//cv::waitKey(0);
		}
#endif

	}
	cap.release();

	if ( detected )
	{
		// display the whole clip with the detected correct rotation.
		cout<< "Face detected in frame rotated upright by "
			<< veed_orient_str[ (size_t)(*orientation) ]
			<< " degrees clockwise."<< endl;
		//cv::waitKey(0);
	}
	else
	{
		cout<< "No face detected in this video stream."<< endl;
	}

	return detected;
}

bool personDetectWith90Rotations( Veed_Orient_T *orient, Rect *roi, Mat *m_detect )
{
	Mat rot90_m, rot180_m, rot270_m;
	size_t flip_flag;
	bool detected;

	// 90 rotation
	cv::transpose( *m_detect, rot90_m );

	// 180 rotation
	flip_flag = ((*m_detect).rows > (*m_detect).cols) ? 0 : 1;
	cv::flip( *m_detect, rot180_m, flip_flag );		// for 180 rotation

	detected = faceDetect( m_detect, roi );
	if ( detected )
	{
		cout<< "Model detected in correct orientation with no rotation."<< endl;
		*orient = VEED_ORIENT_0;
#ifdef DISPLAY_VIDEO_ORIENT90
		cv::waitKey(0);
#endif
		return true;;
	}
	//cv::waitKey(0);

	flip_flag = (rot90_m.rows > rot90_m.cols) ? 0 : 1;
	cv::flip( rot90_m, rot270_m, flip_flag );			// for 270 rotation clockwise

	// for 90 rotation clockwise
	detected = faceDetect( &rot90_m, roi );
	if ( detected )
	{
		cout<< "Model detected in correct orientation with 90 degree clockwise rotation."<< endl;
		*orient = VEED_ORIENT_90;
#ifdef DISPLAY_VIDEO_ORIENT90
		cv::waitKey(0);
#endif
		rot90_m.release();
		return true;
	}
	rot90_m.release();
	//cv::waitKey(0);

	// for 270 rotation clockwise
	detected = faceDetect( &rot270_m, roi );
	if ( detected )
	{
		cout<< "Model detected in correct orientation with 270 degree clockwise rotation."<< endl;
		*orient = VEED_ORIENT_270;
#ifdef DISPLAY_VIDEO_ORIENT90
		cv::waitKey(0);
#endif
		rot270_m.release();
		return true;
	}
	rot270_m.release();
	//cv::waitKey(0);

	detected = faceDetect( &rot180_m, roi );
	if ( detected )
	{
		cout<< "Model detected in correct orientation with 180 degree rotation."<< endl;
		*orient = VEED_ORIENT_180;
#ifdef DISPLAY_VIDEO_ORIENT90
		cv::waitKey(0);
#endif
		rot180_m.release();
		return true;
	}
	rot180_m.release();

	cout<< "Face or person in upright position not detected in this frame."<< endl;
	return false;
}

// Assumption: There is only one model in the frame.
bool faceDetect( cv::Mat *frame, cv::Rect *roi )
{
	vector<Rect> faces;
	vector<Rect> bodies;
	vector<Rect> ubodies;
	Mat	frame_gray, frame_gray1;
	bool faceDetected = false;
	bool bodyDetected = false, personDetected = false;
	double area, a;
	Rect fullBody;
	Point bcentre;	// centre of the body

/*	------- Body crop no longer being done here.
 	int body_top_y = 1;					// top of frame
	int body_bottom_y = frame->rows;	// bottom of frame
	int body_left_x = 1;
	int body_right_x = frame->cols;
*/

	cvtColor( *frame, frame_gray1, CV_RGB2GRAY );
	equalizeHist( frame_gray1, frame_gray );
	frame_gray1.release();

	// detect full-body
	full_body_cascade.detectMultiScale( frame_gray, bodies, 1.1, 2, 18|9, Size(100,300));
	bodyDetected = ( bodies.size() > 0 );
	// Select the largest one as the model's full-body
	if ( bodyDetected )
	{
		area = 0;
		for( size_t j = 0; j < bodies.size(); j++ )
		{
			a = bodies[j].width * bodies[j].height;
			if ( a > area )
			{
				area = a;
				fullBody = bodies[j];
			}
		}

		bcentre.x = fullBody.x + fullBody.width*0.5;
		bcentre.y = fullBody.y + fullBody.height*0.5;

		//body_top_y = fullBody.y;
		//body_bottom_y = fullBody.y + fullBody.height;
	}


	// Detect face.
	//int face_body_top_y = frame->rows;
	Point face_centre;
	//int face_height, face_width;
	int selected_face = 0;
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size( 16, 24) );
	for( size_t i = 0; i < faces.size(); i++ )
	{
		Point fcentre( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );

		Mat faceROI = frame_gray( faces[i] );
		std::vector<Rect> eyes;
		// In each face, detect eyes
		eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size( 8, 8) );
		faceROI.release();

		// Detected at least one face with 2 eyes the top half of the face.
		faceDetected = faceDetected |  ( eyes.size() == 2 ) ;

		// draw face
		ellipse( *frame, fcentre, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

		// Person also detected if a face with at least one eye is found within the upper half of the frame.
		personDetected = personDetected ||
						( ( faceDetected | ( eyes.size() == 1 ) ) && ( fcentre.y < ( frame->rows * 0.5 ) ) );

		// Person detected if a face with at least one eye is found within the upper half of the full-body.
		personDetected = personDetected ||
					    ( ( faceDetected |  ( eyes.size() == 1 ) )
					    && ( fcentre.x > fullBody.x ) && ( fcentre.x < (fullBody.x + fullBody.width ))
				        && ( fcentre.y > fullBody.y ) && ( fcentre.y < (fullBody.x + fullBody.width*0.5 )) );



		// Detect eyes in the face.
		for( size_t j = 0; j < eyes.size(); j++ )
		{
			Point ecentre( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
			int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
			if ( faceDetected )
			{
				// We want to detect an upright face. i.e. Both eyes must be above the centre of the face in the frame.
				if ( fcentre.y < ecentre.y )
				{
					faceDetected = false;
				}
			}
			if ( faceDetected || personDetected )
			{
				// draw eyes
				circle( *frame, ecentre, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
			}
		}

		// If there are multiple faces, pick the topmost face.
		if ( i > 0 )
		{
			if ( faces[i].y < faces[selected_face].y )
			{
				selected_face = i;
			}
		}

		/* ------- Body crop no longer done here -----------------------
		if ( faceDetected || personDetected )
		{
			// draw full-body with adjustments for the face
			if ( bodyDetected )
			{
				//ellipse( *frame, bcentre, Size( fullBody.width*0.5, fullBody.height*0.5), 0, 0, 360, Scalar( 140, 0, 0 ), 4, 8, 0 );
				int new_body_top_y = fcentre.y - faces[i].height * 1.2;
				if ( new_body_top_y < 1 )
				{
					new_body_top_y = 1;
				}
				if ( new_body_top_y < face_body_top_y )	// select this face
				{
					face_body_top_y = new_body_top_y;
				}

			}
			else
			{
				face_centre = fcentre;
				face_height = faces[i].height;
				face_width = faces[i].width;
			}
		}
		*/
	}
	frame_gray.release();

	/* ------- Body Crop no longer being done here as face proportions
	 *
	if ( bodyDetected )
	{
		// draw full body
		Point bcentre( fullBody.x + fullBody.width*0.5, fullBody.y+ + fullBody.height*0.5 );
		body_top_y = bcentre.y - ( fullBody.height * 0.52);
		if ( faceDetected || personDetected )
		{
			body_top_y = face_body_top_y;
		}

		body_bottom_y = bcentre.y + ( fullBody.height * 0.55);
		if ( body_bottom_y > frame->rows )
		{
			body_bottom_y = frame->rows - 4;
		}
		body_left_x = bcentre.x - ( fullBody.width * 0.4);
		body_right_x = bcentre.x + ( fullBody.width * 0.4);

		rectangle( *frame, Point( body_left_x, body_top_y ),
					Point( body_right_x, body_bottom_y),
				   Scalar( 140, 0, 0 ), 4 );
	}
	else if ( faceDetected || personDetected )
	{
		body_top_y = face_centre.y - face_height;
		body_bottom_y = face_centre.y + 12 * face_height;
		if ( body_bottom_y > frame->rows )
		{
			body_bottom_y = frame->rows - 4;
		}

		body_left_x = face_centre.x -  2.25 * face_width;
		if ( body_left_x < 1 )
		{
			body_left_x = 4;
		}

		body_right_x = face_centre.x + 2.5 * face_width;
		if ( body_right_x > frame->cols )
		{
			body_right_x = frame->cols - 4;
		}

		rectangle( *frame, Point( body_left_x, body_top_y ),
						   Point( body_right_x, body_bottom_y),
						   Scalar( 140, 0, 0 ), 4 );

		cout<< "( "<< body_left_x<< ", "<< body_top_y<< ")      ";
		cout<< "( "<< body_right_x<< ", "<< body_bottom_y<< ")"<< endl;
		bodyDetected = true;
	}

	// Paint areas outside body rectangle white.
	if ( (faceDetected  ||  personDetected) && ( bodyDetected ) )
	{
		roi->x = body_left_x;
		roi->y = body_top_y;
		roi->width = body_right_x - body_left_x;
		roi->height = body_bottom_y - body_top_y;

		Scalar white( 255, 255, 255);
		(*frame)( Rect( 0, 0, roi->x, frame->rows ) ) = white;
		(*frame)( Rect( body_right_x, 0, frame->cols - body_right_x, frame->rows ) ) = white;
		(*frame)( Rect( roi->x, 0, body_right_x - roi->x, roi->y ) ) = white;
		(*frame)( Rect( roi->x, body_bottom_y, body_right_x - roi->x, frame->rows - body_bottom_y ) )= white;

	}
	*/


#ifdef DISPLAY_VIDEO_ORIENT90
	imshow( veed_face_detect_win, *frame );
#endif

	// return the selected face as the ROI
	if ( ( roi != NULL ) && (faces.size() > 0) )
	{
		roi->x = faces[selected_face].x;
		roi->y = faces[selected_face].y;
		roi->width = faces[selected_face].width;
		roi->height = faces[selected_face].height;
	}

	return faceDetected  ||  personDetected;
}

/*
bool bodyDetect( cv::Mat *frame )
{
	vector<Rect> bodies;
	vector<Rect> ubodies;
	Mat	frame_gray;
	bool oriented = false;
	bool bodyDetected = false, ubodyDetected = false;
	double area, a;
	Rect fullBody;

	cvtColor( *frame, frame_gray, CV_RGB2GRAY );
	equalizeHist( frame_gray, frame_gray );

	// detect full-body
	full_body_cascade.detectMultiScale(frame_gray, bodies, 1.1, 2, 18|9, Size(300,500));
	bodyDetected = ( bodies.size() > 0 );
	if ( bodies.size() > 0 )	// Select the largest full-body
	{
		area = 0;
		for( size_t j = 0; j < bodies.size(); j++ )
		{
			a = bodies[j].width * bodies[j].height;
			if ( a > area )
			{
				area = a;
				fullBody = bodies[j];
			}
		}
	}


	// Failing Face Detect do an upper body search.

	// detect upper-body
//	upper_body_cascade.detectMultiScale(frame_gray, ubodies, 1.1, 2, 18|9, Size(200,400));
	ubodyDetected = ( ubodies.size() == 1 );	// There's just one, to prevent false detections.


	if ( bodyDetected && ubodyDetected )
	{
		Point ubcentre( ubodies[0].x + ubodies[0].width*0.5, ubodies[0].y+ + ubodies[0].height*0.5 );

		oriented =  ( fullBody.height > fullBody.width )	// full-body is vertical
				 && ( ( ubodies[0].y + ubodies[0].height ) < ( fullBody.y + fullBody.height*0.5 ) )
															// upper body is fully above the centre of the full-body
				 // Upper body center is within the top half ov the full body.
				 && ( ubcentre.y > fullBody.y )
				 && ( ubcentre.x > fullBody.x )
				 && ( ubcentre.x < (fullBody.x + fullBody.width ) );

		if ( oriented )
		{
			// draw full body
			Point bcentre( fullBody.x + fullBody.width*0.5, fullBody.y+ + fullBody.height*0.5 );
			rectangle( *frame, Point( bcentre.x - ( fullBody.width * 0.4), bcentre.y - ( fullBody.height * 0.52) ),
					   Point( bcentre.x + ( fullBody.width * 0.4), bcentre.y + ( fullBody.height * 0.52) ),
					   Scalar( 140, 0, 0 ), 4 );
			// draw upper-body
			rectangle( *frame, Point( ubcentre.x - ( ubodies[0].width * 0.4), ubcentre.y - ( ubodies[0].height * 0.4) ),
					   Point( ubcentre.x + ( ubodies[0].width * 0.4), ubcentre.y + ( ubodies[0].height / 2) ),
					   Scalar( 0, 140, 0 ), 4 );
		}
	}

	imshow( veed_face_detect_win, *frame );
	return oriented;
}
*/


void rotate90Video_ffmpeg( string video_avi, string write_file, Veed_Orient_T orient )
{
	string cmd = string("ffmpeg -y -i ") + video_avi + " -b:v 10M ";

	switch (orient)
	{
		case VEED_ORIENT_0   : cmd += write_file;
							   break;

		case VEED_ORIENT_90  : cmd += ( "-vf \"transpose=1\" " + write_file );
							   break;

		case VEED_ORIENT_270 : cmd += ( "-vf \"transpose=2\" " +  write_file );
							   break;

		case VEED_ORIENT_180 : cmd += ( "-vf \"transpose=2,transpose=2\" " + write_file);
							   break;
	}

	cout<< "cmd: "<< cmd.c_str()<< endl;
	system( cmd.c_str() );
}

void rotate90Video( string video_avi, string write_file, Veed_Orient_T orient, Rect *roi )
{
	Mat m_bgr;
	size_t frame_count =0;
	bool b;

	cout<< "Opening video for write with rotation ["<< video_avi.c_str()<< "] . . . " << endl;
	cv::VideoCapture cap( video_avi.c_str() );
	cv::VideoWriter writer;

	while (1)
	{
		try
		{
			if ( cap.read(m_bgr) == false )
			{
				cout<< frame_count<< " frames"<< endl;
				break;
			}
		}
		catch (...)
		{
			cout<< frame_count<< " frames"<< endl;
			break;
		}
		++frame_count;
		//if ( frame_count == 300 ) break;

		if ( orient == VEED_ORIENT_90 )
		{
			cv::transpose( m_bgr, m_bgr );
		}
		else if ( orient == VEED_ORIENT_180 )
		{
			cv::flip( m_bgr, m_bgr, 1);
		}
		else if ( orient == VEED_ORIENT_270 )
		{
			cv::transpose( m_bgr, m_bgr );
			cv::flip( m_bgr, m_bgr, 0);
		}

		if ( roi != NULL )
		{
			removeOutsideROI( &m_bgr, roi );
		}

		if ( frame_count <= 1 )
		{
			//b = writer.open( write_file, CV_FOURCC('X','2','6','4'), 25, m_bgr.size() );
			b = writer.open( write_file, CV_FOURCC('M','J','P','G'), 25, m_bgr.size() );
			if ( b )
			{
				cout<< "Video file ["<< write_file<< "] opened for writing "
					<< " with "<< veed_orient_str[(unsigned int)orient]<< " degree rotation."<< endl;
			}
			else
			{
				cout<< "Video file ["<< write_file<< "] failed to open for writing!"<< endl;
				//char ch; cin >> ch;
				return;
			}
		}
		writer.write( m_bgr );
	}
	writer.release();
	cap.release();
	cout<< "Video written."<< endl;
	usleep(100000);
}


void removeOutsideROI( Mat *frame, Rect *roi)
{
	if ( roi == NULL )
	{
		return;
	}

	int right_x = roi->x + roi->width;
	int bottom_y = roi->y + roi->height;

	Scalar white( 255, 255, 255);
	(*frame)( Rect( 0, 0, roi->x, frame->rows ) ) = white;
	(*frame)( Rect( right_x, 0, frame->cols - right_x, frame->rows ) ) = white;
	(*frame)( Rect( roi->x, 0, right_x - roi->x, roi->y ) ) = white;
	(*frame)( Rect( roi->x, bottom_y, right_x - roi->x, frame->rows - bottom_y ) )= white;
}

Mat frame_rotate(Mat *src, double angle)
{
    Mat dst;
    Point2f pt((*src).cols/2., (*src).rows/2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    warpAffine( *src, dst, r, Size((*src).cols, (*src).rows));
    return dst;
}

Mat frame_rotate90( Mat *src )
{
	Mat frame = (*src).clone();
	transpose(frame, frame);
	return frame;
}

Mat frame_rotate180( Mat *src)
{
	Mat frame = (*src).clone();
	flip(frame, frame, 1);
	return frame;
}



/*-------------------------------------------------------------------------
 * Test functions.
 */


void test()
{
}

