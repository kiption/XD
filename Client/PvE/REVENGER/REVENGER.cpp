// LabProject07-9-7.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "REVENGER.h"
#include "GameFramework.h"
#include "BillboardObjectsShader.h"
#include "Network.h"//Server
#include <thread>//Server
#include <chrono>//Server
using namespace chrono;
#define MAX_LOADSTRING 100

HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];


CGameFramework					gGameFramework;


ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

void networkThreadFunc();
void uiThreadFunc();

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg{};
	HACCEL hAccelTable;

	(void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT0797ANIMATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT0797ANIMATION));

	// Server Code
	thread networkThread(networkThreadFunc);
	networkThread.detach();

	thread uiThread(uiThreadFunc);
	uiThread.detach();
	//==

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
			if (gGameFramework.m_nMode == OPENINGSCENE && gGameFramework.m_nMode != SCENE1STAGE)
			{
				if (stage1_enter_ok) {
					gGameFramework.ChangeScene(SCENE1STAGE);
					gGameFramework.setPosition_Self(my_info.m_pos);
					gGameFramework.setVectors_Self(my_info.m_right_vec, my_info.m_up_vec, my_info.m_look_vec);
				}
			}
			else
			{
				if (gGameFramework.m_nMode == SCENE1STAGE) {
					if (stage2_enter_ok) {
						gGameFramework.ChangeScene(SCENE2STAGE);
					}
				}

				//==================================================
				//					서버 연결 확인
				//==================================================
				// 1초마다 서버로 핑을 던져서 살아있는지 확인합니다.
				if (chrono::system_clock::now() > last_ping + chrono::seconds(3)) {
					CS_PING_PACKET ping_packet;
					ping_packet.type = CS_PING;
					ping_packet.size = sizeof(CS_PING_PACKET);
					sendPacket(&ping_packet, active_servernum);
					last_ping = chrono::system_clock::now();
					//cout << "ping" << endl;
				}

				//==================================================
				//				   플레이어 동기화
				//==================================================
				//1. 좌표 및 벡터 업데이트
				// 1) 다른 플레이어
				for (int i = 0; i < MAX_USER; ++i) {
					if (i == my_info.m_id) continue;
					if (other_players[i].m_state == OBJ_ST_RUNNING) {
						gGameFramework.setPosition_OtherPlayer(i, other_players[i].m_pos);
						gGameFramework.setVectors_OtherPlayer(i, other_players[i].m_right_vec, other_players[i].m_up_vec, other_players[i].m_look_vec);
					}
					else if (other_players[i].m_state == OBJ_ST_LOGOUT) {
						other_players[i].InfoClear();
						gGameFramework.remove_OtherPlayer(i);
					}
				}

				// 2. NPC
				for (int i{}; i < MAX_NPCS; ++i) {
					//cout << npcs_info[i].m_id << "번째 Pos:" << npcs_info[i].m_pos.x << ',' << npcs_info[i].m_pos.y << ',' << npcs_info[i].m_pos.z << endl;
					if (npcs_info[i].m_id == -1) {
						continue;
					}

					gGameFramework.setPosition_Npc(npcs_info[i].m_id, npcs_info[i].m_pos);
					gGameFramework.setVectors_Npc(npcs_info[i].m_id, npcs_info[i].m_right_vec, npcs_info[i].m_up_vec, npcs_info[i].m_look_vec);
					((Stage1*)gGameFramework.m_pScene)->SmokePosition = npcs_info[i].m_pos;
					//((Stage1*)gGameFramework.m_pScene)->m_pBillboardShader[3]->ParticlePosition = npcs_info[i].m_pos;
				}

				//==================================================
				// 2. 객체 인게임 상태 업데이트 (자기 자신 제외, 자기 자신은 클라 독자적으로 돌아가기 때문)
				//  1) Other Players
				for (int i = 0; i < MAX_USER; ++i) {
					if (i == my_id) break;

					if (other_players[i].m_new_state_update) {
						switch (other_players[i].m_ingame_state) {
						case PL_ST_IDLE:
							gGameFramework.otherPlayerReturnToIdle(i);
							break;
						case PL_ST_MOVE_FRONT: // 앞으로 이동
							gGameFramework.otherPlayerForwardMotion(i);
							break;
						case PL_ST_MOVE_BACK: // 뒤로 이동
							gGameFramework.otherPlayerBackwardMotion(i);
							break;
						case PL_ST_MOVE_SIDE: // 옆으로 이동
							gGameFramework.otherPlayerSfrateMotion(i);
							break;
						case PL_ST_ATTACK:
							gGameFramework.otherPlayerShootingMotion(i);
							break;
							// + 구르기 및 점프
						case PL_ST_DEAD:
							gGameFramework.otherPlayerDyingMotion(i);
							break;
						}

						other_players[i].m_new_state_update = false;
					}
				}

				//  2) NPC
				for (int i = 0; i < MAX_NPCS; ++i) {
					if (npcs_info[i].m_id == -1) continue;

					if (npcs_info[i].m_new_state_update) {
						switch (npcs_info[i].m_ingame_state) {
						case PL_ST_IDLE:
							break;
						case PL_ST_MOVE_FRONT: // 앞으로 이동
							break;
						case PL_ST_MOVE_BACK: // 뒤로 이동
							break;
						case PL_ST_MOVE_SIDE: // 옆으로 이동
							break;
						case PL_ST_ATTACK:
							break;
						case PL_ST_DEAD:
							((Stage1*)gGameFramework.m_pScene)->m_ppShaders[0]->m_ppObjects[10 + i]->m_xmf4x4ToParent._42 = 0.0f;
							break;
						}

						npcs_info[i].m_new_state_update = false;
					}
				}

				//  3) Dummies ([TEST] NPC 완성전까지 임시 코드)
				for (int i = 0; i < 5; ++i) {
					if (dummies[i].m_new_state_update) {
						if (dummies[i].m_ingame_state == PL_ST_DEAD) {
							// 여기에 더미 죽는 모션 실행1
							// (더미 죽는 모션이 한 사이클 완료되면 객체를 날려버리던가 scale 해주면 됨.)
							//gGameFramework.otherPlayerDyingMotion(i);
							gGameFramework.CollisionDummiesObjects(i);

							dummies[i].m_new_state_update = false;
						}
					}
				}

				//==================================================
				//					  UI 동기화
				//==================================================

				//==================================================
				//					Map 충돌 관련
				//==================================================
				for (auto& mapobj : stage1_mapobj_info) {

					/*if (gGameFramework.CollisionMap_by_PLAYER(XMFLOAT3(mapobj.m_xoobb.Center), XMFLOAT3(mapobj.m_xoobb.Extents), gGameFramework.m_pPlayer))
					{
						gGameFramework.m_pPlayer->SetPosition(gGameFramework.m_pPlayer->m_xmf3BeforeCollidedPosition, false);
					}*/
				}

				//gGameFramework.CollisionEndWorldObject(XMFLOAT3(0.0, 0.0, 0.0), XMFLOAT3(1200.0, 110.0, 1200.0));
				gGameFramework.CollisionMap_by_BULLET(XMFLOAT3(/*MAP CENTER*/), XMFLOAT3(/*MAP EXTENTS*/));
				gGameFramework.CollisionNPC_by_BULLET(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/));
				gGameFramework.CollisionNPC_by_MAP(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/), XMFLOAT3(/*MAP CENTER*/), XMFLOAT3(/*MAP EXTENTS*/));
				gGameFramework.CollisionNPC_by_PLAYER(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/));
			}
		}
		gGameFramework.FrameAdvance();
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
	wcex.hCursor = ::LoadCursor(NULL, IDC_CROSS);
	wcex.hbrBackground = (HBRUSH)(COLOR_MENU + 4);
	wcex.lpszMenuName = L"REVENGER";
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDM_ABOUT));

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


