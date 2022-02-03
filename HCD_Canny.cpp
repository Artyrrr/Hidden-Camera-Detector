#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

Mat img_Canny, current_frame;

void ResultImage(int mini, int maxi, Vec3b color, int n);

int main()
{
	Mat img_active = imread("./examples/img_active.jpg"); // "активный" кадр (текущий!)
	if (img_active.empty()) // Проверка на ошибки (загружено изображение или нет)
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}

	Mat img_passive = imread("./examples/img_passive.jpg"); // "пассивный" кадр
	if (img_passive.empty()) // Проверка на ошибки (загружено изображение или нет)
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}

	TickMeter timer;
	timer.start();
	/// Получение разностного изображения (картинки должны быть одного разрешения)
	Mat diff_img;
	absdiff(img_active, img_passive, diff_img);
	imshow("diff_image", diff_img); // проверка

	/// Получение изображения с оттенками серого
	Mat img_gray;
	cvtColor(diff_img, img_gray, COLOR_BGR2GRAY);

	/// Фильтрация кадра
	Mat img_blurred;         
	medianBlur(img_gray, img_blurred, 3);
	imshow("img_blurred", img_blurred);

	/// Задание порогов и применение детектора Canny
	int lowTh = 100;
	int highTh = 100;
	Canny(img_blurred, img_Canny, lowTh, highTh);
	namedWindow("img_Canny", WINDOW_NORMAL);
	imshow("img_Canny", img_Canny); // проверка

	/// Придание цвета краям
	Mat dst;
	dst = Scalar::all(0);
	diff_img.copyTo(dst, img_Canny);
	current_frame = img_active.clone();
	namedWindow("dst", WINDOW_NORMAL);
	imshow("dst", dst); 

	ResultImage(3, 11, Vec3b(0, 255, 0), 0); // Задание площади. 9-11 на кадрах Варлашина; 1-7 для видео с Ютуба
	timer.stop();
	std::cout << "my function:" << timer.getTimeMilli() << std::endl;

	imshow("draw_frame", current_frame);
	waitKey(0);
	destroyAllWindows(); // Уничтожение всех окон
	return(0);
}

void ResultImage(int mini, int maxi, Vec3b color, int n)
{
	Moments mnts, imnts, lmnts;
	vector<vector<Point>> cnts;

	// Поиск контуров объектов
	findContours(img_Canny, cnts, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	double d = 0;
	double minum = 1000;
	/// Проход по всем контурам
	for (int i = 0; i < cnts.size(); i++)
	{
		// Если размер контура  не меньше минимума и не превышает максимум, то рисуем его и находим его центр масс
		if (cnts[i].size() > mini&& cnts[i].size() < maxi)
		{
			// Находим момент контура
			mnts = moments(cnts[i]);
			if (n == 0)
			{
				d = sqrt((lmnts.m10 / lmnts.m00 - mnts.m10 / mnts.m00) * (lmnts.m10 / lmnts.m00 - mnts.m10 / mnts.m00) +
					(lmnts.m01 / lmnts.m00 - mnts.m01 / mnts.m00) * (lmnts.m01 / lmnts.m00 - mnts.m01 / mnts.m00));
				if (minum > d)
				{
					// Обновление максимального расстояния
					minum = d;
					imnts = mnts;
				}
			}
			else if (n == 1)
			{
				lmnts = mnts; // для запоминания
				imnts = mnts; // для рисования
			}
			polylines(current_frame, cnts[i], true, color, 2, 8); // Рисование контура
		}
	}
	circle(current_frame, Point(imnts.m10 / imnts.m00, imnts.m01 / imnts.m00), 5, Vec3b(0, 0, 0), -1);
}