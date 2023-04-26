#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <chrono>
#include <random>

#include "MapObjects.h"
#include "Constant.h"
#include "MathFuncs.h"
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

Coordinate basic_coordinate;	// �⺻(�ʱ�) ��ǥ��

chrono::system_clock::time_point g_s_start_time;	// ���� ���۽ð�  (����: ms)
milliseconds g_curr_servertime;
bool b_isfirstplayer;	// ù player��������. (ù Ŭ�� ���Ӻ��� �����ð��� �帣���� �ϱ� ����)
mutex servertime_lock;	// �����ð� lock

vector<City_Info>Cities;

vector<MapObject> buildings_info;	// Map Buildings CollideBox

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
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// ���� Look, Right, Up Vectors
	chrono::system_clock::time_point death_time;
	chrono::system_clock::time_point last_move_rotate_keyinput_time;	// ���������� Ű �Է��� �� �ð�

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

chrono::system_clock::time_point shoot_time;

HANDLE h_iocp;				// IOCP �ڵ�
SOCKET g_sc_listensock;		// Ŭ���̾�Ʈ ��� listen����
SOCKET g_ss_listensock;		// ����Ȯ�� ���� �� ��� listen ����
SOCKET g_relay_sock;		// �����̼��� �� ��� listen ����

SOCKET left_ex_server_sock;								// ���� ��ȣ�� ����
SOCKET right_ex_server_sock;							// ���� ��ȣ�� ����

int my_server_id;										// �� ���� �ĺ���ȣ
bool b_active_server;									// Active �����ΰ�?
array<HA_SERVER, MAX_SERVER> extended_servers;			// HA������ ���� ����Ȯ��� ������
HA_SERVER relayserver;	// �����̼���


