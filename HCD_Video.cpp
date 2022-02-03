#include "camLib.h"

using namespace cv;
using namespace std;

/// Выделение памяти для хранения кадров и запуска камеры 
Camera cam1;
const int frame_buffer = 50; // Максимальный предел кадров, которые могут быть помещены в стек, прежде чем весь стек будет удален
vector<Mat> frame_stack = *new vector<Mat>[frame_buffer * sizeof(cam1.captureVideo())]; // Cтек кадров, содержащий кадры, считанные с камеры (до кадров frame_buffer)
vector<Mat> flare_stack = *new vector<Mat>[frame_buffer * sizeof(cam1.captureVideo())]; // Cтек кадров, взятых из стека frame_stack и обработанные с использованием функций OpenCV
int stop_sig = 0; // Глобальный сигнал остановки (при значении 1, все потоки останавливаются и возвращаются в основную процедуру)
Mat current_frame, mask;

void ProcessFrame(void);
void GrabFrame(void);
void ResultImage(Vec3b color);

int main()
{
	Mat frame; // Захваченные отдельные кадры
	Mat contour; // Видеопоток, показывающий счетчики объектов

	// Запуск бесконечного цикла для захвата кадров
	// Этот бесконечный цикл останавливается нажатием клавиши ESC
	frame_stack.clear();
	flare_stack.clear();
	thread t1(GrabFrame);
	thread t2(ProcessFrame);
	while (1)
	{
		if (flare_stack.size() >= 2)
		{
			contour = flare_stack.back();
			imshow("Contour Video", contour);
		}

		if (waitKey(1) == 30) // Ожидание нажатия клавиши «esc» в течение 30 мс. Если нажата клавиша «esc» - цикл обрывается
		{
			cout << "Main: esc key is pressed by user" << endl;
			stop_sig = 1; // Сигнал потокам, чтобы закончить их запуск

			frame_stack.clear();
			flare_stack.clear();
			break;
		}
	}
	t1.join();
	t2.join();
	return 0;
}

void ProcessFrame(void) 
{
	Mat frame;
	Mat contour;
	Mat frame_prev;

	char charCheckForEscKey = 0;
	while (charCheckForEscKey != 27)
	{
		/// Проверка - есть ли данные в буфере кадров. Если стек кадров не пустой, захватывается новый кадр, не удаляя его для дальнейшей обработки
		if (!frame_stack.empty())
		{  
			frame = frame_stack.front();   // Взятие первого кадра из исходного стека без его удаления
			frame_prev = Mat::zeros(frame.size(), frame.type()); // Нулевой кадр (чёрный кадр)

			/// Получение разностного кадра
			Mat frameAbsDiff;
			absdiff(frame, frame_prev, frameAbsDiff); // без знака
			//imshow("frameAbsDiff", frameAbsDiff); 

			if (waitKey(90) == 27)
				break;

			frame.copyTo(frame_prev);

			/// HSV
			Mat HSV_frame;
			cvtColor(frameAbsDiff, HSV_frame, COLOR_BGR2HSV); // Преобразование из BGR в HSV
			//imshow("HSV_frame", HSV_frame);

			/// Задание границ HSV
			//inRange(frameAbsDiff, Scalar(132, 107, 170), Scalar(150, 110, 200), mask); // test2 video
			//inRange(frameAbsDiff, Scalar(140, 100, 190), Scalar(160, 120, 210), mask); // test1 video
			//inRange(frameAbsDiff, Scalar(140, 100, 200), Scalar(150, 120, 210), mask); // test1 video (HSV)
			//inRange(frameAbsDiff, Scalar(140, 100, 210), Scalar(150, 120, 218), mask); // test5 video (HSV)
			inRange(frameAbsDiff, Scalar(132, 107, 170), Scalar(150, 110, 200), mask); // test2 video (HSV)
			//inRange(frameAbsDiff, Scalar(120, 100, 0), Scalar(180, 171, 215), mask);
			namedWindow("frame", WINDOW_KEEPRATIO);
			int hue = 149;
			int sat = 45;
			int val = 14;
			int hue_h = 180;
			int sat_h = 146;
			int val_h = 255;
			//createTrackbar("hue", "frame", &hue, 255, 0);
			//createTrackbar("sat", "frame", &sat, 255, 0);
			//createTrackbar("val", "frame", &val, 255, 0);
			//createTrackbar("hue_h", "frame", &hue_h, 255, 0);
			//createTrackbar("sat_h", "frame", &sat_h, 255, 0);
			//createTrackbar("val_h", "frame", &val_h, 255, 0);
			/*while (waitKey(30) != 13) {
				inRange(HSV_frame, Scalar(hue, sat, val), Scalar(hue_h, sat_h, val_h), mask);
				//erode(mask_cop, mask_cop, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
				//dilate(mask_cop, mask_cop, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
				imshow("frame", mask);
			}*/
			imshow("mask", mask);
			current_frame = frame.clone(); // Текущий кадр, для рисования обнаруженных бликов
			ResultImage(Vec3b(0, 255, 0)); // Задание площади (в варианте для HSV не используется, но её возможно внедрить)
			
			imshow("draw_frame", current_frame);
			charCheckForEscKey = waitKey(1);        // delay and get key press
		}

		/// Если в стеке бликов более 2 кадров, последний кадр удаляется (в конце стека)
		if (flare_stack.size() > 2)
		{	
			flare_stack.pop_back();
		}
		/// Если доступен новый обработанный кадр и стек еще не заполнен, новый обработанный кадр помещается вперёд стека:
		if (!contour.empty() && flare_stack.size() < frame_buffer)
		{
			flare_stack.push_back(contour);
		}
		/// Очистка стека, только если он заполнен
		else if (flare_stack.size() >= frame_buffer)
		{
			flare_stack.clear();
		}
	}
	cout << "process_frame: esc key is pressed by user" << endl;
	return;
}

