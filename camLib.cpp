#include "camLib.h"

using namespace std;

VideoCapture cap(0); // ��������� ����������

/// �����������
Camera::Camera(void) 
{
	//�������� ����������������� �������� ������
	cout << " Camera warming up..." << endl;
	int isrunning = 0;
	if (!cap.isOpened())  // ���� �� �������, ������� �� ���������
	{
		cout << "Cannot open the video cam" << endl;
	}
	else 
	{
		isrunning = 1;
	}

	if (isrunning == 0) 
	{
		cout << "Camara did not start up - Exiting..." << endl;
		this->~Camera();
	}
	/// �������������� ����������� ������� ������
	cap.set(CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CAP_PROP_FRAME_HEIGHT, 1080);
	cap.set(CAP_PROP_FPS, 30);
	width = cap.get(CAP_PROP_FRAME_WIDTH); // ��������� ������ ������ �����
	height = cap.get(CAP_PROP_FRAME_HEIGHT); // ��������� ������ ������ �����
	fps = cap.get(CAP_PROP_FPS); // ��������� ���������� ������ � ������� � ����������

	cout << "Frame size : " << width << " x " << height << " --- fps: " << fps << endl;
}

/// ����������
Camera::~Camera(void) 
{
	cout << "Shutting down camera and closing files..." << endl;
	cap.release();
}

/// ������� ������� � ������
Mat Camera::captureVideo(void) 
{
	cap >> frame;
	return frame;
}