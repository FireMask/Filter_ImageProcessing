#include <opencv2\imgproc\imgproc.hpp>
#include <Windows.h>
#include <CommCtrl.h>
#include <stdio.h>
using namespace cv;

#define EULER 2.7182818
#define PI 3.1416

class VideoImages {
public:
	Mat frame;
	int numFrame;
	VideoImages(){}
};

class VideoInfo {
public:
	VideoImages *frames;
	int fps;
	float framerate;
	int width;
	int heigth;
	int filterCount;
	int codec;
	VideoInfo(int fps, float framerate, int width, int heigth, int codec) {
		this->frames = new VideoImages[fps];
		this->fps = fps;
		this->framerate = framerate;
		this->width = width;
		this->heigth = heigth;
		this->codec = codec;
		this->filterCount = 0;
	}
};

class AjuestesColor{
public:
	static int saturate(int value){
		if (value > 255)
			value = 255;
		else if (value < 0)
			value = 0;
		return value;
	}
};

class Mascara :private AjuestesColor {
public:
	float *mask;
	int size;
	float sum;
	int limitus;
	int x, y;
	void printValues() {
		char *a;
		for (int i = 0; i < size; i++) {
			a = new char[2];
			_itoa(mask[i], a, 10);
			OutputDebugString(a);
		}
	}
	Mascara(int size) {
		this->size = size;
		mask = new float[size*size];
	}
	Mascara(int size, float*mask) {
		this->size = size;
		this->mask = mask;
	}
	Mat ApplyMask(Mat *img, HWND hWnd) {
		Mat result;
		img->copyTo(result);
		float sum = getSigma();
		int limit = (size - 1) / 2;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, img->rows));
		int x, y;
		int channels = img->channels();
		unsigned char *p = img->data;
		unsigned char *res = result.data;
		Mascara *m = new Mascara(size);
		for (y = limit; y < (img->rows - limit); y++) {
			for (x = limit; x < (img->cols - limit); x++) {
				getBitmapArea(m, img, x, y, size, 1);
				res[result.step * y + x * channels + 0] = saturate(abs(multiply(m).getSigma() / sum));
				getBitmapArea(m, img, x, y, size, 2);
				res[result.step * y + x * channels + 1] = saturate(abs(multiply(m).getSigma() / sum));
				getBitmapArea(m, img, x, y, size, 3);
				res[result.step * y + x * channels + 2] = saturate(abs(multiply(m).getSigma() / sum));
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		delete m;
		return result;
	}
	float getSigma() {
		sum = 0;
		for (x = 0; x < size; x++)
			for (y = 0; y < size; y++)
				sum += mask[y * size + x];
		return (sum > 0) ? sum : 1;
	}
	Mascara multiply(Mascara *m) {
		limitus = ((size - 1) / 2);
		for (x = -limitus; x <= limitus; x++)
			for (y = -limitus; y <= limitus; y++)
				m->setValue(x, y, getValueof(x, y) * m->getValueof(x, y));
		return *m;
	}
	void setValue(int x, int y, float value) {
		if (x == -1)
			x = 0;
		else if (x == 0)
			x = 1;
		else if (x == 1)
			x = 2;

		if (y == -1)
			y = 0;
		else if (y == 0)
			y = 1;
		else if (y == 1)
			y = 2;

		mask[y * size + x] = value;
	}
	float getValueof(int x, int y) {
		if (x == -1)
			x = 0;
		else if (x == 0)
			x = 1;
		else if (x == 1)
			x = 2;

		if (y == -1)
			y = 0;
		else if (y == 0)
			y = 1;
		else if (y == 1)
			y = 2;

		return mask[y * size + x];
	}
	float getPixel(Mat *img, int x, int y, int channel){
		unsigned char *p = (unsigned char*)img->data;
		int channels = img->channels();
		switch (channel) {
		case 1:
			return p[img->step * y + x * channels + 0];
		case 2:
			return p[img->step * y + x * channels + 1];
		case 3:
			return p[img->step * y + x * channels + 2];
		default:
			return -1;
		}
	}
	void getBitmapArea(Mascara *m, Mat *data, int px, int py, int tamaño, int canal) {
		limitus = ((tamaño - 1) / 2);

		for (y = py - limitus; y <= (py + limitus); y++)
			for (x = px - limitus; x <= (px + limitus); x++)
				switch (canal) {
				case 1:
					m->setValue(x - px, y - py, getPixel(data, x, y, 1));
					break;
				case 2:
					m->setValue(x - px, y - py, getPixel(data, x, y, 2));
					break;
				case 3:
					m->setValue(x - px, y - py, getPixel(data, x, y, 3));
					break;
				}
	}
	int getSize() {
		return size * size;
	}
	int getMidValue() {
		float *m = mask;
		std::sort(m, m + (size*size));
		return (int)m[((size * size - 1) / 2) + 1];
	}
	int getAncho() {
		return size;
	}
	void fillMask(float value) {
		limitus = ((size - 1) / 2);
		for (x = -limitus; x <= limitus; x++)
			for (y = -limitus; y <= limitus; y++)
				setValue(x, y, value);
	}
};

