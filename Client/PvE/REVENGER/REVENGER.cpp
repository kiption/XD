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

wchar_t* charToWchar(char* str) {	// char -> wchar
	size_t cn;
	wchar_t wchar_arr[100] = L"";

	setlocale(LC_ALL, "Korean");//로케일 설정
	mbstowcs_s(&cn, wchar_arr, 100, str, 100);
	return wchar_arr;
}

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
	// Config파일에서 서버 주소를 읽어옵니다
	bool b_lobby = false;
	bool b_logic = false;
	int word_count = 0;
	int server_num = 0;

	ifstream in("ServerIpAddr.config");
	cout << "서버 IP주소를 읽어옵니다... " << endl;
	if (in.is_open()) {
		string word;
		while (in >> word) {
			if (word == "LOBBY") {
				b_lobby = true;
				word_count++;
			}
			else if (word == "LOGIC") {
				b_logic = true;
				word_count++;
			}
			else {
				if (!b_lobby && !b_logic) continue;

				if (b_lobby) {
					if (word_count == 1) {
						server_num = stoi(word);
						word_count++;
					}
					else if (word_count == 2) {
						if (server_num == 0) {
							strcpy_s(IPADDR_LOBBY0, word.c_str());
							cout << "로비0: " << IPADDR_LOBBY0 << endl;

							// 한 줄의 마지막이므로 값 초기화
							b_lobby = false;
							server_num = 0;
							word_count = 0;
						}
						else {
							strcpy_s(IPADDR_LOBBY1, word.c_str());
							cout << "로비1: " << IPADDR_LOBBY1 << endl;

							// 한 줄의 마지막이므로 값 초기화
							b_lobby = false;
							server_num = 0;
							word_count = 0;
						}
					}
				}
				else if (b_logic) {
					if (word_count == 1) {
						server_num = stoi(word);
						word_count++;
					}
					else if (word_count == 2) {
						if (server_num == 0) {
							strcpy_s(IPADDR_LOGIC0, word.c_str());
							cout << "로직0: " << IPADDR_LOGIC0 << endl;

							// 한 줄의 마지막이므로 값 초기화
							b_logic = false;
							server_num = 0;
							word_count = 0;
						}
						else {
							strcpy_s(IPADDR_LOGIC1, word.c_str());
							cout << "로직1: " << IPADDR_LOGIC1 << endl;

							// 한 줄의 마지막이므로 값 초기화
							b_logic = false;
							server_num = 0;
							word_count = 0;
						}
					}
				}
			}
		}
		in.close();
	}

	// 서버 초기설정
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
				// UI를 통한 조작 구현
				switch (gGameFramework.m_LoginScene) {
				case gGameFramework.LS_OPENING: // 게임 시작, 종료 
					// 게임 시작
					if (gGameFramework.m_GameClick[0]) {
						CLBY_REQUEST_LOBBYINFO_PACKET request_lobby_pack;
						request_lobby_pack.size = sizeof(CLBY_REQUEST_LOBBYINFO_PACKET);
						request_lobby_pack.type = CLBY_REQUEST_LOBBYINFO;
						sendPacket(&request_lobby_pack);
						gGameFramework.m_GameClick[0] = false;
						game_enter_ok = false;
					}
					if (game_enter_ok) {									   // 서버에서 방 진입을 허락해줘야
						gGameFramework.m_LoginScene = gGameFramework.LS_LOBBY; // 로비 화면으로 이동함.
						game_enter_ok = false;
					}

					// 종료
					if (gGameFramework.m_GameClick[1]) {
						CLBY_GAME_EXIT_PACKET exit_packet;
						exit_packet.size = sizeof(CLBY_GAME_EXIT_PACKET);
						exit_packet.type = CLBY_GAME_EXIT;
						sendPacket(&exit_packet);
						gGameFramework.m_GameClick[1] = false;
						game_exit_ok = false;
					}
					if (game_exit_ok) {
						gGameFramework.OnDestroy();
						return 0;
					}
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
						gGameFramework.currRoomName = charToWchar(curr_room.room_name);
						cout << "생성이름: ";
						wprintf(gGameFramework.currRoomName);
						cout << endl;
						gGameFramework.m_myRoomNum = curr_room_id;
						gGameFramework.m_roominMyId = my_room_index;

						ls_room_enter_ok = false;
					}

					// 방 선택 입장
					for (int i = 0; i < 8; ++i) {
						if (gGameFramework.m_LobbyRoomClick[i]) {
							if (i + 1 > lobby_rooms.size()) break;
							cout << "선택한 방ID: " << lobby_rooms[i].room_id << ", 이름: " << lobby_rooms[i].room_name << endl;

							CLBY_ROOM_ENTER_PACKET room_enter_pack;
							room_enter_pack.size = sizeof(CLBY_ROOM_ENTER_PACKET);
							room_enter_pack.type = CLBY_ROOM_ENTER;
							room_enter_pack.room_id = lobby_rooms[i].room_id;
							sendPacket(&room_enter_pack);

							gGameFramework.m_LobbyRoomClick[i] = false;
							break;
						}
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
						for (int i{}; i < gGameFramework.m_MAX_USER; ++i) {
							gGameFramework.m_MyRoom_Info[i].clear();
						}
						gGameFramework.m_LoginScene = gGameFramework.LS_LOBBY;	// 로비으로 이동함.

						gGameFramework.m_RoomBackButton = false;
					}

					// 역할 변경
					if (gGameFramework.role_change_h2a_click) {	// heli -> army (헬기 -> 사람)
						if (players_info[my_id].m_role == ROLE_RIFLE) {
							CLBY_ROLE_CHANGE_PACKET notchoose_request_pack;
							notchoose_request_pack.size = sizeof(CLBY_ROLE_CHANGE_PACKET);
							notchoose_request_pack.type = CLBY_ROLE_CHANGE;
							notchoose_request_pack.role = ROLE_NOTCHOOSE;
							sendPacket(&notchoose_request_pack);
						}
						else {
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
							CLBY_ROLE_CHANGE_PACKET notchoose_request_pack;
							notchoose_request_pack.size = sizeof(CLBY_ROLE_CHANGE_PACKET);
							notchoose_request_pack.type = CLBY_ROLE_CHANGE;
							notchoose_request_pack.role = ROLE_NOTCHOOSE;
							sendPacket(&notchoose_request_pack);
						}
						else {
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
						// 동시에 클라이언트 여러 대가 가면 꼬일 수 있기에 입장 순서를 정해주기 위함
						if (my_id != 0) {
							Sleep(300 * my_id);
						}

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
						login_pack.role = players_info[my_id].m_role;
						login_pack.inroom_index = my_id;
						login_pack.room_id = curr_room_id;
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
						setlocale(LC_ALL, "Korean");
						wcstombs_s(nullptr, create_room_pack.room_name, sizeof(create_room_pack.room_name)
							, gGameFramework.createRoomName, sizeof(create_room_pack.room_name));
						sendPacket(&create_room_pack);

						gGameFramework.m_CreateRoomOkButton = false;
					}
					if (ls_room_enter_ok) {								// 서버에서 방 생성을 끝내면
						gGameFramework.m_LoginScene = gGameFramework.LS_ROOM;	// 방으로 이동함.
						gGameFramework.currRoomName = charToWchar(curr_room.room_name);
						cout << "생성이름: ";
						wprintf(gGameFramework.currRoomName);
						cout << endl;
						gGameFramework.m_myRoomNum = curr_room_id;
						gGameFramework.m_roominMyId = my_room_index;

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
					if (players_info[i].m_state == OBJ_ST_RUNNING)
					{
						if (players_info[i].m_role == ROLE_RIFLE)
						{
							// 군인은 무조건 index가 5,6
							gGameFramework.setPosition_SoldiarOtherPlayer(i, players_info[i].m_pos);
							gGameFramework.setVectors_SoldiarOtherPlayer(i, players_info[i].m_right_vec, players_info[i].m_up_vec, players_info[i].m_look_vec);
						}
						else if (players_info[i].m_role == ROLE_HELI)
						{
							// 헬기는 무조건 index가 7
							gGameFramework.setPosition_HeliOtherPlayer(players_info[i].m_pos);
							gGameFramework.setVectors_HeliOtherPlayer(players_info[i].m_right_vec, players_info[i].m_up_vec, players_info[i].m_look_vec);
						}
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
						gGameFramework.AttackMotionNPC(i);
						gGameFramework.NpcUnderAttack(npcs_info[i].m_attack_dir, i);
						npcs_info[i].m_attack_on = false;

					}
					if (!npcs_info[i].m_attack_on) {
						gGameFramework.NpcNoneUnderAttack();
					}

				}

				// 3. 리스폰할 때
				if (respawn_trigger) {
					if (respawn_id == my_id) {	// 자기 자신
						gGameFramework.setPosition_Self(players_info[my_id].m_pos);
						gGameFramework.setVectors_Self(players_info[my_id].m_right_vec, players_info[my_id].m_up_vec, players_info[my_id].m_look_vec);
					}
					else {	// 다른 플레이어 리스폰
						//gGameFramework.OtherPlayerResponeMotion();
					}

					respawn_trigger = false;
					respawn_id = -1;
				}

				// 4. 만약 죽어있는 상태면 캐릭터 조작이 불가능하게 막아야합니다.
				if ((players_info[my_id].m_ingame_state == PL_ST_DEAD) && (!gGameFramework.player_dead)) {
					gGameFramework.MyPlayerDieMotion();
					gGameFramework.m_HeliPlayerWarnningUISwitch = false;
					gGameFramework.player_dead = true;
				}
				if ((players_info[my_id].m_ingame_state != PL_ST_DEAD) && (gGameFramework.player_dead)) {
					gGameFramework.MyPlayerRespawnMotion();
					gGameFramework.m_HeliPlayerWarnningUISwitch = false;
					gGameFramework.player_dead = false;
				}

				// 5. 총쏘는거
				if (trigger_otherplayer_attack) {
					// 여기에서 총알 연출!
					if (players_info[otherplayer_attack_id].m_role == ROLE_RIFLE) {
						// 사람도 하게된다면 여기서 하면됨.
					}
					else if (players_info[otherplayer_attack_id].m_role == ROLE_HELI)
					{
						gGameFramework.HeliPlayerUnderAttack(otherplayer_attack_dir);
					}

					trigger_otherplayer_attack = false;
					otherplayer_attack_id = 0;
					otherplayer_attack_dir = { 0, 0, 0 };
				}

				//==================================================
				// 2. 객체 인게임 상태 업데이트 (자기 자신 제외, 자기 자신은 클라 독자적으로 돌아가기 때문)
				//  1) Other Players
				for (int i = 0; i < MAX_USER; ++i) {
					if (i == my_id) continue;

					// 피격 이펙트
					if (players_info[i].m_damaged_effect_on) {

						players_info[i].m_damaged_effect_on = false;
					}

					// 모션 전환
					if (players_info[i].m_new_state_update) {
						if (players_info[i].m_role == ROLE_RIFLE) {
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
							}
						}
						else if (players_info[i].m_role == ROLE_HELI) {
							switch (players_info[i].m_ingame_state) {
							case PL_ST_DEAD:
								gGameFramework.otherHeliPlayerDyingMotion();
								break;
							}
						}
						players_info[i].m_new_state_update = false;
					}
				}

				//  2) NPC
				for (int i = 0; i < MAX_NPCS; ++i) {
					if (npcs_info[i].m_id == -1) continue;

					if (npcs_info[i].m_damaged_effect_on) {	// 피격 이펙트
						gGameFramework.NpcHittingMotion(i);
						npcs_info[i].m_damaged_effect_on = false;
					}

					if (npcs_info[i].m_new_state_update) {
						switch (npcs_info[i].m_ingame_state) {
						case PL_ST_MOVE_FRONT: // 앞으로 이동
							if (i >= 5) {
								gGameFramework.MoveMotionNPC(i);
							}
							break;
						case PL_ST_IDLE:

							break;
						case PL_ST_MOVE_BACK: // 뒤로 이동
						case PL_ST_MOVE_SIDE: // 옆으로 이동
							break;
						case PL_ST_ATTACK:
							if (i >= 5) {
								gGameFramework.AttackMotionNPC(i);
							}
							break;
						case PL_ST_DEAD:
							//((Stage1*)gGameFramework.m_pScene)->m_ppShaders[0]->m_ppObjects[10 + i]->m_xmf4x4ToParent._42 = 0.0f;
							gGameFramework.DyingMotionNPC(i);
							break;
						}
						npcs_info[i].m_new_state_update = false;
					}
				}

				// 고도 경보 (1.5초 주기로)
				if (b_height_alert) {
					bool speak_alert = false;
					if (b_first_height_alert) {
						speak_alert = true;
						b_first_height_alert = false;
					}
					else if (height_alert_time + 1500ms <= system_clock::now()) {
						speak_alert = true;
					}

					if (speak_alert) {
						// 경보 출력
						gamesound.PlayHightLimitSound();
						height_alert_time = system_clock::now();
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
				}
				else
				{
					((CSpriteObjectsShader*)((Stage1*)gGameFramework.m_pScene)->m_ppSpriteBillboard[0])->m_bActive = false;
				}

				if (!q_bullet_hit_pos_ground.empty()) {
					XMFLOAT3 ground_collide_pos = q_bullet_hit_pos_ground.front();
					q_bullet_hit_pos_ground.pop();
					gGameFramework.CollisionMap_by_BULLET(ground_collide_pos);
				}
				else
				{

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
	case WM_CHAR:
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
			case SEND_KEY_W:
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

			case SEND_KEY_S:
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

			case SEND_KEY_A:
			case SEND_KEY_D:
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

			case SEND_KEY_R:
				// 재장전
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET input_rkey_pack;
				input_rkey_pack.type = CS_INPUT_KEYBOARD;
				input_rkey_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				input_rkey_pack.keytype = PACKET_KEY_R;
				sendPacket(&input_rkey_pack);
				break;

			case SEND_KEY_Q:	// 상승
				if (players_info[my_id].m_role != ROLE_HELI) break;	// 상승은 헬기만 가능
				CS_MOVE_PACKET move_up_pack;
				move_up_pack.size = sizeof(CS_MOVE_PACKET);
				move_up_pack.type = CS_MOVE;
				move_up_pack.x = gGameFramework.getMyPosition().x;
				move_up_pack.y = gGameFramework.getMyPosition().y;
				move_up_pack.z = gGameFramework.getMyPosition().z;
				move_up_pack.direction = MV_FRONT;
				sendPacket(&move_up_pack);
				break;

			case SEND_KEY_E:	// 하강
				if (players_info[my_id].m_role != ROLE_HELI) break;	// 상승은 헬기만 가능
				CS_MOVE_PACKET move_down_pack;
				move_down_pack.size = sizeof(CS_MOVE_PACKET);
				move_down_pack.type = CS_MOVE;
				move_down_pack.x = gGameFramework.getMyPosition().x;
				move_down_pack.y = gGameFramework.getMyPosition().y;
				move_down_pack.z = gGameFramework.getMyPosition().z;
				move_down_pack.direction = MV_FRONT;
				sendPacket(&move_down_pack);
				break;

			case SEND_KEYUP_MOVEKEY:
				if (gGameFramework.m_nMode == OPENINGSCENE) break;
				if (players_info[my_id].m_role != ROLE_RIFLE) break;
				CS_INPUT_KEYBOARD_PACKET mv_keyup_pack;
				mv_keyup_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				mv_keyup_pack.type = CS_INPUT_KEYBOARD;
				mv_keyup_pack.keytype = PACKET_KEYUP_MOVEKEY;
				sendPacket(&mv_keyup_pack);
				break;

			case SEND_KEY_INSERT:	// 치트키: 무적
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET cheatkey_immortal_pack;
				cheatkey_immortal_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				cheatkey_immortal_pack.type = CS_INPUT_KEYBOARD;
				cheatkey_immortal_pack.keytype = PACKET_KEY_INSERT;
				sendPacket(&cheatkey_immortal_pack);
				break;

			case SEND_KEY_DELETE:	// 치트키: 원샷원킬
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET cheatkey_oneshotonekill_pack;
				cheatkey_oneshotonekill_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				cheatkey_oneshotonekill_pack.type = CS_INPUT_KEYBOARD;
				cheatkey_oneshotonekill_pack.keytype = PACKET_KEY_DELETE;
				sendPacket(&cheatkey_oneshotonekill_pack);
				break;

			case SEND_KEY_END:	// 치트키: NPC 전부 죽이기
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET cheatkey_allkill_pack;
				cheatkey_allkill_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				cheatkey_allkill_pack.type = CS_INPUT_KEYBOARD;
				cheatkey_allkill_pack.keytype = PACKET_KEY_END;
				sendPacket(&cheatkey_allkill_pack);
				break;

			case SEND_KEY_PGUP:	// 치트키: 힐링
				if (gGameFramework.m_nMode == OPENINGSCENE) break;

				CS_INPUT_KEYBOARD_PACKET cheatkey_healing_pack;
				cheatkey_healing_pack.size = sizeof(CS_INPUT_KEYBOARD_PACKET);
				cheatkey_healing_pack.type = CS_INPUT_KEYBOARD;
				cheatkey_healing_pack.keytype = PACKET_KEY_PGUP;
				sendPacket(&cheatkey_healing_pack);
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

		// 4. 파티클 충돌
		if (gGameFramework.m_nMode == SCENE1STAGE) {
			if (players_info[my_id].m_role == ROLE_HELI && ((Stage1*)gGameFramework.m_pScene)->m_bHeliParticleCollisionCheck == true) {
				CS_PARTICLE_COLLIDE_PACKET heli_particle_coll_pack;
				heli_particle_coll_pack.size = sizeof(CS_PARTICLE_COLLIDE_PACKET);
				heli_particle_coll_pack.type = CS_PARTICLE_COLLIDE;
				heli_particle_coll_pack.particle_mass = 1.0f;
				sendPacket(&heli_particle_coll_pack);

				((Stage1*)gGameFramework.m_pScene)->m_bHeliParticleCollisionCheck = false;
			}
			if (players_info[my_id].m_role == ROLE_RIFLE && ((Stage1*)gGameFramework.m_pScene)->m_bHumanParticleCollisionCheck == true) {
				CS_PARTICLE_COLLIDE_PACKET human_particle_coll_pack;
				human_particle_coll_pack.size = sizeof(CS_PARTICLE_COLLIDE_PACKET);
				human_particle_coll_pack.type = CS_PARTICLE_COLLIDE;
				human_particle_coll_pack.particle_mass = 1.0f;
				sendPacket(&human_particle_coll_pack);

				((Stage1*)gGameFramework.m_pScene)->m_bHumanParticleCollisionCheck = false;
			}
		}

		// 5. 헬기 벽 충돌
		if (players_info[my_id].m_role == ROLE_HELI && gGameFramework.b_heli_mapcollide == true) {
			if (gGameFramework.b_heli_mapcollide_cooldown == 100) {
				CS_HELI_MAP_COLLIDE_PACKET heli_collide_pack;
				heli_collide_pack.size = sizeof(CS_HELI_MAP_COLLIDE_PACKET);
				heli_collide_pack.type = CS_HELI_MAP_COLLIDE;
				sendPacket(&heli_collide_pack);

				gGameFramework.b_heli_mapcollide_cooldown--;
			}
			else {
				gGameFramework.b_heli_mapcollide_cooldown--;

				if (gGameFramework.b_heli_mapcollide_cooldown == 0) {
					gGameFramework.b_heli_mapcollide = false;
				}
			}
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

void uiThreadFunc() {
	while (1) {
		if (gGameFramework.m_nMode == OPENINGSCENE) {
			// UI를 통한 조작 구현
			switch (gGameFramework.m_LoginScene) {
			case gGameFramework.LS_OPENING: // 게임 시작, 설정, 종료 
				break;

			case gGameFramework.LS_LOBBY:	// 로비
				if (trigger_lobby_update) {	// 로비에 있는 방 정보 업데이트 트리거
					game_enter_ok = false;	// 오프닝에서 넘어왔을때
					// 한번 초기화했다가
					gGameFramework.m_LobbyRoom_Info.clear();

					// 다시 값을 집어넣는다.
					for (auto& curr_room : lobby_rooms) {
						LobbyRoom new_room;

						new_room.currnum_of_people = curr_room.user_count;
						memcpy(new_room.name, charToWchar(curr_room.room_name), 40);
						new_room.num = curr_room.room_id;
						if (curr_room.room_state == R_ST_WAIT) {
							new_room.ready_state = 1;
						}
						else if (curr_room.room_state == R_ST_FULL) {
							new_room.ready_state = 2;
						}
						else if (curr_room.room_state == R_ST_INGAME) {
							new_room.ready_state = 3;
						}

						gGameFramework.m_LobbyRoom_Info.emplace_back(new_room);
					}

					trigger_lobby_update = false;
				}
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
					}
					else if (players_info[role_change_member_id].m_role == ROLE_RIFLE) {
						gGameFramework.m_MyRoom_Info[role_change_member_id].armyCheck = true;
						gGameFramework.m_MyRoom_Info[role_change_member_id].HeliCheck = false;
						if (role_change_member_id == my_id)
							gGameFramework.m_ingame_role = gGameFramework.R_RIFLE;
					}
					else if (players_info[role_change_member_id].m_role == ROLE_HELI) {
						gGameFramework.m_MyRoom_Info[role_change_member_id].armyCheck = false;
						gGameFramework.m_MyRoom_Info[role_change_member_id].HeliCheck = true;
						if (role_change_member_id == my_id)
							gGameFramework.m_ingame_role = gGameFramework.R_HELI;
					}
					role_change_member_id = -1;
					trigger_role_change = false;
				}

				// 인 게임 진입 UI
				if (stage1_enter_ok) {
					gGameFramework.m_infoReady = false;
					b_room_AnyOneNotReady = false;
					gGameFramework.m_infoChoose = false;
					b_room_PlzChooseRole = false;

					gGameFramework.m_ingame = true;
				}
				// 역할 선택해주세요.
				if (b_room_PlzChooseRole) {
					gGameFramework.m_infoReady = false;
					b_room_AnyOneNotReady = false;

					gGameFramework.m_infoChoose = true;
					if (gGameFramework.m_infoChooseTime >= 10.0f) {
						gGameFramework.m_infoChooseTime = 0.0f;
						b_room_PlzChooseRole = false;
						gGameFramework.m_infoChoose = false;
					}
				}

				// 준비 안한 사람이 있습니다.
				if (b_room_AnyOneNotReady) {
					gGameFramework.m_infoChoose = false;
					b_room_PlzChooseRole = false;

					gGameFramework.m_infoReady = true;
					if (gGameFramework.m_infoReadyTime >= 10.0f) {
						gGameFramework.m_infoReadyTime = 0.0f;
						b_room_AnyOneNotReady = false;
						gGameFramework.m_infoReady = false;
					}
				}

				if (trigger_leave_member) {	// 다른 유저 방 퇴장 트리거
					if (0 <= left_member_id && left_member_id < MAX_USER) {
						gGameFramework.setRoomUserInfo(left_member_id, L"\0", RM_ST_EMPTY);

						gGameFramework.m_MyRoom_Info[left_member_id].armyCheck = false;
						gGameFramework.m_MyRoom_Info[left_member_id].HeliCheck = false;

						cout << "Client[" << left_member_id << "]의 퇴장 처리가 완료되었습니다." << endl;
					}
					left_member_id = -1;
					trigger_leave_member = false;
				}

				if (trigger_room_update) {	// 방 데이터 전체 업데이트 트리거
					for (int i = 0; i < MAX_USER; ++i) {
						if (curr_room.user_state[i] == RM_ST_EMPTY) continue;
						gGameFramework.setRoomUserInfo(i, charToWchar(players_info[i].m_name), curr_room.user_state[i]);

						if (players_info[i].m_role == ROLE_NOTCHOOSE) {
							gGameFramework.m_MyRoom_Info[i].armyCheck = false;
							gGameFramework.m_MyRoom_Info[i].HeliCheck = false;
							if (i == my_id)
								gGameFramework.m_ingame_role = gGameFramework.R_NONE;
						}
						else if (players_info[i].m_role == ROLE_RIFLE) {
							gGameFramework.m_MyRoom_Info[i].armyCheck = true;
							gGameFramework.m_MyRoom_Info[i].HeliCheck = false;
							if (i == my_id)
								gGameFramework.m_ingame_role = gGameFramework.R_RIFLE;
						}
						else if (players_info[i].m_role == ROLE_HELI) {
							gGameFramework.m_MyRoom_Info[i].armyCheck = false;
							gGameFramework.m_MyRoom_Info[i].HeliCheck = true;
							if (i == my_id)
								gGameFramework.m_ingame_role = gGameFramework.R_HELI;
						}

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

			if (!gGameFramework.m_missionClear && b_startTime && !b_gameover) {
				if (timelimit_sec <= 0) {
					gGameFramework.m_missionFailed = true;
					gGameFramework.m_missionClear = false;
					gGameFramework.m_spendYourlife = false;
				}
			}

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
					int curr_percentage = static_cast<int>(stage_missions[1].curr / stage_missions[1].goal * 100);
					if (curr_percentage >= 100) curr_percentage = 100;

					gGameFramework.m_occupationnum = curr_percentage;
				}

				break;
			case SCENE2STAGE:
				//gGameFramework.m_remainNPC = stage_missions[2].goal - stage_missions[2].curr;
				break;
			}

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

			// 8. 피격 효과 UI
			if (players_info[my_id].m_damaged_effect_on) {

				gGameFramework.m_BloodSplatterOn = true;
				players_info[my_id].m_damaged_effect_on = false;
			}
			
			// 9. HP 적을때 UI
			if (players_info[my_id].m_near_death_hp) {
				if (players_info[my_id].m_role == ROLE_RIFLE) {

				}
				else if (players_info[my_id].m_role == ROLE_HELI) {
					gGameFramework.m_HeliPlayerWarnningUISwitch = true;
					gGameFramework.m_bHeliHittingMotion = true;
				}
			}
			else {
				if (players_info[my_id].m_role == ROLE_RIFLE) {

				}
				else if (players_info[my_id].m_role == ROLE_HELI) {
					if (gGameFramework.m_HeliPlayerWarnningUISwitch)
						gGameFramework.m_HeliPlayerWarnningUISwitch = false;

				}
			}

			// 10. HP, 이름 인게임 동기화
			bool idcheck = false;
			bool OutCheck = false;
			for (int i{}; i < gGameFramework.m_CurrentPlayerNum; ++i) {
				if (players_info[i].m_id == -1) {
					OutCheck = true;
					continue;
				}
				if (i == my_id) {
					idcheck = true;
					continue;
				}
				if (idcheck) {
					if (OutCheck) {
						gGameFramework.m_otherHP[i - 2] = players_info[i].m_hp;

						wchar_t* converttext = ConvertToWideChar(players_info[i].m_name);

						wcscpy_s(gGameFramework.m_OtherName[i - 2], converttext);
					}
					else {
						gGameFramework.m_otherHP[i - 1] = players_info[i].m_hp;

						wchar_t* converttext = ConvertToWideChar(players_info[i].m_name);

						wcscpy_s(gGameFramework.m_OtherName[i - 1], converttext);
					}
				}
				else {
					if (OutCheck) {
						gGameFramework.m_otherHP[i - 1] = players_info[i].m_hp;

						wchar_t* converttext = ConvertToWideChar(players_info[i].m_name);

						wcscpy_s(gGameFramework.m_OtherName[i - 1], converttext);
					}
					else {
						gGameFramework.m_otherHP[i] = players_info[i].m_hp;
						wchar_t* converttext = ConvertToWideChar(players_info[i].m_name);

						wcscpy_s(gGameFramework.m_OtherName[i], converttext);
					}
				}
			}

			// 11. 게임 오버 UI
			if (b_gameover && !gGameFramework.m_missionClear && !gGameFramework.m_missionFailed) {
				gGameFramework.m_spendYourlife = true;
				gGameFramework.m_missionClear = false;
				gGameFramework.m_missionFailed = false;
			}

			// 12. Life 표시
			wchar_t PlayerLifeCount[20];
			_itow_s(players_info[my_id].m_life, PlayerLifeCount, sizeof(PlayerLifeCount), 10);
			wcscpy_s(gGameFramework.m_mylifeCount, PlayerLifeCount);
		}

		this_thread::yield();
	}
}

