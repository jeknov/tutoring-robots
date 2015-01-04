#include "Player.hpp"

HeadTracker::HeadTracker()
{
	colorBuffer = NULL;
	depthBuffer = NULL;
	faceTrackerResult = NULL;

	faceTracked = false;

	faceImage = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HEIGHT), IPL_DEPTH_8U, COLOR_CHANNELS);
}

HeadTracker::~HeadTracker()
{
	if(!colorBuffer == NULL)
		colorBuffer->Release();

	if(!depthBuffer == NULL)
		depthBuffer->Release();

	if(!faceTracker == NULL)
		faceTracker->Release();
	
	if(!faceTrackerResult == NULL)
		faceTrackerResult->Release();
}

int HeadTracker::init()
{
	HRESULT hr = S_OK;
	POINT viewOffset = {0, 0};

	hr = getVideoConfig(&videoConfig);
	hr = getDepthConfig(&depthConfig);

	faceTracker = FTCreateFaceTracker();
	hr = faceTracker->Initialize(&videoConfig, &depthConfig, NULL, NULL); 
	if(FAILED(hr))
	{
		printf("failed to create face tracker!\n");
		return -1;
	}

	colorBuffer = FTCreateImage();
	depthBuffer = FTCreateImage();

	if(colorBuffer == NULL || depthBuffer == NULL)
	{
		printf("failed to create face tracker buffers!\n");
		return -1;
	}
	
	colorBuffer->Allocate(videoConfig.Width, videoConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8);
	depthBuffer->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3);

	sensorData.pVideoFrame = colorBuffer;
	sensorData.pDepthFrame = depthBuffer;
	sensorData.ViewOffset = viewOffset;
	sensorData.ZoomFactor = 1.0f;

	hr = faceTracker->CreateFTResult(&faceTrackerResult);
	if(FAILED(hr))
	{
		printf("failed to create face tracker result!\n");
		return -1;
	}

	return 0;
}

int HeadTracker::track()
{
	HRESULT hr;

	if(!faceTracked)
	{
		hr = faceTracker->StartTracking(&sensorData, NULL, NULL, faceTrackerResult);
		if(SUCCEEDED(hr) && SUCCEEDED(faceTrackerResult->GetStatus()) && SUCCEEDED(faceTrackerResult->GetFaceRect(&faceRect)))
			faceTracked = true;
		else
			faceTracked = false;
	}
	else
	{
		hr = faceTracker->ContinueTracking(&sensorData, NULL, faceTrackerResult);
		if(SUCCEEDED(hr) && SUCCEEDED(faceTrackerResult->GetStatus()) && SUCCEEDED(faceTrackerResult->GetFaceRect(&faceRect)))
			faceTracked = true;
		else
			faceTracked = false;
	}

	drawFace(cvScalar(255), 1, *faceImage);
	return 0;
}

HRESULT HeadTracker::getVideoConfig(FT_CAMERA_CONFIG* videoConfig)
{
	if(!videoConfig)
		return E_POINTER;

	videoConfig->FocalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	//videoConfig->FocalLength = 500.0f;
	videoConfig->Width = COLOR_WIDTH;
	videoConfig->Height = COLOR_HEIGHT;

	return(S_OK);
}

HRESULT HeadTracker::getDepthConfig(FT_CAMERA_CONFIG* depthConfig)
{
	if(!depthConfig)
		return E_POINTER;

	depthConfig->FocalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	depthConfig->Width = DEPTH_WIDTH;
	depthConfig->Height = DEPTH_HEIGHT;

	return S_OK;
}

int HeadTracker::updateColorBuffer(const byte* data, const int length)
{
	if(!colorBuffer == NULL)
	{
		memcpy(colorBuffer->GetBuffer(), data, std::min(colorBuffer->GetBufferSize(), static_cast<UINT>(length)));
		memcpy(faceImage->imageData, data, COLOR_WIDTH*COLOR_HEIGHT*COLOR_CHANNELS);
	}

	return 0;
}

int HeadTracker::updateDepthBuffer(const byte* data, const int length)
{
	if(!depthBuffer == NULL)
		memcpy(depthBuffer->GetBuffer(), data, std::min(depthBuffer->GetBufferSize(), static_cast<UINT>(length)));

	return 0;
}

