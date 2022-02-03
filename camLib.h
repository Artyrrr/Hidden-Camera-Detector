#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <thread>
 
using namespace cv;
using namespace std;
 
class Camera 
{
	public:
		Camera(void);
		~Camera(void);
		Mat captureVideo(void);
		
	private:
		Mat frame;
		double width;
		double height;
		double fps;	
};