void GrabFrame(void) 
{
	Mat frame;

	frame_stack.clear();
	while (!stop_sig)
	{
		frame = cam1.captureVideo(); // Захват кадра с камеры
		/// Если в стеке более 2 кадров - удалить один кадр сзади
		if (frame_stack.size() > 2) 
		{	
			frame_stack.pop_back(); // Эта строка удаляет последний кадр из стека
		}
		/// Если стек не заполнен, добавить кадр в начало стека
		if (frame_stack.size() < frame_buffer)
		{
			frame_stack.push_back(frame);	// Эта строка помещает кадр за кадром в конец стека 
		}
		/// Очистка стека, когда он заполнен
		else 
		{
			frame_stack.clear();
		}

	}
	cout << "grabFrame: esc key is pressed by user" << endl;
	return;
}

/// Поиск контуров объектов
void ResultImage(Vec3b color)
{
	vector<vector<Point>> contours;
	findContours(mask, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> contours_poly(contours.size());
	
	for (unsigned int i = 0; i < contours_poly.size(); i++)
	{
		approxPolyDP(contours[i], contours_poly[i], 1, true);
		cout << "# of contour points: " << contours_poly[i].size() << endl; // Количество найденных бликов
		
		for (unsigned int j = 0; j < contours_poly[i].size(); j++)
		{
			cout << "Point(x,y)=" << contours_poly[i][j] << endl; // Вывод координат обнаруженных бликов
		}
		
		if (contourArea(contours_poly[i]) < 1) // Задание размера площади, меньше которой будет рисоваться в кадре
		{
			cout << " Area: " << contourArea(contours_poly[i]) << endl;
			if ((contours_poly[i].size()) < 2)
			{
				drawContours(current_frame, contours, -1, Scalar(0, 255, 0), 6); // Нарисовать найденные блики
				/// Создание рамки
				Rect rect = boundingRect(contours[i]);
				Point pt1, pt2;
				pt1.x = rect.x - 20;
				pt1.y = rect.y - 20;
				pt2.x = rect.x + 20;
				pt2.y = rect.y + 20;
				rectangle(current_frame, pt1, pt2, Scalar(0, 255, 255));
			}
		}
	}
}