void networkThreadFunc()
{
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	active_servernum = MAX_LOGIC_SERVER - 1;

	CS_LOGIN_PACKET login_pack;
	login_pack.size = sizeof(CS_LOGIN_PACKET);
	login_pack.type = CS_LOGIN;
	strcpy_s(login_pack.name, "COPTER");

	// Active Server에 연결
	sockets[active_servernum] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server0_addr;
	ZeroMemory(&server0_addr, sizeof(server0_addr));
	server0_addr.sin_family = AF_INET;
	int new_portnum = PORTNUM_LOGIC_0 + active_servernum;
	server0_addr.sin_port = htons(new_portnum);
	//inet_pton(AF_INET, SERVER_ADDR, &server0_addr.sin_addr);// 루프백
	inet_pton(AF_INET, LOGIC1_ADDR, &server0_addr.sin_addr);
	connect(sockets[active_servernum], reinterpret_cast<sockaddr*>(&server0_addr), sizeof(server0_addr));

	sendPacket(&login_pack, active_servernum);
	recvPacket(active_servernum);

	stage1_enter_ok = false;
	stage2_enter_ok = false;
	last_ping = last_pong = chrono::system_clock::now();

	//==================================================
	// thread loop
	while (1) {
		// Recv Callback호출을 위한 SleepEX
		SleepEx(1, TRUE);

		//==================================================
		//		새로운 키 입력을 서버에게 전달합니다.
		//==================================================
		// 1. 키보드 입력
		if (gGameFramework.checkNewInput_Keyboard()) {
			short keyValue = gGameFramework.popInputVal_Keyboard();

			switch (keyValue) {
			case PACKET_KEY_NUM1:
				if (gGameFramework.m_nMode == OPENINGSCENE) {
					CS_INPUT_KEYBOARD_PACKET keyinput_pack;
					keyinput_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
					keyinput_pack.type = CS_INPUT_KEYBOARD;
					keyinput_pack.keytype = keyValue;
					sendPacket(&keyinput_pack, active_servernum);
					break;
				}
			case PACKET_KEY_NUM2:
				if (gGameFramework.m_nMode == SCENE1STAGE) {
					CS_INPUT_KEYBOARD_PACKET keyinput_pack;
					keyinput_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
					keyinput_pack.type = CS_INPUT_KEYBOARD;
					keyinput_pack.keytype = keyValue;
					sendPacket(&keyinput_pack, active_servernum);
					break;
				}
			case PACKET_KEY_W:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_MOVE_PACKET move_front_pack;
				move_front_pack.size = sizeof(CS_MOVE_PACKET);
				move_front_pack.type = CS_MOVE;
				move_front_pack.x = gGameFramework.getMyPosition().x;
				move_front_pack.y = gGameFramework.getMyPosition().y;
				move_front_pack.z = gGameFramework.getMyPosition().z;
				move_front_pack.direction = MV_FRONT;
				sendPacket(&move_front_pack, active_servernum);
				break;

			case PACKET_KEY_S:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_MOVE_PACKET move_back_pack;
				move_back_pack.size = sizeof(CS_MOVE_PACKET);
				move_back_pack.type = CS_MOVE;
				move_back_pack.x = gGameFramework.getMyPosition().x;
				move_back_pack.y = gGameFramework.getMyPosition().y;
				move_back_pack.z = gGameFramework.getMyPosition().z;
				move_back_pack.direction = MV_BACK;
				sendPacket(&move_back_pack, active_servernum);
				break;

			case PACKET_KEY_A:
			case PACKET_KEY_D:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_MOVE_PACKET move_side_pack;
				move_side_pack.size = sizeof(CS_MOVE_PACKET);
				move_side_pack.type = CS_MOVE;
				move_side_pack.x = gGameFramework.getMyPosition().x;
				move_side_pack.y = gGameFramework.getMyPosition().y;
				move_side_pack.z = gGameFramework.getMyPosition().z;
				move_side_pack.direction = MV_SIDE;
				sendPacket(&move_side_pack, active_servernum);
				break;

			case PACKET_KEY_SPACEBAR:
				// 점프
				cout << "메인에 있는 스페이스바 \n" << endl;
				break;

			case PACKET_KEYUP_MOVEKEY:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET mv_keyup_pack;
				mv_keyup_pack.type = CS_INPUT_KEYBOARD;
				mv_keyup_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				mv_keyup_pack.keytype = PACKET_KEYUP_MOVEKEY;
				sendPacket(&mv_keyup_pack, active_servernum);
				break;

			default:
				cout << "[KeyInput Error] Unknown Key Type." << endl;
			}

		}

		// 2. 마우스 입력
		if (gGameFramework.checkNewInput_Mouse()) {
			MouseInputVal mouseValue = gGameFramework.popInputVal_Mouse();

			switch (mouseValue.button) {
			case SEND_NONCLICK:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_ROTATE_PACKET yaw_rotate_pack;
				yaw_rotate_pack.size = sizeof(CS_ROTATE_PACKET);
				yaw_rotate_pack.type = CS_ROTATE;
				yaw_rotate_pack.right_x = gGameFramework.getMyRightVec().x;
				yaw_rotate_pack.right_y = gGameFramework.getMyRightVec().y;
				yaw_rotate_pack.right_z = gGameFramework.getMyRightVec().z;
				yaw_rotate_pack.up_x = gGameFramework.getMyUpVec().x;
				yaw_rotate_pack.up_y = gGameFramework.getMyUpVec().y;
				yaw_rotate_pack.up_z = gGameFramework.getMyUpVec().z;
				yaw_rotate_pack.look_x = gGameFramework.getMyLookVec().x;
				yaw_rotate_pack.look_y = gGameFramework.getMyLookVec().y;
				yaw_rotate_pack.look_z = gGameFramework.getMyLookVec().z;
				sendPacket(&yaw_rotate_pack, active_servernum);
				break;
			case SEND_BUTTON_L:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_ATTACK_PACKET attack_pack;
				attack_pack.size = sizeof(CS_ATTACK_PACKET);
				attack_pack.type = CS_ATTACK;
				sendPacket(&attack_pack, active_servernum);
				break;
			case SEND_BUTTON_R:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				// 기능 미구현
				break;
			}
		}
		this_thread::yield();
	}
}


