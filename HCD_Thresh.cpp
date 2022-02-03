#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

Mat close_img, current_frame;

void ResultImage(int mini, int maxi, Vec3b color, int n);

int main()
{
	/// Загрузка изображений
	Mat img_active = imread("./examples/img_active.jpg"); // активный кадр
	if (img_active.empty())
	{
		cout << "Could not open or find the image!\n" << endl;
		return -1;
	}

	Mat img_passive = imread("./examples/img_passive.jpg"); // пассивный кадр
	if (img_passive.empty())
	{
		cout << "Could not open or find the image!\n" << endl;
		return -1;
	}

	TickMeter timer;
	timer.start();
	// Сегментация MeanShift
	//Mat image_MeanShift_active, image_MeanShift_passive;
	//int spatial_radius = 1; //очень важный параметр (влияет на скорость работы и на выделение участков)
	//int color_radius = 25; //25 - уровень, при котором находятся камеры
	//int pyramid_levels = 1; // при увеличении выделяет больше областей, появляются мелкие шумы

	//TermCriteria term_criteria{};
	//term_criteria.maxCount = 5; // как ни меняй - разницы нет
	//term_criteria.epsilon = 1; // как ни меняй - разницы нет
	//term_criteria.type = TermCriteria::COUNT + TermCriteria::EPS;

	//pyrMeanShiftFiltering(img_active, image_MeanShift_active, spatial_radius, color_radius, pyramid_levels, term_criteria); // пассивный кадр
	//imshow("MeanShift_1", image_MeanShift_active); //проверка
	//pyrMeanShiftFiltering(img_passive, image_MeanShift_passive, spatial_radius, color_radius, pyramid_levels, term_criteria); // активный кадр
	//imshow("MeanShift_2", image_MeanShift_passive); //проверка

	/// Получение разностного изображения (картинки должны быть одного разрешения)
	Mat diff_img;
	absdiff(img_active, img_passive, diff_img);
	imshow("diff_img", diff_img); // проверка

	/// Получение изображения с оттенками серого
	Mat img_gray;
	cvtColor(diff_img, img_gray, COLOR_BGR2GRAY);
	
	/// Пороговое преобразование
	Mat threshold_val_1;
	threshold(img_gray, threshold_val_1, 50, 255, THRESH_BINARY); // более высокий порог, при котором теряется геометрия
	imshow("threshold_val_1", threshold_val_1); //проверка

	/// Морфологическое открытие (опционально, меняет геометрию)
	Mat open_img;
	Mat kernel_1 = getStructuringElement(MORPH_RECT, Size(2, 2));
	morphologyEx(threshold_val_1, open_img, MORPH_OPEN, kernel_1);
	imshow("Open", open_img); //проверка

	/// Морфологическое закрытие (опционально, меняет геометрию)
	Mat kernel_2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(open_img, close_img, MORPH_CLOSE, kernel_2);
	imshow("Close", close_img); //проверка

	/// Фильтрация кадра (опционально, меняет геометрию)
	//Mat img_blurred;
	//GaussianBlur(close_img, img_blurred, Size(3, 3), 0, 0);

	current_frame = img_active.clone();
	ResultImage(3, 10, Vec3b(0, 255, 0), 0); // Задание площади. 9-11 на кадрах Варлашина; 1-7 для видео с Ютуба
	timer.stop();
	std::cout << "my function:" << timer.getTimeMilli() << std::endl;

	imshow("draw_frame", current_frame);
	waitKey(0);
	destroyAllWindows(); // Уничтожение всех окон
	return 0;
}

void ResultImage(int mini, int maxi, Vec3b color, int n)
{
	Moments mnts, imnts, lmnts;
	vector<vector<Point>> cnts;
	/// Поиск контуров объектов
	findContours(close_img, cnts, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	double d = 0;
	double minum = 1000;
	/// Проход по всем контурам
	for (int i = 0; i < cnts.size(); i++)
	{
		/// Если размер контура  не меньше минимума и не превышает максимум, то рисуем его и находим его центр масс
		if (cnts[i].size() > mini&& cnts[i].size() < maxi)
		{
			mnts = moments(cnts[i]);
			if (n == 0)
			{
				d = sqrt((lmnts.m10 / lmnts.m00 - mnts.m10 / mnts.m00) * (lmnts.m10 / lmnts.m00 - mnts.m10 / mnts.m00) +
					(lmnts.m01 / lmnts.m00 - mnts.m01 / mnts.m00) * (lmnts.m01 / lmnts.m00 - mnts.m01 / mnts.m00));
				if (minum > d)
				{
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