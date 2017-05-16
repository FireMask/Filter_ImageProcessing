#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <Windows.h>
#include "Stuff.h"
#include "resource.h"
#include "Filtros.h"
#include <CommCtrl.h>

#include <string>

using namespace cv;

//Global Varialbes
Lista *imageList;
Mat original;
Filter flt;
VideoCapture vidOriginal;
VideoInfo *newvideo;
Histograma *hist;

char* source_window = "Source image";
bool imageReady = false;
bool videoReady = false;
int defaultSize = 720;

char filtros[13][30] = { "Media", "Media Ponderada", "Mediana", "Blur", "Gaussiano", "Lapaciano", "Menos Laplaciano", "Sobel", "Corrección Logaritmica", "Potencia", "Sharpen", "Paso Bajo", "Sustracción de la Media" };
char efectos[7][30] = { "Blanco y Negro", "Luminosidad", "Promedio", "Luminancia", "Sepia", "Binario", "Negativo" };

BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg){
	case WM_INITDIALOG:{
		imageList = new Lista();
		hist = new Histograma();
		flt.setHWND(hWnd);
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(GetDlgItem(hWnd, PROGRESS_BAR), PBM_SETSTEP, 1, NULL);
		for (int i = 0; i < 13; i++)
			SendMessage(GetDlgItem(hWnd, FILTER_BOX), CB_ADDSTRING, NULL, (LPARAM)filtros[i]);
		for (int i = 0; i < 7; i++)
			SendMessage(GetDlgItem(hWnd, EFFECT_BOX), CB_ADDSTRING, NULL, (LPARAM)efectos[i]);
		break;
	}
	case WM_COMMAND:{
		switch(wParam){
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
					imageReady = true;
					videoReady = false;
				}
				break;
			}
			case SELECT_VIDEO:{
				String videoPath(getFileNameVideo());
				if (videoPath.compare("")) {
					vidOriginal.open(videoPath);
					if (vidOriginal.isOpened()) {
						newvideo = loadVideo(&vidOriginal, hWnd);
						videoReady = true;
						imageReady = false;
					}
				}
				break;
			}
			case HISTO:
				hist->createHistogram(imageList->getCurrent());
				hist->plotHistogram(imageList->getCurrent());
				break;
			case PLAY_VIDEO:{
				if(videoReady)
					playVideo(newvideo);
				break;
			}
			case RESET:{
				if (imageReady) {
					imshow(source_window, GetSquareImage(original, defaultSize));
					imageList = new Lista();
					imageList->Add(new Imagen(original));
				}
				break;
			}
			case BACK_FILTER:
				if (imageReady)
					if (imageList->count > 0) {
						imageList->backFilter();
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
				break;
			case FORWARD_FILTER:
				if(imageReady)
					if (imageList->curr < imageList->count) {
						imageList->forwardFilter();
						imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					}
				break;
			case APPLY_FILTER:{
				int value = SendMessage(GetDlgItem(hWnd, FILTER_BOX), CB_GETCURSEL, NULL, NULL);
				if (value != -1) {
					if (imageReady) {
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

								break;
							case 3: //Blur
								//imageList->AddNew();
								//blur(imageList->getPrevCurrent(), imageList->getCurrent(), Size(5, 5));
								//imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
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
							
							break;
						case 3: //Blur
							
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
				}
				if (imageReady)
					updateFilterCount(hWnd, imageList->count - 1);
				if (videoReady)
					updateFilterCount(hWnd, newvideo->filterCount);
				break;
			}
			case APPLY_EFFECT: {			
				int value = SendMessage(GetDlgItem(hWnd, EFFECT_BOX), CB_GETCURSEL, NULL, NULL);
				if (value != -1 && imageReady || videoReady) {
					switch (value) {
						case 0: //Blanco y Negro
							if (imageReady) {
								imageList->AddNew(Filter().Binario(imageList->getPrevCurrent(), 127, hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if (videoReady)
								Filter().BinarioVideo(newvideo, hWnd);
							break;
						case 1: //Luminosidad
							if (imageReady) {
								imageList->AddNew(Filter().Luminosidad(imageList->getPrevCurrent(), hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if (videoReady)
								Filter().LuminosidadVideo(newvideo, hWnd);
							break;
						case 2: //Promedio
							if (imageReady) {
								imageList->AddNew(Filter().Average(imageList->getPrevCurrent(), hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if (videoReady)
								Filter().AverageVideo(newvideo, hWnd);
							break;
						case 3: //Luminancia
							if (imageReady) {
								imageList->AddNew(Filter().Luminancia(imageList->getPrevCurrent(), hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if (videoReady)
								Filter().LuminanciaVideo(newvideo, hWnd);
							break;
						case 4: //Sepia
							if (imageReady) {
								imageList->AddNew(Filter().Sepia(imageList->getPrevCurrent(), hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if (videoReady)
								Filter().SepiaVideo(newvideo, hWnd);
							break;
						case 5: //Binario
							if (imageReady) {
								imageList->AddNew(Filter().Binario(imageList->getPrevCurrent(), 155, hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if (videoReady)
								Filter().BinarioVideo(newvideo, hWnd);
							break;
						case 6: //Negativo
							int a = 0;
							if (imageReady) {
								imageList->AddNew(Filter().Negativo(imageList->getPrevCurrent(), hWnd));
								imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							}
							if(videoReady)
								Filter().NegativoVideo(newvideo, hWnd);
							break;
					}
				}
				if(imageReady)
					updateFilterCount(hWnd, imageList->count - 1);
				if(videoReady)
					updateFilterCount(hWnd, newvideo->filterCount);
				break;
			}
		}
	}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
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