void uiThreadFunc() {
	while (1) {
		if (gGameFramework.m_nMode != OPENINGSCENE) {
			// 1. 총알 업데이트
			gGameFramework.m_currbullet = my_info.m_bullet;

			// 2. HP 업데이트
			gGameFramework.m_currHp = my_info.m_hp;

			// 3. 시간 동기화
			gGameFramework.m_10MinOfTime = servertime_sec / 600;
			gGameFramework.m_1MinOfTime = (servertime_sec - gGameFramework.m_10MinOfTime * 600) / 60;
			gGameFramework.m_10SecOftime = (servertime_sec - gGameFramework.m_1MinOfTime * 60) / 10;
			gGameFramework.m_1SecOfTime = servertime_sec % 10;

			// 4. 미션 진행상황 동기화
			switch (gGameFramework.m_nMode) {
			case OPENINGSCENE:
				break;
			case SCENE1STAGE:
				if (gGameFramework.m_mainmissionnum == 0) {	// 0번 미션
					// ~~ 생존
					gGameFramework.m_remainNPC = stage_missions[1].goal - stage_missions[1].curr;

					// 적 모두 처치 ~~ / 20
					// 이거는 GameFramework에서 알아서 되고 있음.

				}
				else if (gGameFramework.m_mainmissionnum == 1) { // 1번 미션
					// 0 생존으로 고정
					gGameFramework.m_remainNPC = 0;

					// 거점 점령 ~~% / 100%
					int curr_percentage = static_cast<int>(stage_missions[1].curr / 5000);
					if (curr_percentage >= 100) curr_percentage = 100;
					
					gGameFramework.m_occupationnum = curr_percentage;
				}
				
				break;
			case SCENE2STAGE:
				//gGameFramework.m_remainNPC = stage_missions[2].goal - stage_missions[2].curr;
				break;
			}

			wchar_t lateNPC[20];
			_itow_s(gGameFramework.m_remainNPC, lateNPC, sizeof(lateNPC), 10);
			wcscpy_s(gGameFramework.m_remainNPCPrint, lateNPC);

			// 5. 미션 종류 동기화
			if (trigger_stage_clear) {
				trigger_stage_clear = false;
				curr_mission_num = 0;

				// 스테이지 전환
				if (gGameFramework.m_nMode == SCENE1STAGE) {
				}
				else if (gGameFramework.m_nMode == SCENE2STAGE) {
				}
			}
			if (trigger_mission_complete) {
				trigger_mission_complete = false;
				gGameFramework.m_mainmissionnum = curr_mission_num;
			}

			// 6. Team 인원 동기화
			gGameFramework.m_CurrentPlayerNum = curr_connection_num;
		}

		this_thread::yield();
	}
}

