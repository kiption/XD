#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <chrono>
#include <random>

#include "Constant.h"
#include "MathFuncs.h"
#include "BulletsMgr.h"
#include "../RelayServer/Protocol.h"
#include "NPC.h"
#include "Timer.h"
#include "CP_KEYS.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };
enum PLAYER_STATE { PL_ST_ALIVE, PL_ST_DEAD };
enum SESSION_TYPE { SESSION_CLIENT, SESSION_EXTENDED_SERVER, SESSION_RELAY };

Coordinate basic_coordinate;	// 기본(초기) 좌표계

chrono::system_clock::time_point g_s_start_time;	// 서버 시작시간  (단위: ms)
milliseconds g_curr_servertime;
bool b_isfirstplayer;	// 첫 player입장인지. (첫 클라 접속부터 서버시간이 흐르도록 하기 위함)
mutex servertime_lock;	// 서버시간 lock

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

class SESSION {
	OVER_EX recv_over;

public:
	mutex s_lock;
	SESSION_STATE s_state;
	int id;
	SOCKET socket;
	int remain_size;
	char name[NAME_SIZE];

	PLAYER_STATE pl_state;
	int hp;
	int bullet;
	XMFLOAT3 pos;									// Position (x, y, z)
	float pitch, yaw, roll;							// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// 현재 Look, Right, Up Vectors
	chrono::system_clock::time_point death_time;
	chrono::system_clock::time_point last_move_rotate_keyinput_time;	// 마지막으로 키 입력이 된 시간

	short curr_stage;

	BoundingOrientedBox m_xoobb;	// Bounding Box

public:
	SESSION()
	{
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		name[0] = 0;

		pl_state = PL_ST_ALIVE;
		hp = 100;
		bullet = 100;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };
		curr_stage = 1;

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void do_send(void* packet)
	{
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void send_login_info_packet();
	void send_move_packet(int client_id, short move_target);
	void send_rotate_packet(int client_id, short rotate_target);
	void send_move_rotate_packet(int client_id, short update_target);

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
};

class HA_SERVER {
	OVER_EX recv_over;

public:
	mutex s_lock;
	SESSION_STATE s_state;
	int id;
	SOCKET socket;
	int remain_size;
	chrono::system_clock::time_point heartbeat_recv_time;
	chrono::system_clock::time_point heartbeat_send_time;

public:
	HA_SERVER() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		heartbeat_recv_time = chrono::system_clock::now();
		heartbeat_send_time = chrono::system_clock::now();
	}
	~HA_SERVER() {}