int HeadTracker::drawFace(const CvScalar color, int thickness, IplImage& image)
{
	UINT numPoints;
	//UINT animUnitCount;

	if(faceTracked)
	{
		if(SUCCEEDED(faceTrackerResult->GetStatus()))
			faceTrackerResult->Get2DShapePoints(&shapePoints, &numPoints);

		if(numPoints < 86)
			return -1;

		//printf("num points: %i", numPoints);
		CvPoint* drawPoints = new CvPoint[numPoints];

		for(UINT pt = 0; pt < numPoints; pt++)
		{
			drawPoints[pt].x = LONG(shapePoints[pt].x + 0.5f);
			drawPoints[pt].y = LONG(shapePoints[pt].y + 0.5f);
		}

		// left eye contour
		for(UINT pt = 0; pt < 8; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt+1)%8], color, thickness);

		// right eye contour
		for(UINT pt = 8; pt < 16; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-8+1)%8+8], color, thickness);

		// left eye brow contour
		for(UINT pt = 16; pt < 26; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-16+1)%10+16], color, thickness);

		// right eye brow contour
		for(UINT pt = 26; pt < 36; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-26+1)%10+26], color, thickness);

		// nose contour
		for(UINT pt = 36; pt < 48; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-36+1)%12+36], color, thickness);

		// outer mouth contour
		for(UINT pt = 48; pt < 60; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-48+1)%12+48], color, thickness);

		// inner mouth contour
		for(UINT pt = 60; pt < 68; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-60+1)%8+60], color, thickness);

		// face contour
		for(UINT pt = 68; pt < 87; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-68+1)%19+68], color, thickness);

		// left eye center
		cvCircle(&image, drawPoints[87], 5, CV_RGB(255, 0, 0), -1, 32, 0);

		// right eye center
		cvCircle(&image, drawPoints[88], 5, CV_RGB(0, 0, 255), -1, 32, 0);

		// nose tip
		cvCircle(&image, drawPoints[89], 5, CV_RGB(255, 255, 0), -1, 32, 0);

		// left eye brow top
		for(UINT pt = 90; pt < 95; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-90+1)%5+90], CV_RGB(255, 0, 0), thickness);

		// right eye brow top
		for(UINT pt = 95; pt < 100; pt++)
			cvLine(&image, drawPoints[pt], drawPoints[(pt-95+1)%5+95], CV_RGB(0, 0, 255), thickness);

		cvCircle(&image, drawPoints[100], 50, CV_RGB(255, 255, 0), -1, 32, 0);

		//printf("x = %i, y = %i\n\n", drawPoints[89].x, drawPoints[89].y);
		delete[] drawPoints;
	}

	if(SUCCEEDED(faceTrackerResult->GetFaceRect(&faceRect)))
	{
		/*POINT leftTop = {faceRect.left, faceRect.top};
		POINT rightTop = {faceRect.right - 1, faceRect.top};
		POINT leftBottom = {faceRect.left, faceRect.bottom - 1};
		POINT rightBottom = {faceRect.right - 1, faceRect.bottom - 1};*/

		LONG width = abs(faceRect.left - faceRect.right);
		LONG height = abs(faceRect.top - faceRect.bottom);

		POINT leftTop = {faceRect.left + width/12, faceRect.top};
		POINT rightTop = {faceRect.right - width/12, faceRect.top};
		POINT leftBottom = {faceRect.left + width/12, faceRect.bottom - 1};
		POINT rightBottom = {faceRect.right - width/12, faceRect.bottom - 1};

		cvLine(&image, cvPoint(leftTop.x, leftTop.y), cvPoint(rightTop.x, rightTop.y), color, thickness);
		cvLine(&image, cvPoint(rightTop.x, rightTop.y), cvPoint(rightBottom.x, rightBottom.y), color, thickness);
		cvLine(&image, cvPoint(rightBottom.x, rightBottom.y), cvPoint(leftBottom.x, leftBottom.y), color, thickness);
		cvLine(&image, cvPoint(leftBottom.x, leftBottom.y), cvPoint(leftTop.x, leftTop.y), color, thickness);
	}

	cvShowImage("face", &image);

	//FLOAT faceScale, faceRot[3], facePos[3];
	//
	//faceTrackerResult->Get3DPose(&faceScale, faceRot, facePos);
	//faceTrackerResult->Get2DShapePoints(&shapePoints, &numPoints);
	//faceTrackerResult->GetAUCoefficients(&animCoefficients, &animUnitCount);	

	//IFTModel* ftModel;
	//faceTracker->GetFaceModel(&ftModel);

	//FT_CAMERA_CONFIG videoCameraConfig, depthCameraConfig; 
	//
	//videoCameraConfig.Height = 640;
	//videoCameraConfig.Width = 480;
	//videoCameraConfig.FocalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;

	//depthCameraConfig.Height = 320;
	//depthCameraConfig.Width = 240;
	//depthCameraConfig.FocalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;

	//HRESULT hr;

	//FLOAT* shapeUnitCoefficients;
	//UINT shapeUnitCount;
	//BOOL converged;

	//hr = faceTracker->GetShapeUnits(&faceScale, &shapeUnitCoefficients, &shapeUnitCount, &converged);	

	//UINT vertexCount = ftModel->GetVertexCount();
	//FT_VECTOR2D* pts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D)*vertexCount));
	//float zoomFactor = 2.0f;
	//
	//hr = ftModel->GetProjectedShape(&videoCameraConfig, zoomFactor, POINT(), shapeUnitCoefficients, ftModel->GetSUCount(), animCoefficients, animUnitCount, faceScale, faceRot, facePos, pts2D, vertexCount);

	//if(SUCCEEDED(hr))
	//{
	//	POINT* p3DMdl   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT)*vertexCount));
	//	if(p3DMdl)
	//	{
	//		for(UINT i = 0; i < vertexCount; ++i)
	//		{
	//			p3DMdl[i].x = LONG(pts2D[i].x + 0.5f);
	//			p3DMdl[i].y = LONG(pts2D[i].y + 0.5f);
	//		}

	//		FT_TRIANGLE* pTriangles;
	//		UINT triangleCount;
	//		hr = ftModel->GetTriangles(&pTriangles, &triangleCount);
	//		if(SUCCEEDED(hr))
	//		{
	//			struct EdgeHashTable
	//			{
	//				UINT32* pEdges;
	//				UINT edgesAlloc;

	//				void Insert(int a, int b) 
	//				{
	//					UINT32 v = (min(a, b) << 16) | max(a, b);
	//					UINT32 index = (v + (v << 8)) * 49157, i;
	//					for(i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
	//					{
	//					}
	//					
	//					pEdges[(index + i) & (edgesAlloc - 1)] = v;
	//				}
	//			} eht;

	//			eht.edgesAlloc = 1 << UINT(log(2.f * (1 + vertexCount + triangleCount)) / log(2.f));
	//			eht.pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht.edgesAlloc));
	//			if(eht.pEdges)
	//			{
	//				ZeroMemory(eht.pEdges, sizeof(UINT32) * eht.edgesAlloc);
	//				for(UINT i = 0; i < triangleCount; ++i)
	//				{ 
	//					eht.Insert(pTriangles[i].i, pTriangles[i].j);
	//					eht.Insert(pTriangles[i].j, pTriangles[i].k);
	//					eht.Insert(pTriangles[i].k, pTriangles[i].i);
	//				}

	//				for(UINT i = 0; i < eht.edgesAlloc; ++i)
	//				{
	//					if(eht.pEdges[i] != 0)
	//					{
	//						cvLine(&image, cvPoint(p3DMdl[eht.pEdges[i] >> 16].x, p3DMdl[eht.pEdges[i] >> 16].y), cvPoint(p3DMdl[eht.pEdges[i] & 0xFFFF].x, p3DMdl[eht.pEdges[i] & 0xFFFF].y), color, thickness);
	//						//pColorImg->DrawLine(p3DMdl[eht.pEdges[i] >> 16], p3DMdl[eht.pEdges[i] & 0xFFFF], color, 1);
	//					}
	//				}
	//			
	//				_freea(eht.pEdges);
	//			}
	//		}
	//	}
	//}
	
	return 0;
}