void SESSION::send_login_info_packet()
{
	if (curr_stage != 1) return;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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
	if (curr_stage != 1) return;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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
	if (curr_stage != 1) return;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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
	if (curr_stage != 1) return;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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
			if (pl.curr_stage != 1) break;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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

		cout << "Server[" << extended_servers[target_id].id << "]�� �ٿ��� �����Ǿ����ϴ�." << endl;	// server message

		// ���� �����
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + target_id);	// �����ڸ�: Actvie����(S: 1, A: 0), �����ڸ�: ����ID
		ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../x64/Release", SW_SHOW);

		// Ŭ���̾�Ʈ���� Active������ �ٿ�Ǿ��ٰ� �˷���.
		if (!b_active_server) {	// ���� Active�� �ƴϸ� ��밡 Active��. (������ 2���ۿ� ���� ����)
			b_active_server = true;
			cout << "���� Server[" << my_server_id << "] �� Active ������ �°ݵǾ����ϴ�. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// ���� �ڽ��� ������ ������ �ٿ�Ǿ��µ�, �� ������ �������� ������ ������ ��� ������ �������� ConnectEx ��û�� �����ϴ�.
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
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
				ha_server_addr.sin_port = htons(target_portnum);	// ����Ȯ��� ���������� �ڱ� �����ʿ� �ִ� ����
				inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect ��õ� ��...\n" << endl;
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

		cout << "�����̼���[" << relayserver.id << "]���� ������ ������ϴ�." << endl;	// server message
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

		// �������� ���� ù��° Ŭ���̾�Ʈ�� �����̶�� �׶����� �����ð��� �帣�� �����մϴ�.
		if (b_isfirstplayer) {
			cout << "���� �ð��� �帣�� �����մϴ�.\n" << endl;
			g_s_start_time = system_clock::now();
			b_isfirstplayer = false;
		}

		// ���� ������ �÷��̾��� �ʱ� ������ �����մϴ�.
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

		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "]��(��) �����Ͽ����ϴ�." << endl;	// server message

		if (!b_active_server) {
			cout << "Stand-By������ ��� ���¸� �����մϴ�." << endl;
			break;	// Active������ �ƴ϶��, Ŭ���̾�Ʈ�� ����Ǿ����� ����ڿ��� �˸��⸸ �ϰ� �ƹ��ϵ� ���� �ʽ��ϴ�.
		}

		cout << "���ο� ������Ʈ�� �����Ǿ����ϴ�! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ")." << endl;

		//====================
		// 1. Player ��ü ����
		// ���� ������ �ִ� ��� Ŭ���̾�Ʈ���� ���ο� Ŭ���̾�Ʈ(client_id)�� ������ �����մϴ�.
		for (int i = 0; i < MAX_USER; ++i) {
			auto& pl = clients[i];

			if (pl.id == client_id) continue;
			if (pl.curr_stage != 1) continue;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.

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

		// ���� ������ Ŭ���̾�Ʈ���� ���� ������ �ִ� ��� Ŭ���̾�Ʈ�� ������ �����մϴ�.
		for (auto& pl : clients) {
			if (pl.id == client_id) continue;
			if (pl.curr_stage != 1) continue;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.

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

		// ���� ������ Ŭ���̾�Ʈ���� ���� ������ �ִ� ��� NPC�� ������ �����մϴ�.
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
				if (clients[j].curr_stage != 1) continue;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
				if (clients[j].s_state == ST_INGAME) {
					clients[j].do_send(&add_npc_packet);
				}
			}
		}

		//====================
		// 2. �� ����
		// ���� ������ Ŭ���̾�Ʈ���� �� ������ �����ݴϴ�.
		for (auto& building : buildings_info) {
			SC_MAP_OBJINFO_PACKET building_packet;
			building_packet.type = SC_MAP_OBJINFO;
			building_packet.size = sizeof(SC_MAP_OBJINFO_PACKET);

			building_packet.pos_x = building.getPosX();
			building_packet.pos_y = building.getPosY();
			building_packet.pos_z = building.getPosZ();

			building_packet.scale_x = building.getScaleX();
			building_packet.scale_y = building.getScaleY();
			building_packet.scale_z = building.getScaleZ();

			clients[client_id].do_send(&building_packet);
		}
		break;
	}// CS_LOGIN end
	case CS_MOVE:
	{
		if (!b_active_server) break;
		CS_MOVE_PACKET* cl_move_packet = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		// 1. �浹üũ�� �Ѵ�.
		
		// 2. �÷��̾ �̵��� �� ���� ������ �̵��ߴٸ� RollBack��Ŷ�� ���� ���� ��ġ�� ���ư����� �Ѵ�.

		// 3. �װ� �ƴ϶�� ���� ������ ������Ʈ �Ѵ�.
		clients[client_id].s_lock.lock();
		clients[client_id].pos = { cl_move_packet->x, cl_move_packet->y, cl_move_packet->z };
		clients[client_id].s_lock.unlock();

		// 4. �ٸ� Ŭ���̾�Ʈ���� �÷��̾ �̵��� ��ġ�� �˷��ش�.
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
		// 1. �浹üũ�� �Ѵ�.
		
		// 2. �÷��̾ �̵��� �� ���� ������ �̵��ߴٸ� RollBack��Ŷ�� ���� ���� ��ġ�� ���ư����� �Ѵ�.

		// 3. �װ� �ƴ϶�� ���� ������ ������Ʈ �Ѵ�.
		clients[client_id].s_lock.lock();
		clients[client_id].m_rightvec = { cl_rotate_packet->right_x, cl_rotate_packet->right_y, cl_rotate_packet->right_z };
		clients[client_id].m_upvec = { cl_rotate_packet->up_x, cl_rotate_packet->up_y, cl_rotate_packet->up_z };
		clients[client_id].m_lookvec = { cl_rotate_packet->look_x, cl_rotate_packet->look_y, cl_rotate_packet->look_z };
		cout << "\n";
		clients[client_id].s_lock.unlock();

		// 4. �ٸ� Ŭ���̾�Ʈ���� �÷��̾ ȸ���� ������ �˷��ش�.
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
		// Bullet ��Ÿ�� üũ
		milliseconds shoot_term = duration_cast<milliseconds>(chrono::system_clock::now() - shoot_time);
		if (shoot_term < milliseconds(SHOOT_COOLDOWN_BULLET)) {	// ��Ÿ���� ������ �ʾҴٸ� �߻����� �ʽ��ϴ�.
			milliseconds left_cooldown = duration_cast<milliseconds>(milliseconds(SHOOT_COOLDOWN_BULLET) - shoot_term);
			break;
		}

		// �÷��̾��� ��ǥ�� �躤�͸� ���� ����ĳ��Ʈ�� �մϴ�.

		// �ǹ� �� ���������� �浹�ϸ� break

		// Player, Npc�� �浹�ϸ� ����� HP�� ���ҽ�Ű�� Ŭ���̾�Ʈ���� �ǰ� ��Ŷ�� ������ �մϴ�.
		
		break;
	}// CS_ATTACK end
	case CS_INPUT_KEYBOARD:
	{
		if (!b_active_server) break;		// Active ������ ��Ŷ�� ó���մϴ�.
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		float sign = 1.0f;					// right/up/look���� �������� �����̴���, �ݴ� �������� �����̴���
		switch (inputkey_p->keytype) {
		case PACKET_KEY_NUM1:
			clients[client_id].s_lock.lock();
			clients[client_id].curr_stage = 1;
			clients[client_id].s_lock.unlock();
			cout << "Client[" << client_id << "] Stage1�� ��ȯ." << endl;//test
			break;
		case PACKET_KEY_NUM2:
			clients[client_id].s_lock.lock();
			clients[client_id].curr_stage = 2;
			clients[client_id].s_lock.unlock();
			cout << "Client[" << client_id << "] Stage2�� ��ȯ." << endl;//test
			break;
		}

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE: {
		if (!b_active_server) break;		// Active ������ ��Ŷ�� ó���մϴ�.
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->buttontype == PACKET_BUTTON_L) {			// ���콺 ��Ŭ��
			// 1�������� ����
			if (clients[client_id].curr_stage == 1) {
				float rotate_scalar = 0.0f;

				clients[client_id].s_lock.lock();

				// Ŭ���̾�Ʈ���� ���� ���Ͱ����� ���� ������ ���͵��� ������Ʈ�մϴ�.

				clients[client_id].s_lock.unlock();

				// �۵� ���� �ٸ� Ŭ���̾�Ʈ���� ȸ�� ����� �˷��ݴϴ�.
				for (auto& other_pl : clients) {
					if (other_pl.id == client_id) continue;
					if (other_pl.s_state != ST_INGAME) continue;

					lock_guard<mutex> lg{ other_pl.s_lock };
					other_pl.send_rotate_packet(client_id, TARGET_PLAYER);
				}
			}
			// 2�������� ����
			else if (clients[client_id].curr_stage == 2) {
				// �̱���
			}

		}
		else if (rt_p->buttontype == PACKET_BUTTON_R) {		// ���콺 ��Ŭ�� �巡��: ��� ����.
			// 1�������� ����
			if (clients[client_id].curr_stage == 1) {

			}
			// 2�������� ����
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
		if (recv_id < my_server_id) {	// A->B->A�� heartbeat�� �� ����Ŭ�� ���������ϱ� ����. (��, ������ �����κ��� Heartbeat�� ������ �� ����Ŭ�� ������ �Ǵ�)
			// Heartbeat�� ���� ���� �������� �ڽ��� Heartbeat�� �����մϴ�.
			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[recv_id].do_send(&hb_packet);										// �ڽſ��� Heartbeat�� ���� �������� �����մϴ�.
			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// ������ �ð��� ������Ʈ
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

		//cout << "Client[" << replica_id << "]�� �����Ͱ� �����Ǿ����ϴ�." << endl;
		//cout << "===================================" << endl;
		//cout << "Name: " << clients[replica_id].name << endl;
		//cout << "Pos: " << clients[replica_id].pos.x << ", " << clients[replica_id].pos.y << ", " << clients[replica_id].pos.z << endl;
		//cout << "LookVec: " << clients[replica_id].m_lookvec.x << ", " << clients[replica_id].m_lookvec.y << ", " << clients[replica_id].m_lookvec.z << endl;
		//cout << "Pitch: " << clients[replica_id].pitch << "/ Yaw: " << clients[replica_id].yaw << "/ Roll: " << clients[replica_id].roll << endl;
		//cout << "===================================\n" << endl;

	}// SS_DATA_REPLICA end
	}
}


