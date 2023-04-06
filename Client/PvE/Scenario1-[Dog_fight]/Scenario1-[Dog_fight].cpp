// Texture_Version07-9-1.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Scenario1-[Dog_fight].h"
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
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	active_servernum = MAX_SERVER - 1;

	CS_LOGIN_PACKET login_pack;
	login_pack.size = sizeof(CS_LOGIN_PACKET);
	login_pack.type = CS_LOGIN;
	strcpy_s(login_pack.name, "COPTER");

	// Active Server에 연결
	sockets[active_servernum] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server0_addr;
	ZeroMemory(&server0_addr, sizeof(server0_addr));
	server0_addr.sin_family = AF_INET;
	int new_portnum = PORT_NUM_S0 + active_servernum;
	server0_addr.sin_port = htons(new_portnum);
	inet_pton(AF_INET, SERVER_ADDR, &server0_addr.sin_addr);
	connect(sockets[active_servernum], reinterpret_cast<sockaddr*>(&server0_addr), sizeof(server0_addr));

	sendPacket(&login_pack, active_servernum);
	recvPacket(active_servernum);
	//==================================================

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_SCENARIO1DOGFIGHT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCENARIO1DOGFIGHT));

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
					sendPacket(&keyinput_pack, active_servernum);
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
					sendPacket(&mouseinput_pack, active_servernum);
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
					if (i == my_info.m_id) continue;

					if (other_players[i].m_state == OBJ_ST_RUNNING) {
						gGameFramework.SetPosition_OtherPlayerObj(i, other_players[i].m_pos);
						gGameFramework.SetVectors_OtherPlayerObj(i, other_players[i].m_right_vec, other_players[i].m_up_vec, other_players[i].m_look_vec);
					}
					else if (other_players[i].m_state == OBJ_ST_LOGOUT) {
						other_players[i].m_state = OBJ_ST_EMPTY;
						gGameFramework.Remove_OtherPlayerObj(i);
					}

					if (gGameFramework.m_nMode == SCENE2STAGE)
					{
						other_players[i].m_state == OBJ_ST_LOGOUT;
						gGameFramework.Remove_OtherPlayerObj(i);
					}
				}

				// 3. Bullet 객체 최신화
				for (int i = 0; i < MAX_BULLET; i++) {
					if (bullets_info[i].m_state == OBJ_ST_EMPTY) continue;

					if (bullets_info[i].m_state == OBJ_ST_LOGOUT) {	// Clear
						gGameFramework.SetPosition_Bullet(i, bullets_info[i].m_pos, bullets_info[i].m_right_vec, bullets_info[i].m_up_vec, bullets_info[i].m_look_vec);
						((CMainPlayer*)gGameFramework.m_pPlayer)->m_ppBullets[i]->SetScale(0.01f, 0.01f, 0.01f);
						((CMainPlayer*)gGameFramework.m_pPlayer)->m_ppBullets[i]->Rotate(45.0, 0.0, 0.0);
						bullets_info[i].returnToInitialState();
					}
					else if (bullets_info[i].m_state == OBJ_ST_RUNNING) {	// Update
						gGameFramework.SetPosition_Bullet(i, bullets_info[i].m_pos, bullets_info[i].m_right_vec, bullets_info[i].m_up_vec, bullets_info[i].m_look_vec);

						((CMainPlayer*)gGameFramework.m_pPlayer)->m_ppBullets[i]->SetScale(4.0, 4.0, 18.0);
						((CMainPlayer*)gGameFramework.m_pPlayer)->m_ppBullets[i]->Rotate(100.0, 0.0, 0.0);
						XMFLOAT3 xmf3bulletPosition = bullets_info[i].m_pos;
						XMFLOAT3 xmf3bulletLook = my_info.m_look_vec;

					}
				}

				myTime++;
				if (myTime % 60 == 0) {
					gGameFramework.m_time++;
					if (gGameFramework.m_time == 10) {
						gGameFramework.m_time = 0;
					}
				}

				// 4. 자신의 총알 개수 최신화 (UI)
				wchar_t MyBullet[20];
				_itow_s(my_info.m_bullet, MyBullet, sizeof(MyBullet), 10);
				wcscpy_s(gGameFramework.m_myBullet, MyBullet);

				// 5. HP 최신화	(UI)
				wchar_t MyHp[20];
				_itow_s(my_info.m_hp, MyHp, sizeof(MyHp), 10);
				wcscpy_s(gGameFramework.m_myhp, MyHp);
				gGameFramework.m_currHp = my_info.m_hp;

				//// 6. NPC 움직임 최신화
				//for (int i{}; i < MAX_NPCS; i++) {
				//	gGameFramework.SetPosition_NPC(npcs_info[i].m_id, npcs_info[i].m_pos);
				//	gGameFramework.SetVectors_NPC(npcs_info[i].m_id, npcs_info[i].m_right_vec, npcs_info[i].m_up_vec, npcs_info[i].m_look_vec);
				//}
			}

			//==================================================
//				충돌 이펙트 구현 코드
			//==================================================
			if (!coll_info.empty()) {
				XMFLOAT3 collision_effect_pos = coll_info.front();
				coll_info.pop();

				std::cout << collision_effect_pos.x << ", " << collision_effect_pos.y << ", " << collision_effect_pos.z << endl;
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
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCENARIO1DOGFIGHT));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_LABPROJECT0801);
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

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	gGameFramework.ChangeSwapChainState();
#endif

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
