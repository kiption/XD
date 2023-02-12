// LabProject07-9-7.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "REVENGER.h"
#include "GameFramework.h"
#include "Network.h"
#define MAX_LOADSTRING 100

HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];

CGameFramework					gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//==================================================
//					Server Code
//==================================================
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	connect(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	strcpy_s(p.name, "COPTER");

	sendPacket(&p);
	recvPacket();
	//==================================================
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT0797ANIMATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT0797ANIMATION));

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
			if (gGameFramework.m_nMode != SCENE2STAGE)
			{

				//==================================================
				//		새로운 키 입력을 서버에게 전달합니다.
				//==================================================
				if (!gGameFramework.CheckNewInputExist_Keyboard()) {
					short keyValue = gGameFramework.PopInputVal_Keyboard();
					CS_INPUT_KEYBOARD_PACKET keyinput_pack;
					keyinput_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
					keyinput_pack.type = CS_INPUT_KEYBOARD;
					keyinput_pack.direction = keyValue;

					cout << "[Keyboard] Send KeyValue - " << keyinput_pack.direction << endl;//test
					sendPacket(&keyinput_pack);
				}

				if (!gGameFramework.CheckNewInputExist_Mouse()) {
					MouseInputVal mouseValue = gGameFramework.PopInputVal_Mouse();
					CS_INPUT_MOUSE_PACKET mouseinput_pack;
					mouseinput_pack.size = sizeof(CS_INPUT_MOUSE_PACKET);
					mouseinput_pack.type = CS_INPUT_MOUSE;
					mouseinput_pack.key_val = mouseValue.button;
					mouseinput_pack.delta_x = mouseValue.delX;
					mouseinput_pack.delta_y = mouseValue.delY;

					cout << "[Mouse] Send KeyValue - " << mouseinput_pack.key_val << endl;//test
					sendPacket(&mouseinput_pack);
				}
				//==================================================

				//==================================================
				//	    서버로부터 받은 값으로 최신화해줍니다.
				//==================================================
				// 1. 자기자신 Player 객체 최신화
				gGameFramework.SetPosition_PlayerObj(my_info.m_pos);
				gGameFramework.SetVectors_PlayerObj(my_info.m_right_vec, my_info.m_up_vec, my_info.m_look_vec);

				// 2. 다른 Player 객체 최신화
				for (int i = 0; i < MAX_USER; i++) {
					if (i == my_info.m_id || other_players[i].m_state == OBJ_ST_EMPTY) continue;

					if (other_players[i].m_state == OBJ_ST_RUNNING) {
						gGameFramework.SetPosition_OtherPlayerObj(i, other_players[i].m_pos);
						gGameFramework.SetVectors_OtherPlayerObj(i, other_players[i].m_right_vec, other_players[i].m_up_vec, other_players[i].m_look_vec);
						
					}
					else if (other_players[i].m_state == OBJ_ST_LOGOUT) {
						other_players[i].m_state = OBJ_ST_EMPTY;
						gGameFramework.Remove_OtherPlayerObj(i);
					}
				}

				// 3. Bullet 객체 최신화
				for (int i = 0; i < MAX_BULLET; i++) {
					if (bullets_info[i].m_state == OBJ_ST_EMPTY) {
						continue;
					}

					if (bullets_info[i].m_state == OBJ_ST_STANDBY) {	// Create
						//gGameFramework.Create_Bullet(i, bullets_info[i].m_pos, bullets_info[i].m_look_vec);

						bullets_info[i].m_state = OBJ_ST_RUNNING;
					}
					else if (bullets_info[i].m_state == OBJ_ST_LOGOUT) {	// Clear
						bullets_info[i].m_id = -1;
						bullets_info[i].m_pos = { 0.0f, 0.0f, 0.0f };
						bullets_info[i].m_right_vec = { 1.0f, 0.0f, 0.0f };
						bullets_info[i].m_up_vec = { 0.0f, 1.0f, 0.0f };
						bullets_info[i].m_look_vec = { 0.0f, 0.0f, 1.0f };
						bullets_info[i].m_state = OBJ_ST_EMPTY;
					}
					else if (bullets_info[i].m_state == OBJ_ST_RUNNING) {	// Update
						gGameFramework.SetPosition_Bullet(i, bullets_info[i].m_pos, bullets_info[i].m_right_vec, bullets_info[i].m_up_vec, bullets_info[i].m_look_vec);
						gGameFramework.m_pScene->m_ppBullets[i]->SetScale(5.0, 5.0, 11.0);
						gGameFramework.m_pScene->m_ppBullets[i]->Rotate(125.0,0.0,0.0);
					}
				}

				// 4. 자신의 총알 개수 최신화
				wchar_t MyBullet[20];
				_itow_s(my_info.m_bullet, MyBullet, sizeof(MyBullet), 10);
				wcscpy_s(gGameFramework.m_myBullet, MyBullet);

				// 5. HP 최신화


			}

			//==================================================

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
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
	::UpdateWindow(hMainWnd);

	return(TRUE);
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
