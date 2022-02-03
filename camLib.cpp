#include "camLib.h"

using namespace std;

VideoCapture cap(0); // Получение видеопотка

/// Конструктор
Camera::Camera(void) 
{
	//Проверка работоспособности открытия камеры
	cout << " Camera warming up..." << endl;
	int isrunning = 0;
	if (!cap.isOpened())  // Если не удалось, выйдите из программы
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
	/// Автоматическое определение размера камеры
	cap.set(CAP_PROP_FRAME_WIDTH, 1920);
	cap.set(CAP_PROP_FRAME_HEIGHT, 1080);
	cap.set(CAP_PROP_FPS, 30);
	width = cap.get(CAP_PROP_FRAME_WIDTH); // Получение ширины кадров видео
	height = cap.get(CAP_PROP_FRAME_HEIGHT); // Получение высоты кадров видео
	fps = cap.get(CAP_PROP_FPS); // Получение количество кадров в секунду с устройства

	cout << "Frame size : " << width << " x " << height << " --- fps: " << fps << endl;
}

/// Деструктор
Camera::~Camera(void) 
{
	cout << "Shutting down camera and closing files..." << endl;
	cap.release();
}

/// Функция доступа к камере
Mat Camera::captureVideo(void) 
{
	cap >> frame;
	return frame;
}