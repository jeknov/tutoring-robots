// windows
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef INC_OLE2
#define INC_OLE2
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// server
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1101"

#include <winsock2.h>
#include <ws2tcpip.h>

// kinect
#include <NuiApi.h>
#include <FaceTrackLib.h>

// opencv
//#include <opencv2\opencv.hpp>
#include <C:\OpenCV\include\opencv2\opencv.hpp>

// namespaces
using namespace cv;
using namespace std;

// tivoli
#include "Kinect.hpp"
#include "HeadTracker.hpp"
#include "Visualizer.hpp"

// globals
#ifndef COLOR_WIDTH
#define COLOR_WIDTH 640
#endif

#ifndef COLOR_HEIGHT
#define COLOR_HEIGHT 480
#endif

#ifndef COLOR_CHANNELS
#define COLOR_CHANNELS 4
#endif

#ifndef DEPTH_WIDTH
#define DEPTH_WIDTH 320
#endif

#ifndef DEPTH_HEIGHT
#define DEPTH_HEIGHT 240
#endif

#ifndef DEPTH_CHANNELS
#define DEPTH_CHANNELS 2
#endif

#ifndef PI
#define PI 3.14159265
#endif