#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio/videoio_c.h>


using namespace std;
using namespace cv;


int main( int argc, char *argv[] )
{
	/*
	for ( size_t i = 0; i < 10000; ++i )
	{
		cv::Mat m( 1080, 1920, CV_8UC3 );
		cv::Mat m1, m2;

		cv::transpose( m, m1 );
		cv::flip( m, m2, 0 );
	}
	*/

	namedWindow("Original", WINDOW_NORMAL | WINDOW_KEEPRATIO );
	cv::moveWindow( "Original", 1260, 0);
	cv::resizeWindow( "Original", 640, 910 );

	for ( int f = 1; f < 18; ++f )
	{
		stringstream ss,ws, ws1;
		ss << "/home/jenni/tmp/" << f << ".MOV";
		ws << "/home/jenni/tmp/"<< f << "_w.mp4";
		ws1 << "/home/jenni/tmp/"<< f << "_w.MOV";

		cout<< "opening video : "<< ss.str()<< endl;
		cv::VideoCapture cap( ss.str() );
		cout<< "video opened"<< endl;

		VideoWriter writer, wr1;
		cv::Mat m_bgr;

		for ( size_t i = 0; i < 10000; ++i )
		{
			try
			{
				if ( cap.read(m_bgr) == false )
				{
					cout<< i<< " frames"<< endl;
					break;
				}
			}
			catch (...)
			{
				cout<< i<< " frames"<< endl;
				break;
			}

			cv::Mat m1;
			imshow( "Original", m_bgr );
			waitKey(1);

			cv::transpose( m_bgr, m1 );
			cv::flip( m1, m1, 0 );

			if ( i == 0 )
			{
				writer.open( ws.str(), CV_FOURCC('X','2','6','4'), 25, m_bgr.size() );
				cout<< "mp4 writer opened."<< endl;
				wr1.open( ws1.str(), CV_FOURCC('X','2','6','4'), 25, m_bgr.size() );
				cout<< "MOV writer opened."<< endl;
			}
			writer.write( m_bgr );
			wr1.write( m_bgr );

		}
		writer.release();
		wr1.release();
		cap.release();

		waitKey(0);
	}


	return 0;
}
