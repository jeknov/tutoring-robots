#include "Player.hpp"

Visualizer::Visualizer()
{
	cvColorImage = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, COLOR_CHANNELS);
	cvVideoImage = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, COLOR_CHANNELS);
	cvDepthImage = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT), IPL_DEPTH_8U, COLOR_CHANNELS);

	cvInitFont(&wordSignText, CV_FONT_HERSHEY_SIMPLEX, 2, 2, 0, 3, CV_AA);
	cvInitFont(&controlText, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, CV_AA);
	cvInitFont(&videoMissingText, CV_FONT_HERSHEY_SIMPLEX, 2, 2, 0, 3, CV_AA);
}

Visualizer::~Visualizer()
{
	cvDestroyAllWindows();

	if(cvColorImage != NULL)
		cvReleaseImage(&cvColorImage);

	if(cvDepthImage != NULL)
		cvReleaseImage(&cvDepthImage);
}

void Visualizer::drawColor(byte* colorData)
{
	memcpy(cvColorImage->imageData, colorData, COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS);

	cvNamedWindow("color", 1);
	cvMoveWindow("color", 0, 0);
	cvShowImage("color", cvColorImage);
}

void Visualizer::drawDepth(byte* depthData)
{
	byte* cv_ptr = reinterpret_cast<byte*>(cvDepthImage->imageData);
	const ushort* b_ptr = reinterpret_cast<const ushort*>(depthData);
	const ushort* b_end = b_ptr + DEPTH_WIDTH*DEPTH_HEIGHT;

	while(b_ptr < b_end)
	{
		ushort depth = NuiDepthPixelToDepth(*b_ptr);
		byte intensity = static_cast<byte>(depth%256);

		*(cv_ptr++) = intensity;
		*(cv_ptr++) = intensity;
		*(cv_ptr++) = intensity;
		++cv_ptr;
		++b_ptr;
	}

	cvNamedWindow("depth", 1);
	//cvMoveWindow("depth", 0, 0);
	cvShowImage("depth", cvDepthImage);
}

void Visualizer::drawBone(const NUI_SKELETON_DATA& skeleton, const CvPoint* points, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
{
	NUI_SKELETON_POSITION_TRACKING_STATE joint0State = skeleton.eSkeletonPositionTrackingState[joint0];
	NUI_SKELETON_POSITION_TRACKING_STATE joint1State = skeleton.eSkeletonPositionTrackingState[joint1];

	if(joint0State == NUI_SKELETON_POSITION_NOT_TRACKED || joint1State == NUI_SKELETON_POSITION_NOT_TRACKED)
		return;

	if(joint0State == NUI_SKELETON_POSITION_INFERRED && joint1State == NUI_SKELETON_POSITION_INFERRED)
		return;

	if(joint0State == NUI_SKELETON_POSITION_TRACKED && joint1State == NUI_SKELETON_POSITION_TRACKED)
		cvLine(cvColorImage, points[joint0], points[joint1], CV_RGB(255, 0, 255), 2, 32);
	else
		cvLine(cvColorImage, points[joint0], points[joint1], CV_RGB(255, 0, 255), 2, 32);
}

void Visualizer::drawSkeleton(const NUI_SKELETON_DATA& skeleton)
{
	CvPoint points[NUI_SKELETON_POSITION_COUNT];
	long depthX, depthY;
	long colorX, colorY;
	ushort depthZ;
	
	for(int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		NuiTransformSkeletonToDepthImage(skeleton.SkeletonPositions[i], &depthX, &depthY, &depthZ);
		NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, 0, depthX, depthY, depthZ, &colorX, &colorY);
		
		points[i].x = colorX;
		points[i].y = colorY;
	}

	// torso
	drawBone(skeleton, points, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	// left arm
	drawBone(skeleton, points, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	// right arm
	drawBone(skeleton, points, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	// left leg
	drawBone(skeleton, points, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	// right leg
	drawBone(skeleton, points, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	drawBone(skeleton, points, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	for(int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		if(skeleton.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED)
			cvCircle(cvColorImage, points[i], 5, CV_RGB(0, 255, 0), 2, 32);

		else if(skeleton.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED)
			cvCircle(cvColorImage, points[i], 5, CV_RGB(255, 0, 0), 2, 32);
	}

	cvShowImage("color", cvColorImage);
}