class Filter:private AjuestesColor{
	HWND hWnd;
	int x, y, l;
	float r, g, b;
	Mat img;
public:
	void setHWND(HWND hWnd){
		this->hWnd = hWnd;
	}
	Mat Luminancia(Mat image, HWND hWnd){
		img = image;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));

		int channels = image.channels();
		int nRows = image.rows;
		int nCols = image.cols * channels;
		uchar *p;

		for (y = 0; y < nRows; ++y) {
			p = image.ptr<uchar>(y);
			for (x = 0; x < (nCols - 3); x += 3) {
				b = saturate(p[x]);
				g = saturate(p[x+1]);
				r = saturate(p[x+2]);
				l = (0.2126*r + 0.7152*g + 0.0722*b);
				p[x]   = l;
				p[x+1] = l;
				p[x+2] = l;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Average(Mat image, HWND hWnd){
		img = image;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		
		int channels = image.channels();
		int nRows = image.rows;
		int nCols = image.cols * channels;
		uchar *p;

		for (y = 0; y < nRows; ++y) {
			p = image.ptr<uchar>(y);
			for (x = 0; x < (nCols - 3); x += 3) {
				b = saturate(p[x]);
				g = saturate(p[x + 1]);
				r = saturate(p[x + 2]);
				l = saturate((b + g + r) / 3);
				p[x]     = l;
				p[x + 1] = l;
				p[x + 2] = l;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Luminosidad(Mat image, HWND hWnd) {
		img = image;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		
		int channels = image.channels();
		int nRows = image.rows;
		int nCols = image.cols * channels;
		uchar *p;
		for (y = 0; y < nRows; ++y) {
			p = image.ptr<uchar>(y);
			for (x = 0; x < (nCols - 3); x += 3) {
				b = saturate(p[x]);
				g = saturate(p[x + 1]);
				r = saturate(p[x + 2]);
				l = (max(r, g, b) + min(r, g, b)) / 2;
				p[x]     = l;
				p[x + 1] = l;
				p[x + 2] = l;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Sepia(Mat image, HWND hWnd) {
		img = image;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		int channels = image.channels();
		int nRows = image.rows;
		int nCols = image.cols * channels;
		uchar *p;
		for (y = 0; y < nRows; ++y) {
			p = image.ptr<uchar>(y);
			for (x = 0; x < (nCols - 3); x += 3) {
				b = saturate(p[x]);
				g = saturate(p[x + 1]);
				r = saturate(p[x + 2]);

				p[x]     = saturate((r * 0.272f) + (g * 0.534f) + (b * 0.131f));
				p[x + 1] = saturate((r * 0.349f) + (g * 0.686f) + (b * 0.168f));
				p[x + 2] = saturate((r * 0.393f) + (g * 0.769f) + (b * 0.189f));
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Negativo(Mat image, HWND hWnd) {
		img = image;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		int channels = image.channels();
		int nRows = image.rows;
		int nCols = image.cols * channels;
		uchar *p;
		for (y = 0; y < nRows; ++y) {
			p = image.ptr<uchar>(y);
			for (x = 0; x < (nCols - 3); x += 3) {
				b = saturate(p[x]);
				g = saturate(p[x + 1]);
				r = saturate(p[x + 2]);

				p[x]     = 255 - b;
				p[x + 1] = 255 - g;
				p[x + 2] = 255 - r;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Binario(Mat image, int threshold, HWND hWnd) {
		img = image;
		Mat result(image);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		int channels = image.channels();
		int nRows = image.rows;
		int nCols = image.cols * channels;
		uchar *p;
		for (y = 0; y < nRows; ++y) {
			p = image.ptr<uchar>(y);
			for (x = 0; x < (nCols - 3); x += 3) {
				if(saturate(p[x]) < threshold){
					p[x]     = 0;
					p[x + 1] = 0;
					p[x + 2] = 0;
				}else{
					p[x]     = 255;
					p[x + 1] = 255;
					p[x + 2] = 255;
				}
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return result;
	}
	Mat Media(Mat *image) {
		Mascara *mask = new Mascara(3);
		mask->fillMask(1);
		return mask->ApplyMask(image, hWnd);
	}
	Mat MediaPonderada(Mat *image) {
		Mascara *mask = new Mascara(3);
		mask->fillMask(1);
		mask->setValue(0, 0, 2);
		return mask->ApplyMask(image, hWnd);
	}
	Mat PasoBajo(Mat *image){
		Mascara *mask = new Mascara(3);
		mask->fillMask(0);
		mask->setValue(0, 0, 1);
		return mask->ApplyMask(image, hWnd);
	}
	Mat SustraccionMedia(Mat *image) {
		Mascara *mask = new Mascara(3);
		mask->fillMask(-1);
		mask->setValue(0, 0, 8);
		return mask->ApplyMask(image, hWnd);
	}
	Mat Laplaciano(Mat *image) {
		Mascara *mask = new Mascara(3);
		mask->fillMask(0);
		mask->setValue(0, 0, -4);
		mask->setValue(-1, 0, 1);
		mask->setValue(0, -1, 1);
		mask->setValue(0, 1, 1);
		mask->setValue(1, 0, 1);
		return mask->ApplyMask(image, hWnd);
	}
	Mat MenosLaplaciano(Mat *image) {
		Mascara *mask = new Mascara(3);
		mask->fillMask(0);
		mask->setValue(0, 0, 5);
		mask->setValue(-1, 0, -1);
		mask->setValue(0, -1, -1);
		mask->setValue(0, 1, -1);
		mask->setValue(1, 0, -1);
		return mask->ApplyMask(image, hWnd);
	}
	Mat Gaussiano(Mat *image, float k = 0.85) {
		Mascara *mask = new Mascara(3);
		float cuad = k * k;

		float esquinas = pow(EULER, (-1 * (2 / cuad))) / (2 * PI * cuad);
		float lados = pow(EULER, (-1 * (1 / cuad))) / (2 * PI * cuad);
		float centro = 1 / (2 * PI * cuad);

		centro = centro / esquinas;
		lados = lados / esquinas;
		esquinas = 1;

		mask->setValue(-1, -1, (int)esquinas);
		mask->setValue(1, -1, (int)esquinas);
		mask->setValue(1, 1, (int)esquinas);
		mask->setValue(-1, 1, (int)esquinas);

		mask->setValue(0, -1, (int)lados);
		mask->setValue(0, 1, (int)lados);
		mask->setValue(-1, 0, (int)lados);
		mask->setValue(1, 0, (int)lados);

		mask->setValue(0, 0, (int)centro);
		return mask->ApplyMask(image, hWnd);
	}
	Mat Sobel(Mat *image){
		Mat arriba;
		Mat abajo;
		Mat res;

		image->copyTo(arriba);
		image->copyTo(abajo);
		image->copyTo(res);

		Mascara *maskf = new Mascara(3);
		maskf->setValue(-1, 1, -1);
		maskf->setValue(0, 1, -2);
		maskf->setValue(1, 1, -1);
		maskf->setValue(-1, 0, 0);
		maskf->setValue(0, 0, 0);
		maskf->setValue(1, 0, 0);
		maskf->setValue(-1, -1, 1);
		maskf->setValue(0, -1, 2);
		maskf->setValue(1, -1, 1);
		arriba = maskf->ApplyMask(image, hWnd);

		Mascara *maskc = new Mascara(3);
		maskc->setValue(-1, 1, -1);
		maskc->setValue(0, 1, 0);
		maskc->setValue(1, 1, 1);
		maskc->setValue(-1, 0, -2);
		maskc->setValue(0, 0, 0);
		maskc->setValue(1, 0, 2);
		maskc->setValue(-1, -1, -1);
		maskc->setValue(0, -1, 0);
		maskc->setValue(1, -1, 1);
		abajo = maskf->ApplyMask(image, hWnd);

		int x, y;

		float r1, g1, b1;
		float r2, g2, b2;
		for (y = 0; y < res.rows; y++) {
			for (x = 0; x < res.cols; x++) { 
				b1 = arriba.at<Vec3b>(y, x)[0];
				g1 = arriba.at<Vec3b>(y, x)[1];
				r1 = arriba.at<Vec3b>(y, x)[2];

				b2 = abajo.at<Vec3b>(y, x)[0];
				g2 = abajo.at<Vec3b>(y, x)[1];
				r2 = abajo.at<Vec3b>(y, x)[2];

				res.at<Vec3b>(y, x)[0] = saturate(sqrt((b1*b1) + (b2*b2)));
				res.at<Vec3b>(y, x)[1] = saturate(sqrt((g1*g1) + (g2*g2)));
				res.at<Vec3b>(y, x)[2] = saturate(sqrt((r1*r1) + (r2*r2)));
			}
		}
		return res;
	}
	Mat CorreccionLogaritmica(Mat *image, float contraste = 0.85){
		int x, y;
		float grayscale;
		for (y = 0; y < image->rows; y++)
			for (x = 0; x < image->cols; x++) {
				grayscale = saturate(contraste * log(1.0f + (float)image->at<Vec3b>(y, x)[0]));
				image->at<Vec3b>(y, x)[0] = grayscale;
				image->at<Vec3b>(y, x)[1] = grayscale;
				image->at<Vec3b>(y, x)[2] = grayscale;
			}
		return *image;
	}
	Mat Potencia(Mat *image, float contraste = 0.85, float potencia = 1) {
		int x, y;
		float grayscale;
		for (y = 0; y < image->rows; y++)
			for (x = 0; x < image->cols; x++) {
				grayscale = saturate(contraste * pow((float)image->at<Vec3b>(y, x)[0], potencia));
				image->at<Vec3b>(y, x)[0] = grayscale;
				image->at<Vec3b>(y, x)[1] = grayscale;
				image->at<Vec3b>(y, x)[2] = grayscale;
			}
		return *image;
	}
	Mat Sharpen(Mat *image){
		Mascara *mask = new Mascara(3);

		float esquinas = 0;
		float centro = 11;
		float lados = -2;

		mask->setValue(-1, -1, esquinas);
		mask->setValue(1, -1, esquinas);
		mask->setValue(1, 1, esquinas);
		mask->setValue(-1, 1, esquinas);

		mask->setValue(0, -1, lados);
		mask->setValue(0, 1, lados);
		mask->setValue(-1, 0, lados);
		mask->setValue(1, 0, lados);

		mask->setValue(0, 0, centro);

		return mask->ApplyMask(image, hWnd);
	}

	void LuminanciaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			vid->frames[i].frame = Luminancia(vid->frames[i].frame, NULL);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void AverageVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			vid->frames[i].frame = Average(vid->frames[i].frame, NULL);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void LuminosidadVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			vid->frames[i].frame = Luminosidad(vid->frames[i].frame, NULL);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void SepiaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			vid->frames[i].frame = Sepia(vid->frames[i].frame, NULL);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void NegativoVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			vid->frames[i].frame = Negativo(vid->frames[i].frame, NULL);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void BinarioVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			vid->frames[i].frame = Binario(vid->frames[i].frame, 127, NULL);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}

	void MediaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			Media(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void MediaPonderadaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			MediaPonderada(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void PasoBajoVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			PasoBajo(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void SustraccionMediaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			SustraccionMedia(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void LaplacianoVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			Laplaciano(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void MenosLaplacianoVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			MenosLaplaciano(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void GaussianoVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			Gaussiano(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void SobelVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			Sobel(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void CorreccionLogaritmicaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			CorreccionLogaritmica(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void PotenciaVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			Potencia(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
	void SharpenVideo(VideoInfo *vid, HWND hWnd) {
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
		for (int i = 0; i < vid->fps; i++) {
			Sharpen(&vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
	}
};

class Histograma:private AjuestesColor{
public:
	int *histograma[3];
	Histograma(){
		histograma[0] = new int[256];
		histograma[1] = new int[256];
		histograma[2] = new int[256];
		for(int i = 0; i < 256; i++){
			histograma[0][i] = 0;
			histograma[1][i] = 0;
			histograma[2][i] = 0;
		}
	}
	void plotHistogram(Mat img){
		Mat dst;
		/// Separate the image in 3 places ( B, G and R )
		std::vector<Mat> bgr_planes;
		split(img, bgr_planes);

		/// Establish the number of bins
		int histSize = 256;

		/// Set the ranges ( for B,G,R) )
		float range[] = { 0, 256 };
		const float* histRange = { range };

		bool uniform = true; bool accumulate = false;

		Mat b_hist, g_hist, r_hist;

		/// Compute the histograms:
		calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
		calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
		calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

		// Draw the histograms for B, G and R
		int hist_w = 512; int hist_h = 400;
		int bin_w = cvRound((double)hist_w / histSize);

		Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

		/// Normalize the result to [ 0, histImage.rows ]
		normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

		/// Draw for each channel
		for (int i = 1; i < histSize; i++) {
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
				Scalar(255, 0, 0), 2, 8, 0);
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))),
				Scalar(0, 255, 0), 2, 8, 0);
			line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
				Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
				Scalar(0, 0, 255), 2, 8, 0);
		}

		/// Display
		namedWindow("Histograma", CV_WINDOW_AUTOSIZE);
		imshow("Histograma", histImage);
	}
	void createHistogram(Mat img){
		int x, y;
		for (y = 0; y < img.rows; y++) {
			for (x = 0; x < img.cols; x++) {
				histograma[0][img.at<Vec3b>(y, x)[0]]++;
				histograma[1][img.at<Vec3b>(y, x)[1]]++;
				histograma[2][img.at<Vec3b>(y, x)[2]]++;
			}
		}
	}
};

struct Imagen{
	Mat imagen;
	Imagen *sig;
	Imagen *ant;
	int num;
	Imagen(Mat img){
		img.copyTo(imagen);
	}
};

class Lista {
public:
	Imagen *inicio;
	Imagen *current;
	int count = 0;
	int curr = 1;
	Lista(){
		inicio = NULL;
	}
	void backFilter(){
		if (curr > 1) {
			current = current->ant;
			curr--;
		}
	}
	void forwardFilter() {
		if (curr < count) {
			current = current->sig;
			curr++;
		}
	}
	Mat getOriginal(){
		return inicio->imagen;
	}
	Mat getPrevCurrent(){
		return current->ant->imagen;
	}
	Mat getCurrent(){
		return current->imagen;
	}
	void AddNew(Mat img){
		img.copyTo(current->imagen);
		Add(new Imagen(img));
		curr++;
	}
	void Add(Imagen *img) {
		if (inicio == NULL) {
			inicio = img;
			inicio->sig = inicio;
			inicio->ant = inicio;
			current = img;
		}
		else {
			inicio->ant->sig = img;
			img->ant = inicio->ant;
			img->sig = inicio;
			inicio->ant = img;
			current = img;
		}
		count++;
		img->num = count;
	}
	void DeleteLast() {
		current = current->ant;
		delete current->sig;
		count--;
	}
};

String getFileNameImage() {
	OPENFILENAME ofn;
	char filename[MAX_PATH];
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(&filename, sizeof(filename));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "Imagen JPG\0*.jpg\0Todos los archivos\0*.*";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&ofn))
		return filename;
	return "";
}

String getFileNameVideo() {
	OPENFILENAME ofn;
	char filename[MAX_PATH];
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(&filename, sizeof(filename));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "Video Mp4\0*.mp4\0Todos los archivos\0*.*";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&ofn))
		return filename;
	return "";
}

String SaveFile(LPCSTR ext) {
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	if(!strcmp(ext, "jpg"))
		ofn.lpstrFilter = "JPG Image (*.jpg)\0";
	if (!strcmp(ext, "avi"))
		ofn.lpstrFilter = "AVI Video (*.avi)\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;

	GetSaveFileName(&ofn);
	return (String)szFileName;
}

Mat GetSquareImage(const Mat& img, int target_width) {
	int width = img.cols,
		height = img.rows;

	Mat square = cv::Mat::zeros(target_width, target_width, img.type());

	int max_dim = (width >= height) ? width : height;
	float scale = ((float)target_width) / max_dim;
	Rect roi;
	if (width >= height) {
		roi.width = target_width;
		roi.x = 0;
		roi.height = height * scale;
		roi.y = (target_width - roi.height) / 2;
	}
	else {
		roi.y = 0;
		roi.height = target_width;
		roi.width = width * scale;
		roi.x = (target_width - roi.width) / 2;
	}

	resize(img, square(roi), roi.size());

	return square;
}

void updateFilterCount(HWND hWnd, int value){
	char count[5];
	_itoa(value, count, 10);
	SetWindowText(GetDlgItem(hWnd, FILTER_COUNT), count);
}

void playVideo(VideoInfo *vid, HWND hWnd){
	namedWindow(video_window, CV_WINDOW_AUTOSIZE);
	SendMessage(GetDlgItem(hWnd, PROGRESS_BAR2), PBM_SETPOS, 0, NULL);
	SendMessage(GetDlgItem(hWnd, PROGRESS_BAR2), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
	Mat frame;
	for (int i = 0; i < vid->fps; i++) {
		imshow(video_window, vid->frames[i].frame);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR2), PBM_SETPOS, i, NULL);
		if (waitKey(30) == 27)
			break;
	}
}

VideoInfo *loadVideo(VideoCapture *vid, HWND hWnd) {
	float fps = vid->get(CV_CAP_PROP_FRAME_COUNT);
	float framerate = vid->get(CV_CAP_PROP_FPS);
	float heigth = vid->get(CV_CAP_PROP_FRAME_HEIGHT);
	float width = vid->get(CV_CAP_PROP_FRAME_WIDTH);
	float codec = vid->get(CV_CAP_PROP_FOURCC);
	VideoInfo *cap = new VideoInfo(fps, framerate, heigth, width, codec);
	SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
	SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, fps));
	for (int i = 0; i < fps; i++) {
		vid->set(CV_CAP_PROP_POS_FRAMES, i);
		vid->read(cap->frames[i].frame);
		cap->frames[i].numFrame = i;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
	}
	return cap;
}

void saveVideoFile(VideoInfo *vid, HWND hWnd) {
	String savePath(SaveFile("avi"));
	VideoWriter vw;
	vw.open(savePath, CV_FOURCC('W','M','V', '2'), vid->framerate, Size(vid->heigth, vid->width));
	SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
	SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, vid->fps));
	if(vw.isOpened())
		for (int i = 0; i < vid->fps; i++) {
			vw.write(vid->frames[i].frame);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
}