	void do_recv() {
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
	void do_send(void* packet) {
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
};

int user_count = 0;
array<SESSION, MAX_USER> clients;
array<ST1_NPC, MAX_NPCS> npcs;

array<Bullets, MAX_BULLET> bullets_arr;
chrono::system_clock::time_point shoot_time;

HANDLE h_iocp;				// IOCP 핸들
SOCKET g_sc_listensock;		// 클라이언트 통신 listen소켓
SOCKET g_ss_listensock;		// 수평확장 서버 간 통신 listen 소켓
SOCKET g_relay_sock;		// 릴레이서버 간 통신 listen 소켓

SOCKET left_ex_server_sock;								// 이전 번호의 서버
SOCKET right_ex_server_sock;							// 다음 번호의 서버

int my_server_id;										// 내 서버 식별번호
bool b_active_server;									// Active 서버인가?
array<HA_SERVER, MAX_SERVER> extended_servers;			// HA구현을 위해 수평확장된 서버들
HA_SERVER relayserver;	// 릴레이서버


void SESSION::send_login_info_packet()
{
	if (curr_stage != 1) return;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
	SC_LOGIN_INFO_PACKET login_info_packet;
	login_info_packet.id = id;
	login_info_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
	login_info_packet.type = SC_LOGIN_INFO;
	login_info_packet.x = pos.x;
	login_info_packet.y = pos.y;
	login_info_packet.z = pos.z;

	login_info_packet.right_x = basic_coordinate.right.x;
	login_info_packet.right_y = basic_coordinate.right.y;
	login_info_packet.right_z = basic_coordinate.right.z;

	login_info_packet.up_x = basic_coordinate.up.x;
	login_info_packet.up_y = basic_coordinate.up.y;
	login_info_packet.up_z = basic_coordinate.up.z;

	login_info_packet.look_x = basic_coordinate.look.x;
	login_info_packet.look_y = basic_coordinate.look.y;
	login_info_packet.look_z = basic_coordinate.look.z;

	login_info_packet.hp = hp;
	login_info_packet.remain_bullet = bullet;

	do_send(&login_info_packet);
}
void SESSION::send_move_packet(int client_id, short move_target)
{
	if (curr_stage != 1) return;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
	SC_MOVE_OBJECT_PACKET move_pl_packet;
	move_pl_packet.target = move_target;
	move_pl_packet.id = client_id;
	move_pl_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
	move_pl_packet.type = SC_MOVE_OBJECT;
	move_pl_packet.x = clients[client_id].pos.x;
	move_pl_packet.y = clients[client_id].pos.y;
	move_pl_packet.z = clients[client_id].pos.z;

	do_send(&move_pl_packet);
}
void SESSION::send_rotate_packet(int client_id, short rotate_target)
{
	if (curr_stage != 1) return;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
	SC_ROTATE_OBJECT_PACKET rotate_pl_packet;
	rotate_pl_packet.target = rotate_target;
	rotate_pl_packet.id = client_id;
	rotate_pl_packet.size = sizeof(SC_ROTATE_OBJECT_PACKET);
	rotate_pl_packet.type = SC_ROTATE_OBJECT;

	rotate_pl_packet.right_x = clients[client_id].m_rightvec.x;
	rotate_pl_packet.right_y = clients[client_id].m_rightvec.y;
	rotate_pl_packet.right_z = clients[client_id].m_rightvec.z;

	rotate_pl_packet.up_x = clients[client_id].m_upvec.x;
	rotate_pl_packet.up_y = clients[client_id].m_upvec.y;
	rotate_pl_packet.up_z = clients[client_id].m_upvec.z;

	rotate_pl_packet.look_x = clients[client_id].m_lookvec.x;
	rotate_pl_packet.look_y = clients[client_id].m_lookvec.y;
	rotate_pl_packet.look_z = clients[client_id].m_lookvec.z;

	do_send(&rotate_pl_packet);
}
void SESSION::send_move_rotate_packet(int client_id, short update_target)
{
	if (curr_stage != 1) return;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
	SC_MOVE_ROTATE_OBJECT_PACKET update_pl_packet;
	update_pl_packet.target = update_target;
	update_pl_packet.id = client_id;
	update_pl_packet.size = sizeof(SC_MOVE_ROTATE_OBJECT_PACKET);
	update_pl_packet.type = SC_MOVE_ROTATE_OBJECT;

	update_pl_packet.x = clients[client_id].pos.x;
	update_pl_packet.y = clients[client_id].pos.y;
	update_pl_packet.z = clients[client_id].pos.z;

	update_pl_packet.right_x = clients[client_id].m_rightvec.x;
	update_pl_packet.right_y = clients[client_id].m_rightvec.y;
	update_pl_packet.right_z = clients[client_id].m_rightvec.z;

	update_pl_packet.up_x = clients[client_id].m_upvec.x;
	update_pl_packet.up_y = clients[client_id].m_upvec.y;
	update_pl_packet.up_z = clients[client_id].m_upvec.z;

	update_pl_packet.look_x = clients[client_id].m_lookvec.x;
	update_pl_packet.look_y = clients[client_id].m_lookvec.y;
	update_pl_packet.look_z = clients[client_id].m_lookvec.z;

	do_send(&update_pl_packet);
}

void disconnect(int target_id, int target)
{
	switch (target) {
	case SESSION_CLIENT:
		clients[target_id].s_lock.lock();
		if (clients[target_id].s_state == ST_FREE) {
			clients[target_id].s_lock.unlock();
			return;
		}
		closesocket(clients[target_id].socket);
		clients[target_id].s_state = ST_FREE;
		clients[target_id].s_lock.unlock();

		user_count--;
		cout << "Player[ID: " << clients[target_id].id << ", name: " << clients[target_id].name << " is log out\n" << endl;	// server message

		for (int i = 0; i < MAX_USER; i++) {
			auto& pl = clients[i];

			if (pl.id == target_id) continue;

			pl.s_lock.lock();
			if (pl.s_state != ST_INGAME) {
				pl.s_lock.unlock();
				continue;
			}
			if (pl.curr_stage != 1) break;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
			SC_REMOVE_OBJECT_PACKET remove_pl_packet;
			remove_pl_packet.target = TARGET_PLAYER;
			remove_pl_packet.id = target_id;
			remove_pl_packet.size = sizeof(remove_pl_packet);
			remove_pl_packet.type = SC_REMOVE_OBJECT;

			pl.do_send(&remove_pl_packet);
			pl.s_lock.unlock();
		}
		break;

	case SESSION_EXTENDED_SERVER:
		extended_servers[target_id].s_lock.lock();
		if (extended_servers[target_id].s_state == ST_FREE) {
			extended_servers[target_id].s_lock.unlock();
			return;
		}
		closesocket(extended_servers[target_id].socket);
		extended_servers[target_id].s_state = ST_FREE;
		extended_servers[target_id].s_lock.unlock();

		cout << "Server[" << extended_servers[target_id].id << "]의 다운이 감지되었습니다." << endl;	// server message

		// 서버 재실행
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + target_id);	// 십의자리: Actvie여부(S: 1, A: 0), 일의자리: 서버ID
		ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../x64/Release", SW_SHOW);

		// 클라이언트에게 Active서버가 다운되었다고 알려줌.
		if (!b_active_server) {	// 내가 Active가 아니면 상대가 Active임. (서버가 2개밖에 없기 때문)
			b_active_server = true;
			cout << "현재 Server[" << my_server_id << "] 가 Active 서버로 승격되었습니다. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// 만약 자신의 오른쪽 서버가 다운되었는데, 그 서버가 서버군의 마지막 서버인 경우 재실행된 서버에게 ConnectEx 요청을 보냅니다.
		if (target_id == MAX_SERVER - 1) {
			if (my_server_id < target_id) {
				// ConnectEx
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				int key = CP_KEY_LOGIC2EXLOGIC + MAX_SERVER - 1;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_LOGIC2EXLOGIC + HA_PORTNUM_S0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버
				inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect 재시도 중...\n" << endl;
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
		}
		break;
		
	case SESSION_RELAY:
		relayserver.s_lock.lock();
		if (relayserver.s_state == ST_FREE) {
			relayserver.s_lock.unlock();
			return;
		}
		closesocket(relayserver.socket);
		relayserver.s_state = ST_FREE;
		relayserver.s_lock.unlock();

		cout << "릴레이서버[" << relayserver.id << "]와의 연결이 끊겼습니다." << endl;	// server message
		break;
	}

}

void process_packet(int client_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* login_packet = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		clients[client_id].s_lock.lock();
		if (clients[client_id].s_state == ST_FREE) {
			clients[client_id].s_lock.unlock();
			break;
		}
		if (clients[client_id].s_state == ST_INGAME) {
			clients[client_id].s_lock.unlock();
			disconnect(client_id, SESSION_CLIENT);
			break;
		}

		// 서버구동 이후 첫번째 클라이언트의 접속이라면 그때부터 서버시간이 흐르기 시작합니다.
		if (b_isfirstplayer) {
			cout << "서버 시간이 흐르기 시작합니다.\n" << endl;
			g_s_start_time = system_clock::now();
			b_isfirstplayer = false;
		}

		// 새로 접속한 플레이어의 초기 정보를 설정합니다.
		clients[client_id].pl_state = PL_ST_ALIVE;
		clients[client_id].curr_stage = 1;

		clients[client_id].pos.x = RESPAWN_POS_X + client_id * 50;
		clients[client_id].pos.y = RESPAWN_POS_Y;
		clients[client_id].pos.z = RESPAWN_POS_Z - client_id * 50;

		clients[client_id].pitch = clients[client_id].yaw = clients[client_id].roll = 0.0f;
		clients[client_id].m_rightvec = basic_coordinate.right;
		clients[client_id].m_upvec = basic_coordinate.up;
		clients[client_id].m_lookvec = basic_coordinate.look;

		clients[client_id].setBB();

		strcpy_s(clients[client_id].name, login_packet->name);

		clients[client_id].send_login_info_packet();
		clients[client_id].s_state = ST_INGAME;

		clients[client_id].s_lock.unlock();

		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "]이(가) 접속하였습니다." << endl;	// server message

		if (!b_active_server) {
			cout << "Stand-By서버는 대기 상태를 유지합니다." << endl;
			break;	// Active서버가 아니라면, 클라이언트가 연결되었음을 사용자에게 알리기만 하고 아무일도 하지 않습니다.
		}

		cout << "새로운 오브젝트가 생성되었습니다! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ")." << endl;

		// 현재 접속해 있는 모든 클라이언트에게 새로운 클라이언트(client_id)의 정보를 전송합니다.
		for (int i = 0; i < MAX_USER; ++i) {
			auto& pl = clients[i];

			if (pl.id == client_id) continue;
			if (pl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.

			pl.s_lock.lock();
			if (pl.s_state != ST_INGAME) {
				pl.s_lock.unlock();
				continue;
			}
			SC_ADD_OBJECT_PACKET add_pl_packet;
			add_pl_packet.target = TARGET_PLAYER;
			add_pl_packet.id = client_id;
			strcpy_s(add_pl_packet.name, login_packet->name);
			add_pl_packet.size = sizeof(add_pl_packet);
			add_pl_packet.type = SC_ADD_OBJECT;

			add_pl_packet.x = clients[client_id].pos.x;
			add_pl_packet.y = clients[client_id].pos.y;
			add_pl_packet.z = clients[client_id].pos.z;

			add_pl_packet.right_x = clients[client_id].m_rightvec.x;
			add_pl_packet.right_y = clients[client_id].m_rightvec.y;
			add_pl_packet.right_z = clients[client_id].m_rightvec.z;

			add_pl_packet.up_x = clients[client_id].m_upvec.x;
			add_pl_packet.up_y = clients[client_id].m_upvec.y;
			add_pl_packet.up_z = clients[client_id].m_upvec.z;

			add_pl_packet.look_x = clients[client_id].m_lookvec.x;
			add_pl_packet.look_y = clients[client_id].m_lookvec.y;
			add_pl_packet.look_z = clients[client_id].m_lookvec.z;

			pl.do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}

		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 클라이언트의 정보를 전송합니다.
		for (auto& pl : clients) {
			if (pl.id == client_id) continue;
			if (pl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.

			pl.s_lock.lock();
			if (pl.s_state != ST_INGAME) {
				pl.s_lock.unlock();
				continue;
			}

			SC_ADD_OBJECT_PACKET add_pl_packet;
			add_pl_packet.target = TARGET_PLAYER;
			add_pl_packet.id = pl.id;
			strcpy_s(add_pl_packet.name, pl.name);
			add_pl_packet.size = sizeof(add_pl_packet);
			add_pl_packet.type = SC_ADD_OBJECT;

			add_pl_packet.x = pl.pos.x;
			add_pl_packet.y = pl.pos.y;
			add_pl_packet.z = pl.pos.z;

			add_pl_packet.right_x = pl.m_rightvec.x;
			add_pl_packet.right_y = pl.m_rightvec.y;
			add_pl_packet.right_z = pl.m_rightvec.z;

			add_pl_packet.up_x = pl.m_upvec.x;
			add_pl_packet.up_y = pl.m_upvec.y;
			add_pl_packet.up_z = pl.m_upvec.z;

			add_pl_packet.look_x = pl.m_lookvec.x;
			add_pl_packet.look_y = pl.m_lookvec.y;
			add_pl_packet.look_z = pl.m_lookvec.z;

			clients[client_id].do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}

		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 NPC의 정보를 전송합니다.
		for (int i = 0; i < MAX_NPCS; i++) {
			SC_ADD_OBJECT_PACKET add_npc_packet;
			add_npc_packet.type = SC_ADD_OBJECT;
			add_npc_packet.size = sizeof(SC_ADD_OBJECT_PACKET);

			add_npc_packet.id = i;
			add_npc_packet.target = TARGET_NPC;
			add_npc_packet.x = npcs[i].GetPosition().x;
			add_npc_packet.y = npcs[i].GetPosition().y;
			add_npc_packet.z = npcs[i].GetPosition().z;

			add_npc_packet.right_x = npcs[i].GetCurr_coordinate().right.x;
			add_npc_packet.right_y = npcs[i].GetCurr_coordinate().right.y;
			add_npc_packet.right_z = npcs[i].GetCurr_coordinate().right.z;

			add_npc_packet.up_x = npcs[i].GetCurr_coordinate().up.x;
			add_npc_packet.up_y = npcs[i].GetCurr_coordinate().up.y;
			add_npc_packet.up_z = npcs[i].GetCurr_coordinate().up.z;

			add_npc_packet.look_x = npcs[i].GetCurr_coordinate().look.x;
			add_npc_packet.look_y = npcs[i].GetCurr_coordinate().look.y;
			add_npc_packet.look_z = npcs[i].GetCurr_coordinate().look.z;

			for (int j = 0; j < MAX_USER; j++) {
				if (clients[j].curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
				if (clients[j].s_state == ST_INGAME) {
					clients[j].do_send(&add_npc_packet);
				}
			}
		}
		break;
	}// CS_LOGIN end
	case CS_MOVE:
	{
		if (!b_active_server) break;
		CS_MOVE_PACKET* cl_move_packet = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		// 1. 충돌체크를 한다.

		// 2. 플레이어가 이동할 수 없는 곳으로 이동했다면 RollBack패킷을 보내 이전 위치로 돌아가도록 한다.

		// 3. 그게 아니라면 세션 정보를 업데이트 한다.
		clients[client_id].s_lock.lock();
		clients[client_id].pos = { cl_move_packet->x, cl_move_packet->y, cl_move_packet->z };
		clients[client_id].s_lock.unlock();
		
		// 4. 다른 클라이언트에게 플레이어가 이동한 위치를 알려준다.
		for (auto& other_pl : clients) {
			if (other_pl.id == client_id) continue;
			if (other_pl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ other_pl.s_lock };
			other_pl.send_move_packet(client_id, TARGET_PLAYER);
		}
		break;
	}// CS_MOVE end
	case CS_ROTATE:
	{
		if (!b_active_server) break;
		CS_ROTATE_PACKET* cl_rotate_packet = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		// 1. 충돌체크를 한다.

		// 2. 플레이어가 이동할 수 없는 곳으로 이동했다면 RollBack패킷을 보내 이전 위치로 돌아가도록 한다.

		// 3. 그게 아니라면 세션 정보를 업데이트 한다.
		clients[client_id].s_lock.lock();
		clients[client_id].m_rightvec = { cl_rotate_packet->right_x, cl_rotate_packet->right_y, cl_rotate_packet->right_z };
		clients[client_id].m_upvec = { cl_rotate_packet->up_x, cl_rotate_packet->up_y, cl_rotate_packet->up_z };
		clients[client_id].m_lookvec = { cl_rotate_packet->look_x, cl_rotate_packet->look_y, cl_rotate_packet->look_z };
		clients[client_id].s_lock.unlock();
		
		// 4. 다른 클라이언트에게 플레이어가 회전한 방향을 알려준다.
		for (auto& other_pl : clients) {
			if (other_pl.id == client_id) continue;
			if (other_pl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ other_pl.s_lock };
			other_pl.send_rotate_packet(client_id, TARGET_PLAYER);
		}
		break;
	}// CS_ROTATE end
	case CS_ATTACK:
	{
		if (!b_active_server) break;
		CS_ATTACK_PACKET* cl_attack_packet = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		// Bullet Cooldown Check
		milliseconds shoot_term = duration_cast<milliseconds>(chrono::system_clock::now() - shoot_time);
		if (shoot_term < milliseconds(SHOOT_COOLDOWN_BULLET)) {	// 쿨타임이 끝나지 않았다면 발사하지 않습니다.
			milliseconds left_cooldown = duration_cast<milliseconds>(milliseconds(SHOOT_COOLDOWN_BULLET) - shoot_term);
			break;
		}
		cout << "발싸\n" << endl;

		// empty space check
		int new_bullet_id = -1;
		int arr_cnt = -1;
		if (clients[client_id].bullet > 0) {		// 남은 총알이 있다면,
			// 총알 하나 사용
			clients[client_id].s_lock.lock();
			clients[client_id].bullet -= 1;
			clients[client_id].s_lock.unlock();

			// 발사한 사용자에게 총알 사용했음을 알려줍니다.
			SC_BULLET_COUNT_PACKET bullet_packet;
			bullet_packet.size = sizeof(bullet_packet);
			bullet_packet.type = SC_BULLET_COUNT;
			bullet_packet.id = client_id;
			bullet_packet.bullet_cnt = clients[client_id].bullet;
			clients[client_id].do_send(&bullet_packet);
		}
		else {	// 남은 탄환이 0이라면 reload
			clients[client_id].s_lock.lock();
			clients[client_id].bullet = 100;
			clients[client_id].s_lock.unlock();

			// 발사한 사용자에게 총알 장전했음을 알려줍니다.
			SC_BULLET_COUNT_PACKET bullet_packet;
			bullet_packet.size = sizeof(bullet_packet);
			bullet_packet.type = SC_BULLET_COUNT;
			bullet_packet.id = client_id;
			bullet_packet.bullet_cnt = clients[client_id].bullet;
			clients[client_id].do_send(&bullet_packet);
		}
		break;
	}// CS_ATTACK end
	case CS_INPUT_KEYBOARD:
	{
		if (!b_active_server) break;		// Active 서버만 패킷을 처리합니다.
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		float sign = 1.0f;					// right/up/look벡터 방향으로 움직이는지, 반대 방향으로 움직이는지
		switch (inputkey_p->keytype) {
		case PACKET_KEY_NUM1:
			clients[client_id].s_lock.lock();
			clients[client_id].curr_stage = 1;
			clients[client_id].s_lock.unlock();
			cout << "Client[" << client_id << "] Stage1로 전환." << endl;//test
			break;
		case PACKET_KEY_NUM2:
			clients[client_id].s_lock.lock();
			clients[client_id].curr_stage = 2;
			clients[client_id].s_lock.unlock();
			cout << "Client[" << client_id << "] Stage2로 전환." << endl;//test
			break;
		}

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE: {
		if (!b_active_server) break;		// Active 서버만 패킷을 처리합니다.
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->buttontype == PACKET_BUTTON_L) {			// 마우스 좌클릭
			// 1스테이지 로직
			if (clients[client_id].curr_stage == 1) {
				float rotate_scalar = 0.0f;

				clients[client_id].s_lock.lock();

				// 클라이언트에서 받은 벡터값으로 세션 정보의 벡터들을 업데이트합니다.

				clients[client_id].s_lock.unlock();

				// 작동 중인 다른 클라이언트에게 회전 결과를 알려줍니다.
				for (auto& other_pl : clients) {
					if (other_pl.id == client_id) continue;
					if (other_pl.s_state != ST_INGAME) continue;

					lock_guard<mutex> lg{ other_pl.s_lock };
					other_pl.send_rotate_packet(client_id, TARGET_PLAYER);
				}
			}
			// 2스테이지 로직
			else if (clients[client_id].curr_stage == 2) {
				// 미구현
			}

		}
		else if (rt_p->buttontype == PACKET_BUTTON_R) {		// 마우스 우클릭 드래그: 기능 미정.
			// 1스테이지 로직
			if (clients[client_id].curr_stage == 1) {

			}
			// 2스테이지 로직
			else if (clients[client_id].curr_stage == 2) {

			}
		}

		break;
	}// CS_INPUT_MOUSE end
	case CS_RELOGIN:
	{
		CS_RELOGIN_PACKET* re_login_pack = reinterpret_cast<CS_RELOGIN_PACKET*>(packet);

		int re_login_id = re_login_pack->id;
		clients[re_login_id].s_lock.lock();
		clients[re_login_id].s_state = ST_INGAME;
		clients[re_login_id].setBB();
		clients[re_login_id].s_lock.unlock();
		//cout << "TEST: clients[" << re_login_id << "] is log-in again." << endl;
		//cout << "===================================" << endl;
		//cout << "Pos: " << clients[re_login_id].pos.x << ", " << clients[re_login_id].pos.y << ", " << clients[re_login_id].pos.z << endl;
		//cout << "LookVec: " << clients[re_login_id].m_lookvec.x << ", " << clients[re_login_id].m_lookvec.y << ", " << clients[re_login_id].m_lookvec.z << endl;
		//cout << "Pitch: " << clients[re_login_id].pitch << "/ Yaw: " << clients[re_login_id].yaw << "/ Roll: " << clients[re_login_id].roll << endl;
		//cout << "===================================" << endl;

		break;
	}// CS_RELOGIN end
	case SS_HEARTBEAT:
	{
		SS_HEARTBEAT_PACKET* heartbeat_pack = reinterpret_cast<SS_HEARTBEAT_PACKET*>(packet);
		int recv_id = heartbeat_pack->sender_id;

		extended_servers[recv_id].heartbeat_recv_time = chrono::system_clock::now();
		if (recv_id < my_server_id) {	// A->B->A로 heartbeat의 한 사이클이 끝나도록하기 위함. (즉, 오른쪽 서버로부터 Heartbeat를 받으면 한 사이클의 끝으로 판단)
			// Heartbeat를 먼저 보낸 서버에게 자신의 Heartbeat를 전송합니다.
			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[recv_id].do_send(&hb_packet);										// 자신에게 Heartbeat를 보낸 서버에게 전송합니다.
			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// 전송한 시간을 업데이트
		}
		break;
	}// SS_HEARTBEAT end
	case SS_DATA_REPLICA:
	{
		SS_DATA_REPLICA_PACKET* replica_pack = reinterpret_cast<SS_DATA_REPLICA_PACKET*>(packet);

		int replica_id = replica_pack->id;
		clients[replica_id].s_lock.lock();
		clients[replica_id].id = replica_id;
		clients[replica_id].hp = replica_pack->hp;
		clients[replica_id].bullet = replica_pack->bullet_cnt;

		strcpy_s(clients[replica_id].name, replica_pack->name);

		clients[replica_id].pos = { replica_pack->x, replica_pack->y, replica_pack->z };

		clients[replica_id].pitch = replica_pack->pitch;
		clients[replica_id].yaw = replica_pack->yaw;
		clients[replica_id].roll = replica_pack->roll;

		clients[replica_id].m_rightvec = { replica_pack->right_x, replica_pack->right_y, replica_pack->right_z };
		clients[replica_id].m_upvec = { replica_pack->up_x, replica_pack->up_y, replica_pack->up_z };
		clients[replica_id].m_lookvec = { replica_pack->look_x, replica_pack->look_y, replica_pack->look_z };
		clients[replica_id].s_lock.unlock();

		//cout << "Client[" << replica_id << "]의 데이터가 복제되었습니다." << endl;
		//cout << "===================================" << endl;
		//cout << "Name: " << clients[replica_id].name << endl;
		//cout << "Pos: " << clients[replica_id].pos.x << ", " << clients[replica_id].pos.y << ", " << clients[replica_id].pos.z << endl;
		//cout << "LookVec: " << clients[replica_id].m_lookvec.x << ", " << clients[replica_id].m_lookvec.y << ", " << clients[replica_id].m_lookvec.z << endl;
		//cout << "Pitch: " << clients[replica_id].pitch << "/ Yaw: " << clients[replica_id].yaw << "/ Roll: " << clients[replica_id].roll << endl;
		//cout << "===================================\n" << endl;

	}// SS_DATA_REPLICA end
	}
}


int get_new_client_id()	// clients의 비어있는 칸을 찾아서 새로운 client의 아이디를 할당해주는 함수
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].s_lock.lock();
		if (clients[i].s_state == ST_FREE) {
			clients[i].s_state = ST_ACCEPTED;
			clients[i].s_lock.unlock();
			user_count++;
			return i;
		}
		clients[i].s_lock.unlock();
	}
	return -1;
}
int find_empty_extended_server() {	// ex_servers의 비어있는 칸을 찾아서 새로운 Server_ex의 아이디를 할당해주는 함수
	for (int i = 0; i < MAX_USER; ++i) {
		extended_servers[i].s_lock.lock();
		if (extended_servers[i].s_state == ST_FREE) {
			extended_servers[i].s_state = ST_ACCEPTED;
			extended_servers[i].heartbeat_recv_time = chrono::system_clock::now();
			extended_servers[i].heartbeat_send_time = chrono::system_clock::now();
			extended_servers[i].s_lock.unlock();
			return i;
		}
		extended_servers[i].s_lock.unlock();
	}
	return -1;
}
void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EX* ex_over = reinterpret_cast<OVER_EX*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_ACCEPT) {
				cout << "Accept Error";
			}
			else if (ex_over->process_type == OP_CONNECT) {
				cout << "Connect Error" << endl;

				// 비동기Conn를 다시 시도합니다.
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_LOGIC2EXLOGIC + HA_PORTNUM_S0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);
				inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect 재시도 중...\n" << endl;
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
			else {
				// 1. Client Error
				if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key - CP_KEY_LOGIC2CLIENT), SESSION_CLIENT);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				// 2. RelayServer Error
				else if (key >= CP_KEY_LOGIC2RELAY && key < CP_KEY_LOGIC2EXLOGIC) {

				}
				// 3. Ex_Server Error
				else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
					//cout << WSAGetLastError() << endl;
					disconnect(static_cast<int>(key - CP_KEY_LOGIC2EXLOGIC), SESSION_EXTENDED_SERVER);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			// 1. Client Accept
			if (key == CP_KEY_LISTEN_CLIENT) {
				SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				int client_id = get_new_client_id();
				if (client_id != -1) {
					// 클라이언트 id, 소켓
					clients[client_id].s_lock.lock();
					clients[client_id].id = client_id;
					clients[client_id].remain_size = 0;
					clients[client_id].socket = c_socket;
					clients[client_id].s_lock.unlock();
					int new_key = client_id + CP_KEY_LOGIC2CLIENT;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, new_key, 0);
					clients[client_id].do_recv();
					c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "Sever is Full" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_sc_listensock, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			// 2. Ex_Server Accept
			else if (key == CP_KEY_LISTEN_EXLOGIC) {
				SOCKET extended_server_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				left_ex_server_sock = extended_server_socket;
				int new_id = find_empty_extended_server();
				if (new_id != -1) {
					//cout << "Sever[" << new_id << "]의 연결요청을 받았습니다.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_LOGIC2EXLOGIC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_iocp, new_key, 0);
					extended_servers[new_id].do_recv();
					extended_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "다른 Sever의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(extended_server_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_ss_listensock, extended_server_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}

			break;
		}//OP_ACPT end
		case OP_RECV: {
			// 1. Client Recv
			if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
				int recved_id = key - CP_KEY_LOGIC2CLIENT;
				if (0 == num_bytes) disconnect(recved_id, SESSION_CLIENT);

				int remain_data = num_bytes + clients[recved_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(recved_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				clients[recved_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				clients[recved_id].do_recv();
			}
			// 2. RelayServer Recv
			else if (key >= CP_KEY_LOGIC2RELAY && key < CP_KEY_LOGIC2EXLOGIC) {

			}
			// 3. Ex_Server Recv
			else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				if (0 == num_bytes) disconnect(key, SESSION_EXTENDED_SERVER);
				int server_id = key - CP_KEY_LOGIC2EXLOGIC;

				int remain_data = num_bytes + extended_servers[server_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(server_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				extended_servers[server_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				extended_servers[server_id].do_recv();
				break;
			}

			break;
		}//OP_RECV end
		case OP_SEND: {
			// 1. Client Send
			if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
				if (0 == num_bytes) disconnect(key - CP_KEY_LOGIC2CLIENT, SESSION_CLIENT);
				delete ex_over;
			}
			// 2. RelayServer Send
			else if (key >= CP_KEY_LOGIC2RELAY && key < CP_KEY_LOGIC2EXLOGIC) {

			}
			// 3. Ex_Server Send
			else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				int server_id = key - CP_KEY_LOGIC2EXLOGIC;
				if (0 == num_bytes) disconnect(server_id, SESSION_EXTENDED_SERVER);
				delete ex_over;
			}

			break;
		}//OP_SEND end
		case OP_CONNECT: {
			// 1. Ex_Server Conn_Ex
			if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				if (FALSE != ret) {
					int server_id = key - CP_KEY_LOGIC2EXLOGIC;
					std::cout << "성공적으로 Server[" << server_id << "]에 연결되었습니다.\n" << endl;
					extended_servers[server_id].id = server_id;
					extended_servers[server_id].remain_size = 0;
					extended_servers[server_id].socket = right_ex_server_sock;
					extended_servers[server_id].s_state = ST_ACCEPTED;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, NULL, 0);
					delete ex_over;
					extended_servers[server_id].do_recv();
				}
			}
		}//OP_CONN end
		}
	}
}