int get_new_client_id()	// clients�� ����ִ� ĭ�� ã�Ƽ� ���ο� client�� ���̵� �Ҵ����ִ� �Լ�
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
int find_empty_extended_server() {	// ex_servers�� ����ִ� ĭ�� ã�Ƽ� ���ο� Server_ex�� ���̵� �Ҵ����ִ� �Լ�
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
				//cout << "Connect Error" << endl;

				// �񵿱�Conn�� �ٽ� �õ��մϴ�.
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
					if (ERROR_IO_PENDING == err_no) {
						//cout << "Server Connect ��õ� ��...\n" << endl;
					}
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
					// Ŭ���̾�Ʈ id, ����
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
					//cout << "Sever[" << new_id << "]�� �����û�� �޾ҽ��ϴ�.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_LOGIC2EXLOGIC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_iocp, new_key, 0);
					extended_servers[new_id].do_recv();
					extended_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "�ٸ� Sever�� �����û�� �޾�����, ���� ������ �� á���ϴ�.\n" << endl;
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
					std::cout << "���������� Server[" << server_id << "]�� ����Ǿ����ϴ�.\n" << endl;
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
		// ���� �ð� ������Ʈ
		if (b_active_server && !b_isfirstplayer) {
			servertime_lock.lock();
			g_curr_servertime = duration_cast<milliseconds>(start_t - g_s_start_time);
			servertime_lock.unlock();
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage != 1) continue;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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
		// --- ������Ʈ



		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}

