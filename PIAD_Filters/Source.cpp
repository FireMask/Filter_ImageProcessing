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

char* source_window = "Source image";
bool imageReady = false;
int defaultSize = 720;

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
	case WM_COMMAND:{
		switch(wParam){
		case SAVE_FILE:{
			String savePath(SaveFile());
			if (savePath.compare("")) {
				try {
					imwrite(savePath, imageList->getCurrent());
				}
				catch (Exception ex) {
					return 1;
				}
			}
			break;
		}
		case SELECT_FILE:{
				String imagePath(getFileName());
				if (imagePath.compare("")) {
					original = imread(imagePath, 1);
					imageList = new Lista();
					imageList->Add(new Imagen(original));
					namedWindow(source_window, CV_WINDOW_AUTOSIZE);
					imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
					imageReady = true;
				}
				break;
			}
			case RESET:{
				imshow(source_window, GetSquareImage(original, defaultSize));
				imageList = new Lista();
				imageList->Add(new Imagen(original));
				break;
			}
			case BACK_FILTER:
				imageList->backFilter();
				imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
				break;
			case FORWARD_FILTER:
				imageList->forwardFilter();
				imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
				break;
			case APPLY_FILTER:{
				int value = SendMessage(GetDlgItem(hWnd, FILTER_BOX), CB_GETCURSEL, NULL, NULL);
				if (value != -1 && imageReady) {
					switch (value) {
						case 0: //Media
							imageList->AddNew();
							imageList->setCurrent(flt.Media(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 1: //Media Ponderada
							imageList->AddNew();
							imageList->setCurrent(flt.MediaPonderada(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 2: //Mediana
							
							break;
						case 3: //Blur
							imageList->AddNew();
							blur(imageList->getPrevCurrent(), imageList->getCurrent(), Size(5, 5));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 4: //Gaussiano
							imageList->AddNew();
							imageList->setCurrent(flt.Gaussiano(&imageList->getPrevCurrent(), .85));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 5: //Laplaciano
							imageList->AddNew();
							imageList->setCurrent(flt.Laplaciano(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 6: //Menos Laplaciano
							imageList->AddNew();
							imageList->setCurrent(flt.MenosLaplaciano(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 7:{ //Sobel
							imageList->AddNew();
							imageList->setCurrent(flt.Sobel(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						}
						case 8: //Corrección Logaritmica
							imageList->AddNew();
							imageList->setCurrent(flt.CorreccionLogaritmica(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 9: //Potencia
							imageList->AddNew();
							imageList->setCurrent(flt.Potencia(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 10: //Sharpen
							imageList->AddNew();
							imageList->setCurrent(flt.Sharpen(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 11: //Paso Bajo
							imageList->AddNew();
							imageList->setCurrent(flt.PasoBajo(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 12: //Sustraccion de la Media
							imageList->AddNew();
							imageList->setCurrent(flt.SustraccionMedia(&imageList->getPrevCurrent()));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
					}
					updateFilterCount(hWnd, imageList);
				}
				break;
			}
			case APPLY_EFFECT: {			
				int value = SendMessage(GetDlgItem(hWnd, EFFECT_BOX), CB_GETCURSEL, NULL, NULL);
				if (value != -1 && imageReady) {
					switch (value) {
						case 0: //Blanco y Negro
							imageList->AddNew();
							imageList->setCurrent(Filter().Binario(imageList->getPrevCurrent(), 127, hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 1: //Luminosidad
							imageList->AddNew();
							imageList->setCurrent(Filter().Luminosidad(imageList->getPrevCurrent(), hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 2: //Promedio
							imageList->AddNew();
							imageList->setCurrent(Filter().Average(imageList->getPrevCurrent(), hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 3: //Luminancia
							imageList->AddNew();
							imageList->setCurrent(Filter().Luminancia(imageList->getPrevCurrent(), hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 4: //Sepia
							imageList->AddNew();
							imageList->setCurrent(Filter().Sepia(imageList->getPrevCurrent(), hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 5: //Binario
							imageList->AddNew();
							imageList->setCurrent(Filter().Binario(imageList->getPrevCurrent(), 155, hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
						case 6: //Negativo
							imageList->AddNew();
							imageList->setCurrent(Filter().Negativo(imageList->getPrevCurrent(), hWnd));
							imshow(source_window, GetSquareImage(imageList->getCurrent(), defaultSize));
							break;
					}
				}
				updateFilterCount(hWnd, imageList);
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