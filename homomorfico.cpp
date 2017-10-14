

#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>

using namespace cv;
using namespace std;


float MAX = 100.0;


//-------PARAMETROS DO FILTRO
float gammal = 0;
float max_gammal = 100;
int gammal_slider = 0;

int d0_slider = 0;
float max_d0 = 256;
float d0 = 0;

float gammah = 0;
float max_gammah = 100;
int gammah_slider = 0;

float c = 0;
float max_c = 100;
int c_slider = 0;
//-------

Mat image, imageDft, padded;

// valores ideais dos tamanhos da imagem
// para calculo da DFT
int dft_M, dft_N;


char TrackbarName[50];

// troca os quadrantes da imagem da DFT
void deslocaDFT(Mat& image) {
	Mat tmp, A, B, C, D;

	// se a imagem tiver tamanho impar, recorta a regiao para
	// evitar cópias de tamanho desigual
	image = image(Rect(0, 0, image.cols & -2, image.rows & -2));
	int cx = image.cols / 2;
	int cy = image.rows / 2;

	// reorganiza os quadrantes da transformada
	// A B   ->  D C
	// C D       B A
	A = image(Rect(0, 0, cx, cy));
	B = image(Rect(cx, 0, cx, cy));
	C = image(Rect(0, cy, cx, cy));
	D = image(Rect(cx, cy, cx, cy));

	// A <-> D
	A.copyTo(tmp);  D.copyTo(A);  tmp.copyTo(D);

	// C <-> B
	C.copyTo(tmp);  B.copyTo(C);  tmp.copyTo(B);
}

void applyFilter() {
	Mat filter = Mat(padded.size(), CV_32FC2, Scalar(0));
	Mat tmp = Mat(dft_M, dft_N, CV_32F);

	for (int i = 0; i < dft_M; i++) {
		for (int j = 0; j < dft_N; j++) {
			//calculo do filtro homomorfico
			float d2 = pow(i - dft_M / 2.0, 2) + pow(j - dft_N / 2.0, 2);
			float exp = -c*(d2 / pow(d0, 2));
			float valor = (gammah - gammal)*(1 - expf(exp)) + gammal;
			tmp.at<float>(i, j) = valor;
		}
	}

	Mat comps[] = { tmp, tmp };
	merge(comps, 2, filter);

	Mat dftClone = imageDft.clone();

	mulSpectrums(dftClone, filter, dftClone, 0);

	deslocaDFT(dftClone);

	idft(dftClone, dftClone);

	vector<Mat> planos;

	split(dftClone, planos);

	normalize(planos[0], planos[0], 0, 1, CV_MINMAX);

	imshow("Homomorfico", planos[0]);
	imshow("Original", image);
}

void on_trackbar_gammal(int, void*) {
	gammal = (float)gammal_slider;
	gammal = max_gammal*gammal / MAX;
	applyFilter();
}

void on_trackbar_d0(int, void *) {
	d0 = d0_slider*max_d0 / MAX;
	applyFilter();
}

void on_trackbar_gammah(int, void*) {
	gammah = gammah_slider*max_gammah / MAX;
	applyFilter();
}

void on_trackbar_c(int, void*) {
	c = c_slider*max_c / MAX;
	applyFilter();
}

int main(int argvc, char** argv) {
	image = imread("bb.jpg", CV_LOAD_IMAGE_GRAYSCALE);



	// identifica os tamanhos otimos para
	// calculo do FFT
	dft_M = getOptimalDFTSize(image.rows);
	dft_N = getOptimalDFTSize(image.cols);

	// realiza o padding da imagem
	Mat_<float> zeros;
	copyMakeBorder(image, padded, 0,
		dft_M - image.rows, 0,
		dft_N - image.cols,
		BORDER_CONSTANT, Scalar::all(0));

	// parte imaginaria da matriz complexa (preenchida com zeros)
	zeros = Mat_<float>::zeros(padded.size());

	// prepara a matriz complexa para ser preenchida
	imageDft = Mat(padded.size(), CV_32FC2, Scalar(0));

	copyMakeBorder(image, padded, 0,
		dft_M - image.rows, 0,
		dft_N - image.cols,
		BORDER_CONSTANT, Scalar::all(0));

	Mat_<float> realInput = Mat_<float>(padded);

	// insere as duas componentes no array de matrizes
	vector<Mat> planos;
	planos.push_back(realInput);
	planos.push_back(zeros);

	// combina o array de matrizes em uma unica
	// componente complexa
	merge(planos, imageDft);

	// calcula o dft
	dft(imageDft, imageDft);
	deslocaDFT(imageDft);

	namedWindow("Homomorfico", 1);

	sprintf_s(TrackbarName, "Gamma High");
	createTrackbar(TrackbarName, "Homomorfico", &gammah_slider, MAX, on_trackbar_gammah);

	sprintf_s(TrackbarName, "Gamma Low");
	createTrackbar(TrackbarName, "Homomorfico", &gammal_slider, MAX, on_trackbar_gammal);

	sprintf_s(TrackbarName, "C");
	createTrackbar(TrackbarName, "Homomorfico", &c_slider, MAX, on_trackbar_c);

	sprintf_s(TrackbarName, "Cutoff D0");
	createTrackbar(TrackbarName, "Homomorfico", &d0_slider, MAX, on_trackbar_d0);


	applyFilter();
	waitKey(0);

	return 0;
}

