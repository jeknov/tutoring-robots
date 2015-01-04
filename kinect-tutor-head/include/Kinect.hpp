class Kinect
{
	public:
		Kinect();
		~Kinect();

		int init(DWORD sensorFlags, DWORD skeletonFlags);
		void run();

		void record(string outDirPath);
		void play(string colorFilePath, string depthFilePath, string skeletonFilePath, string faceFilePath);

	private:
		void timeOfDay(timeval* p, void* tz);

		INuiSensor* tmpSensor;
		INuiSensor* sensor;
		HRESULT result;

		HANDLE colorStreamHandle;
		HANDLE depthStreamHandle;

		NUI_IMAGE_FRAME colorFrame;
		NUI_IMAGE_FRAME depthFrame;
		NUI_SKELETON_FRAME skeletonFrame;

		NUI_LOCKED_RECT colorRect;
		NUI_LOCKED_RECT depthRect;

		timeval frameTime;

		byte* colorData;
		byte* depthData;
		IplImage* image;

		bool visualizeColor;
		bool visualizeDepth;
		bool visualizeSkeleton;

		//string trial;
};