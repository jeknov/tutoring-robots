class HeadTracker
{
	public:
		HeadTracker();
		~HeadTracker();

		int init();
		int track();
		
		int updateColorBuffer(const byte* data, const int length);
		int updateDepthBuffer(const byte* data, const int length);
		
		int drawFace(const CvScalar color, int thickness, IplImage& image);
		void saveData(timeval frameTime, FILE* file);

		RECT faceRect;
		bool faceTracked;
	
	private:
		HRESULT getVideoConfig(FT_CAMERA_CONFIG* videoConfig);
		HRESULT getDepthConfig(FT_CAMERA_CONFIG* depthConfig);

		IFTImage* colorBuffer;
		IFTImage* depthBuffer;
		IFTFaceTracker* faceTracker;
		IFTResult* faceTrackerResult;

		FT_CAMERA_CONFIG videoConfig;
		FT_CAMERA_CONFIG depthConfig;
		FT_SENSOR_DATA sensorData;

		FT_VECTOR2D* shapePoints;
		FLOAT* shapeCoefficients;
		FLOAT* animCoefficients;

		IplImage* faceImage;
};