#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

Mat current_frame, mask;

void ResultImage(Vec3b color);

int main()
{
    /// Загрузка изображений
    Mat img_active = imread("./examples/img_active.jpg"); // "Актиный" кадр
    if (img_active.empty())
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }

    Mat img_passive = imread("./examples/img_passive.jpg"); // "Пассивный" кадр
    if (img_passive.empty())
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }

    TickMeter timer;
    timer.start();
    /// Сегментация MeanShift
    Mat img_MeanShift_active, img_MeanShift_passive;
    int spatial_radius = 1; //очень важный параметр (влияет на скорость работы и на выделение участков)
    int color_radius = 25; //25 - уровень, при котором находятся камеры
    int pyramid_levels = 1; // при увеличении выделяет больше областей, появляются мелкие шумы

    TermCriteria term_criteria{};
    term_criteria.maxCount = 5; // как ни меняй - разницы нет
    term_criteria.epsilon = 1; // как ни меняй - разницы нет
    term_criteria.type = TermCriteria::COUNT + TermCriteria::EPS;

    pyrMeanShiftFiltering(img_active, img_MeanShift_active, spatial_radius, color_radius, pyramid_levels, term_criteria);
    imshow("img_MeanShift_active", img_MeanShift_active); //проверка
    pyrMeanShiftFiltering(img_passive, img_MeanShift_passive, spatial_radius, color_radius, pyramid_levels, term_criteria);
    imshow("img_MeanShift_passive", img_MeanShift_passive); //проверка

    /// Получение разностного изображения (картинки должны быть одного разрешения)
    Mat diff_img;
    absdiff(img_MeanShift_active, img_MeanShift_passive, diff_img);
    imshow("diff_img", diff_img); // проверка

    /// Задание границ HSV
    inRange(diff_img, Scalar(68, 145, 0), Scalar(180, 171, 215), mask);
    
    current_frame = img_active.clone(); // Текущий кадр, для рисования обнаруженных бликов
    ResultImage(Vec3b(0, 255, 0)); // Задание площади (в варианте для HSV не используется, но её возможно внедрить)
    timer.stop();
    std::cout << "my function:" << timer.getTimeMilli() << std::endl;

    imshow("draw_frame", current_frame);
    waitKey(0);
    destroyAllWindows(); // Уничтожение всех окон
    return 0;
}

/// Поиск контуров объектов
void ResultImage(Vec3b color)
{
    Moments mnts, imnts, lmnts;
    vector<vector<Point>> cnts;
    findContours(mask, cnts, RETR_EXTERNAL, CHAIN_APPROX_NONE);




    for (unsigned int i = 0; i < cnts.size(); i++)
    {
        std::cout << "# of contour points: " << cnts[i].size() << std::endl;

        for (unsigned int j = 0; j < cnts[i].size(); j++)
        {
            std::cout << "Point(x,y)=" << cnts[i][j] << std::endl; // Вывод координат обнаруженных бликов
        }


        if (contourArea(cnts[i]) < 1) // Задание размера площади, которые будут рисоваться в кадре
        {
            std::cout << " Area: " << contourArea(cnts[i]) << std::endl;
            drawContours(current_frame, cnts, -1, Scalar(0, 255, 0), 6);
        }
    }

    /*
    // обходим все контуры
    for (int i = 0; i < cnts.size(); i++)
    {
        // вычисляем площадь и периметр контура
        double area = fabs(contourArea(current));
        double perim = (current);

        // 1/4*CV_PI = 0,079577
        if (area / (perim * perim) > 0.07 && area / (perim * perim) < 0.087) { // в 10% интервале
            // нарисуем контур
            arcLength(_image, closed);
          
            drawContours(current_frame, cnts, -1, Scalar(0, 255, 0), 6);
        }
    }*/
}