void timerFunc() {
	while (true) {
		auto start_t = system_clock::now();
		// ================================
		// 서버 시간 업데이트
		if (b_active_server && !b_isfirstplayer) {
			servertime_lock.lock();
			g_curr_servertime = duration_cast<milliseconds>(start_t - g_s_start_time);
			servertime_lock.unlock();
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
				SC_TIME_TICKING_PACKET ticking_packet;
				ticking_packet.size = sizeof(SC_TIME_TICKING_PACKET);
				ticking_packet.type = SC_TIME_TICKING;
				int left_time = STAGE1_TIMELIMIT * 1000 - static_cast<int>(g_curr_servertime.count());
				if (left_time < 0) left_time = 0;
				ticking_packet.servertime_ms = left_time;
				cl.do_send(&ticking_packet);
			}
		}
		// ================================
		// Bullet
		for (auto& bullet : bullets_arr) {
			if (bullet.getId() == -1) continue;

			if (bullet.calcDistance(bullet.getInitialPos()) > BULLET_RANGE) {	// 만약 총알이 초기 위치로부터 멀리 떨어졌다면 제거합니다.
				// 객체 제거패킷을 모든 클라이언트에게 보냅니다.
				SC_REMOVE_OBJECT_PACKET remove_bullet_packet;
				remove_bullet_packet.target = TARGET_BULLET;
				remove_bullet_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
				remove_bullet_packet.id = bullet.getId();
				remove_bullet_packet.type = SC_REMOVE_OBJECT;

				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
					cl.do_send(&remove_bullet_packet);
				}

				bullet.clear();
			}
			else {
				bullet.m_objlock.lock();
				bullet.moveObj(bullet.getLookvector(), BULLET_MOVE_SCALAR);		// 총알을 앞으로 이동시킵니다.
				bullet.setBB_ex(XMFLOAT3(VULCAN_BULLET_BBSIZE_X, VULCAN_BULLET_BBSIZE_Y, VULCAN_BULLET_BBSIZE_Z));	// 바운딩박스 업데이트
				bullet.m_objlock.unlock();

				bool bullet_alive = true;	// 총알이 충돌하지 않고 잘 살아있는지

				// 충돌검사
				// =================
				// 1. Bullet-Player
				for (auto& pl : clients) {
					if (bullet.getOwner() == pl.id) continue;							// 총을 쏜 사람은 충돌체크 X
					if (bullet.calcDistance(pl.pos) > BULLET_RANGE)	continue;			// 총알 사거리보다 멀리 떨어진 플레이어는 충돌체크 X

					if (bullet.intersectsCheck(pl.m_xoobb)) {
						// 1) 총알 객체 제거
						bullet.m_objlock.lock();
						bullet.clear();
						bullet_alive = false;
						bullet.m_objlock.unlock();

						SC_REMOVE_OBJECT_PACKET remove_bullet_packet;
						remove_bullet_packet.target = TARGET_BULLET;
						remove_bullet_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
						remove_bullet_packet.id = bullet.getId();
						remove_bullet_packet.type = SC_REMOVE_OBJECT;
						for (auto& cl : clients) {
							if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
							if (cl.s_state == ST_INGAME)
								cl.do_send(&remove_bullet_packet);
						}

						// 2) 플레이어의 충돌 후처리
						pl.s_lock.lock();
						pl.hp -= BULLET_DAMAGE;
						if (pl.hp <= 0) {
							pl.hp = 0;
							pl.pl_state = PL_ST_DEAD;
							pl.death_time = chrono::system_clock::now();
						}
						pl.s_lock.unlock();

						SC_DAMAGED_PACKET damaged_by_bullet_packet;
						damaged_by_bullet_packet.size = sizeof(SC_DAMAGED_PACKET);
						damaged_by_bullet_packet.type = SC_DAMAGED;
						damaged_by_bullet_packet.target = TARGET_PLAYER;
						damaged_by_bullet_packet.id = pl.id;
						damaged_by_bullet_packet.dec_hp = BULLET_DAMAGE;
						if (pl.curr_stage == 1) {// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
							pl.do_send(&damaged_by_bullet_packet);
						}

					}//bullet.intersectsCheck end
				}//for(auto& pl:clients) end
				// =================
				// 2. Bullet-NPC
				for (auto& npc_obj : npcs) {
					// 총알 사거리보다 멀리 떨어진 플레이어는 충돌체크 X
					if (bullet.calcDistance(npc_obj.GetPosition()) > BULLET_RANGE)	continue;	

					// 충돌검사.
					if (bullet.intersectsCheck(npc_obj.m_xoobb)) {
						cout << "Bullet is Collide with NPC[" << npc_obj.GetID() << "]!" << endl;//test
						// 1) 총알 객체 제거
						bullet.m_objlock.lock();
						bullet.clear();
						bullet_alive = false;
						bullet.m_objlock.unlock();

						SC_REMOVE_OBJECT_PACKET remove_bullet_packet;
						remove_bullet_packet.target = TARGET_BULLET;
						remove_bullet_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
						remove_bullet_packet.id = bullet.getId();
						remove_bullet_packet.type = SC_REMOVE_OBJECT;
						for (auto& cl : clients) {
							if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
							if (cl.s_state != ST_INGAME) continue;
							cl.do_send(&remove_bullet_packet);
						}

						// 2) NPC 충돌 후처리
						npc_obj.ST1_Damege_Calc(bullet.getOwner());

						// NPC가 사망하면 객체(npc)제거패킷을 접속중인 모든 클라이언트에게 보냅니다.
						if (npc_obj.GetState() == NPC_DEATH) {
							SC_REMOVE_OBJECT_PACKET rm_npc_packet;
							rm_npc_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
							rm_npc_packet.type = SC_REMOVE_OBJECT;
							rm_npc_packet.target = TARGET_NPC;
							rm_npc_packet.id = npc_obj.GetID();
							for (auto& cl : clients) {
								if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
								if (cl.pl_state != ST_INGAME) continue;
								cl.do_send(&rm_npc_packet);
							}
						}
						else {
							// 접속중인 모든 플레이어에게 npc의 HP감소 사실을 알립니다.
							SC_DAMAGED_PACKET npc_damaged_by_bullet_packet;
							npc_damaged_by_bullet_packet.size = sizeof(SC_DAMAGED_PACKET);
							npc_damaged_by_bullet_packet.target = TARGET_PLAYER;
							npc_damaged_by_bullet_packet.id = npc_obj.GetID();
							npc_damaged_by_bullet_packet.type = SC_DAMAGED;
							npc_damaged_by_bullet_packet.dec_hp = BULLET_DAMAGE;

							for (auto& cl : clients) {
								if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
								if (cl.pl_state != ST_INGAME) continue;
								cl.do_send(&npc_damaged_by_bullet_packet);
							}
						}

					}//bullet.intersectsCheck end
				}//for(auto& npc:npcs) end

				// 최종적으로 총알이 어디에도 충돌하지 않았을 때에만 총알의 이동패킷을 보냅니다.
				if (bullet_alive) {
					SC_MOVE_OBJECT_PACKET move_bullet_packet;
					move_bullet_packet.target = TARGET_BULLET;
					move_bullet_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
					move_bullet_packet.id = bullet.getId();
					move_bullet_packet.type = SC_MOVE_OBJECT;
					move_bullet_packet.x = bullet.getPos().x;
					move_bullet_packet.y = bullet.getPos().y;
					move_bullet_packet.z = bullet.getPos().z;

					for (auto& cl : clients) {
						if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
						if (cl.s_state == ST_INGAME)
							cl.do_send(&move_bullet_packet);
					}
				}
			}
		}//for (auto& bullet : bullets_arr) end

		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}

