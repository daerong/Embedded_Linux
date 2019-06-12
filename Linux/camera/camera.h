
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>

//#include <linux/videodev2.h>
#include <linux/fb.h>
//////#include <linux/mxcfb.h>

//Add opencv header
//#include <opencv2/opencv.hpp>
#include "./include/opencv2/highgui/highgui.hpp"
#include "./include/opencv/highgui.h"
#include "./include/opencv/cv.h"
#include "./include/opencv2/core/core.hpp"


//define cv with opencv
using namespace cv;
using namespace std;

typedef unsigned int U32;
typedef unsigned char U16;
typedef struct _user_fb {
	int xres;
	int yres;
	int bpps;
} user_fb;
unsigned int makepixel(U32 r, U32 g, U32 b){
	//return (U32) ((r<<16 | g<<8) |b);
	return (U32)((b<<16)|(g<<8)|(r));
}


