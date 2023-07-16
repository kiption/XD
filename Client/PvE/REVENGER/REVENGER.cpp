// LabProject07-9-7.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "REVENGER.h"
#include "GameFramework.h"
#include "GameSound.h"
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
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	stage1_enter_ok = false;
	stage2_enter_ok = false;

	gGameFramework.m_MAX_USER = MAX_USER;

	// 로비 서버에 연결
	curr_servertype = SERVER_LOBBY;
	active_servernum = 0;

	lby_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN lby_addr;
	ZeroMemory(&lby_addr, sizeof(lby_addr));
	lby_addr.sin_family = AF_INET;
	int new_portnum = PORTNUM_LOBBY_0 + active_servernum;
	lby_addr.sin_port = htons(new_portnum);
	inet_pton(AF_INET, IPADDR_LOBBY0, &lby_addr.sin_addr);
	connect(lby_socket, reinterpret_cast<sockaddr*>(&lby_addr), sizeof(lby_addr));

	CLBY_CONNECT_PACKET conn_packet;
	conn_packet.size = sizeof(CLBY_CONNECT_PACKET);
	conn_packet.type = CLBY_CONNECT;
	srand(time(NULL));
	int randnum = rand() % 100;
	string static_str = "Player";
	string variable_str = to_string(randnum);
	string full_name = static_str + variable_str;
	strcpy_s(conn_packet.name, full_name.c_str());

	sendPacket(&conn_packet);
	recvPacket();

	// 스레드 생성
	thread networkThread(networkThreadFunc);
	networkThread.detach();

	thread uiThread(uiThreadFunc);
	uiThread.detach();
	//====

	while (1)
	{
		SleepEx(0, TRUE);//S

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
			if (gGameFramework.m_nMode == OPENINGSCENE)
			{
				if (gGameFramework.m_bLoginInfoSend && gGameFramework.m_LoginClick[3]) {
					// id, pw, ip 받은 거 char로 바꾸는 곳		
					char id[20] = { 0 };
					char pw[20] = { 0 };
					char ip[20] = { 0 };

					size_t idLength = wcslen(gGameFramework.m_LoginID);
					size_t pwLength = wcslen(gGameFramework.m_LoginPW);
					size_t ipLength = wcslen(gGameFramework.m_LoginIP);

					wcstombs_s(nullptr, id, sizeof(id), gGameFramework.m_LoginID, idLength);
					wcstombs_s(nullptr, pw, sizeof(pw), gGameFramework.m_LoginPW, pwLength);
					wcstombs_s(nullptr, ip, sizeof(ip), gGameFramework.m_LoginIP, ipLength);

					gGameFramework.m_bLoginInfoSend = false;
				}

				// UI를 통한 조작 구현
				switch (gGameFramework.m_LoginScene) {
				case gGameFramework.LS_LOGIN:	// ID/PW 입력 창
					break;

				case gGameFramework.LS_OPENING: // 게임 시작, 설정, 종료 
					break;

				case gGameFramework.LS_LOBBY:	// 로비
					// 빠른시작
					if (gGameFramework.m_LobbyClick[0]) {
						CLBY_QUICK_MATCH_PACKET quick_match_pack;
						quick_match_pack.size = sizeof(CLBY_QUICK_MATCH_PACKET);
						quick_match_pack.type = CLBY_QUICK_MATCH;
						sendPacket(&quick_match_pack);

						gGameFramework.m_LobbyClick[0] = false;
					}
					if (ls_room_enter_ok) {										// 서버에서 방 진입을 허락해줘야
						gGameFramework.m_LoginScene = gGameFramework.LS_ROOM;	// 방으로 이동함.
						gGameFramework.m_roominMyId = my_room_index;
						cout << "m_roominMyId: " << gGameFramework.m_roominMyId << endl;

						ls_room_enter_ok = false;
					}

					break;

				case gGameFramework.LS_ROOM: // 게임 방
					// 방나가기
					if (gGameFramework.m_RoomBackButton) {
						CLBY_LEAVE_ROOM_PACKET leave_room_pack;
						leave_room_pack.size = sizeof(CLBY_LEAVE_ROOM_PACKET);
						leave_room_pack.type = CLBY_LEAVE_ROOM;
						sendPacket(&leave_room_pack);

						CurrRoomInfoClear();
						gGameFramework.m_LoginScene = gGameFramework.LS_LOBBY;	// 로비으로 이동함.
						for (int i{}; i < gGameFramework.m_MAX_USER; ++i) {
							gGameFramework.m_MyRoom_Info[i].clear();
						}

						gGameFramework.m_RoomBackButton = false;
					}

					// 역할 변경
					if (gGameFramework.role_change_h2a_click) {	// heli -> army (헬기 -> 사람)
						if (players_info[my_id].m_role == ROLE_RIFLE) {
							cout << "[Test] 역할(사람) 취소" << endl;

							CLBY_ROLE_CHANGE_PACKET notchoose_request_pack;
							notchoose_request_pack.size = sizeof(CLBY_ROLE_CHANGE_PACKET);
							notchoose_request_pack.type = CLBY_ROLE_CHANGE;
							notchoose_request_pack.role = ROLE_NOTCHOOSE;
							sendPacket(&notchoose_request_pack);
						}
						else {
							cout << "[Test] 헬기 -> 사람" << endl;

							CLBY_ROLE_CHANGE_PACKET heli2human_request_pack;
							heli2human_request_pack.size = sizeof(CLBY_ROLE_CHANGE_PACKET);
							heli2human_request_pack.type = CLBY_ROLE_CHANGE;
							heli2human_request_pack.role = ROLE_RIFLE;
							sendPacket(&heli2human_request_pack);
						}

						gGameFramework.role_change_h2a_click = false;
					}
					if (gGameFramework.role_change_a2h_click) { // army -> heli (사람 -> 헬기)
						if (players_info[my_id].m_role == ROLE_HELI) {
							cout << "[Test] 역할(헬기) 취소" << endl;

							CLBY_ROLE_CHANGE_PACKET notchoose_request_pack;
							notchoose_request_pack.size = sizeof(CLBY_ROLE_CHANGE_PACKET);
							notchoose_request_pack.type = CLBY_ROLE_CHANGE;
							notchoose_request_pack.role = ROLE_NOTCHOOSE;
							sendPacket(&notchoose_request_pack);
						}
						else {
							cout << "[Test] 사람 -> 헬기" << endl;

							CLBY_ROLE_CHANGE_PACKET human2heli_request_pack;
							human2heli_request_pack.size = sizeof(CLBY_ROLE_CHANGE_PACKET);
							human2heli_request_pack.type = CLBY_ROLE_CHANGE;
							human2heli_request_pack.role = ROLE_HELI;
							sendPacket(&human2heli_request_pack);
						}

						gGameFramework.role_change_a2h_click = false;
					}

					// 게임시작
					if (gGameFramework.m_RoomClick[0]) {
						// 방장만 시작 패킷을 보낼 수 있다
						if (!b_room_manager) {
							gGameFramework.m_RoomClick[0] = false;
							break;
						}

						/* [치트키] 작업 편의성을 위해 1명만 접속해도 게임 시작이 가능하도록 주석처리하였음.
						// 방에 사람이 세 명이 있는지 확인합니다.
						if (curr_room.user_count < MAX_USER) {
							gGameFramework.m_RoomClick[0] = false;
							break;
						}
						*/

						// 모든 사람이 준비가 되어있는지 확인합니다.
						bool all_ready = true;
						/* [치트키] 작업 편의성을 위해 1명만 접속해도 게임 시작이 가능하도록 주석처리하였음.
						for (int i = 0; i < MAX_USER; ++i) {
							if (curr_room.user_state[i] == RM_ST_MANAGER) continue;	// 방장은 준비상태가 없다.
							if (curr_room.user_state[i] == RM_ST_NONREADY) {
								all_ready = false;
								break;
							}
						}
						if (!all_ready) {	// 세 명 모두 준비되어있어야 시작이 가능하다.
							gGameFramework.m_RoomClick[0] = false;
							break;
						}
						*/

						CLBY_GAME_START_PACKET start_packet;
						start_packet.size = sizeof(CLBY_GAME_START_PACKET);
						start_packet.type = CLBY_GAME_START;
						sendPacket(&start_packet);

						gGameFramework.m_RoomClick[0] = false;
					}

					// 준비
					if (gGameFramework.m_RoomClick[1]) {
						if (b_room_manager) {
							// 방장은 준비 상태가 없습니다.
							gGameFramework.m_RoomClick[1] = false;
							break;
						}

						CLBY_GAME_READY_PACKET ready_packet;
						ready_packet.size = sizeof(CLBY_GAME_READY_PACKET);
						ready_packet.type = CLBY_GAME_READY;
						sendPacket(&ready_packet);

						gGameFramework.m_RoomClick[1] = false;
					}

					// 다음 스테이지로 전환
					if (stage1_enter_ok) {
						// 로직 서버에 연결
						curr_servertype = SERVER_LOGIC;
						active_servernum = MAX_LOGIC_SERVER - 1;

						lgc_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
						SOCKADDR_IN server0_addr;
						ZeroMemory(&server0_addr, sizeof(server0_addr));
						server0_addr.sin_family = AF_INET;
						int new_portnum = PORTNUM_LOGIC_0 + active_servernum;
						server0_addr.sin_port = htons(new_portnum);
						//inet_pton(AF_INET, SERVER_ADDR, &server0_addr.sin_addr);// 루프백
						inet_pton(AF_INET, IPADDR_LOGIC1, &server0_addr.sin_addr);
						connect(lgc_socket, reinterpret_cast<sockaddr*>(&server0_addr), sizeof(server0_addr));

						CS_LOGIN_PACKET login_pack;
						login_pack.size = sizeof(CS_LOGIN_PACKET);
						login_pack.type = CS_LOGIN;
						strcpy_s(login_pack.name, full_name.c_str());
						sendPacket(&login_pack);
						recvPacket();

						stage1_enter_ok = false;
						gGameFramework.m_Max_NPCs = MAX_NPCS;
					}
					if (trigger_stage1_playerinfo_load && trigger_stage1_mapinfo_load) { // 로직 서버에서 스테이지1 관련 정보를 모두 받아야 Stage1 씬으로 넘어갈 수 있다.
						for (int i{}; i < stage1_mapobj_info.size(); ++i) {
							MapObjectsInfo origintemp = stage1_mapobj_info[i];
							CollideMapInfo frametemp;
							frametemp.m_pos = origintemp.m_pos;
							frametemp.m_scale = { origintemp.m_scale.x, origintemp.m_scale.y, origintemp.m_scale.z };
							frametemp.m_local_forward = origintemp.m_local_forward;
							frametemp.m_local_right = origintemp.m_local_right;
							frametemp.m_local_rotate = origintemp.m_local_rotate;
							frametemp.m_angle_aob = origintemp.m_angle_aob;
							frametemp.m_angle_boc = origintemp.m_angle_boc;
							frametemp.id = i;
							frametemp.setBB();

							gGameFramework.mapcol_info.emplace_back(frametemp);
						}

						trigger_stage1_playerinfo_load = false;
						trigger_stage1_mapinfo_load = false;

						gGameFramework.ChangeScene(SCENE1STAGE);
						gGameFramework.setPosition_Self(players_info[my_id].m_pos);
						gGameFramework.setVectors_Self(players_info[my_id].m_right_vec, players_info[my_id].m_up_vec, players_info[my_id].m_look_vec);
					}

					break;

				case gGameFramework.LS_CREATE_ROOM:	// 방 생성 창
					// 방 생성 확인버튼
					if (gGameFramework.m_CreateRoomOkButton) {
						CLBY_CREATE_ROOM_PACKET create_room_pack;
						create_room_pack.size = sizeof(CLBY_CREATE_ROOM_PACKET);
						create_room_pack.type = CLBY_CREATE_ROOM;
						strcpy_s(create_room_pack.room_name, "This is Room Name");
						sendPacket(&create_room_pack);

						gGameFramework.m_CreateRoomOkButton = false;
					}
					if (ls_room_enter_ok) {								// 서버에서 방 생성을 끝내면
						gGameFramework.m_LoginScene = gGameFramework.LS_ROOM;	// 방으로 이동함.
						gGameFramework.m_roominMyId = my_room_index;
						cout << "m_roominMyId: " << gGameFramework.m_roominMyId << endl;

						ls_room_enter_ok = false;
					}

					break;
				}// switch end
			}
			else
			{
				//==================================================
				//					서버 연결 확인
				//==================================================
				// 1초마다 서버로 핑을 던져서 살아있는지 확인합니다.
				if (chrono::system_clock::now() > last_ping + chrono::seconds(3)) {
					CS_PING_PACKET ping_packet;
					ping_packet.type = CS_PING;
					ping_packet.size = sizeof(CS_PING_PACKET);
					sendPacket(&ping_packet);
					last_ping = chrono::system_clock::now();
					//cout << "ping" << endl;
				}

				//==================================================
				//				   플레이어 동기화
				//==================================================
				//1. 좌표 및 벡터 업데이트
				// 1) 다른 플레이어
				for (int i = 0; i < MAX_USER; ++i) {
					if (i == my_id) continue;
					if (players_info[i].m_state == OBJ_ST_RUNNING) {
						gGameFramework.setPosition_OtherPlayer(i, players_info[i].m_pos);
						gGameFramework.setVectors_OtherPlayer(i, players_info[i].m_right_vec, players_info[i].m_up_vec, players_info[i].m_look_vec);
					}
					else if (players_info[i].m_state == OBJ_ST_LOGOUT) {
						players_info[i].InfoClear();
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

					if (npcs_info[i].m_attack_on) {
						gGameFramework.HeliNpcUnderAttack(i, npcs_info[i].m_attack_dir);
						npcs_info[i].m_attack_on = false;
					}

					//((Stage1*)gGameFramework.m_pScene)->m_pBillboardShader[3]->ParticlePosition = npcs_info[i].m_pos;
				}

				// 3. 리스폰할 때는 자기 자신을 움직여줘야합니다.
				if (respawn_trigger) {
					gGameFramework.setPosition_Self(players_info[my_id].m_pos);
					gGameFramework.setVectors_Self(players_info[my_id].m_right_vec, players_info[my_id].m_up_vec, players_info[my_id].m_look_vec);
					respawn_trigger = false;
				}

				// 4. 만약 죽어있는 상태면 캐릭터 조작이 불가능하게 막아야합니다.
				if ((players_info[my_id].m_ingame_state == PL_ST_DEAD) && (!gGameFramework.player_dead)) {
					gGameFramework.player_dead = true;
				}
				if ((players_info[my_id].m_ingame_state != PL_ST_DEAD) && (gGameFramework.player_dead)) {
					gGameFramework.player_dead = false;
				}

				//==================================================
				// 2. 객체 인게임 상태 업데이트 (자기 자신 제외, 자기 자신은 클라 독자적으로 돌아가기 때문)
				//  1) Other Players
				for (int i = 0; i < MAX_USER; ++i) {
					if (i == my_id) break;

					if (players_info[i].m_new_state_update) {
						switch (players_info[i].m_ingame_state) {
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
						case PL_ST_DAMAGED:
							break;
						}

						players_info[i].m_new_state_update = false;
					}
				}

				//  2) NPC
				for (int i = 0; i < MAX_NPCS; ++i) {
					if (npcs_info[i].m_id == -1) continue;

					if (npcs_info[i].m_new_state_update) {
						switch (npcs_info[i].m_ingame_state) {
						case PL_ST_MOVE_FRONT: // 앞으로 이동
							if (i >= 5) {
								gGameFramework.MoveMotionNPC(i);
							}
							break;
						case PL_ST_IDLE:
						case PL_ST_MOVE_BACK: // 뒤로 이동
						case PL_ST_MOVE_SIDE: // 옆으로 이동
							break;
						case PL_ST_ATTACK:
							break;
						case PL_ST_DEAD:
							//((Stage1*)gGameFramework.m_pScene)->m_ppShaders[0]->m_ppObjects[10 + i]->m_xmf4x4ToParent._42 = 0.0f;
							gGameFramework.DyingMotionNPC(i);
							break;
						case PL_ST_DAMAGED:
							gGameFramework.NpcHittingMotion(i);
							break;
						}
						npcs_info[i].m_new_state_update = false;
					}
				}


				//==================================================
				//					Map 충돌 관련
				//==================================================
				if (!q_bullet_hit_pos_mapobj.empty()) {
					XMFLOAT3 mabobj_collide_pos = q_bullet_hit_pos_mapobj.front();
					mabobj_collide_pos.y += 3.0f;
					q_bullet_hit_pos_mapobj.pop();
					gGameFramework.CollisionMap_by_BULLET(mabobj_collide_pos);
					//cout << "맵 충 돌" << mabobj_collide_pos.x << ", " << mabobj_collide_pos.y << ", " << mabobj_collide_pos.z << endl;
				}

				gGameFramework.CollisionNPC_by_BULLET(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/));
				gGameFramework.CollisionNPC_by_MAP(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/), XMFLOAT3(/*MAP CENTER*/), XMFLOAT3(/*MAP EXTENTS*/));
				gGameFramework.CollisionNPC_by_PLAYER(XMFLOAT3(/*NPC CENTER*/), XMFLOAT3(/*NPC EXTENTS*/));
			}

			gGameFramework.ProcessInput();
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
	HWND hMainWnd = CreateWindow(szWindowClass, L"Loading...", dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

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
	case WM_IME_COMPOSITION:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;	
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
	last_ping = last_pong = chrono::system_clock::now();

	//==================================================
	// thread loop
	while (1) {
		//==================================================
		//		새로운 키 입력을 서버에게 전달합니다.
		//==================================================
		// 1. 키보드 입력
		if (gGameFramework.checkNewInput_Keyboard()) {
			short keyValue = gGameFramework.popInputVal_Keyboard();

			switch (keyValue) {
			case PACKET_KEY_W:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_MOVE_PACKET move_front_pack;
				move_front_pack.size = sizeof(CS_MOVE_PACKET);
				move_front_pack.type = CS_MOVE;
				move_front_pack.x = gGameFramework.getMyPosition().x;
				move_front_pack.y = gGameFramework.getMyPosition().y;
				move_front_pack.z = gGameFramework.getMyPosition().z;
				move_front_pack.direction = MV_FRONT;
				sendPacket(&move_front_pack);
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
				sendPacket(&move_back_pack);
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
				sendPacket(&move_side_pack);
				break;

			case PACKET_KEY_R:
				// 재장전
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET input_rkey_pack;
				input_rkey_pack.type = CS_INPUT_KEYBOARD;
				input_rkey_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				input_rkey_pack.keytype = PACKET_KEY_R;
				sendPacket(&input_rkey_pack);
				break;

			case PACKET_KEY_SPACEBAR:
				// 점프
				break;

			case PACKET_KEYUP_MOVEKEY:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET mv_keyup_pack;
				mv_keyup_pack.type = CS_INPUT_KEYBOARD;
				mv_keyup_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				mv_keyup_pack.keytype = PACKET_KEYUP_MOVEKEY;
				sendPacket(&mv_keyup_pack);
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
				yaw_rotate_pack.cam_look_x = gGameFramework.getMyCameraLookVec().x;
				yaw_rotate_pack.cam_look_y = gGameFramework.getMyCameraLookVec().y;
				yaw_rotate_pack.cam_look_z = gGameFramework.getMyCameraLookVec().z;
				sendPacket(&yaw_rotate_pack);
				break;
			case SEND_BUTTON_L:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_ATTACK_PACKET attack_pack;
				attack_pack.size = sizeof(CS_ATTACK_PACKET);
				attack_pack.type = CS_ATTACK;
				sendPacket(&attack_pack);
				break;
			case SEND_BUTTON_R:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				// 기능 미구현
				break;
			}
		}

		// 3. 채팅
		while (!gGameFramework.m_mychat_log.empty()) {
			CS_CHAT_PACKET chat_msg_packet;
			chat_msg_packet.size = sizeof(CS_CHAT_PACKET);
			chat_msg_packet.type = CS_CHAT;
			strcpy_s(chat_msg_packet.msg, gGameFramework.m_mychat_log.front().chatData);
			gGameFramework.m_mychat_log.pop();
			sendPacket(&chat_msg_packet);
		}

		this_thread::yield();
	}
}