void HeadTracker::saveData(timeval frameTime, FILE* file)
{
	//fwrite(&frameTime, sizeof(timeval), 1, file);
	//fwrite(&faceTrackerResult, sizeof(IFTResult), 1, file);

	HRESULT hr;
	RECT faceRect;
	FLOAT faceScale;
	FLOAT faceRotation[3];
	FLOAT faceTranslation[3];
	UINT numPoints;
	UINT numCoefficients;

	hr = faceTrackerResult->GetStatus();
	fwrite(&frameTime, sizeof(timeval), 1, file);
	fwrite(&hr, sizeof(hr), 1, file);

	faceTrackerResult->GetFaceRect(&faceRect);
	fwrite(&faceRect, sizeof(faceRect), 1, file);

	faceTrackerResult->Get3DPose(&faceScale, faceRotation, faceTranslation);
	fwrite(&faceScale, sizeof(faceScale), 1, file);
	fwrite(faceRotation, sizeof(*faceRotation), 3, file);
	fwrite(faceTranslation, sizeof(*faceTranslation), 3, file);

	faceTrackerResult->Get2DShapePoints(&shapePoints, &numPoints);
	fwrite(&numPoints, sizeof(numPoints), 1, file);
	fwrite(shapePoints, sizeof(*shapePoints), numPoints, file);

	faceTrackerResult->GetAUCoefficients(&shapeCoefficients, &numCoefficients);
	fwrite(&numCoefficients, sizeof(numCoefficients), 1, file);
	fwrite(shapeCoefficients, sizeof(*shapeCoefficients), numCoefficients, file);
}