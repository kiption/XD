// LabProject07-9-7.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "REVENGER.h"
#include "GameFramework.h"
#include "Network.h"//Server
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
	//server0_addr.sin_port = htons(PORTNUM_RELAY2CLIENT_0);		// 릴레이서버로 연결하려면 123,124를 지우고 여기를 주석해제하면됨.
	inet_pton(AF_INET, SERVER_ADDR, &server0_addr.sin_addr);
	connect(sockets[active_servernum], reinterpret_cast<sockaddr*>(&server0_addr), sizeof(server0_addr));

	sendPacket(&login_pack, active_servernum);
	recvPacket(active_servernum);
	//==================================================

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	(void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT0797ANIMATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, &hwnd, nCmdShow))return(FALSE);
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
			if (gGameFramework.m_nMode != SCENE2STAGE && gGameFramework.m_nMode != OPENINGSCENE)
			{

				//==================================================
				//		새로운 키 입력을 서버에게 전달합니다.
				//==================================================
				if (gGameFramework.checkNewInput_Keyboard()) {
					short keyValue = gGameFramework.popInputVal_Keyboard();

					switch (keyValue) {
					case PACKET_KEY_NUM1:
					case PACKET_KEY_NUM2:
						CS_INPUT_KEYBOARD_PACKET keyinput_pack;
						keyinput_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
						keyinput_pack.type = CS_INPUT_KEYBOARD;
						keyinput_pack.keytype = keyValue;
						sendPacket(&keyinput_pack, active_servernum);
						break;
					case PACKET_KEY_W:
					case PACKET_KEY_S:
						CS_MOVE_PACKET move_pack;
						move_pack.size = sizeof(CS_MOVE_PACKET);
						move_pack.type = CS_MOVE;
						move_pack.x = gGameFramework.getMyPosition().x;
						move_pack.y = gGameFramework.getMyPosition().y;
						move_pack.z = gGameFramework.getMyPosition().z;
						sendPacket(&move_pack, active_servernum);
						break;
					case PACKET_KEY_A:
					case PACKET_KEY_D:
						CS_ROTATE_PACKET rotate_pack;
						rotate_pack.size = sizeof(CS_ROTATE_PACKET);
						rotate_pack.type = CS_ROTATE;
						rotate_pack.right_x = gGameFramework.getMyRightVec().x;
						rotate_pack.right_y = gGameFramework.getMyRightVec().y;
						rotate_pack.right_z = gGameFramework.getMyRightVec().z;
						rotate_pack.up_x = gGameFramework.getMyUpVec().x;
						rotate_pack.up_y = gGameFramework.getMyUpVec().y;
						rotate_pack.up_z = gGameFramework.getMyUpVec().z;
						rotate_pack.look_x = gGameFramework.getMyLookVec().x;
						rotate_pack.look_y = gGameFramework.getMyLookVec().y;
						rotate_pack.look_z = gGameFramework.getMyLookVec().z;
						sendPacket(&rotate_pack, active_servernum);
						break;
					case PACKET_KEY_UP:
					case PACKET_KEY_DOWN:
					case PACKET_KEY_LEFT:
					case PACKET_KEY_RIGHT:
						CS_MOVE_PACKET move_pack2;
						move_pack2.size = sizeof(CS_MOVE_PACKET);
						move_pack2.type = CS_MOVE;
						move_pack2.x = gGameFramework.getMyPosition().x;
						move_pack2.y = gGameFramework.getMyPosition().y;
						move_pack2.z = gGameFramework.getMyPosition().z;
						sendPacket(&move_pack2, active_servernum);
						break;
					case PACKET_KEY_SPACEBAR:
						//CS_ATTACK_PACKET attack_pack;
						//attack_pack.size = sizeof(CS_ATTACK_PACKET);
						//attack_pack.type = CS_ATTACK;
						//sendPacket(&rotate_pack, active_servernum);
						break;
					default:
						cout << "[KeyInput Error] Unknown Key Type." << endl;
					}

				}
				if (gGameFramework.checkNewInput_Mouse()) {
					MouseInputVal mouseValue = gGameFramework.popInputVal_Mouse();
					CS_INPUT_MOUSE_PACKET mouseinput_pack;
					mouseinput_pack.size = sizeof(CS_INPUT_MOUSE_PACKET);
					mouseinput_pack.type = CS_INPUT_MOUSE;
					mouseinput_pack.buttontype = mouseValue.button;
					mouseinput_pack.delta_x = mouseValue.delX;
					mouseinput_pack.delta_y = mouseValue.delY;

					sendPacket(&mouseinput_pack, active_servernum);
				}

				//==================================================
				//				   플레이어 동기화
				//==================================================
				//1. 다른 플레이어 업데이트
				for (int i = 0; i < MAX_USER; i++) {
					if (i == my_info.m_id) continue;
					if (other_players[i].m_state == OBJ_ST_RUNNING) {
						gGameFramework.setPosition_OtherPlayer(i, other_players[i].m_pos);
						gGameFramework.setVectors_OtherPlayer(i, other_players[i].m_right_vec, other_players[i].m_up_vec, other_players[i].m_look_vec);
					}
					else if (other_players[i].m_state == OBJ_ST_LOGOUT) {
						other_players[i].m_state = OBJ_ST_EMPTY;
						gGameFramework.remove_OtherPlayer(i);
					}

					if (gGameFramework.m_nMode == SCENE2STAGE)
					{
						other_players[i].m_state = OBJ_ST_LOGOUT;
						other_players[i].InfoClear();
						gGameFramework.remove_OtherPlayer(i);
					}
				}

				// 2. NPC 움직임 최신화
				if (gGameFramework.m_nMode == SCENE1STAGE) {
					for (int i{}; i < MAX_NPCS; i++) {
						//cout << npcs_info[i].m_id << "번째 Pos:" << npcs_info[i].m_pos.x << ',' << npcs_info[i].m_pos.y << ',' << npcs_info[i].m_pos.z << endl;
						gGameFramework.setPosition_Npc(npcs_info[i].m_id, npcs_info[i].m_pos);
						gGameFramework.setVectors_Npc(npcs_info[i].m_id, npcs_info[i].m_right_vec, npcs_info[i].m_up_vec, npcs_info[i].m_look_vec);
						gGameFramework.m_pScene->SmokePosition = npcs_info[i].m_pos;
						((Stage1*)gGameFramework.m_pScene)->m_ppSpriteBillboard[0]->m_ppObjects[0]->SetPosition(npcs_info[i].m_pos);
					}
					//cout << "====================" << endl;
				}

				//==================================================
				//					  UI 동기화
				//==================================================
				// 1. 총알 업데이트
				wchar_t MyBullet[20];
				_itow_s(my_info.m_bullet, MyBullet, sizeof(MyBullet), 10);
				wcscpy_s(gGameFramework.m_myBullet, MyBullet);

				// 2. HP 업데이트
				wchar_t MyHp[20];
				_itow_s(my_info.m_hp, MyHp, sizeof(MyHp), 10);
				wcscpy_s(gGameFramework.m_myhp, MyHp);
				gGameFramework.m_currHp = my_info.m_hp;

				// 3. 시간 동기화
				gGameFramework.m_10MinOfTime = servertime_sec / 600;
				gGameFramework.m_1MinOfTime = (servertime_sec - gGameFramework.m_10MinOfTime * 600) / 60;
				gGameFramework.m_10SecOftime = (servertime_sec - gGameFramework.m_1MinOfTime * 60) / 10;
				gGameFramework.m_1SecOfTime = servertime_sec % 10;

				// 4. NPC 객체 수 동기화
				gGameFramework.m_remainNPC = left_npc;
				//==================================================
				//					충돌 이펙트 관련
				//==================================================
				// 1. 자기 자신
				if (my_info.m_damaged_effect_on) {
					// 여기에 이펙트 넣어줘.
				}

				// 2. 다른 플레이어
				for (auto& other_pl : other_players) {
					if (other_pl.m_damaged_effect_on) {
						//((Stage1*)gGameFramework.m_pScene)->m_ppSpriteBillboard[0]->m_ppObjects[0]->SetPosition(other_pl.m_pos);
					}
				}

				// 3. NPC
				for (auto& npc : npcs_info) {
					if (npc.m_damaged_effect_on) {
						// 여기에 이펙트 넣어줘.
					}
				}

				//==================================================

				//==================================================
				//					  Map Test
				//==================================================
				for (auto& mapobj : stage1_mapobj_info) {
					/*cout << "Map Obj - Pos: " << mapobj.m_pos.x << ", " << mapobj.m_pos.y << ", " << mapobj.m_pos.z
						<< " Scale: " << mapobj.m_scale.x << ", " << mapobj.m_scale.y << ", " << mapobj.m_scale.z << "." << endl;*/
					
					gGameFramework.CollisionMap_by_PLAYER(XMFLOAT3(mapobj.m_xoobb.Center), XMFLOAT3(mapobj.m_xoobb.Extents));
					
				}
				//gGameFramework.CollisionEndWorldObject(XMFLOAT3(0.0, 0.0, 0.0), XMFLOAT3(1200.0, 110.0, 1200.0));
				gGameFramework.CollisionMap_by_BULLET(XMFLOAT3(/*MAP CENTER*/), XMFLOAT3(/*MAP EXTENTS*/));
				gGameFramework.CollisionNPC_by_BULLET(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/));
				gGameFramework.CollisionNPC_by_MAP(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/), XMFLOAT3(/*MAP CENTER*/), XMFLOAT3(/*MAP EXTENTS*/));
				gGameFramework.CollisionNPC_by_PLAYER(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/));
				//==================================================
			}
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