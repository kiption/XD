#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <chrono>
#include "ObjectsInfo.h"
#include "GameSound.h"

GameSound gamesound;

#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

//==================================================
enum SERVER_TYPE { SERVER_LOGIN, SERVER_LOBBY, SERVER_LOGIC };
short curr_servertype = -1;
short active_servernum = -1;

char IPADDR_LOBBY0[16];
char IPADDR_LOBBY1[16];

char IPADDR_LOGIC0[16];
char IPADDR_LOGIC1[16];

SOCKET lgn_socket;	// �α��μ��� ����
SOCKET lby_socket;	// �κ񼭹� ����
SOCKET lgc_socket;	// �������� ����

//==================================================
int my_id;

//==================================================
float servertime_ms;    // ���� �����ð�

volatile int timelimit_ms;       // ���������� ���ѽð�
volatile int timelimit_sec;

//==================================================
// �� ��ȯ ����
volatile bool stage1_enter_ok;
volatile bool stage2_enter_ok;

volatile bool game_enter_ok = false;
volatile bool game_exit_ok = false;

bool trigger_stage1_playerinfo_load = false;
bool trigger_stage1_mapinfo_load = false;

//==================================================
// �α��ξ� UI ����
bool ls_login_enter_ok = false;
bool ls_opening_enter_ok = false;
bool ls_lobby_enter_ok = false;
bool ls_room_enter_ok = false;
bool ls_create_room_enter_ok = false;

bool trigger_lobby_update = false;	// �κ� ���� �� ���� ������Ʈ Ʈ���� (�κ�ȭ�鿡 ��� ��� ���� ������Ʈ�� ����)
bool trigger_new_member = false; // ���ο� ���� �� ���� Ʈ����
bool trigger_leave_member = false; // ���ο� ���� �� ���� Ʈ����
bool trigger_room_update = false; // �� ���� ������ ���� Ʈ���� (�� ������ ��ü�� ������Ʈ�ϱ� ����)
bool trigger_role_change = false; // ���� ���� ���� Ʈ����

int new_member_id = -1;
int left_member_id = -1;
int role_change_member_id = -1;

//==================================================
// �ΰ��� ����
volatile bool respawn_trigger = false;
volatile int respawn_id = -1;

bool trigger_otherplayer_attack = false;	// �ٸ� �÷��̾� ���� ����
int otherplayer_attack_id;					// " id
XMFLOAT3 otherplayer_attack_dir;			// " ����

bool trigger_healpack_update = false;	// ���� on/off Ʈ����
bool healpack_effect_on = true;			// ���� on ������Ʈ�ΰ� ?
int updated_healpack_id = -1;			// ������Ʈ�Ǵ� ���� ID

bool b_gameover = false;	// ���� ���� Ʈ����

bool b_height_alert = false;						// ��� �� �溸
bool b_first_height_alert = false;
chrono::system_clock::time_point height_alert_time; // �溸 �︰ �ð�

//==================================================
int curr_connection_num = 1;

chrono::system_clock::time_point last_ping;   // ping�� ������ ���� �ð�
chrono::system_clock::time_point last_pong;   // ������ ping�� ���� ������ ���� �ð�

//==================================================
class Mission
{
public:
	short type;
	float goal;
	float curr;

public:
	Mission() {
		type = MISSION_KILL;
		goal = 0.0f;
		curr = 0.0f;
	}
};
array<Mission, TOTAL_STAGE + 1> stage_missions;
bool trigger_mission_complete = false;
bool trigger_stage_clear = false;
short curr_mission_num = 0;

//==================================================
struct message {
	char name[NAME_SIZE];
	char msg[CHAT_SIZE];
};
constexpr int MAX_SAVED_MSG = 8;
message chat_logs;
bool chat_comeTome = true;

//==================================================
enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };
class OVER_EX {
public:
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	char send_buf[BUF_SIZE];
	PACKET_PROCESS_TYPE process_type;

	OVER_EX()
	{
		wsabuf.len = BUF_SIZE;
		wsabuf.buf = send_buf;
		process_type = OP_RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	OVER_EX(char* packet)
	{
		wsabuf.len = packet[0];
		wsabuf.buf = send_buf;
		ZeroMemory(&overlapped, sizeof(overlapped));
		process_type = OP_SEND;
		memcpy(send_buf, packet, packet[0]);
	}
};

void disconnect() {
	curr_servertype = -1;
	active_servernum = -1;
	lby_socket = 0;
}

void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
OVER_EX g_recv_over;
void recvPacket()
{
	//cout << "Do RECV" << endl;
	DWORD recv_flag = 0;

	memset(&g_recv_over.overlapped, 0, sizeof(g_recv_over.overlapped));
	g_recv_over.wsabuf.len = BUF_SIZE;
	g_recv_over.wsabuf.buf = g_recv_over.send_buf;

	switch (curr_servertype) {
	case SERVER_LOGIN:
		break;
	case SERVER_LOBBY:
		if (WSARecv(lby_socket, &g_recv_over.wsabuf, 1, 0, &recv_flag, &g_recv_over.overlapped, recvCallback) == SOCKET_ERROR) {
			if (GetLastError() != WSA_IO_PENDING)
				cout << "[WSARecv Error] code: " << GetLastError() << "\n" << endl;
		}
		break;
	case SERVER_LOGIC:
		if (WSARecv(lgc_socket, &g_recv_over.wsabuf, 1, 0, &recv_flag, &g_recv_over.overlapped, recvCallback) == SOCKET_ERROR) {
			if (GetLastError() != WSA_IO_PENDING)
				cout << "[WSARecv Error] code: " << GetLastError() << "\n" << endl;
		}
		break;
	}
}
void sendPacket(void* packet)
{
	//cout << "Do SEND" << endl;
	OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };

	switch (curr_servertype) {
	case SERVER_LOGIN:
		break;
	case SERVER_LOBBY:
		if (WSASend(lby_socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback) == SOCKET_ERROR) {
			int err_no = GetLastError();
			if (err_no == WSAECONNRESET) {   // ������ ������ ��Ȳ
				closesocket(lby_socket);

				int new_portnum = 0;
				if (active_servernum == 0) {		// Active: 0 -> 1
					active_servernum = 1;
					new_portnum = PORTNUM_LOBBY_1;
				}
				else if (active_servernum == 1) {	// Active: 1 -> 0
					active_servernum = 0;
					new_portnum = PORTNUM_LOBBY_0;
				}

				lby_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
				SOCKADDR_IN newserver_addr;
				ZeroMemory(&newserver_addr, sizeof(newserver_addr));
				newserver_addr.sin_family = AF_INET;
				newserver_addr.sin_port = htons(new_portnum);
				//inet_pton(AF_INET, SERVER_ADDR, &newserver_addr.sin_addr);//������

				// REMOTE
				if (active_servernum == 0) {
					inet_pton(AF_INET, IPADDR_LOGIC0, &newserver_addr.sin_addr);
				}
				else if (active_servernum == 1) {
					inet_pton(AF_INET, IPADDR_LOGIC1, &newserver_addr.sin_addr);
				}
				connect(lby_socket, reinterpret_cast<sockaddr*>(&newserver_addr), sizeof(newserver_addr));

				CS_RELOGIN_PACKET re_login_pack;
				re_login_pack.size = sizeof(CS_RELOGIN_PACKET);
				re_login_pack.type = CS_RELOGIN;
				re_login_pack.id = my_id;
				sendPacket(&re_login_pack);
				recvPacket();
			}
			cout << "[WSASend Error] code: " << err_no << "\n" << endl;
		}
		break;
	case SERVER_LOGIC:
		if (WSASend(lgc_socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback) == SOCKET_ERROR) {
			int err_no = GetLastError();
			if (err_no == WSAECONNRESET) {   // ������ ������ ��Ȳ
				closesocket(lgc_socket);

				int new_portnum = 0;
				if (active_servernum == 0) {		// Active: 0 -> 1
					active_servernum = 1;
					new_portnum = PORTNUM_LOGIC_1;
				}
				else if (active_servernum == 1) {	// Active: 1 -> 0
					active_servernum = 0;
					new_portnum = PORTNUM_LOGIC_0;
				}

				lgc_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
				SOCKADDR_IN newserver_addr;
				ZeroMemory(&newserver_addr, sizeof(newserver_addr));
				newserver_addr.sin_family = AF_INET;
				newserver_addr.sin_port = htons(new_portnum);
				//inet_pton(AF_INET, SERVER_ADDR, &newserver_addr.sin_addr);//������

				// REMOTE
				if (active_servernum == 0) {
					inet_pton(AF_INET, IPADDR_LOGIC0, &newserver_addr.sin_addr);
				}
				else if (active_servernum == 1) {
					inet_pton(AF_INET, IPADDR_LOGIC1, &newserver_addr.sin_addr);
				}
				connect(lby_socket, reinterpret_cast<sockaddr*>(&newserver_addr), sizeof(newserver_addr));

				CS_RELOGIN_PACKET re_login_pack;
				re_login_pack.size = sizeof(CS_RELOGIN_PACKET);
				re_login_pack.type = CS_RELOGIN;
				re_login_pack.id = my_id;
				sendPacket(&re_login_pack);
				recvPacket();
			}
			cout << "[WSASend Error] code: " << err_no << "\n" << endl;
		}
		break;
	}
}

void processPacket(char* ptr);
void processData(char* net_buf, size_t io_byte);
void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	if (num_bytes == 0) {
		return;
	}

	processData(g_recv_over.send_buf, num_bytes);

	recvPacket();
}
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	delete over;
	return;
}


