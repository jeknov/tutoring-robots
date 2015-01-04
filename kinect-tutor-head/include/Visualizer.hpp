class Visualizer
{
	public:
		Visualizer();
		~Visualizer();

		void drawColor(byte* colorData);
		void drawDepth(byte* depthData);
		void drawBone(const NUI_SKELETON_DATA& skeleton, const CvPoint* points, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1);
		void drawSkeleton(const NUI_SKELETON_DATA& skeleton);

	private:
		IplImage* cvColorImage;
		IplImage* cvVideoImage;
		IplImage* cvDepthImage;

		CvFont wordSignText;
		CvFont controlText;
		CvFont videoMissingText;
};