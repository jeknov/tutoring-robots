#include "Player.hpp"

Kinect::Kinect()
{
	colorData = new byte[COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS];
	depthData = new byte[DEPTH_WIDTH*DEPTH_HEIGHT*DEPTH_CHANNELS];

	visualizeColor = true;
	visualizeDepth = true;
	visualizeSkeleton = true;

	image = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, COLOR_CHANNELS);

	//trial = "0";
}

Kinect::~Kinect()
{
	delete[] colorData;
	delete[] depthData;

	sensor->NuiShutdown();
}

int Kinect::init(DWORD sensorFlags, DWORD skeletonFlags)
{
	int sensorCount;

	result = NuiGetSensorCount(&sensorCount);
	if(FAILED(result))
		return result;

	// catch no sensor status and throw
	if(sensorCount == 0)
		exit(1);

	for(int i = 0; i < sensorCount; i++)
	{
		result = NuiCreateSensorByIndex(i, &tmpSensor);
		if(FAILED(result))
			continue;

		result = tmpSensor->NuiStatus();
		if(result == S_OK)
		{
			sensor = tmpSensor;
			break;
		}

		tmpSensor->Release();
	}

	if(sensor != NULL)
	{
		result = sensor->NuiInitialize(sensorFlags); 
		if(SUCCEEDED(result))
		{
			result = sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, NULL, &colorStreamHandle);
			result = sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, 0, 2, NULL, &depthStreamHandle);

			result = sensor->NuiSkeletonTrackingEnable(NULL, skeletonFlags);
		}
	}

	return result;
}

void Kinect::run()
{
	Visualizer visualizer;
	HeadTracker headTracker;

	result = headTracker.init();

	while(true)
	{
		result = sensor->NuiImageStreamGetNextFrame(colorStreamHandle, 0, &colorFrame);
		if(SUCCEEDED(result))
		{
			result = colorFrame.pFrameTexture->LockRect(0, &colorRect, NULL, 0);
			if(SUCCEEDED(result) && colorRect.Pitch != 0)
			{
				if(visualizeColor)
					visualizer.drawColor(colorRect.pBits);
			}

			headTracker.updateColorBuffer(colorRect.pBits, COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS);

			colorFrame.pFrameTexture->UnlockRect(0);
			sensor->NuiImageStreamReleaseFrame(colorStreamHandle, &colorFrame);
		}

		result = sensor->NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);
		if(SUCCEEDED(result))
		{
			result = depthFrame.pFrameTexture->LockRect(0, &depthRect, NULL, 0);
			if(SUCCEEDED(result) && depthRect.Pitch != 0)
			{
				if(visualizeDepth)
					visualizer.drawDepth(depthRect.pBits);
			}

			headTracker.updateDepthBuffer(depthRect.pBits, DEPTH_WIDTH*DEPTH_HEIGHT*DEPTH_CHANNELS);

			depthFrame.pFrameTexture->UnlockRect(0);
			sensor->NuiImageStreamReleaseFrame(depthStreamHandle, &depthFrame);
		}

		result = sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
		if(SUCCEEDED(result))
		{
			sensor->NuiTransformSmooth(&skeletonFrame, NULL);

			for(int i = 0; i < NUI_SKELETON_COUNT; i++)
			{
				if(skeletonFrame.SkeletonData[i].eTrackingState != NUI_SKELETON_TRACKED)
					continue;

				if(visualizeSkeleton)
					visualizer.drawSkeleton(skeletonFrame.SkeletonData[i]);
			}
		}

		headTracker.track();

		if(cvWaitKey(1) == 27)
			break;
	}
}

