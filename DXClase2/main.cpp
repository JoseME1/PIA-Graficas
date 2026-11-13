//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment (lib, "dinput8.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <dinput.h>
#include <fstream>
using namespace std;
#include "DXRender.h"

LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;

const int Width = 1920;
const int Height = 1080;

// Pantalla completa sin bordes (F11): la ventana cubre el monitor y la barra de
// tareas queda detras. El backbuffer ya es de 1920x1080, asi que no hay que
// redimensionar el swapchain.
static bool pantallaCompleta = false;
static bool f11Presionada = false;
static bool escPresionada = false;
static RECT ventanaAnterior;
static LONG_PTR estiloAnterior = 0;

void AlternarPantallaCompleta(HWND hWnd)
{
	if (!pantallaCompleta)
	{
		GetWindowRect(hWnd, &ventanaAnterior);
		estiloAnterior = GetWindowLongPtr(hWnd, GWL_STYLE);

		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);

		SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(hWnd, HWND_TOP,
			mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
		pantallaCompleta = true;
	}
	else
	{
		SetWindowLongPtr(hWnd, GWL_STYLE, estiloAnterior);
		SetWindowPos(hWnd, NULL,
			ventanaAnterior.left, ventanaAnterior.top,
			ventanaAnterior.right - ventanaAnterior.left,
			ventanaAnterior.bottom - ventanaAnterior.top,
			SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
		pantallaCompleta = false;
	}
}

DXRender *dxrender;

//Function Prototypes//
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed);
int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	dxrender = new DXRender();
	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!dxrender->InitDevice(hwnd, Width, Height))	//Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!dxrender->InitScene(Width, Height))	//Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if(!dxrender->InitDirectInput(hInstance, hwnd))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	messageloop();

	dxrender->ReleaseObjects();

	return 0;
}

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	} WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(
		NULL,
		WndClassName,
		L"Plantilla dx11",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

int messageloop(){
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
			);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
			// run game code

			dxrender->UpdateScene();
			dxrender->DetectInput();
			dxrender->DrawScene();

		}
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE && !escPresionada){
			escPresionada = true;
			if (dxrender)
				dxrender->AlternarMenuPausa();
		}
		if (wParam == VK_F11 && !f11Presionada){
			f11Presionada = true;
			AlternarPantallaCompleta(hwnd);
		}
		return 0;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			escPresionada = false;
		if (wParam == VK_F11)
			f11Presionada = false;
		return 0;

	case WM_ACTIVATEAPP:
		if (dxrender)
			dxrender->ReacquireInput();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}