// Chat UI 관련 (convert char to wchat_t)
wchar_t* ConvertToWideChar(const char* str) {
	if (str == nullptr)
		return nullptr;

	size_t size = 0;
	mbstowcs_s(&size, nullptr, 0, str, 0);

	wchar_t* wideStr = new wchar_t[size + 1];
	mbstowcs_s(nullptr, wideStr, size + 1, str, size);

	return wideStr;
}

wchar_t* charToWchar(char* str) {	// char -> wchar
	size_t cn;
	wchar_t wchar_arr[100] = L"";

	setlocale(LC_ALL, "Korean");//로케일 설정
	mbstowcs_s(&cn, wchar_arr, 100, str, 100);
	return wchar_arr;
}
void uiThreadFunc() {
	while (1) {
		if (gGameFramework.m_nMode == OPENINGSCENE) {
			// UI를 통한 조작 구현
			switch (gGameFramework.m_LoginScene) {
			case gGameFramework.LS_LOGIN:	// ID/PW 입력 창
				break;

			case gGameFramework.LS_OPENING: // 게임 시작, 설정, 종료 
				break;

			case gGameFramework.LS_LOBBY:	// 로비
				break;

			case gGameFramework.LS_ROOM: // 게임 방
				if (trigger_new_member) {	// 새로운 유저 방 입장 트리거
					if (0 <= new_member_id && new_member_id < MAX_USER) {
						//gGameFramework.newUserJoinRoom(L"\0");
						gGameFramework.setRoomUserInfo(new_member_id, charToWchar(players_info[new_member_id].m_name), RM_ST_NONREADY);

						cout << "[" << new_member_id << "] Name: " << players_info[new_member_id].m_name << " is Update." << endl;
					}
					new_member_id = -1;
					trigger_new_member = false;
				}

				if (trigger_role_change) {	// 멤버 역할 변경 트리거
					if (players_info[role_change_member_id].m_role == ROLE_NOTCHOOSE) {
						gGameFramework.m_MyRoom_Info[role_change_member_id].armyCheck = false;
						gGameFramework.m_MyRoom_Info[role_change_member_id].HeliCheck = false;
						if (role_change_member_id == my_id)
							gGameFramework.m_ingame_role = gGameFramework.R_NONE;
						cout << "Client[" << role_change_member_id << "]의 역할이 [선택 안함]으로 바뀌었음." << endl;
					}
					else if (players_info[role_change_member_id].m_role == ROLE_RIFLE) {
						gGameFramework.m_MyRoom_Info[role_change_member_id].armyCheck = true;
						gGameFramework.m_MyRoom_Info[role_change_member_id].HeliCheck = false;
						if (role_change_member_id == my_id)
							gGameFramework.m_ingame_role = gGameFramework.R_RIFLE;
						cout << "Client[" << role_change_member_id << "]의 역할이 [Rifle]로 바뀌었음." << endl;
					}
					else if (players_info[role_change_member_id].m_role == ROLE_HELI) {
						gGameFramework.m_MyRoom_Info[role_change_member_id].armyCheck = false;
						gGameFramework.m_MyRoom_Info[role_change_member_id].HeliCheck = true;
						if (role_change_member_id == my_id)
							gGameFramework.m_ingame_role = gGameFramework.R_HELI;
						cout << "Client[" << role_change_member_id << "]의 역할이 [Heli]로 바뀌었음." << endl;
					}
					role_change_member_id = -1;
					trigger_role_change = false;
				}

				if (trigger_leave_member) {	// 다른 유저 방 퇴장 트리거
					if (0 <= left_member_id && left_member_id < MAX_USER) {
						gGameFramework.setRoomUserInfo(left_member_id, L"\0", RM_ST_EMPTY);
					}
					left_member_id = -1;
					trigger_leave_member = false;
				}

				if (trigger_room_update) {	// 방 데이터 전체 업데이트 트리거
					for (int i = 0; i < MAX_USER; ++i) {
						if (curr_room.user_state[i] == RM_ST_EMPTY) continue;
						gGameFramework.setRoomUserInfo(i, charToWchar(players_info[i].m_name), curr_room.user_state[i]);
						cout << "[" << i << "] Name: " << players_info[i].m_name << " is Update." << endl; \
					}

					trigger_room_update = false;
				}
				break;

			case gGameFramework.LS_CREATE_ROOM:	// 방 생성 창
				break;
			}
		}
		else {
			// 1. 총알 업데이트
			gGameFramework.m_currbullet = players_info[my_id].m_bullet;

			// 2. HP 업데이트
			int currHP = players_info[my_id].m_hp;
			gGameFramework.m_currHp = currHP;

			// 3. 시간 동기화
			gGameFramework.m_10MinOfTime = timelimit_sec / 600;
			gGameFramework.m_1MinOfTime = (timelimit_sec - gGameFramework.m_10MinOfTime * 600) / 60;
			gGameFramework.m_10SecOftime = (timelimit_sec - gGameFramework.m_1MinOfTime * 60) / 10;
			gGameFramework.m_1SecOfTime = timelimit_sec % 10;

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

			// 7. 채팅 동기화
			if (chat_comeTome) {
				ChatInfo textinfo;

				char temptext[80];
				char* name = chat_logs.name;
				char* msg = chat_logs.msg;

				size_t nameLength = strlen(name);
				size_t msgLength = strlen(msg);

				if (nameLength < 80 - 2) { // tempText의 크기에서 ": "를 뺀 값보다 작은지 확인
					strcpy_s(temptext, sizeof(temptext) / sizeof(temptext[0]), name);
					strcat_s(temptext, sizeof(temptext) / sizeof(temptext[0]), ": "); // ":" 추가
					strncat_s(temptext, sizeof(temptext) / sizeof(temptext[0]), msg, _TRUNCATE);

					cout << temptext << endl;

					wchar_t* converttext = ConvertToWideChar(temptext);

					wcscpy_s(textinfo.chatData, converttext);

					gGameFramework.m_chat_info.push(textinfo);
				}

				chat_comeTome = false;
				if (gGameFramework.m_chat_info.size() > 10) {
					while (true) {
						gGameFramework.m_chat_info.pop();
						if (gGameFramework.m_chat_info.size() <= 10) break;
					}
				}
			}

		}

		this_thread::yield();
	}
}