void Kinect::record(string outDirPath)
{
	//FILE* colorFile;
	FILE* depthFile;
	FILE* skeletonFile;
	FILE* faceFile;

	errno_t err;
	timeval frameTime;

	timeOfDay(&frameTime, NULL);

	/*string colorFilePath = outDirPath + "color_" + trial;
	if((err = fopen_s(&colorFile, colorFilePath.c_str(), "wb")) != 0)
	{
		printf("'COLOR DATA' file was not created\n" );

		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}*/

	string timeVal;
	stringstream strstream;
	strstream << frameTime.tv_sec;
	strstream >> timeVal;

	string depthFilePath = outDirPath + "depth_" + timeVal;
	if((err = fopen_s(&depthFile, depthFilePath.c_str(), "wb")) != 0)
	{
		printf("'DEPTH DATA' file was not created\n" );
	
		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}

	string skeletonFilePath = outDirPath + "skeleton_" + timeVal;
	if((err = fopen_s(&skeletonFile, skeletonFilePath.c_str(), "wb")) != 0)
	{
		printf("'SKELETON DATA' file was not created\n" );
	
		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}

	string faceFilePath = outDirPath + "face_" + timeVal;
	if((err = fopen_s(&faceFile, faceFilePath.c_str(), "wb")) != 0)
	{
		printf("'FACE DATA' file was not created\n" );

		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}

	Visualizer visualizer;
	HeadTracker headTracker;

	result = headTracker.init();

	while(true)
	{
		timeOfDay(&frameTime, NULL);

		result = sensor->NuiImageStreamGetNextFrame(colorStreamHandle, 0, &colorFrame);
		if(SUCCEEDED(result))
		{
			result = colorFrame.pFrameTexture->LockRect(0, &colorRect, NULL, 0);
			if(SUCCEEDED(result) && colorRect.Pitch != 0)
			{
				memcpy(colorData, colorRect.pBits, colorFrame.pFrameTexture->BufferLen());

				if(visualizeColor)
					visualizer.drawColor(colorRect.pBits);
			}

			headTracker.updateColorBuffer(colorRect.pBits, COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS);

			colorFrame.pFrameTexture->UnlockRect(0);
			sensor->NuiImageStreamReleaseFrame(colorStreamHandle, &colorFrame);
		}

		result = sensor->NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);
		if(SUCCEEDED(result))
		{
			result = depthFrame.pFrameTexture->LockRect(0, &depthRect, NULL, 0);
			if(SUCCEEDED(result) && depthRect.Pitch != 0)
			{
				memcpy(depthData, depthRect.pBits, depthFrame.pFrameTexture->BufferLen());

				if(visualizeDepth)
					visualizer.drawDepth(depthRect.pBits);
			}

			headTracker.updateDepthBuffer(depthRect.pBits, DEPTH_WIDTH*DEPTH_HEIGHT*DEPTH_CHANNELS);

			depthFrame.pFrameTexture->UnlockRect(0);
			sensor->NuiImageStreamReleaseFrame(depthStreamHandle, &depthFrame);
		}

		result = sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
		if(SUCCEEDED(result))
		{
			sensor->NuiTransformSmooth(&skeletonFrame, NULL);

			for(int i = 0; i < NUI_SKELETON_COUNT; i++)
			{
				if(skeletonFrame.SkeletonData[i].eTrackingState != NUI_SKELETON_TRACKED)
					continue;

				if(visualizeSkeleton)
					visualizer.drawSkeleton(skeletonFrame.SkeletonData[i]);
			}
		}

		//fwrite(&frameTime, sizeof(timeval), 1, colorFile);
		//fwrite(colorData, sizeof(byte),  COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS, colorFile);

		fwrite(&frameTime, sizeof(timeval), 1, depthFile);
		fwrite(depthData, sizeof(byte), DEPTH_WIDTH*DEPTH_HEIGHT*DEPTH_CHANNELS, depthFile);

		fwrite(&frameTime, sizeof(timeval), 1, skeletonFile);
		fwrite(&skeletonFrame, sizeof(skeletonFrame), 1, skeletonFile);

		headTracker.saveData(frameTime, faceFile);

		headTracker.track();

		if(cvWaitKey(1) == 27)
			break;
	}

	/*if(colorFile != NULL)
		fclose(colorFile);*/

	if(depthFile != NULL)
		fclose(depthFile);

	if(skeletonFile != NULL)
		fclose(skeletonFile);

	if(faceFile != NULL)
		fclose(faceFile);
}

