#ifndef __CRYPT_OHLCV_FMT_H__
#define __CRYPT_OHLCV_FMT_H__

#include <config.h>


#define VEED_IMAGE_DATA_DIR					"/home/data/veed/images"
#define VEED_WRITE_DIR						"/home/data/veed/write"

#define VEED_MODEL_ROOT_DIR					"/usr/local/opt/opencv/data/"
#define VEED_MODEL_HAARCASCADE_DIR			"/usr/local/opt/opencv/data/haarcascades/"

#define VEED_DEFAULT_WRITE_FRAME_RATE		25

#define VEED_ORIENT_NUM_FRAMES_TO_CHECK		150
#define VEED_ORIENT_FRAME_HOP				2


enum Veed_Person_Detection_T
{
	VEED_FACE_DETECT = 0,
	VEED_FULL_BODY_DETECT,
	VEED_UPPER_BODY_DETECT,
	VEED_LOWER_BODY_DETECT
};


#endif