void heartBeatFunc() {	// Heartbeat관련 스레드 함수
	while (true) {
		auto start_t = system_clock::now();
		// 오른쪽 서버로 Heartbeat를 보냅니다. (왼쪽 서버가 오른쪽 서버로 전송하기 때문에 가장 마지막 서버는 보내지 않습니다.)
		if (my_server_id != MAX_SERVER - 1) {
			if (extended_servers[my_server_id].s_state != ST_ACCEPTED)	continue;
			if (extended_servers[my_server_id + 1].s_state != ST_ACCEPTED) continue;

			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[my_server_id + 1].do_send(&hb_packet);	// 오른쪽 서버에 전송합니다.

			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// 전송한 시간을 업데이트
		}

		// 오랫동안 Heartbeat를 받지 못한 서버구성원이 있는지 확인합니다.
		// 1. 오른쪽 서버 검사	(가장 왼쪽에 있는 서버 구성원은 검사하지 않습니다.)
		if (my_server_id != 0) {
			if (extended_servers[my_server_id - 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id - 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id - 1 << "]에게 Heartbeat를 오랫동안 받지 못했습니다. 서버 다운으로 간주합니다." << endl;
					disconnect(my_server_id - 1, SESSION_EXTENDED_SERVER);
				}
			}
		}
		// 2. 왼쪽 서버 검사 (가장 오른쪽에 있는 서버 구성원은 검사하지 않습니다.)
		if (my_server_id != MAX_SERVER - 1) {
			if (extended_servers[my_server_id + 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id + 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id + 1 << "]에게 Heartbeat를 오랫동안 받지 못했습니다. 서버 다운으로 간주합니다." << endl;
					disconnect(my_server_id + 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		// 스레드 대기
		auto curr_t = system_clock::now();
		if (curr_t - start_t < static_cast<milliseconds>(HB_SEND_CYCLE)) {
			this_thread::sleep_for(static_cast<milliseconds>(HB_SEND_CYCLE) - (curr_t - start_t));
		}
	}
}
void replicaSessions() {	// 서버간 세션데이터를 복제하는 함수
	while (true) {
		if (!b_active_server) continue;

		auto start_t = system_clock::now();

		int standby_id = -1;
		if (my_server_id == 0)		standby_id = 1;
		else if (my_server_id == 1) standby_id = 0;

		if (extended_servers[standby_id].s_state == ST_ACCEPTED) {
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;

				SS_DATA_REPLICA_PACKET replica_pack;
				replica_pack.type = SS_DATA_REPLICA;
				replica_pack.size = sizeof(SS_DATA_REPLICA_PACKET);

				replica_pack.target = TARGET_PLAYER;
				replica_pack.id = cl.id;
				strcpy_s(replica_pack.name, cl.name);
				replica_pack.hp = cl.hp;
				replica_pack.bullet_cnt = cl.bullet;

				replica_pack.x = cl.pos.x;
				replica_pack.y = cl.pos.y;
				replica_pack.z = cl.pos.z;

				replica_pack.roll = cl.roll;
				replica_pack.yaw = cl.yaw;
				replica_pack.pitch = cl.pitch;

				replica_pack.right_x = cl.m_rightvec.x;
				replica_pack.right_y = cl.m_rightvec.y;
				replica_pack.right_z = cl.m_rightvec.z;

				replica_pack.up_x = cl.m_upvec.x;
				replica_pack.up_y = cl.m_upvec.y;
				replica_pack.up_z = cl.m_upvec.z;

				replica_pack.look_x = cl.m_lookvec.x;
				replica_pack.look_y = cl.m_lookvec.y;
				replica_pack.look_z = cl.m_lookvec.z;

				extended_servers[standby_id].do_send(&replica_pack);

				//cout << "TEST: Client[" << cl.id << "]의 정보를 Sever[" << standby_id << "]에게 전달합니다." << endl;
			}
		}

		auto curr_t = system_clock::now();
		if (curr_t - start_t < static_cast<milliseconds>(HA_REPLICA_CYCLE)) {
			this_thread::sleep_for(static_cast<milliseconds>(HA_REPLICA_CYCLE) - (curr_t - start_t));
		}
	}
}


void init_npc()
{
	for (int i{}; i < MAX_NPCS; i++) {
		int npc_id = i;
		npcs[i].SetID(npc_id);
		npcs[i].SetNpcType(NPC_Helicopter);
		npcs[i].SetRotate(0.0f, 0.0f, 0.0f);
		//npcs[i].SetActive(false);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float>AirHigh(50, 270);
		uniform_real_distribution<float>AirPos(600, 800);

		npcs[i].SetPosition(AirPos(dre), AirHigh(dre), AirPos(dre));
		npcs[i].SetOrgPosition(npcs[i].GetPosition());

		uniform_real_distribution<float>rTheta(0.6f, 1.0f);
		npcs[i].SetTheta(rTheta(dre));
		npcs[i].SetAcc(npcs[i].GetTheta());

		uniform_int_distribution<int>rRange(100, 200);
		npcs[i].SetRange(rRange(dre));
	}
}

void MoveNPC()
{
	auto start_t = system_clock::now();
	while (true) {
		auto curr_t = system_clock::now();
		if (curr_t - start_t < 500ms)
			this_thread::sleep_for(500ms - (curr_t - start_t));
		start_t = curr_t;

		for (int i = 0; i < MAX_NPCS; ++i) {
			// 클라이언트들과 NPC 사이의 거리 계산
			for (auto& cl : clients) {
				npcs[i].Caculation_Distance(cl.pos, cl.id);
			}
			//cout << i << "번째 Status - " << npcs[i].GetState() << endl;

			// NPC가 추적하려는 아이디가 있는지부터 확인, 있으면 추적 대상 플레이어 좌표를 임시 저장
			if (npcs[i].GetChaseID() != -1) {
				npcs[i].SetUser_Pos(clients[npcs[i].GetChaseID()].pos, npcs[i].GetChaseID());
			}

			// 상태마다 다른 움직임을 하는 매니지먼트
			npcs[i].ST1_State_Manegement(npcs[i].GetState());
			
			// Send Move&Rotate Packet
			SC_MOVE_ROTATE_OBJECT_PACKET npc_update_packet;
			npc_update_packet.size = sizeof(SC_MOVE_ROTATE_OBJECT_PACKET);
			npc_update_packet.type = SC_MOVE_ROTATE_OBJECT;

			npc_update_packet.target = TARGET_NPC;
			npc_update_packet.id = npcs[i].GetID();

			npc_update_packet.x = npcs[i].GetPosition().x;
			npc_update_packet.y = npcs[i].GetPosition().y;
			npc_update_packet.z = npcs[i].GetPosition().z;

			npc_update_packet.right_x = npcs[i].GetCurr_coordinate().right.x;
			npc_update_packet.right_y = npcs[i].GetCurr_coordinate().right.y;
			npc_update_packet.right_z = npcs[i].GetCurr_coordinate().right.z;

			npc_update_packet.up_x = npcs[i].GetCurr_coordinate().up.x;
			npc_update_packet.up_y = npcs[i].GetCurr_coordinate().up.y;
			npc_update_packet.up_z = npcs[i].GetCurr_coordinate().up.z;

			npc_update_packet.look_x = npcs[i].GetCurr_coordinate().look.x;
			npc_update_packet.look_y = npcs[i].GetCurr_coordinate().look.y;
			npc_update_packet.look_z = npcs[i].GetCurr_coordinate().look.z;

			for (auto& send_target : clients) {
				if (send_target.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
				if (send_target.s_state != ST_INGAME) continue;

				lock_guard<mutex> lg{ send_target.s_lock };
				send_target.do_send(&npc_update_packet);
			}
		}
	}
}


int main(int argc, char* argv[])
{
	b_isfirstplayer = true;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	// [ HA - 서버 ID, 포트번호 지정 ]
	// 어떤 서버를 가동할 것인지를 명령행 인수로 입력받아서 그 서버에 포트 번호를 부여합니다.
	my_server_id = 0;		// 서버 구분을 위한 지정번호
	int sc_portnum = -1;	// 클라이언트 통신용 포트번호
	int ss_portnum = -1;	// 서버 간 통신용 포트번호
	if (argc > 1) {			// 입력된 명령행 인수가 있을 때 (주로 서버다운으로 인한 서버 재실행때 사용됨)
		// Serve ID지정
		my_server_id = atoi(argv[1]) % 10;

		// Active 여부 결정
		if (my_server_id == MAX_SERVER - 1) {
			b_active_server = true;
		}
		else {
			b_active_server = false;
		}
	}
	else {				// 만약 입력된 명령행 인수가 없다면 입력을 받습니다.
		cout << "실행할 서버: ";
		cin >> my_server_id;

		// Active 여부 결정
		switch (my_server_id) {
		case 0:	// 0번 서버
			b_active_server = false;
			break;
		case 1:	// 1번 서버
			b_active_server = true;
			break;
		default:
			cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
			return 0;
		}
	}

	// 서버번호에 따라 포트번호를 지정해줍니다.
	switch (my_server_id) {
	case 0:	// 0번 서버
		sc_portnum = PORT_NUM_S0;
		ss_portnum = HA_PORTNUM_S0;
		break;
	case 1:	// 1번 서버
		sc_portnum = PORT_NUM_S1;
		ss_portnum = HA_PORTNUM_S1;
		break;
	default:
		cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
		return 0;
	}
	cout << "Server[" << my_server_id << "] 가 가동되었습니다. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else cout << "Stand-By";
	cout << " / S - C PORT : " << sc_portnum << " / S - S PORT : " << ss_portnum << " ]" << endl;

	//======================================================================
	// [ HA - Relay서버 연결 ]
	int active_relay_serverid = 0;							// [[[추후에 현재 Active 상태에 있는 릴레이서버의 id와 포트번호로 바꿀 예정임!]]]
	int active_relay_portnum = PORTNUM_RELAY2LOGIC_0;		//

	// Relay Connect
	// Relay서버가 Logic서버를 실행시켜주기 때문에, 비동기Connect를 할 필요가 없음.
	// ConnectEx test
	SOCKET temp_relays = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	GUID guid_relay = WSAID_CONNECTEX;
	DWORD bytes_relay = 0;
	LPFN_CONNECTEX connectExFP_relay;
	::WSAIoctl(temp_relays, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid_relay, sizeof(guid_relay), &connectExFP_relay, sizeof(connectExFP_relay), &bytes_relay, nullptr, nullptr);
	closesocket(temp_relays);

	SOCKADDR_IN relaysvr_addr;
	ZeroMemory(&relaysvr_addr, sizeof(relaysvr_addr));
	relaysvr_addr.sin_family = AF_INET;
	g_relay_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
	int ret = ::bind(g_relay_sock, reinterpret_cast<LPSOCKADDR>(&relaysvr_addr), sizeof(relaysvr_addr));
	if (ret != 0) {
		cout << "Bind Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	OVER_EX* con_over_relay = new OVER_EX;
	con_over_relay->process_type = OP_CONNECT;
	HANDLE hret_relay = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_relay_sock), h_iocp, CP_KEY_LOGIC2RELAY, 0);
	if (NULL == hret_relay) {
		cout << "CreateIoCompletoinPort Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	ZeroMemory(&relaysvr_addr, sizeof(relaysvr_addr));
	relaysvr_addr.sin_family = AF_INET;
	relaysvr_addr.sin_port = htons(PORTNUM_RELAY2LOGIC_0);
	inet_pton(AF_INET, "127.0.0.1", &relaysvr_addr.sin_addr);

	BOOL bret_relay = connectExFP_relay(g_relay_sock, reinterpret_cast<sockaddr*>(&relaysvr_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over_relay->overlapped);
	if (FALSE == bret_relay) {
		int err_no = GetLastError();
		if (ERROR_IO_PENDING == err_no)
			cout << "Connect 시도 중...\n" << endl;
		else {
			cout << "ConnectEX Error - " << err_no << endl;
			cout << WSAGetLastError() << endl;
		}
	}
	//test end


	//======================================================================
	// [ HA - 서버 수평확장 ]
	// HA Listen Socket (서버 간 통신을 위한 Listen소켓)
	g_ss_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN ha_server_addr;
	memset(&ha_server_addr, 0, sizeof(ha_server_addr));
	ha_server_addr.sin_family = AF_INET;
	ha_server_addr.sin_port = htons(ss_portnum);
	ha_server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_ss_listensock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(ha_server_addr));
	listen(g_ss_listensock, SOMAXCONN);
	SOCKADDR_IN ha_addr;
	int ha_addr_size = sizeof(ha_addr);

	// HA Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ss_listensock), h_iocp, CP_KEY_LISTEN_EXLOGIC, 0);
	right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX ha_over;
	ha_over.process_type = OP_ACCEPT;
	ha_over.wsabuf.buf = reinterpret_cast<CHAR*>(right_ex_server_sock);
	AcceptEx(g_ss_listensock, right_ex_server_sock, ha_over.send_buf, 0, ha_addr_size + 16, ha_addr_size + 16, 0, &ha_over.overlapped);

	// 수평확장된 서버의 마지막 구성원이 아니라면, 오른쪽에 있는 서버에 비동기connect 요청을 보냅니다.
	if (my_server_id < MAX_SERVER - 1) {
		int right_servernum = my_server_id + 1;
		int right_portnum = ss_portnum + 1;
		cout << "다른 이중화 서버(Server[" << right_servernum << "] (S-S PORT: " << right_portnum << ")에 비동기Connect를 요청합니다." << endl;

		// ConnectEx
		SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		GUID guid = WSAID_CONNECTEX;
		DWORD bytes = 0;
		LPFN_CONNECTEX connectExFP;
		::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
		closesocket(temp_s);

		SOCKADDR_IN ha_server_addr;
		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
		int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
		if (ret != 0) {
			cout << "Bind Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		OVER_EX* con_over = new OVER_EX;
		con_over->process_type = OP_CONNECT;
		int key_num = CP_KEY_LOGIC2EXLOGIC + right_servernum;
		HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key_num, 0);
		if (NULL == hret) {
			cout << "CreateIoCompletoinPort Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		ha_server_addr.sin_port = htons(right_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버
		inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

		BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
		if (FALSE == bret) {
			int err_no = GetLastError();
			if (ERROR_IO_PENDING == err_no)
				cout << "Server Connect 시도 중...\n" << endl;
			else {
				cout << "ConnectEX Error - " << err_no << endl;
				cout << WSAGetLastError() << endl;
			}
		}
	}
	else {
		cout << "마지막 서버구성원이므로 Connect를 수행하지않습니다.\n" << endl;
	}
	extended_servers[my_server_id].id = my_server_id;
	extended_servers[my_server_id].s_state = ST_ACCEPTED;

	//======================================================================
	// [ Main ]
	init_npc();
	shoot_time = chrono::system_clock::now();

	//======================================================================
	// [ Main - 클라이언트 연결 ]
	// Client Listen Socket (클라이언트-서버 통신을 위한 Listen소켓)
	g_sc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(sc_portnum);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_sc_listensock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_sc_listensock, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	// Client Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_sc_listensock), h_iocp, CP_KEY_LISTEN_CLIENT, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_sc_listensock, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);			// 클라이언트-서버 통신용 Worker스레드

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);				// 클라이언트 로직 타이머스레드
	timer_threads.emplace_back(heartBeatFunc);			// 서버 간 Heartbeat교환 스레드
	timer_threads.emplace_back(MoveNPC);				// NPC 로직 스레드
	timer_threads.emplace_back(replicaSessions);	// 서버 간 세션데이터 복제 스레드

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_sc_listensock);
	WSACleanup();
}