void Kinect::play(string colorFilePath, string depthFilePath, string skeletonFilePath, string faceFilePath)
{
	Visualizer visualizer;

	//FILE* colorFile;
	FILE* depthFile;
	FILE* skeletonFile;
	FILE* faceFile;

	NUI_SKELETON_FRAME skeletonFrame;
	
	HRESULT hr;
	RECT faceRect;
	FLOAT faceScale;
	FLOAT faceRotation[3];
	FLOAT faceTranslation[3];
	UINT numPoints;
	UINT numCoefficients;

	FT_VECTOR2D* shapePoints;
	FLOAT* shapeCoefficients;
	FLOAT* animCoefficients;
	CvScalar color = cvScalar(255);
	int thickness = 1;

	errno_t err;
	timeval frameTime;

	/*if((err = fopen_s(&colorFile, colorFilePath.c_str(), "rb")) != 0)
	{
		printf("'COLOR DATA' file was not loaded\n" );

		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}*/

	if((err = fopen_s(&depthFile, depthFilePath.c_str(), "rb")) != 0)
	{
		printf("'DEPTH DATA' file was not loaded\n" );

		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}

	if((err = fopen_s(&skeletonFile, skeletonFilePath.c_str(), "rb")) != 0)
	{
		printf("'SKELETON DATA' file was not loaded\n" );

		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}

	if((err = fopen_s(&faceFile, faceFilePath.c_str(), "rb")) != 0)
	{
		printf("'FACE DATA' file was not loaded\n" );

		cout << "press ENTER to quit...";
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		exit(-1);
	}

	while(true)
	{
		/*if(!feof(colorFile))
		{
			fread(&frameTime, sizeof(timeval), 1, colorFile);
			fread(colorData, sizeof(byte), COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS, colorFile);

			memcpy(image->imageData, colorData, COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS);

			if(visualizeColor)
				visualizer.drawColor(colorData);
		}*/

		if(!feof(depthFile))
		{
			fread(&frameTime, sizeof(timeval), 1, depthFile);
			fread(depthData, sizeof(byte), DEPTH_WIDTH*DEPTH_HEIGHT*DEPTH_CHANNELS, depthFile);

			if(visualizeDepth)
				visualizer.drawDepth(depthData);
		}

		if(!feof(skeletonFile))
		{
			fread(&frameTime, sizeof(timeval), 1, skeletonFile);
			fread(&skeletonFrame, sizeof(skeletonFrame), 1, skeletonFile);

			for(int i = 0; i < NUI_SKELETON_COUNT; i++)
			{
				if(skeletonFrame.SkeletonData[i].eTrackingState != NUI_SKELETON_TRACKED)
					continue;

				if(visualizeSkeleton)
					visualizer.drawSkeleton(skeletonFrame.SkeletonData[i]);
			}
		}

		if(!feof(faceFile))
		{
			fread(&frameTime, sizeof(timeval), 1, faceFile);
			fread(&hr, sizeof(hr), 1, faceFile);
			fread(&faceRect, sizeof(faceRect), 1, faceFile);

	
			fread(&faceScale, sizeof(faceScale), 1, faceFile);
			fread(faceRotation, sizeof(*faceRotation), 3, faceFile);
			fread(faceTranslation, sizeof(*faceTranslation), 3, faceFile);

			fread(&numPoints, sizeof(numPoints), 1, faceFile);
			fread(shapePoints, sizeof(*shapePoints), numPoints, faceFile);

			fread(&numCoefficients, sizeof(numCoefficients), 1, faceFile);
			fread(shapeCoefficients, sizeof(*shapeCoefficients), numCoefficients, faceFile);

			LONG width = abs(faceRect.left - faceRect.right);
			LONG height = abs(faceRect.top - faceRect.bottom);

			POINT leftTop = {faceRect.left + width/12, faceRect.top};
			POINT rightTop = {faceRect.right - width/12, faceRect.top};
			POINT leftBottom = {faceRect.left + width/12, faceRect.bottom - 1};
			POINT rightBottom = {faceRect.right - width/12, faceRect.bottom - 1};

			cout << leftTop.x << "," << leftTop.y << endl;
			cvLine(image, cvPoint(leftTop.x, leftTop.y), cvPoint(rightTop.x, rightTop.y), color, thickness);
			cvLine(image, cvPoint(rightTop.x, rightTop.y), cvPoint(rightBottom.x, rightBottom.y), color, thickness);
			cvLine(image, cvPoint(rightBottom.x, rightBottom.y), cvPoint(leftBottom.x, leftBottom.y), color, thickness);
			cvLine(image, cvPoint(leftBottom.x, leftBottom.y), cvPoint(leftTop.x, leftTop.y), color, thickness);
		}

		else
			break;
		
		if(cvWaitKey(30) == 27)
			break;

		cvShowImage("face", image);
	}

	/*if(colorFile != NULL)
		fclose(colorFile);*/

	if(depthFile != NULL)
		fclose(depthFile);

	if(skeletonFile != NULL)
		fclose(skeletonFile);
}

void Kinect::timeOfDay(timeval* p, void* tz)
{
	union
	{
		/* time since 1 Jan 1601 in 100ns units */
		long long ns100;
		FILETIME ft;
	} now;

	GetSystemTimeAsFileTime(&(now.ft));
	p->tv_usec = (long)((now.ns100/10LL)%1000000LL);
	p->tv_sec = (long)((now.ns100-(116444736000000000LL))/10000000LL);
}