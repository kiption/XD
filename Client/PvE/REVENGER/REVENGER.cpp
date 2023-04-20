﻿// LabProject07-9-7.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "REVENGER.h"
#include "GameFramework.h"
#include "BillboardObjectsShader.h"
#define MAX_LOADSTRING 100

HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];
const WCHAR CLASS_NAME[] = L"MFPlay Window Class";
const WCHAR WINDOW_NAME[] = L"MFPlay Sample Application";
CGameFramework					gGameFramework;
void    OnClose(HWND hwnd);
void    OnFileOpen(HWND hwnd);
HRESULT PlayMediaFile(HWND hwnd, const WCHAR* sURL);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, HWND*, int);
BOOL    InitializeWindow(HINSTANCE, HWND* pHwnd);
void OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT* pEvent);
void OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT* pEvent);
void OnMediaItemEND(MFP_PLAYBACK_ENDED_EVENT* /*pEvent*/);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
#include <mfplay.h>
#include <Shlwapi.h>
BOOL                    g_bHasVideo = FALSE;
bool					g_TurnOpening = false;

class MediaPlayerCallback : public IMFPMediaPlayerCallback
{
	long m_cRef; // Reference count

public:

	MediaPlayerCallback() : m_cRef(1)
	{
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(MediaPlayerCallback, IMFPMediaPlayerCallback),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		ULONG count = InterlockedDecrement(&m_cRef);
		if (count == 0)
		{
			delete this;
			return 0;
		}
		return count;
	}

	// IMFPMediaPlayerCallback methods
	void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader);
};


void MediaPlayerCallback::OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader)
{
	if (FAILED(pEventHeader->hrEvent))
	{
		//ShowErrorMessage(L"Playback error", pEventHeader->hrEvent);
		return;
	}

	switch (pEventHeader->eEventType)
	{
	case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
		OnMediaItemCreated(MFP_GET_MEDIAITEM_CREATED_EVENT(pEventHeader));
		break;

	case MFP_EVENT_TYPE_MEDIAITEM_SET:
		OnMediaItemSet(MFP_GET_MEDIAITEM_SET_EVENT(pEventHeader));
		break;
	case MFP_EVENT_TYPE_PLAYBACK_ENDED:
		OnMediaItemEND(MFP_GET_PLAYBACK_ENDED_EVENT(pEventHeader));
		break;
	}
}

