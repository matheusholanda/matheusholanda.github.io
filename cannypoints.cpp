#include <iostream>
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace cv;

#define STEP 5
#define JITTER 3

//Limiar do Canny
int thr_slider = 10;
int thr_slider_max = 200;
//Raio do Pontilhismo
int raio_slider = 10;
int raio_slider_max = 200;



char TrackbarName[50];

Mat image, border, points;
int width, height;

void on_trackbar_canny(int, void*) {
	vector<int> yrange;
	vector<int> xrange;

	Mat frame;

	int x, y, gray;

	srand(time(0));

	xrange.resize(height / STEP);
	yrange.resize(width / STEP);

	iota(xrange.begin(), xrange.end(), 0);
	iota(yrange.begin(), yrange.end(), 0);

	for (uint i = 0; i<xrange.size(); i++) {
		xrange[i] = xrange[i] * STEP + STEP / 2;
	}

	for (uint i = 0; i<yrange.size(); i++) {
		yrange[i] = yrange[i] * STEP + STEP / 2;
	}

	points = Mat(height, width, CV_8U, Scalar(255));


	//pontos primarios em pontos randomicos
	random_shuffle(xrange.begin(), xrange.end());
	for (auto i : xrange) {
		random_shuffle(yrange.begin(), yrange.end());
		for (auto j : yrange) {
			x = i + rand() % (2 * JITTER) - JITTER + 1;
			y = j + rand() % (2 * JITTER) - JITTER + 1;
			gray = image.at<uchar>(x, y);
			circle(points,
				cv::Point(y, x),
				(raio_slider / 50) + 1,
				CV_RGB(gray, gray, gray),
				-1,
				CV_AA);
		}
	}

	Canny(image, border, thr_slider, 3 * thr_slider);


	//reforço das bordas
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			if (border.at<uchar>(i, j) == 255) {
				gray = image.at<uchar>(i, j);
				circle(points,
					cv::Point(j, i),
					(raio_slider / 50) + 1,
					CV_RGB(gray, gray, gray),
					-1,
					CV_AA);
			}
		}
	}
	imwrite("cannypoints.png", points);
	imshow("Canny Pontilhismo", points);
}


int main(int argc, char**argv) {

	image = imread("blade2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	width = image.size().width;
	height = image.size().height;
	namedWindow("Canny Pontilhismo", WINDOW_AUTOSIZE);

	sprintf_s(TrackbarName, "Bordas");
	createTrackbar(TrackbarName, "Canny Pontilhismo", &thr_slider, thr_slider_max, on_trackbar_canny);
	on_trackbar_canny(thr_slider, 0);

	sprintf_s(TrackbarName, "Raio");
	createTrackbar(TrackbarName, "Canny Pontilhismo", &raio_slider, raio_slider_max, on_trackbar_canny);
	on_trackbar_canny(raio_slider, 0);

	waitKey(0);
	return 0;
}