void heartBeatFunc() {	// Heartbeat���� ������ �Լ�
	while (true) {
		auto start_t = system_clock::now();
		// ������ ������ Heartbeat�� �����ϴ�. (���� ������ ������ ������ �����ϱ� ������ ���� ������ ������ ������ �ʽ��ϴ�.)
		if (my_server_id != MAX_SERVER - 1) {
			if (extended_servers[my_server_id].s_state != ST_ACCEPTED)	continue;
			if (extended_servers[my_server_id + 1].s_state != ST_ACCEPTED) continue;

			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[my_server_id + 1].do_send(&hb_packet);	// ������ ������ �����մϴ�.

			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// ������ �ð��� ������Ʈ
		}

		// �������� Heartbeat�� ���� ���� ������������ �ִ��� Ȯ���մϴ�.
		// 1. ������ ���� �˻�	(���� ���ʿ� �ִ� ���� �������� �˻����� �ʽ��ϴ�.)
		if (my_server_id != 0) {
			if (extended_servers[my_server_id - 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id - 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id - 1 << "]���� Heartbeat�� �������� ���� ���߽��ϴ�. ���� �ٿ����� �����մϴ�." << endl;
					disconnect(my_server_id - 1, SESSION_EXTENDED_SERVER);
				}
			}
		}
		// 2. ���� ���� �˻� (���� �����ʿ� �ִ� ���� �������� �˻����� �ʽ��ϴ�.)
		if (my_server_id != MAX_SERVER - 1) {
			if (extended_servers[my_server_id + 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id + 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id + 1 << "]���� Heartbeat�� �������� ���� ���߽��ϴ�. ���� �ٿ����� �����մϴ�." << endl;
					disconnect(my_server_id + 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		// ������ ���
		auto curr_t = system_clock::now();
		if (curr_t - start_t < static_cast<milliseconds>(HB_SEND_CYCLE)) {
			this_thread::sleep_for(static_cast<milliseconds>(HB_SEND_CYCLE) - (curr_t - start_t));
		}
	}
}
void replicaSessions() {	// ������ ���ǵ����͸� �����ϴ� �Լ�
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

				//cout << "TEST: Client[" << cl.id << "]�� ������ Sever[" << standby_id << "]���� �����մϴ�." << endl;
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
	for (int i{}; i < 3; ++i) {
		City_Info temp;
		temp.id = i;
		temp.Centerx = C_cx[i];
		temp.Centerz = C_cz[i];

		for (int j{}; j < 3; ++j) {
			temp.SectionNum[j].ID = j;
			temp.SectionNum[j].lx = LX_range[3 * i + j];
			temp.SectionNum[j].lz = LZ_range[3 * i + j];
			temp.SectionNum[j].sx = SX_range[3 * i + j];
			temp.SectionNum[j].sz = SZ_range[3 * i + j];
		}
		Cities.emplace_back(temp);
	}

	/*for (int i{}; i < Cities.size(); ++i) {
		Cities[i].print();
	}*/

	for (int i{}; i < MAX_NPCS; i++) {
		int npc_id = i;
		npcs[i].SetID(npc_id);
		npcs[i].SetNpcType(NPC_Helicopter);
		npcs[i].SetRotate(0.0f, 0.0f, 0.0f);
		//npcs[i].SetActive(false);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float>AirHigh(50, 270);
		
		uniform_int_distribution<int>Sec_num(0, 2);

		int ran_num = Sec_num(dre);

		switch (ran_num)
		{
		case 0:
		{
			npcs[i].SetIdleCity(ran_num);
			int sec = Sec_num(dre);
			float lx, lz, sx, sz = 0;
			switch (sec)
			{
			case 0:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			case 1:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			case 2:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			}
			uniform_real_distribution<float>AirXPos(sx, lx);
			uniform_real_distribution<float>AirZPos(sz, lz);
			npcs[i].SetPosition(AirXPos(dre), AirHigh(dre), AirZPos(dre));
			npcs[i].SetIdleSection(sec);
		}
		break;
		case 1:
		{
			npcs[i].SetIdleCity(ran_num);
			int sec = Sec_num(dre);
			float lx, lz, sx, sz = 0;
			switch (sec)
			{
			case 0:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			case 1:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			case 2:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			}
			uniform_real_distribution<float>AirXPos(sx, lx);
			uniform_real_distribution<float>AirZPos(sz, lz);
			npcs[i].SetPosition(AirXPos(dre), AirHigh(dre), AirZPos(dre));
			npcs[i].SetIdleSection(sec);
		}
		break;
		case 2:
		{
			npcs[i].SetIdleCity(ran_num);
			int sec = Sec_num(dre);
			float lx, lz, sx, sz = 0;
			switch (sec)
			{
			case 0:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			case 1:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			case 2:
			{
				sx = Cities[ran_num].SectionNum[sec].sx;
				lx = Cities[ran_num].SectionNum[sec].lx;
				sz = Cities[ran_num].SectionNum[sec].sz;
				lz = Cities[ran_num].SectionNum[sec].lz;
			}
			break;
			}
			uniform_real_distribution<float>AirXPos(sx, lx);
			uniform_real_distribution<float>AirZPos(sz, lz);
			npcs[i].SetPosition(AirXPos(dre), AirHigh(dre), AirZPos(dre));
			npcs[i].SetIdleSection(sec);
		}
		break;
		}
		uniform_real_distribution<float>SpdSet(1.1f, 3.5f);
		float speed = SpdSet(dre);
		npcs[i].SetSpeed(speed);
		npcs[i].SetInitSection(Cities);
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
			// Ŭ���̾�Ʈ��� NPC ������ �Ÿ� ���
			
			for (auto& cl : clients) {
				if (cl.id != -1) {
					npcs[i].Caculation_Distance(cl.pos, cl.id);
				}
			}
			//cout << i << "��° Status - " << npcs[i].GetState() << endl;

			// NPC�� �����Ϸ��� ���̵� �ִ������� Ȯ��, ������ ���� ��� �÷��̾� ��ǥ�� �ӽ� ����
			if (npcs[i].GetChaseID() != -1) {
				npcs[i].SetUser_Pos(clients[npcs[i].GetChaseID()].pos, npcs[i].GetChaseID());
			}

			// npc pos Ȯ��
		/*	cout << i << "��° NPC�� ���� ID: " << npcs[i].GetIdleCity() << ", NPC�� ���� ID: " << npcs[i].GetIdleSection() << endl;
			cout << i << "��° NPC�� Pos: " << npcs[i].GetPosition().x << ',' << npcs[i].GetPosition().y << ',' << npcs[i].GetPosition().z << endl;
			cout << i << "��° NPC�� ����: " << npcs[i].GetState() << endl;*/

			// ���¸��� �ٸ� �������� �ϴ� �Ŵ�����Ʈ
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
				if (send_target.curr_stage != 1) continue;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
				if (send_target.s_state != ST_INGAME) continue;

				lock_guard<mutex> lg{ send_target.s_lock };
				send_target.do_send(&npc_update_packet);
			}
		}

		//cout << "=============" << endl;
	}
}


int main(int argc, char* argv[])
{
	b_isfirstplayer = true;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	// [ HA - ���� ID, ��Ʈ��ȣ ���� ]
	// � ������ ������ �������� ����� �μ��� �Է¹޾Ƽ� �� ������ ��Ʈ ��ȣ�� �ο��մϴ�.
	my_server_id = 0;		// ���� ������ ���� ������ȣ
	int sc_portnum = -1;	// Ŭ���̾�Ʈ ��ſ� ��Ʈ��ȣ
	int ss_portnum = -1;	// ���� �� ��ſ� ��Ʈ��ȣ
	if (argc > 1) {			// �Էµ� ����� �μ��� ���� �� (�ַ� �����ٿ����� ���� ���� ����ට ����)
		// Serve ID����
		my_server_id = atoi(argv[1]) % 10;

		// Active ���� ����
		if (my_server_id == MAX_SERVER - 1) {
			b_active_server = true;
		}
		else {
			b_active_server = false;
		}
	}
	else {				// ���� �Էµ� ����� �μ��� ���ٸ� �Է��� �޽��ϴ�.
		cout << "������ ����: ";
		cin >> my_server_id;

		// Active ���� ����
		switch (my_server_id) {
		case 0:	// 0�� ����
			b_active_server = false;
			break;
		case 1:	// 1�� ����
			b_active_server = true;
			break;
		default:
			cout << "�߸��� ���� �ԷµǾ����ϴ�. ���α׷��� �����մϴ�." << endl;
			return 0;
		}
	}

	// ������ȣ�� ���� ��Ʈ��ȣ�� �������ݴϴ�.
	switch (my_server_id) {
	case 0:	// 0�� ����
		sc_portnum = PORT_NUM_S0;
		ss_portnum = HA_PORTNUM_S0;
		break;
	case 1:	// 1�� ����
		sc_portnum = PORT_NUM_S1;
		ss_portnum = HA_PORTNUM_S1;
		break;
	default:
		cout << "�߸��� ���� �ԷµǾ����ϴ�. ���α׷��� �����մϴ�." << endl;
		return 0;
	}
	cout << "Server[" << my_server_id << "] �� �����Ǿ����ϴ�. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else cout << "Stand-By";
	cout << " / S - C PORT : " << sc_portnum << " / S - S PORT : " << ss_portnum << " ]" << endl;

	//======================================================================
	// [ HA - Relay���� ���� ]
	int active_relay_serverid = 0;							// [[[���Ŀ� ���� Active ���¿� �ִ� �����̼����� id�� ��Ʈ��ȣ�� �ٲ� ������!]]]
	int active_relay_portnum = PORTNUM_RELAY2LOGIC_0;		//

	// Relay Connect
	// Relay������ Logic������ ��������ֱ� ������, �񵿱�Connect�� �� �ʿ䰡 ����.
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
	g_relay_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
			cout << "Connect �õ� ��...\n" << endl;
		else {
			cout << "ConnectEX Error - " << err_no << endl;
			cout << WSAGetLastError() << endl;
		}
	}
	//test end


	//======================================================================
	// [ HA - ���� ����Ȯ�� ]
	// HA Listen Socket (���� �� ����� ���� Listen����)
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

	// ����Ȯ��� ������ ������ �������� �ƴ϶��, �����ʿ� �ִ� ������ �񵿱�connect ��û�� �����ϴ�.
	if (my_server_id < MAX_SERVER - 1) {
		int right_servernum = my_server_id + 1;
		int right_portnum = ss_portnum + 1;
		cout << "�ٸ� ����ȭ ����(Server[" << right_servernum << "] (S-S PORT: " << right_portnum << ")�� �񵿱�Connect�� ��û�մϴ�." << endl;

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
		right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
		ha_server_addr.sin_port = htons(right_portnum);	// ����Ȯ��� ���������� �ڱ� �����ʿ� �ִ� ����
		inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

		BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
		if (FALSE == bret) {
			int err_no = GetLastError();
			if (ERROR_IO_PENDING == err_no)
				cout << "Server Connect �õ� ��...\n" << endl;
			else {
				cout << "ConnectEX Error - " << err_no << endl;
				cout << WSAGetLastError() << endl;
			}
		}
	}
	else {
		cout << "������ �����������̹Ƿ� Connect�� ���������ʽ��ϴ�.\n" << endl;
	}
	extended_servers[my_server_id].id = my_server_id;
	extended_servers[my_server_id].s_state = ST_ACCEPTED;

	//======================================================================
	// [ Main ]
	init_npc();
	shoot_time = chrono::system_clock::now();

	// [ Main - �� ���� �ε� ]
	// 1. ���丮 �˻�
	string filename;
	vector<string> readTargets;

	filesystem::path collidebox_path(".\\Collideboxes");
	if (filesystem::exists(collidebox_path)) {
		filesystem::recursive_directory_iterator itr(collidebox_path);
		while (itr != filesystem::end(itr)) {
			const filesystem::directory_entry& entry = *itr;
			//cout << entry.path().string() << endl;
			string path_name = entry.path().string();
			if (path_name.find(".txt") != string::npos) {	// .txt �� �� ���ϸ� �����մϴ�. (���丮 �̸��� �ִ� path ����)
				readTargets.push_back(path_name);
			}
			itr++;
		}
	}
	else {
		cout << "[Directory Search Error] Unknown Directory." << endl;
	}

	// 2. ���� �б�
	for (auto& fname : readTargets) {
		cout << "[Map Loading...] " << fname;
		//string fname = readTargets[0];
		ifstream txtfile(fname);

		string line;

		int line_cnt = 0;

		char b_pos = 0;
		int pos_count = 0;

		char b_scale = 0;
		int scale_count = 0;

		float tmp_pos[3] = { 0.f, 0.f, 0.f }; // ���� ��ǥ������ �ӽ� ������ ����, 3�� ������ ���Ϳ� �־��ְ� �������.
		float tmp_scale[3] = { 0.f, 0.f, 0.f }; // ���� ũ�������� �ӽ� ������ ����, 3�� ������ ���Ϳ� �־��ְ� �������.
		if (txtfile.is_open()) {
			while (txtfile >> line) {
				if (line == "Position:") {
					b_pos = 1;
					pos_count = 0;
				}
				else if (line == "Size:") {
					b_scale = 1;
					scale_count = 0;
				}
				else {
					if (b_pos == 1) {
						tmp_pos[pos_count] = string2data(line);

						if (pos_count == 2) {
							tmp_pos[pos_count] = string2data(line);

							b_pos = 0;
							pos_count = 0;
						}
						else {
							pos_count += 1;
						}
					}
					else if (b_scale == 1) {
						tmp_scale[scale_count] = string2data(line);

						if (scale_count == 2) {
							tmp_scale[scale_count] = string2data(line);
							b_scale = 0;
							scale_count = 0;

							MapObject tmp_mapobj(tmp_pos[0], tmp_pos[1], tmp_pos[2], tmp_scale[0], tmp_scale[1], tmp_scale[2]);
							buildings_info.push_back(tmp_mapobj);
							memset(tmp_pos, 0, sizeof(tmp_pos));
							memset(tmp_scale, 0, sizeof(tmp_scale));
						}
						else {
							scale_count += 1;
						}
					}
				}
				line_cnt++;
			}
			cout << " ---- OK." << endl;
		}
		else {
			cout << "[Error] Unknown File." << endl;
		}
		txtfile.close();
	}
	cout << "\n";

	//======================================================================
	// [ Main - Ŭ���̾�Ʈ ���� ]
	// Client Listen Socket (Ŭ���̾�Ʈ-���� ����� ���� Listen����)
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
		worker_threads.emplace_back(do_worker);			// Ŭ���̾�Ʈ-���� ��ſ� Worker������

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);				// Ŭ���̾�Ʈ ���� Ÿ�̸ӽ�����
	timer_threads.emplace_back(heartBeatFunc);			// ���� �� Heartbeat��ȯ ������
	timer_threads.emplace_back(MoveNPC);				// NPC ���� ������
	timer_threads.emplace_back(replicaSessions);	// ���� �� ���ǵ����� ���� ������

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_sc_listensock);
	WSACleanup();
}