IMFPMediaPlayer* g_pPlayer = NULL;      // The MFPlay player object.
MediaPlayerCallback* g_pPlayerCB = NULL;    // Application callback object.

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg{};
	HWND hwnd{};
	HACCEL hAccelTable;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	(void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT0797ANIMATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, &hwnd, nCmdShow))return(FALSE);
	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT0797ANIMATION));

	/*if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		return 0;
	}*/
	//if (g_TurnOpening == false) {
	//	PlayMediaFile(hwnd, L"C:/Users/재성/Desktop/Direct12-예제 프로젝트/LabProject02-1/Opening/TEST.avi");
	//	while (GetMessage(&msg, NULL, 0, 0))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//		if (g_TurnOpening == true)
	//		{
	//			DestroyWindow(hwnd);
	//			CoUninitialize();
	//			break;
	//		}
	//	}*/

	//}
	//else
	//{
	//}

	//if (g_TurnOpening == false)
	//{
	//	PlayMediaFile(hwnd, L"C:/Users/재성/Desktop/Direct12-예제 프로젝트/LabProject02-1/Opening/TEST.avi");
	//	while (GetMessage(&msg, NULL, 0, 0))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//		if (g_TurnOpening == true)break;
	//	}

	//}
	//else
	//{
	while (1)
	{


		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			gGameFramework.FrameAdvance();
		}
	}

	gGameFramework.OnDestroy();
	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABPROJECT0797ANIMATION));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, HWND* MainWnd, int nCmdShow)
{

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;

	HWND hwnd = CreateWindow(
		szWindowClass,
		szTitle,
		dwStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	//if (g_TurnOpening == false)
	//{

	//	WNDCLASS wc = { 0 };

	//	wc.lpfnWndProc = WndProc;
	//	wc.hInstance = GetModuleHandle(NULL);
	//	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//	wc.lpszClassName = CLASS_NAME;
	//	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	//	if (!RegisterClass(&wc))
	//	{
	//		return FALSE;
	//	}
	//	*MainWnd = hwnd;
	//	ShowWindow(hwnd, SW_SHOWDEFAULT);
	//	UpdateWindow(hwnd);

	//}
	//else
	//{
	//}
	if (!hwnd)
	{
		return FALSE;
	}

	gGameFramework.OnCreate(hInstance, hwnd);
	*MainWnd = hwnd;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	return TRUE;
}

BOOL InitializeWindow(HINSTANCE hInstance, HWND* pHwnd)
{
	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;

	HWND hwnd = CreateWindow(
		szWindowClass,
		szTitle,
		dwStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	if (g_TurnOpening == false)
	{

		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = WndProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = CLASS_NAME;
		wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
		if (!RegisterClass(&wc))
		{
			return FALSE;
		}
		*pHwnd = hwnd;
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

	}
	if (g_TurnOpening == true)
	{

		ghAppInstance = hInstance;
		RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
		DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
		AdjustWindowRect(&rc, dwStyle, FALSE);
		HWND hwnd = CreateWindow(
			szWindowClass,
			szTitle,
			dwStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rc.right - rc.left,
			rc.bottom - rc.top,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);
		if (!hwnd)
		{
			return FALSE;
		}

		gGameFramework.OnCreate(hInstance, hwnd);
		*pHwnd = hwnd;
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);
		//#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
		//		gGameFramework.ChangeSwapChainState();
		//#endif

	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		return 1;
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}




// Menu handlers

//// Constants 
//const WCHAR CLASS_NAME[] = L"MFPlay Window Class";
//const WCHAR WINDOW_NAME[] = L"MFPlay Sample Application";
// Global variables

void OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT* pEvent)
{
	HRESULT hr = S_OK;

	// The media item was created successfully.

	if (g_pPlayer)
	{
		BOOL bHasVideo = FALSE, bIsSelected = FALSE;

		// Check if the media item contains video.
		hr = pEvent->pMediaItem->HasVideo(&bHasVideo, &bIsSelected);

		if (FAILED(hr)) { goto done; }

		g_bHasVideo = bHasVideo && bIsSelected;

		// Set the media item on the player. This method completes asynchronously.
		hr = g_pPlayer->SetMediaItem(pEvent->pMediaItem);
	}

done:
	if (FAILED(hr))
	{
		//ShowErrorMessage(L"Error playing this file.", hr);
	}
}




void OnFileOpen(HWND hwnd)
{
	HRESULT hr = S_OK;

	IFileOpenDialog* pFileOpen = NULL;
	IShellItem* pItem = NULL;

	PWSTR pwszFilePath = NULL;

	// Create the FileOpenDialog object.
	hr = CoCreateInstance(
		__uuidof(FileOpenDialog),
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pFileOpen)
	);

	if (FAILED(hr)) { goto done; }


	hr = pFileOpen->SetTitle(L"Select a File to Play");

	if (FAILED(hr)) { goto done; }


	// Show the file-open dialog.
	hr = pFileOpen->Show(hwnd);

	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{
		// User cancelled.
		hr = S_OK;
		goto done;
	}
	if (FAILED(hr)) { goto done; }


	// Get the file name from the dialog.
	hr = pFileOpen->GetResult(&pItem);

	if (FAILED(hr)) { goto done; }


	hr = pItem->GetDisplayName(SIGDN_URL, &pwszFilePath);

	if (FAILED(hr)) { goto done; }


	// Open the media file.
	hr = PlayMediaFile(hwnd, pwszFilePath);

	if (FAILED(hr)) { goto done; }

done:
	if (FAILED(hr))
	{
		//ShowErrorMessage(L"Could not open file.", hr);
	}

	CoTaskMemFree(pwszFilePath);

	if (pItem)
	{
		pItem->Release();
	}
	if (pFileOpen)
	{
		pFileOpen->Release();
	}
}
void OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT* /*pEvent*/)
{
	HRESULT hr = S_OK;

	hr = g_pPlayer->Play();

	//if (FAILED(hr))
	//{
	//	ShowErrorMessage(L"IMFPMediaPlayer::Play failed.", hr);
	//}
}
void OnMediaItemEND(MFP_PLAYBACK_ENDED_EVENT* /*pEvent*/)
{
	HRESULT hr = S_OK;

	hr = g_pPlayer->Stop();
	g_TurnOpening = true;
	//if (FAILED(hr))
	//{
	//	ShowErrorMessage(L"IMFPMediaPlayer::Play failed.", hr);
	//}
}
#include <winapifamily.h>
//const WCHAR* sURL = L"C:\\Users\\Public\\Videos\\example.wmv";
HRESULT PlayMediaFile(HWND hwnd, const WCHAR* sURL)
{
	HRESULT hr = S_OK;

	// Create the MFPlayer object.
	if (g_pPlayer == NULL)
	{
		g_pPlayerCB = new (std::nothrow) MediaPlayerCallback();

		if (g_pPlayerCB == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}

		hr = MFPCreateMediaPlayer(
			NULL,
			FALSE,          // Start playback automatically?
			0,              // Flags
			g_pPlayerCB,    // Callback pointer
			hwnd,           // Video window
			&g_pPlayer
		);

		if (FAILED(hr)) { goto done; }
	}

	// Create a new media item for this URL.
	hr = g_pPlayer->CreateMediaItemFromURL(sURL, FALSE, 0, NULL);

	// The CreateMediaItemFromURL method completes asynchronously. 
	// The application will receive an MFP_EVENT_TYPE_MEDIAITEM_CREATED 
	// event. See MediaPlayerCallback::OnMediaPlayerEvent().


done:
	return hr;
}
void OnClose(HWND /*hwnd*/)
{
	if (g_pPlayer)
	{
		g_pPlayer->Shutdown();
		g_pPlayer->Release();
		g_pPlayer = NULL;
	}

	if (g_pPlayerCB)
	{
		g_pPlayerCB->Release();
		g_pPlayerCB = NULL;
	}

	PostQuitMessage(0);
}