void processPacket(char* ptr)
{
	switch (ptr[1])
	{
		//==========
		// �κ񼭹�
	case LBYC_MATCH_FAIL:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_MATCH_FAIL_PACKET* recv_packet = reinterpret_cast<LBYC_MATCH_FAIL_PACKET*>(ptr);

		// Error Message
		switch (recv_packet->fail_reason) {
		case MATCH_FAIL_NOEMPTYROOM:
			cout << "����ִ� ���� ���� ��Ī�� �����Ͽ����ϴ�. ���� ���� �������ּ���.\n" << endl;
			break;
		default:
			cout << "�� �� ���� ������ ��Ī�� �����Ͽ����ϴ�.\n" << endl;
		}

		break;
	}// LBYC_MATCH_FAIL case end
	case LBYC_ROOM_JOIN:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_ROOM_JOIN_PACKET* recv_packet = reinterpret_cast<LBYC_ROOM_JOIN_PACKET*>(ptr);

		// �ڽ��� ������ ���� ������ ������Ʈ�մϴ�.
		curr_room_id = recv_packet->room_id;
		curr_room.room_id = recv_packet->room_id;
		strcpy_s(curr_room.room_name, recv_packet->room_name);
		curr_room.user_count = recv_packet->member_count;
		if (curr_room.user_count == MAX_USER) curr_room.room_state = R_ST_FULL;
		else								  curr_room.room_state = R_ST_WAIT;

		for (int i = 0; i < MAX_USER; ++i) {
			curr_room.user_state[i] = recv_packet->member_state[i];
		}

		// ������ �濡 �ִ� ���� ������ ������Ʈ�մϴ�.
		for (int i = 0; i < MAX_USER; ++i) {
			if (recv_packet->member_state[i] == RM_ST_EMPTY) continue;

			players_info[i].m_state = ST_INGAME;
			players_info[i].m_id = i;
			strcpy_s(players_info[i].m_name, recv_packet->member_name[i]);
			players_info[i].m_role = recv_packet->member_role[i];
		}

		// ��� ���õ� �� �� ������ ������Ʈ�մϴ�.
		my_id = recv_packet->your_roomindex;
		my_room_index = recv_packet->your_roomindex;
		if (recv_packet->b_manager == b_TRUE) b_room_manager = true;
		else								  b_room_manager = false;

		// Debug
		cout << "Room[ID: " << curr_room.room_id << ", Name: " << curr_room.room_name << "]�� �����մϴ�. (�� �����ο�: " << curr_room.user_count << "��)" << endl;
		cout << "����� Room Index�� " << my_room_index << "�Դϴ�." << endl;
		if (b_room_manager) cout << "����� �� ���� �����Դϴ�." << endl;
		if (recv_packet->member_count != 1) {
			cout << "����� �����ϱ� ������ �濡 �ִ� ���� �����Դϴ�." << endl;
			for (int i = 0; i < MAX_USER; ++i) {
				if (players_info[i].m_state != ST_INGAME) continue;
				if (i == my_room_index) continue;
				cout << "  - [" << i << "] ID: " << players_info[i].m_id << ", Name: " << players_info[i].m_name << endl;
			}
		}
		cout << "\n";

		// �α��ξ��� �� UI�� ���� Ʈ���Ÿ� true�� ����ϴ�.
		ls_room_enter_ok = true;

		// �� UI ���� ���ο� ���� Ʈ���Ÿ� true�� ����ϴ� (���� �濡 �ִ� ������ �߰��� ����)
		trigger_room_update = true;

		break;
	}// LBYC_ROOM_JOIN case end
	case LBYC_ADD_ROOM:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_ADD_ROOM_PACKET* recv_packet = reinterpret_cast<LBYC_ADD_ROOM_PACKET*>(ptr);

		RoomInfo new_room;
		new_room.room_id = recv_packet->room_id;
		strcpy_s(new_room.room_name, recv_packet->room_name);
		new_room.room_state = recv_packet->room_state;
		new_room.user_count = recv_packet->user_count;
		lobby_rooms.push_back(new_room);

		cout << "���ο� Room�� �߰��մϴ�." << endl;
		cout << "[ID: " << new_room.room_id << ", �̸�: " << new_room.room_name;
		cout << ", ���� �ο�: " << new_room.user_count << "��, �� ����: ";
		if (new_room.room_state == R_ST_WAIT) cout << "�����";
		else if (new_room.room_state == R_ST_FULL) cout << "������";
		else if (new_room.room_state == R_ST_INGAME) cout << "������";
		cout << ".] \n" << endl;

		trigger_lobby_update = true;
		break;
	}// LBYC_ADD_ROOM case end
	case LBYC_ROOM_USERCOUNT:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_ROOM_USERCOUNT_PACKET* recv_packet = reinterpret_cast<LBYC_ROOM_USERCOUNT_PACKET*>(ptr);

		// �κ� ǥ�õ� �� ������ ������Ʈ�մϴ�.
		int recv_room_id = recv_packet->room_id;
		for (auto& room : lobby_rooms) {
			if (room.room_id == recv_room_id) {
				room.user_count = recv_packet->user_count;
				cout << "Room[" << recv_room_id << "]�� ���� �ο��� " << room.user_count << "���� �Ǿ����ϴ�.\n" << endl;

				if ((room.room_state != R_ST_FULL) && (room.user_count == MAX_USER)) {
					room.room_state = R_ST_FULL;
				}
				if ((room.room_state == R_ST_FULL) && (room.user_count != MAX_USER)) {
					room.room_state = R_ST_WAIT;
				}
			}
		}
		trigger_lobby_update = true;

		break;
	}// LBYC_ROOM_USERCOUNT case end
	case LBYC_ROOM_NEW_MEMBER:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_ROOM_NEW_MEMBER_PACKET* recv_packet = reinterpret_cast<LBYC_ROOM_NEW_MEMBER_PACKET*>(ptr);

		int new_member_index = recv_packet->new_member_roomindex;

		// ���� �� ���� ������Ʈ (��ü �� ���� lobby_room �� ������Ʈ�� �ʿ䰡 ����. lobby_room�� �κ� �������� ������Ʈ�Ѵ�.)
		curr_room.user_count++;
		if (curr_room.user_count >= MAX_USER) {
			curr_room.room_state = R_ST_FULL;
			cout << "Room[" << curr_room_id << "] ���� ����.\n" << endl;
		}

		curr_room.user_state[new_member_index] = RM_ST_NONREADY;

		// ���� ���� ������Ʈ
		players_info[new_member_index].m_state = ST_INGAME;
		players_info[new_member_index].m_id = new_member_index;
		strcpy_s(players_info[new_member_index].m_name, recv_packet->new_member_name);

		// �� UI ���� ���ο� ���� Ʈ���Ÿ� true�� ����ϴ�
		trigger_new_member = true;
		new_member_id = new_member_index;

		// Debug
		cout << "Room[" << curr_room_id << "]�� ���ο� ����[ID: " << players_info[new_member_index].m_id
			<< ", Name: " << players_info[new_member_index].m_name << "]�� �����Ͽ����ϴ�.\n" << endl;

		break;
	}// LBYC_ROOM_NEW_MEMBER case end
	case LBYC_ROOM_LEFT_MEMBER:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_ROOM_LEFT_MEMBER_PACKET* recv_packet = reinterpret_cast<LBYC_ROOM_LEFT_MEMBER_PACKET*>(ptr);

		int left_member_index = recv_packet->left_member_roomindex;

		// Debug
		cout << "Room[" << curr_room_id << "]���� ����[ID: " << players_info[left_member_index].m_id
			<< ", Name: " << players_info[left_member_index].m_name << "]�� �����Ͽ����ϴ�.\n" << endl;

		// ���� �� ���� ������Ʈ (��ü �� ���� lobby_room �� ������Ʈ�� �ʿ䰡 ����. lobby_room�� �κ� �������� ������Ʈ�Ѵ�.)
		curr_room.user_count--;

		// ���� ���� ������Ʈ
		players_info[left_member_index].InfoClear();

		// �� UI ���� ���� ���� Ʈ���Ÿ� true�� ����ϴ�
		trigger_leave_member = true;
		left_member_id = left_member_index;

		break;
	}// LBYC_ROOM_LEFT_MEMBER case end
	case LBYC_LOBBY_CLEAR:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_LOBBY_CLEAR_PACKET* recv_packet = reinterpret_cast<LBYC_LOBBY_CLEAR_PACKET*>(ptr);

		game_enter_ok = true;
		players_info[my_id].m_role = ROLE_NOTCHOOSE;
		cout << "�κ� ���� �� ���� ��ü�� �ʱ�ȭ�մϴ�." << endl;
		lobby_rooms.clear();
		trigger_lobby_update = true;

		break;
	}// LBYC_LOBBY_CLEAR case end
	case LBYC_MEMBER_STATE:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_MEMBER_STATE_PACKET* recv_packet = reinterpret_cast<LBYC_MEMBER_STATE_PACKET*>(ptr);

		curr_room.user_state[recv_packet->member_id] = recv_packet->member_state;
		if (recv_packet->member_state == RM_ST_NONREADY)
			cout << "Client[" << recv_packet->member_id << "]�� �غ� �����Ǿ���.\n" << endl;
		else if (recv_packet->member_state == RM_ST_READY)
			cout << "Client[" << recv_packet->member_id << "]�� �غ� �Ϸ�Ǿ���.\n" << endl;

		trigger_room_update = true;

		break;
	}// LBYC_MEMBER_STATE case end
	case LBYC_ROLE_CHANGE:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_ROLE_CHANGE_PACKET* recv_packet = reinterpret_cast<LBYC_ROLE_CHANGE_PACKET*>(ptr);

		int member_index = recv_packet->member_id;

		players_info[member_index].m_role = recv_packet->role;
		role_change_member_id = member_index;
		trigger_role_change = true;
		break;
	}// LBYC_ROLE_CHANGE case end
	case LBYC_GAME_START:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_GAME_START_PACKET* recv_packet = reinterpret_cast<LBYC_GAME_START_PACKET*>(ptr);

		cout << "���� ������ �㰡�޾ҽ��ϴ�.\n" << endl;
		stage1_enter_ok = true;

		break;
	}// LBYC_GAME_START case end
	case LBYC_GAME_EXIT:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_GAME_EXIT_PACKET* recv_packet = reinterpret_cast<LBYC_GAME_EXIT_PACKET*>(ptr);

		cout << "���� ���Ḧ �㰡�޾ҽ��ϴ�.\n" << endl;
		game_exit_ok = true;

		break;
	}// LBYC_GAME_EXIT case end
	case LBYC_POPUP:
	{
		if (curr_servertype != SERVER_LOBBY) break;
		LBYC_POPUP_PACKET* recv_packet = reinterpret_cast<LBYC_POPUP_PACKET*>(ptr);

		switch (recv_packet->msg) {
		case POPUPMSG_PLZCHOOSEROLE:	// ���� ���� X
			cout << "���� �غ�X" << endl;
			b_room_PlzChooseRole = true;
			break;

		case POPUPMSG_ANYONENOTREADY:	// ������ �غ� ������.
			cout << "������ �غ� ������" << endl;
			b_room_AnyOneNotReady = true;
			break;
		}

		break;
	}// LBYC_POPUP case end
	//==========
	// ��������
	case SC_LOGIN_INFO:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_LOGIN_INFO_PACKET* recv_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		// Player �ʱ����� ����
		players_info[my_id].m_hp = recv_packet->hp;
		players_info[my_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		players_info[my_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		players_info[my_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
		players_info[my_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		players_info[my_id].m_bullet = recv_packet->remain_bullet;
		players_info[my_id].m_state = OBJ_ST_RUNNING;
		players_info[my_id].m_ingame_state = PL_ST_IDLE;
		players_info[my_id].m_new_state_update = true;
		cout << "�ڱ� �ڽ��� ������ �޾ҽ��ϴ�. (Myid: " << my_id << ")" << endl;

		trigger_stage1_playerinfo_load = true;
		break;
	}// SC_LOGIN_INFO case end
	case SC_ADD_OBJECT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_ADD_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Add Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;

			if (0 <= recv_id && recv_id < MAX_USER) {      // Player �߰�
				players_info[recv_id].m_id = recv_id;
				strcpy_s(players_info[recv_id].m_name, recv_packet->name);
				players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
				players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
				players_info[recv_id].m_state = OBJ_ST_RUNNING;
				players_info[recv_id].m_ingame_state = recv_packet->obj_state;
				players_info[recv_id].m_new_state_update = true;
				cout << "�ٸ� �÷��̾�(ID: " << recv_id << ")�� ������ �޾ҽ��ϴ�." << endl;

				curr_connection_num++;
			}
			else {
				cout << "[SC_ADD Error] Unknown ID. (Input ID: " << recv_id << ")" << endl;
			}
		}
		// 2. Add NPC (Helicopter)
		else if (recv_packet->target == TARGET_NPC) {
			if (npcs_info[recv_id].m_state != OBJ_ST_EMPTY) break;

			npcs_info[recv_id].m_id = recv_id;
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			npcs_info[recv_id].m_state = OBJ_ST_RUNNING;
			npcs_info[recv_id].m_ingame_state = recv_packet->obj_state;
			npcs_info[recv_id].m_new_state_update = true;
		}
		else {
			cout << "[ADD ERROR] Unknown Target!" << endl;
		}
		break;
	}// SC_ADD_OBJECT case end
	case SC_MOVE_OBJECT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_MOVE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Move Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;             // �ڱ��ڽ��� Ŭ�󿡼� �������ְ� ����.
			if (recv_id < 0 || recv_id > MAX_USER) break;   // �߸��� ID��

			players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };      // ����Players Object �̵�
			switch (recv_packet->direction) {
			case MV_FRONT:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_FRONT;
				break;
			case MV_BACK:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_BACK;
				break;
			case MV_SIDE:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_SIDE;
				break;
			}
			players_info[recv_id].m_new_state_update = true;
		}
		// 2. Move Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_ingame_state = PL_ST_MOVE_FRONT;
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			npcs_info[recv_id].m_new_state_update = true;
		}
		else {
			cout << "[MOVE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_MOVE_OBJECT case end
	case SC_ROTATE_OBJECT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_ROTATE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_ROTATE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Rotate Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;            // �ڱ��ڽ��� Ŭ�󿡼� �������ְ� ����.
			if (recv_id < 0 || recv_id > MAX_USER) break;   // �߸��� ID��

			// ��� Object ȸ��
			players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		// 2. Rotate Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_ROTATE_PLAYER case end
	case SC_MOVE_ROTATE_OBJECT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_MOVE_ROTATE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_MOVE_ROTATE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Rotate Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;            // �ڱ��ڽ��� Ŭ�󿡼� �������ְ� ����.
			if (recv_id < 0 || recv_id > MAX_USER) break;   // �߸��� ID��
			players_info[recv_id].m_id = recv_id;
			// ��� Object �̵�
			players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			switch (recv_packet->direction) {
			case MV_FRONT:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_FRONT;
				break;
			case MV_BACK:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_BACK;
				break;
			case MV_SIDE:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_SIDE;
				break;
			}
			players_info[recv_id].m_new_state_update = true;
			// ��� Object ȸ��
			players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		// 2. Rotate Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_id = recv_id;
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };

			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[MOVE&ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_MOVE_ROTATE_PLAYER case end
	case SC_HEIGHT_ALERT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_HEIGHT_ALERT_PACKET* recv_packet = reinterpret_cast<SC_HEIGHT_ALERT_PACKET*>(ptr);

		if (recv_packet->alert_on == 0) {
			// �溸 Off
			b_height_alert = false;
		}
		else if (recv_packet->alert_on == 1) {
			// �溸 On
			b_height_alert = true;
			b_first_height_alert = true;
		}
		break;
	}// SC_HEIGHT_ALERT case end
	case SC_REMOVE_OBJECT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_REMOVE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Remove Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) {
				// �ڱ��ڽ� ���ֱ�
			}
			else if (0 <= recv_id && recv_id < MAX_USER) {
				// ��� Player ���ֱ�
				players_info[recv_id].m_id = -1;
				strcpy_s(players_info[recv_id].m_name, "\0");
				players_info[recv_id].m_pos = { 0.f ,0.f ,0.f };
				players_info[recv_id].m_state = OBJ_ST_LOGOUT;

				curr_connection_num--;
				if (curr_connection_num < 1) curr_connection_num = 1;
			}
		}
		// 2. Remove Npc
		else if (recv_packet->target == TARGET_NPC) {
			int npc_id = recv_id - MAX_USER;
			npcs_info[npc_id].m_id = -1;
			npcs_info[npc_id].m_pos = { 0.f ,0.f ,0.f };
			npcs_info[npc_id].m_state = OBJ_ST_LOGOUT;
		}
		else {
			cout << "[REMOVE ERROR] Unknown Target!" << endl;
		}

		break;
	}//SC_REMOVE_PLAYER case end
	case SC_DAMAGED:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_DAMAGED_PACKET* recv_packet = reinterpret_cast<SC_DAMAGED_PACKET*>(ptr);
		int recv_id = recv_packet->id;
		int damaged_sound_volume = recv_packet->sound_volume;

		// Player Damaged
		if (recv_packet->target == TARGET_PLAYER) {
			// �Ÿ��� ���� ����
			if (damaged_sound_volume == VOL_LOW) {			// �ָ� �־ �۰� �鸮�� �Ҹ�
				if (players_info[recv_id].m_role == ROLE_RIFLE)
				{
					gamesound.HumancollisionSound();
					gamesound.HumanColliChannel->setVolume(0.05f);

				}
				if (players_info[recv_id].m_role == ROLE_HELI)
				{
					gamesound.HelicollisionSound();
					gamesound.HeliColliChannel->setVolume(0.05f);
				}
			}
			else if (damaged_sound_volume == VOL_MID) {		// ������ �Ÿ��� �־ �����ϰ� �鸮�� �Ҹ�
				if (players_info[recv_id].m_role == ROLE_RIFLE)
				{
					gamesound.HumancollisionSound();
					gamesound.HumanColliChannel->setVolume(0.25f);
				}
				if (players_info[recv_id].m_role == ROLE_HELI)
				{
					gamesound.HelicollisionSound();
					gamesound.HeliColliChannel->setVolume(0.25f);
				}

			}
			else if (damaged_sound_volume == VOL_HIGH) {	// �����̿� �־ ũ�� �鸮�� �Ҹ�
				if (players_info[recv_id].m_role == ROLE_RIFLE)
				{
					gamesound.HumancollisionSound();
					gamesound.HumanColliChannel->setVolume(0.5f);
				}
				if (players_info[recv_id].m_role == ROLE_HELI)
				{
					gamesound.HelicollisionSound();
					gamesound.HeliColliChannel->setVolume(0.5f);
				}
			}

			// �ǰ� ó��
			players_info[recv_id].m_hp -= recv_packet->damage;
			players_info[recv_id].m_damaged_effect_on = true;
			if (players_info[recv_id].m_hp < 0)
			{
				players_info[recv_packet->id].m_hp = 0;
			}
			else if (players_info[recv_id].m_hp <= 30 && players_info[recv_packet->id].m_near_death_hp == false)
			{
				if (recv_id == my_id)
				{
					if (players_info[recv_id].m_role == ROLE_RIFLE)
					{
						gamesound.PlayHearBeatSound();
					}
					if (players_info[recv_id].m_role == ROLE_HELI)
					{
						gamesound.PlayHeliWarnningSound();
					}
					players_info[recv_packet->id].m_near_death_hp = true;
				}
			}
		}
		// NPC Damaged
		else if (recv_packet->target == TARGET_NPC) {
			// �Ÿ��� ���� ����
			if (damaged_sound_volume == VOL_LOW) {			// �ָ� �־ �۰� �鸮�� �Ҹ�
				if (recv_id < MAX_NPC_HELI)
				{
					gamesound.HelicollisionSound();
					gamesound.HeliColliChannel->setVolume(0.05f);
				}
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
				{
					gamesound.HumancollisionSound();
					gamesound.HumanColliChannel->setVolume(0.05f);
				}
			}
			else if (damaged_sound_volume == VOL_MID) {		// ������ �Ÿ��� �־ �����ϰ� �鸮�� �Ҹ�
				if (recv_id < MAX_NPC_HELI)
				{
					gamesound.HelicollisionSound();
					gamesound.HeliColliChannel->setVolume(0.25f);
				}
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
				{
					gamesound.HumancollisionSound();
					gamesound.HumanColliChannel->setVolume(0.25f);
				}
			}
			else if (damaged_sound_volume == VOL_HIGH) {	// �����̿� �־ ũ�� �鸮�� �Ҹ�
				if (recv_id < MAX_NPC_HELI)
				{
					gamesound.HelicollisionSound();
					gamesound.HeliColliChannel->setVolume(0.5f);
				}
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
				{
					gamesound.HumancollisionSound();
					gamesound.HumanColliChannel->setVolume(0.5f);
				}
			}

			// �ǰ� ó��
			npcs_info[recv_id].m_hp -= recv_packet->damage;
			npcs_info[recv_id].m_damaged_effect_on = true;
			if (npcs_info[recv_id].m_hp < 0) npcs_info[recv_packet->id].m_hp = 0;
		}

		break;
	}//SC_DAMAGED case end
	case SC_HEALING:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_HEALING_PACKET* recv_packet = reinterpret_cast<SC_HEALING_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		players_info[recv_id].m_hp += recv_packet->value;
		if (players_info[recv_id].m_hp > 100)
			players_info[recv_id].m_hp = 100;

		else if (players_info[recv_id].m_hp > 30 && players_info[recv_packet->id].m_near_death_hp == true) {
			players_info[recv_packet->id].m_near_death_hp = false;
			gamesound.pauseHeartBeat();
		}

		break;
	}// SC_HEALING case end
	case SC_HEALPACK:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_HEALPACK_PACKET* recv_packet = reinterpret_cast<SC_HEALPACK_PACKET*>(ptr);
		int recv_id = recv_packet->healpack_id;

		if (recv_packet->isused == 1) {
			healpack_effect_on = false;
			gamesound.PlayHealingSound();
			cout << recv_id << "��° ���� ����." << endl;
		}
		else if (recv_packet->isused == 0) {
			healpack_effect_on = true;
			cout << recv_id << "��° ���� ������." << endl;
		}

		trigger_healpack_update = true;
		updated_healpack_id = recv_id;

		break;
	}// SC_HEALPACK case end
	case SC_ATTACK:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_ATTACK_PACKET* recv_packet = reinterpret_cast<SC_ATTACK_PACKET*>(ptr);

		int recv_id = recv_packet->id;
		int atk_sound_volume = recv_packet->sound_volume;
		if (recv_packet->obj_type == TARGET_PLAYER) {
			//cout << "Client[" << recv_packet->id << "]�� ��������." << endl;
			// ���� Ʈ����
			trigger_otherplayer_attack = true;
			otherplayer_attack_id = recv_id;
			otherplayer_attack_dir = { recv_packet->atklook_x, recv_packet->atklook_y, recv_packet->atklook_z };

			if (atk_sound_volume == VOL_LOW) {			// �ָ� �־ �۰� �鸮�� �Ѽ�
				//cout << "���� �Ѽ�" << endl;
				if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
				{
					gamesound.PlayShotSound();
					gamesound.shotChannel->setVolume(0.05f);
				}
				if (players_info[recv_packet->id].m_role == ROLE_HELI)
				{
					gamesound.PlayHeliShotSound();
					gamesound.HelishotChannel->setVolume(0.05f);
				}
			}
			else if (atk_sound_volume == VOL_MID) {		// ������ �Ÿ��� �־ �����ϰ� �鸮�� �Ѽ�
				//cout << "������ �Ѽ�" << endl;
				if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
				{
					gamesound.PlayShotSound();
					gamesound.shotChannel->setVolume(0.1f);
				}
				if (players_info[recv_packet->id].m_role == ROLE_HELI)
				{
					gamesound.PlayHeliShotSound();
					gamesound.HelishotChannel->setVolume(0.25f);
				}
			}
			else if (atk_sound_volume == VOL_HIGH) {	// �����̿� �־ ũ�� �鸮�� �Ѽ�
				//cout << "ū �Ѽ�" << endl;
				if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
				{
					gamesound.PlayShotSound();
					gamesound.shotChannel->setVolume(0.3f);
				}
				if (players_info[recv_packet->id].m_role == ROLE_HELI)
				{
					gamesound.PlayHeliShotSound();
					gamesound.HelishotChannel->setVolume(0.5f);
				}
			}

			if (recv_packet->id == my_id) {
				if (players_info[recv_packet->id].m_bullet >= 1)
				{
					players_info[recv_packet->id].m_bullet--;
					if (players_info[recv_packet->id].m_role == ROLE_RIFLE)gamesound.PlayFallDownEmptyBullet();
				}
				/*if (players_info[recv_packet->id].m_bullet <= 0)
				{
					if (players_info[recv_packet->id].m_role == ROLE_RIFLE)gamesound.PlayEmptyShot();
				}*/
			}
		}	/*	if (recv_id < MAX_NPC_HELI)
		{
			gamesound.HelicollisionSound();
			gamesound.HeliColliChannel->setVolume(0.05f);
		}
		if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
		{
			gamesound.HumancollisionSound();
			gamesound.HumanColliChannel->setVolume(0.05f);
		}*/
		else if (recv_packet->obj_type == TARGET_NPC) {
			//cout << "NPC[" << recv_packet->id << "]�� ��������." << endl;

			// NPC ���� ������ ���� ����
			npcs_info[recv_id].m_attack_dir.x = recv_packet->atklook_x;
			npcs_info[recv_id].m_attack_dir.y = recv_packet->atklook_y;
			npcs_info[recv_id].m_attack_dir.z = recv_packet->atklook_z;
			npcs_info[recv_id].m_attack_on = true;

			if (atk_sound_volume == VOL_LOW) {			// �ָ� �־ �۰� �鸮�� �Ѽ�
				if (recv_id < MAX_NPC_HELI)
				{
					gamesound.PlayHeliShotSound();
					gamesound.HelishotChannel->setVolume(0.05f);
				}
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
				{
					gamesound.PlayShotSound();
					gamesound.shotChannel->setVolume(0.05f);
				}
				//cout << "���� �Ѽ�" << endl;
			}
			else if (atk_sound_volume == VOL_MID) {		// ������ �Ÿ��� �־ �����ϰ� �鸮�� �Ѽ�
				if (recv_id < MAX_NPC_HELI)
				{
					gamesound.PlayHeliShotSound();
					gamesound.HelishotChannel->setVolume(0.25f);
				}
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
				{
					gamesound.PlayShotSound();
					gamesound.shotChannel->setVolume(0.25f);
				}
				//cout << "������ �Ѽ�" << endl;
			}
			else if (atk_sound_volume == VOL_HIGH) {	// �����̿� �־ ũ�� �鸮�� �Ѽ�
				if (recv_id < MAX_NPC_HELI)
				{
					gamesound.PlayHeliShotSound();
					gamesound.HelishotChannel->setVolume(0.5f);
				}
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN)
				{
					gamesound.PlayShotSound();
					gamesound.shotChannel->setVolume(0.5f);
				}
				//cout << "ū �Ѽ�" << endl;
			}
			/*else
			{
				gamesound.PauseNpcShotSound();
			}*/
		}
		break;
	}// SC_ATTACK case end
	case SC_RELOAD:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_RELOAD_PACKET* recv_packet = reinterpret_cast<SC_RELOAD_PACKET*>(ptr);

		// ����
		if (recv_packet->id == my_id) {
			players_info[recv_packet->id].m_bullet = recv_packet->bullet_cnt;
		}
		if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
		{
			gamesound.reloadSound();
		}
		if (players_info[recv_packet->id].m_role == ROLE_HELI)
		{
			gamesound.reloadSound();
		}
		// ���� �Ҹ�
		int sound_volume = recv_packet->sound_volume;
		if (sound_volume == VOL_LOW) {			// �ָ� �־ �۰� �鸮�� �Ҹ�
			//cout << "���� �Ҹ�" << endl;
			if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
			{
				gamesound.reloadSound();
				gamesound.reloadChannel->setVolume(0.05f);
			}
			if (players_info[recv_packet->id].m_role == ROLE_HELI)
			{
				gamesound.reloadSound();
				gamesound.reloadChannel->setVolume(0.05f);
			}
		}
		else if (sound_volume == VOL_MID) {		// ������ �Ÿ��� �־ �����ϰ� �鸮�� �Ҹ�
			//cout << "������ �Ҹ�" << endl;
			if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
			{
				gamesound.reloadSound();
				gamesound.reloadChannel->setVolume(0.25f);
			}
			if (players_info[recv_packet->id].m_role == ROLE_HELI)
			{
				gamesound.reloadSound();
				gamesound.reloadChannel->setVolume(0.25f);
			}
		}
		else if (sound_volume == VOL_HIGH) {	// �����̿� �־ ũ�� �鸮�� �Ҹ�
			//cout << "ū �Ҹ�" << endl;
			if (players_info[recv_packet->id].m_role == ROLE_RIFLE)
			{
				gamesound.reloadSound();
				gamesound.reloadChannel->setVolume(0.5f);
			}
			if (players_info[recv_packet->id].m_role == ROLE_HELI)
			{
				gamesound.reloadSound();
				gamesound.reloadChannel->setVolume(0.5f);
			}
		}

		break;
	}// case SC_RELOAD case end
	case SC_CHANGE_SCENE:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_CHANGE_SCENE_PACKET* recv_packet = reinterpret_cast<SC_CHANGE_SCENE_PACKET*>(ptr);

		int recv_id = recv_packet->id;
		players_info[recv_id].curr_scene = recv_packet->scene_num;

		if (recv_id == my_id) {
			if (recv_packet->scene_num == 1) {
				stage1_enter_ok = true;
			}
			else if (recv_packet->scene_num == 2) {
				stage2_enter_ok = true;
			}
		}

		break;
	}//SC_CHANGE_SCENE case end
	case SC_OBJECT_STATE:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_OBJECT_STATE_PACKET* recv_packet = reinterpret_cast<SC_OBJECT_STATE_PACKET*>(ptr);

		int recv_id = recv_packet->id;
		short recvd_target = recv_packet->target;
		short recvd_state = recv_packet->state;

		if (recvd_target == TARGET_PLAYER) {
			players_info[recv_id].m_ingame_state = recvd_state;
			players_info[recv_id].m_new_state_update = true;
			switch (recvd_state) {
			case PL_ST_IDLE:
				// �̹� ������ ���¸� �ٲ���� ������ ���⿡�� �� �� ����.
				break;
			case PL_ST_ATTACK:
				break;
			case PL_ST_DEAD:
				if (players_info[recv_id].m_role == ROLE_RIFLE) gamesound.HumancollisionSound();
				if (players_info[recv_id].m_role == ROLE_HELI) gamesound.HeliiShotDownSound();
				if (recv_id == my_id) {
					gamesound.pauseHeartBeat();
					gamesound.PauseHeliWarnningSound();
					b_gameover = true;
				}
				players_info[recv_id].m_hp = 0;
				players_info[recv_id].m_damaged_effect_on = true;
				break;
			}
		}
		else if (recvd_target == TARGET_NPC) {
			npcs_info[recv_id].m_ingame_state = recvd_state;
			npcs_info[recv_id].m_new_state_update = true;
			switch (recvd_state) {
			case PL_ST_IDLE:
				break;
			case PL_ST_CHASE:
				break;
			case PL_ST_ATTACK:
				break;
			case PL_ST_DEAD:
				if (recv_id < MAX_NPC_HELI) gamesound.HeliiShotDownSound();
				if (MAX_NPC_HELI <= recv_id < MAX_NPC_HUMAN) gamesound.HumancollisionSound();
				npcs_info[recv_id].m_hp = 0;
				cout << "NPC[" << recv_id << "] �׾���" << endl;
				break;
			}
		}

		break;
	}//SC_OBJECT_STATE case end
	case SC_BULLET_COLLIDE_POS:
	{
		SC_BULLET_COLLIDE_POS_PACKET* recv_packet = reinterpret_cast<SC_BULLET_COLLIDE_POS_PACKET*>(ptr);

		XMFLOAT3 collide_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		switch (recv_packet->collide_target) {
		case C_OBJ_MAPOBJ:
			q_bullet_hit_pos_mapobj.push(XMFLOAT3{ recv_packet->x, recv_packet->y, recv_packet->z });
			break;
		case C_OBJ_GROUND:
			q_bullet_hit_pos_ground.push(XMFLOAT3{ recv_packet->x, recv_packet->y, recv_packet->z });
			break;
		}

		break;
	}//SC_BULLET_COLLIDE_POS case end
	case SC_MISSION:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_MISSION_PACKET* recv_packet = reinterpret_cast<SC_MISSION_PACKET*>(ptr);

		short stage_num = recv_packet->stage_num;
		stage_missions[stage_num].type = recv_packet->mission_type;
		stage_missions[stage_num].goal = recv_packet->mission_goal;
		stage_missions[stage_num].curr = recv_packet->mission_curr;

		break;
	}//SC_MISSION case end
	case SC_MISSION_COMPLETE:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_MISSION_COMPLETE_PACKET* recv_packet = reinterpret_cast<SC_MISSION_COMPLETE_PACKET*>(ptr);

		if (recv_packet->stage_num == 1) {
			if (curr_mission_num >= ST1_MISSION_NUM) {
				trigger_stage_clear = true;
				stage_missions[1].curr = 0;
			}
			else {
				trigger_mission_complete = true;
				curr_mission_num = recv_packet->mission_num + 1;
			}
		}
		else if (recv_packet->stage_num == 2) {
			//
		}

		break;
	}//SC_MISSION_COMPLETE case end
	case SC_TIME_TICKING:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_TIME_TICKING_PACKET* recv_packet = reinterpret_cast<SC_TIME_TICKING_PACKET*>(ptr);

		servertime_ms = recv_packet->servertime_ms;
		if (servertime_ms <= 0) servertime_ms = 1;
		timelimit_ms = STAGE1_TIMELIMIT * 1000 - servertime_ms;
		timelimit_sec = timelimit_ms / 1000;
		b_startTime = true;
		break;
	}//SC_TIME_TICKING case end
	case SC_CHAT:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_CHAT_PACKET* recv_packet = reinterpret_cast<SC_CHAT_PACKET*>(ptr);

		chat_comeTome = true;
		strcpy_s(chat_logs.name, recv_packet->name);
		strcpy_s(chat_logs.msg, recv_packet->msg);

		break;
	}
	case SC_MAP_OBJINFO:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_MAP_OBJINFO_PACKET* recv_packet = reinterpret_cast<SC_MAP_OBJINFO_PACKET*>(ptr);

		MapObjectsInfo temp;
		temp.m_pos = { recv_packet->center_x, recv_packet->center_y, recv_packet->center_z };
		temp.m_scale = { recv_packet->scale_x, recv_packet->scale_y, recv_packet->scale_z };
		temp.m_local_forward = { recv_packet->forward_x, recv_packet->forward_y, recv_packet->forward_z };
		temp.m_local_right = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		temp.m_local_rotate = { recv_packet->rotate_x, recv_packet->rotate_y, recv_packet->rotate_z };
		temp.m_angle_aob = recv_packet->aob;
		temp.m_angle_boc = recv_packet->boc;
		temp.setBB();
		stage1_mapobj_info.push_back(temp);

		trigger_stage1_mapinfo_load = true;
		break;
	}//SC_MAP_OBJINFO case end
	case SC_PING_RETURN:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_PING_RETURN_PACKET* recv_packet = reinterpret_cast<SC_PING_RETURN_PACKET*>(ptr);

		if (recv_packet->ping_sender_id == my_id) {
			last_pong = chrono::system_clock::now();
			//cout << "pong\n" << endl;
		}
		break;
	}//SC_PING_RETURN case end
	case SC_ACTIVE_DOWN:
	{
		if (curr_servertype != SERVER_LOGIC) break;
		SC_ACTIVE_DOWN_PACKET* recv_packet = reinterpret_cast<SC_ACTIVE_DOWN_PACKET*>(ptr);

		if (recv_packet->prev_s_id == active_servernum) {
			active_servernum = recv_packet->my_s_id;
		}
		break;
	}//SC_BULLET_COUNT case end
	//==========
	// NPC����
	case NPC_MOVE:
	{
		NPC_MOVE_PACKET* recv_packet = reinterpret_cast<NPC_MOVE_PACKET*>(ptr);

		short recv_id = recv_packet->n_id;
		npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		npcs_info[recv_id].m_ingame_state = PL_ST_MOVE_FRONT;
		npcs_info[recv_id].m_new_state_update = true;
		break;
	}
	case NPC_ROTATE:
	{
		NPC_ROTATE_PACKET* recv_packet = reinterpret_cast<NPC_ROTATE_PACKET*>(ptr);

		short recv_id = recv_packet->n_id;
		npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
		npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };

		break;
	}
	}
}

void processData(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			processPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}