#include <opencv2\imgproc\imgproc.hpp>
#include <Windows.h>
#include <CommCtrl.h>
using namespace cv;

#define EULER 2.7182818
#define PI 3.1416

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

class Mascara:private AjuestesColor{
	float *mask;
	int size;
	float sum;
	int limitus;
	int x, y;
public:
	void printValues(){
		char *a;
		for(int i = 0; i<size; i++){
			a = new char[2];
			_itoa(mask[i], a, 10);
			OutputDebugString(a);
		}
	}
	Mascara(int size){
		this->size = size;
		mask = new float[size*size];
	}
	Mascara(int size, float*mask) {
		this->size = size;
		this->mask = mask;
	}
	Mat ApplyMask(Mat *img, HWND hWnd){
		Mat result;
		img->copyTo(result);
		float sum = getSigma();
		float limit = (size - 1) / 2;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, img->rows));
		Mascara *m = new Mascara(size);
		int *lim = new int;
		for (unsigned short y = limit; y < (img->rows - limit); y++) {
			for (unsigned short x = limit; x < (img->cols - limit); x++) {
				result.at<Vec3b>(y, x)[0] = saturate(abs(multiply(lim, getBitmapArea(lim, m, img, x, y, size, 1)).getSigma() / sum));
				result.at<Vec3b>(y, x)[1] = saturate(abs(multiply(lim, getBitmapArea(lim, m, img, x, y, size, 2)).getSigma() / sum));
				result.at<Vec3b>(y, x)[2] = saturate(abs(multiply(lim, getBitmapArea(lim, m, img, x, y, size, 3)).getSigma() / sum));
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return result;
	}
	float getSigma() {
		sum = 0;
		for (x = 0; x < size; x++)
			for (y = 0; y < size; y++)
				sum += mask[y * size + x];
		return (sum > 0) ? sum : 1;
	}
	Mascara multiply(int *limite, Mascara *m) {
		*limite = ((size - 1) / 2);
		for (x = -*limite; x <= *limite; x++)
			for (y = -*limite; y <= *limite; y++)
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

		float resul = mask[y * size + x];
		return resul;
	}
	Mascara *getBitmapArea(int *limite, Mascara *m, Mat *data, int px, int py, int tamaño, int canal) {
		m = new Mascara(tamaño);
		*limite = ((tamaño - 1) / 2);

		for (y = py - *limite; y <= (py + *limite); y++)
			for (x = px - *limite; x <= (px + *limite); x++)
				switch (canal) {
				case 1:
					m->setValue(x - px, y - py, data->at<Vec3b>(y, x)[0]);
					break;
				case 2:
					m->setValue(x - px, y - py, data->at<Vec3b>(y, x)[1]);
					break;
				case 3:
					m->setValue(x - px, y - py, data->at<Vec3b>(y, x)[2]);
					break;
				}
		return m;
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
public:
	void setHWND(HWND hWnd){
		this->hWnd = hWnd;
	}
	Mat Luminancia(Mat image, HWND hWnd){
		Mat img = image;
		int r, g, b;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		for (int y = 0; y < img.rows; y++) {
			for (int x = 0; x < img.cols; x++) {
				b = saturate(img.at<Vec3b>(y, x)[0]);
				g = saturate(img.at<Vec3b>(y, x)[1]);
				r = saturate(img.at<Vec3b>(y, x)[2]);
				
				int l = (0.2126*r + 0.7152*g + 0.0722*b);

				img.at<Vec3b>(y, x)[0] = l;
				img.at<Vec3b>(y, x)[1] = l;
				img.at<Vec3b>(y, x)[2] = l;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Average(Mat image, HWND hWnd){
		Mat img = image;
		int r, g, b;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		for (int y = 0; y < img.rows; y++) {
			for (int x = 0; x < img.cols; x++) {
				b = saturate(img.at<Vec3b>(y, x)[0]);
				g = saturate(img.at<Vec3b>(y, x)[1]);
				r = saturate(img.at<Vec3b>(y, x)[2]);

				int l = saturate((b + g + r) / 3);

				img.at<Vec3b>(y, x)[0] = l;
				img.at<Vec3b>(y, x)[1] = l;
				img.at<Vec3b>(y, x)[2] = l;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Luminosidad(Mat image, HWND hWnd) {
		Mat img = image;
		int r, g, b;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		for (int y = 0; y < img.rows; y++) {
			for (int x = 0; x < img.cols; x++) {
				b = saturate(img.at<Vec3b>(y, x)[0]);
				g = saturate(img.at<Vec3b>(y, x)[1]);
				r = saturate(img.at<Vec3b>(y, x)[2]);

				int l = (max(r, g, b) + min(r, g, b)) / 2;

				img.at<Vec3b>(y, x)[0] = l;
				img.at<Vec3b>(y, x)[1] = l;
				img.at<Vec3b>(y, x)[2] = l;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Sepia(Mat image, HWND hWnd) {
		Mat img = image;
		float r, g, b;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		for (int y = 0; y < img.rows; y++) {
			for (int x = 0; x < img.cols; x++) {
				b = saturate(img.at<Vec3b>(y, x)[0]);
				g = saturate(img.at<Vec3b>(y, x)[1]);
				r = saturate(img.at<Vec3b>(y, x)[2]);

				int l = saturate((b + g + r) / 3);

				img.at<Vec3b>(y, x)[2] = saturate((r * 0.393f) + (g * 0.769f) + (b * 0.189f));
				img.at<Vec3b>(y, x)[1] = saturate((r * 0.349f) + (g * 0.686f) + (b * 0.168f));
				img.at<Vec3b>(y, x)[0] = saturate((r * 0.272f) + (g * 0.534f) + (b * 0.131f));
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Negativo(Mat image, HWND hWnd) {
		Mat img = image;
		float r, g, b;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		for (int y = 0; y < img.rows; y++) {
			for (int x = 0; x < img.cols; x++) {
				b = saturate(img.at<Vec3b>(y, x)[0]);
				g = saturate(img.at<Vec3b>(y, x)[1]);
				r = saturate(img.at<Vec3b>(y, x)[2]);

				img.at<Vec3b>(y, x)[2] = 255 - b;
				img.at<Vec3b>(y, x)[1] = 255 - g;
				img.at<Vec3b>(y, x)[0] = 255 - r;
			}
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_STEPIT, NULL, NULL);
		}
		return img;
	}
	Mat Binario(Mat image, int threshold, HWND hWnd) {
		Mat img = image;
		Mat result(image);
		float r, g, b;
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETPOS, 0, NULL);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, NULL, MAKELPARAM(0, image.rows));
		for (int y = 0; y < img.rows; y++) {
			for (int x = 0; x < img.cols; x++) {
				if(saturate(img.at<Vec3b>(y, x)[0]) < threshold){
					result.at<Vec3b>(y, x)[2] = 0;
					result.at<Vec3b>(y, x)[1] = 0;
					result.at<Vec3b>(y, x)[0] = 0;
				}else{
					result.at<Vec3b>(y, x)[2] = 255;
					result.at<Vec3b>(y, x)[1] = 255;
					result.at<Vec3b>(y, x)[0] = 255;
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

class Lista{
public:
	Imagen *inicio;
	Imagen *current;
	int count = 0;
	Lista(){
		inicio = NULL;
	}
	void backFilter(){
		current = current->ant;
	}
	void forwardFilter() {
		current = current->sig;
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
	void setCurrent(Mat img){
		img.copyTo(current->imagen);
	}
	void AddNew(){
		Add(new Imagen(current->imagen));
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

String getFileName() {
	OPENFILENAME ofn;
	char filename[MAX_PATH];
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(&filename, sizeof(filename));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "Imagen JPG\0*.jpg\0Imagen BMP\0*.bmp\0Todos los archivos\0*.*";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&ofn))
		return filename;
	return "";
}


String SaveFile() {
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "JPG Image (*.jpg)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "jpg";

	GetSaveFileName(&ofn);
	return szFileName;
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

void updateFilterCount(HWND hWnd, Lista *imageList){
	char count[5];
	_itoa(imageList->count - 1, count, 10);
	SetWindowText(GetDlgItem(hWnd, FILTER_COUNT), count);
}