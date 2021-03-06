#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <Windows.h>
#include "Stuff.h"
#include "resource.h"

char* source_window = "Imagen";
char* video_window = "Video";
char* camera_window = "Camara";

enum VideoFilter {
	BlancoYNegro,
	Luminosidad,
	Promedio,
	Luminancia,
	Sepia,
	Binario,
	Negativo,
	Media,
	MediaPonderada,
	Mediana,
	Blur,
	Gaussiano,
	Laplaciano,
	MenosLaplaciano,
	SobelF,
	CorreccionLogaritmica,
	Potencia,
	Sharpen,
	PasoBajo,
	SustraccionMedia,
	EqualizacionNormal,
	EqualizacionSimple,
	EqualizacionUniforme,
	EqualizacionExponencial,
	EqualizacionColor,
	Original = -1
};

#include "Filtros.h"
#include <CommCtrl.h>

using namespace cv;

//Global Varialbes
Lista *imageList;
Mat original;
Filter flt;
VideoCapture vidOriginal;
VideoInfo *newvideo;
Histograma *hist;
VideoCapture *cam;

HWND timer1;

#define ID_TIMER1 1

void stopCamera(HWND);

bool imageReady = false;
bool videoReady = false;
int defaultSize = 720;
int filtroCam = -1;

bool camaraActiva = false;

char filtros[13][30] = { "Media", "Media Ponderada", "Mediana", "Blur", "Gaussiano", "Lapaciano", "Menos Laplaciano", "Sobel", "Corrección Logaritmica", "Potencia", "Sharpen", "Paso Bajo", "Sustracción de la Media" };
char efectos[7][30] = { "Blanco y Negro", "Luminosidad", "Promedio", "Luminancia", "Sepia", "Binario", "Negativo" };

BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg){
		case WM_INITDIALOG:{
			imageList = new Lista();
			flt.setHWND(hWnd);
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETSTEP, 1, NULL);
			for (int i = 0; i < 13; i++)
				SendMessage(GetDlgItem(hWnd, FILTER_BOX), CB_ADDSTRING, NULL, (LPARAM)filtros[i]);
			for (int i = 0; i < 7; i++)
				SendMessage(GetDlgItem(hWnd, EFFECT_BOX), CB_ADDSTRING, NULL, (LPARAM)efectos[i]);
			break;
		}
		case WM_TIMER:{
			switch(wParam){
				case ID_TIMER1:
					Mat frame;
					cam->read(frame);
					switch(filtroCam){
						case BlancoYNegro:
							Filter().Binario(frame, 100, NULL);
							break;
						case Luminosidad:
							Filter().Luminosidad(frame, NULL);
							break;
						case Promedio:
							Filter().Average(frame, NULL);
							break;
						case Luminancia:
							Filter().Luminancia(frame, NULL);
							break;
						case Sepia:
							Filter().Sepia(frame, NULL);
							break;
						case Binario:
							Filter().Binario(frame, 85, NULL);
							break;
						case Negativo:
							Filter().Negativo(frame, NULL);
							break;
						case Media:
							frame = Filter().Media(&frame);
							break;
						case MediaPonderada:
							frame = Filter().MediaPonderada(&frame);
							break;
						case Mediana:
							frame = Filter().Media(&frame);
							break;
						case Blur:
							frame = Filter().Media(&frame);
							break;
						case Gaussiano:
							frame = Filter().Gaussiano(&frame);
							break;
						case Laplaciano:
							frame = Filter().Laplaciano(&frame);
							break;
						case MenosLaplaciano:
							frame = Filter().MenosLaplaciano(&frame);
							break;
						case SobelF:
							frame = Filter().Sobel(&frame);
							break;
						case CorreccionLogaritmica:
							frame = Filter().CorreccionLogaritmica(&frame);
							break;
						case Potencia:
							frame = Filter().Potencia(&frame);
							break;
						case Sharpen:
							frame = Filter().Sharpen(&frame);
							break;
						case PasoBajo:
							frame = Filter().PasoBajo(&frame);
							break;
						case SustraccionMedia:
							frame = Filter().SustraccionMedia(&frame);
							break;
						case EqualizacionNormal:
							hist = new Histograma();
							hist->Equalize(&frame, EqualizacionNormal);
							break;
						case EqualizacionSimple:
							hist = new Histograma();
							hist->Equalize(&frame, EqualizacionSimple);
							break;
						case EqualizacionUniforme:
							hist = new Histograma();
							hist->Equalize(&frame, EqualizacionUniforme);
							break;
						case EqualizacionExponencial:
							hist = new Histograma();
							hist->Equalize(&frame, EqualizacionExponencial);
							break;
						case EqualizacionColor:
							hist = new Histograma();
							hist->EqualizeColor(&frame);
							break;
						}
					namedWindow(camera_window, WINDOW_AUTOSIZE);
					imshow(camera_window, frame);
					break;
			}
			break;
		}
		case WM_COMMAND:{
			switch(wParam){
				case CAMERA:{
					if (!camaraActiva) {
						cam = new VideoCapture(0);
						if (cam->isOpened()) {
							SetTimer(hWnd, ID_TIMER1, 1, NULL);
							camaraActiva = true;
							imageReady = false;
							videoReady = false;
						}else{
							MessageBox(hWnd, "No se detecto una camara", "Error", MB_ICONERROR);
						}
					}else {
						stopCamera(hWnd);
						delete cam;
					}
					break;
				}
				case SAVE_FILE:{
					if (imageReady) {
						if (imageList->count > 0) {
							String savePath(SaveFile("jpg"));
							if (savePath.compare(""))
								imwrite(savePath, imageList->getCurrent());
						}
					}
					if(videoReady){
						saveVideoFile(newvideo, hWnd);
					}
					break;
				}
				case SELECT_IMAGE:{
					String imagePath(getFileNameImage());
					if (imagePath.compare("")) {
						original = imread(imagePath, 1);
						imageList = new Lista();
						imageList->Add(new Imagen(original));
						namedWindow(source_window, CV_WINDOW_AUTOSIZE);
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
						stopCamera(hWnd);
						camaraActiva = false;
						imageReady = true;
						videoReady = false;
					}
					break;
				}
				case SET_ORIGINAL:{
					filtroCam = Original;
					break;
				}
				case SELECT_VIDEO:{
					String videoPath(getFileNameVideo());
					if (videoPath.compare("")) {
						vidOriginal.open(videoPath);
						if (vidOriginal.isOpened()) {
							newvideo = loadVideo(&vidOriginal, hWnd);
							stopCamera(hWnd);
							videoReady = true;
							imageReady = false;
							camaraActiva = false;
						}
					}
					break;
				}
				case HISTO:{
					if (imageReady) {
						hist = new Histograma();
						hist->createHistogram(&imageList->getCurrent());
						hist->plotHistogram(imageList->getCurrent());
					}
					break;
				}
				case PLAY_VIDEO:{
					if(videoReady)
						playVideo(newvideo, hWnd);
					break;
				}
				case RESET:{
					if (imageReady) {
						imshow(source_window, GetSquareImage(original, defaultSize));
						imageList = new Lista();
						imageList->Add(new Imagen(original));
					}
					if(camaraActiva)
						filtroCam = Original;
					break;
				}
				case BACK_FILTER:{
					if (imageReady)
						if (imageList->count > 0) {
							imageList->backFilter();
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
						}
					break;
				}
				case FORWARD_FILTER:{
					if(imageReady)
						if (imageList->curr < imageList->count) {
							imageList->forwardFilter();
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
						}
					break;
				}
				case APPLY_FILTER:{
					int value = SendMessage(GetDlgItem(hWnd, FILTER_BOX), CB_GETCURSEL, NULL, NULL);
					if (value != -1) {
						if(imageReady) {
							switch (value) {
								case 0: //Media
									imageList->AddNew(flt.Media(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 1: //Media Ponderada
									imageList->AddNew(flt.MediaPonderada(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 2: //Mediana
									imageList->AddNew(flt.Media(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 3: //Blur
									imageList->AddNew(flt.Media(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 4: //Gaussiano
									imageList->AddNew(flt.Gaussiano(&imageList->getPrevCurrent(), .85));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 5: //Laplaciano
									imageList->AddNew(flt.Laplaciano(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 6: //Menos Laplaciano
									imageList->AddNew(flt.MenosLaplaciano(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 7: //Sobel
									imageList->AddNew(flt.Sobel(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 8: //Corrección Logaritmica
									imageList->AddNew(flt.CorreccionLogaritmica(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 9: //Potencia
									imageList->AddNew(flt.Potencia(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 10: //Sharpen
									imageList->AddNew(flt.Sharpen(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 11: //Paso Bajo
									imageList->AddNew(flt.PasoBajo(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								case 12: //Sustraccion de la Media
									imageList->AddNew(flt.SustraccionMedia(&imageList->getPrevCurrent()));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
									break;
								}
						}
						if(videoReady){
							switch (value) {
							case 0: //Media
								Filter().MediaVideo(newvideo, hWnd);
								break;
							case 1: //Media Ponderada
								Filter().MediaPonderadaVideo(newvideo, hWnd);
								break;
							case 2: //Mediana
								Filter().MediaVideo(newvideo, hWnd);
								break;
							case 3: //Blur
								Filter().MediaVideo(newvideo, hWnd);
								break;
							case 4: //Gaussiano
								Filter().GaussianoVideo(newvideo, hWnd);
								break;
							case 5: //Laplaciano
								Filter().LaplacianoVideo(newvideo, hWnd);
								break;
							case 6: //Menos Laplaciano
								Filter().MenosLaplacianoVideo(newvideo, hWnd);
								break;
							case 7: //Sobel
								Filter().SobelVideo(newvideo, hWnd);
								break;
							case 8: //Corrección Logaritmica
								Filter().CorreccionLogaritmicaVideo(newvideo, hWnd);
								break;
							case 9: //Potencia
								Filter().PotenciaVideo(newvideo, hWnd);
								break;
							case 10: //Sharpen
								Filter().SharpenVideo(newvideo, hWnd);
								break;
							case 11: //Paso Bajo
								Filter().PasoBajoVideo(newvideo, hWnd);
								break;
							case 12: //Sustraccion de la Media
								Filter().SustraccionMediaVideo(newvideo, hWnd);
								break;
							}
						}
						if(camaraActiva){
							switch (value) {
							case 0: //Media
								filtroCam = Media;
								break;
							case 1: //Media Ponderada
								filtroCam = MediaPonderada;
								break;
							case 2: //Mediana
								filtroCam = Mediana;
								break;
							case 3: //Blur
								filtroCam = Blur;
								break;
							case 4: //Gaussiano
								filtroCam = Gaussiano;
								break;
							case 5: //Laplaciano
								filtroCam = Laplaciano;
								break;
							case 6: //Menos Laplaciano
								filtroCam = MenosLaplaciano;
								break;
							case 7: //Sobel
								filtroCam = SobelF;
								break;
							case 8: //Corrección Logaritmica
								filtroCam = CorreccionLogaritmica;
								break;
							case 9: //Potencia
								filtroCam = Potencia;
								break;
							case 10: //Sharpen
								filtroCam = Sharpen;
								break;
							case 11: //Paso Bajo
								filtroCam = PasoBajo;
								break;
							case 12: //Sustraccion de la Media
								filtroCam = SustraccionMedia;
								break;
							}
						}
					}
					if (imageReady)
						updateFilterCount(hWnd, imageList->count - 1);
					if (videoReady)
						updateFilterCount(hWnd, newvideo->filterCount);
					break;
				}
				case APPLY_EFFECT: {			
					int value = SendMessage(GetDlgItem(hWnd, EFFECT_BOX), CB_GETCURSEL, NULL, NULL);
					if (value != -1 && imageReady || videoReady || camaraActiva) {
						switch (value) {
							case BlancoYNegro: //Blanco y Negro
								if (imageReady) {
									imageList->AddNew(Filter().Binario(imageList->getPrevCurrent(), 127, hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if (videoReady)
									Filter().BinarioVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = BlancoYNegro;
								break;
							case Luminosidad: //Luminosidad
								if (imageReady) {
									imageList->AddNew(Filter().Luminosidad(imageList->getPrevCurrent(), hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if (videoReady)
									Filter().LuminosidadVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = Luminosidad;
								break;
							case Promedio: //Promedio
								if (imageReady) {
									imageList->AddNew(Filter().Average(imageList->getPrevCurrent(), hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if (videoReady)
									Filter().AverageVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = Promedio;
								break;
							case Luminancia: //Luminancia
								if (imageReady) {
									imageList->AddNew(Filter().Luminancia(imageList->getPrevCurrent(), hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if (videoReady)
									Filter().LuminanciaVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = Luminancia;
								break;
							case Sepia: //Sepia
								if (imageReady) {
									imageList->AddNew(Filter().Sepia(imageList->getPrevCurrent(), hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if (videoReady)
									Filter().SepiaVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = Sepia;
								break;
							case Binario: //Binario
								if (imageReady) {
									imageList->AddNew(Filter().Binario(imageList->getPrevCurrent(), 155, hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if (videoReady)
									Filter().BinarioVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = Binario;
								break;
							case Negativo: //Negativo
								int a = 0;
								if (imageReady) {
									imageList->AddNew(Filter().Negativo(imageList->getPrevCurrent(), hWnd));
									imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
								}
								if(videoReady)
									Filter().NegativoVideo(newvideo, hWnd);
								if (camaraActiva)
									filtroCam = Negativo;
								break;
						}
					}
					if(imageReady)
						updateFilterCount(hWnd, imageList->count - 1);
					if(videoReady)
						updateFilterCount(hWnd, newvideo->filterCount);
					break;
				}
				case EQUALIZAR:{
					if (imageReady) {
						hist = new Histograma();
						imageList->AddNew(hist->Equalize(&imageList->getCurrent(), EqualizacionNormal));
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
					if(camaraActiva){
						filtroCam = EqualizacionNormal;
					}
					if(videoReady){
						hist = new Histograma();
						hist->EqualizeVideo(newvideo, EqualizacionNormal, hWnd);
					}
					break;
				}
				case EQUALIZAR_SIMPLE:{
					if (imageReady) {
						hist = new Histograma();
						imageList->AddNew(hist->Equalize(&imageList->getCurrent(), EqualizacionSimple));
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
					if (camaraActiva) {
						filtroCam = EqualizacionSimple;
					}
					if (videoReady) {
						hist = new Histograma();
						hist->EqualizeVideo(newvideo, EqualizacionSimple, hWnd);
					}
					break;
				}
				case EQUALIZAR_UNIFORME:{
					if (imageReady) {
						hist = new Histograma();
						imageList->AddNew(hist->Equalize(&imageList->getCurrent(), EqualizacionUniforme));
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
					if (camaraActiva) {
						filtroCam = EqualizacionUniforme;
					}
					if (videoReady) {
						hist = new Histograma();
						hist->EqualizeVideo(newvideo, EqualizacionUniforme, hWnd);
					}
					break;
				}
				case EQUALIZAR_EXPONENCIAL:{
					if (imageReady) {
						hist = new Histograma();
						imageList->AddNew(hist->Equalize(&imageList->getCurrent(), EqualizacionExponencial));
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
					if (camaraActiva) {
						filtroCam = EqualizacionExponencial;
					}
					if (videoReady) {
						hist = new Histograma();
						hist->EqualizeVideo(newvideo, EqualizacionExponencial, hWnd);
					}
					break;
				}
				case EQUALIZAR_COLOR:{
					if (imageReady) {
						hist = new Histograma();
						imageList->AddNew(hist->EqualizeColor(&imageList->getCurrent()));
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
					if (camaraActiva) {
						filtroCam = EqualizacionColor;
					}
					if (videoReady) {
						hist = new Histograma();
						hist->EqualizeVideoColor(newvideo, hWnd);
					}
					break;
				}
			}
			break;
		}
		case WM_CLOSE:{
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
	}
	return false;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR, int nShowCmd) {
	HWND hWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(PIAD), NULL, (DLGPROC)DlgProc, 0);
	ShowWindow(hWnd, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void stopCamera(HWND hWnd){
	KillTimer(hWnd, ID_TIMER1);
	camaraActiva = false;
	cvDestroyWindow(camera_window);
}