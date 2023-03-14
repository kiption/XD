#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <chrono>
#include <random>

#include "Constant.h"
#include "BulletsMgr.h"
#include "../RelayServer/Protocol.h"
#include "NPC.h"
#include "Timer.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };
enum PLAYER_STATE { PL_ST_ALIVE, PL_ST_DEAD };
enum SESSION_TYPE { SESSION_CLIENT, SESSION_EXTENDED_SERVER };

Coordinate basic_coordinate;	// �⺻(�ʱ�) ��ǥ��

class OVER_EXP {
public:
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	char send_buf[BUF_SIZE];
	PACKET_PROCESS_TYPE process_type;

	OVER_EXP()
	{
		wsabuf.len = BUF_SIZE;
		wsabuf.buf = send_buf;
		process_type = OP_RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	OVER_EXP(char* packet)
	{
		wsabuf.len = packet[0];
		wsabuf.buf = send_buf;
		ZeroMemory(&overlapped, sizeof(overlapped));
		process_type = OP_SEND;
		memcpy(send_buf, packet, packet[0]);
	}
};

class SESSION {
	OVER_EXP recv_over;

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
	XMFLOAT3 pos;								// Position (x, y, z)
	float pitch, yaw, roll;						// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;	// ���� Look, Right, Up Vectors
	chrono::system_clock::time_point death_time;

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

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(heli_bbsize_x, heli_bbsize_y, heli_bbsize_z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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
		OVER_EXP* s_data = new OVER_EXP{ reinterpret_cast<char*>(packet) };
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

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(heli_bbsize_x, heli_bbsize_y, heli_bbsize_z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
};

class HA_SERVER {
	OVER_EXP recv_over;

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
		OVER_EXP* s_data = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
};

int online_player_cnt = 0;
array<SESSION, MAX_USER + MAX_NPCS> clients;		// 0 ~ MAX_USER-1: Player,	 MAX_USER ~ MAX_USER+MAX_NPCS: NPC
array<NPC, MAX_NPCS> npcs;

array<Bullets, MAX_BULLET> bullets_arr;
chrono::system_clock::time_point shoot_time;

HANDLE h_sc_iocp;			// Ŭ���̾�Ʈ ��� IOCP �ڵ�
SOCKET g_sc_listensock;		// Ŭ���̾�Ʈ ��� listen����
HANDLE h_ss_iocp;			// ����Ȯ�� ���� �� ��� IOCP �ڵ�
SOCKET g_ss_listensock;		// ����Ȯ�� ���� �� ��� listen ����
HANDLE h_relay_iocp;		// �����̼��� �� ��� IOCP �ڵ�
SOCKET g_relay_sock;		// �����̼��� �� ��� listen ����

SOCKET left_ex_server_sock;								// ���� ��ȣ�� ����
SOCKET right_ex_server_sock;							// ���� ��ȣ�� ����

int my_server_id;										// �� ���� �ĺ���ȣ
array<HA_SERVER, 1> relay_servers;						// Rel
array<HA_SERVER, MAX_SERVER> extended_servers;			// HA������ ���� ����Ȯ��� ������



void SESSION::send_login_info_packet()
{
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

		online_player_cnt--;
		cout << "Player[ID: " << clients[target_id].id << ", name: " << clients[target_id].name << " is log out" << endl;	// server message

		for (int i = 0; i < MAX_USER; i++) {
			auto& pl = clients[i];

			if (pl.id == target_id) continue;

			pl.s_lock.lock();
			if (pl.s_state != ST_INGAME) {
				pl.s_lock.unlock();
				continue;
			}
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

		cout << "Server[" << extended_servers[target_id].id << "]�� �ٿ�Ǿ����ϴ�. ������ ������մϴ�." << endl;	// server message

		// ���� �����
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", target_id);
		ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../x64/Release", SW_SHOW);
		break;
	}

}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].s_lock.lock();
		if (clients[i].s_state == ST_FREE) {
			clients[i].s_state = ST_ACCEPTED;
			clients[i].s_lock.unlock();
			online_player_cnt++;
			return i;
		}
		clients[i].s_lock.unlock();
	}
	return -1;
}

void process_packet(int client_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
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

		// ���� ������ �÷��̾��� �ʱ� ������ �����մϴ�.
		clients[client_id].pl_state = PL_ST_ALIVE;
		clients[client_id].pos.x = 1500 + client_id * 50;
		clients[client_id].pos.y = 400 + client_id * 20;
		clients[client_id].pos.z = 1265 - client_id * 50;
		cout << "A new object is successfully created! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ")." << endl;

		clients[client_id].pitch = clients[client_id].yaw = clients[client_id].roll = 0.0f;
		clients[client_id].m_rightvec = basic_coordinate.right;
		clients[client_id].m_upvec = basic_coordinate.up;
		clients[client_id].m_lookvec = basic_coordinate.look;

		clients[client_id].setBB();

		strcpy_s(clients[client_id].name, login_packet->name);

		clients[client_id].send_login_info_packet();
		clients[client_id].s_state = ST_INGAME;

		clients[client_id].s_lock.unlock();

		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is log in" << endl;	// server message

		// ���� ������ �ִ� ��� Ŭ���̾�Ʈ���� ���ο� Ŭ���̾�Ʈ(client_id)�� ������ �����մϴ�.
		for (int i = 0; i < MAX_USER; ++i) {		
			auto& pl = clients[i];

			if (pl.id == client_id) continue;

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
				if (clients[j].s_state == ST_INGAME) {
					clients[j].do_send(&add_npc_packet);
				}
			}
		}
		break;
	}// CS_LOGIN end
	case CS_INPUT_KEYBOARD: {
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		enum InputKey { KEY_Q, KEY_E, KEY_A, KEY_D, KEY_S, KEY_W, KEY_SPACEBAR };

		for (int i = 0; i <= 6; i++) {
			if ((inputkey_p->direction >> i) & 1) {
				float sign = 1.0f;					// right/up/look���� �������� �����̴���, �ݴ� �������� �����̴���
				switch (i) {
				case KEY_Q:
				case KEY_E:
					clients[client_id].s_lock.lock();
					// ���� ��� ����.

					// unlock
					clients[client_id].s_lock.unlock();
					break;

				case KEY_A:			// D, A�� ��ü�� yawȸ�� Ű�Դϴ�. ��ü�� y�� �������� ȸ����ŵ�ϴ�.
					sign = -1.0f;	// A�� right���� �ݴ� �������� �����̱� ������ -1�� �����ݴϴ�.
					[[fallthrough]]
				case KEY_D:
					clients[client_id].s_lock.lock();
					// yaw ����
					clients[client_id].yaw += sign * YAW_ROTATE_SCALAR;

					// right, up, look ���� ������Ʈ
					clients[client_id].m_rightvec = calcRotate(basic_coordinate.right
						, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
					clients[client_id].m_upvec = calcRotate(basic_coordinate.up
						, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
					clients[client_id].m_lookvec = calcRotate(basic_coordinate.look
						, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);

					// unlock
					clients[client_id].s_lock.unlock();

					// �۵� ���� ��� Ŭ���̾�Ʈ���� ȸ�� ����� �˷��ݴϴ�.
					for (int j = 0; j < MAX_USER; j++) {
						auto& pl = clients[j];
						lock_guard<mutex> lg{ pl.s_lock };
						if (pl.s_state == ST_INGAME)
							pl.send_rotate_packet(client_id, TARGET_PLAYER);
					}
					break;

				case KEY_S:			// W, S�� ������� ���� Ű�Դϴ�. ��ü�� ��� �Ǵ� �ϰ���ŵ�ϴ�.
					sign = -1.0f;	// S�� up���� �ݴ� �������� �����̱� ������ -1�� �����ݴϴ�.
					[[fallthrough]]
				case KEY_W:
					clients[client_id].s_lock.lock();

					// �̵� ��� & ��� ������Ʈ
					XMFLOAT3 move_result = calcMove(clients[client_id].pos, clients[client_id].m_upvec, ENGINE_SCALAR * sign);
					clients[client_id].pos = move_result;

					// �ٿ�� �ڽ� ������Ʈ
					clients[client_id].setBB();

					// unlock
					clients[client_id].s_lock.unlock();

					// �۵� ���� ��� Ŭ���̾�Ʈ���� �̵� ����� �˷��ݴϴ�.
					for (int j = 0; j < MAX_USER; j++) {
						auto& pl = clients[j];
						lock_guard<mutex> lg{ pl.s_lock };
						if (pl.s_state == ST_INGAME)
							pl.send_move_packet(client_id, TARGET_PLAYER);
					}

					break;

				case KEY_SPACEBAR:	// �����̽��ٴ� ���� Ű�Դϴ�. �ٶ󺸴� �������� ���� ���ϴ�.
					// bullet lock (���� ����)

					// Bullet Cooldown Check
					milliseconds shoot_term = duration_cast<milliseconds>(chrono::system_clock::now() - shoot_time);
					if (shoot_term < milliseconds(SHOOT_COOLDOWN_BULLET)) {	// ��Ÿ���� ������ �ʾҴٸ� �߻����� �ʽ��ϴ�.
						milliseconds left_cooldown = duration_cast<milliseconds>(milliseconds(SHOOT_COOLDOWN_BULLET) - shoot_term);
						break;
					}

					// empty space check
					int new_bullet_id = -1;
					int arr_cnt = -1;
					if (clients[client_id].bullet > 0) {		// ���� �Ѿ��� ���� ������
						while (arr_cnt < MAX_BULLET / MAX_USER) {
							if (bullets_arr[arr_cnt].getId() == -1) {
								new_bullet_id = arr_cnt + 10 * client_id;	// 0�� Ŭ��: 0-99, 1�� Ŭ��: 100-199, 2�� Ŭ��: 200-299
								break;
							}
							else {
								arr_cnt++;
							}
						}

						// ���Ϳ� �����ִ� ������ ���� ������ �߻��մϴ�.
						if (new_bullet_id != -1) {
							// shoot time update
							shoot_time = chrono::system_clock::now();

							// �Ѿ� �ϳ� ���
							clients[client_id].s_lock.lock();
							clients[client_id].bullet -= 1;

							// �߻��� ����ڿ��� �Ѿ� ��������� �˷��ݴϴ�.
							SC_BULLET_COUNT_PACKET bullet_packet;
							bullet_packet.size = sizeof(bullet_packet);
							bullet_packet.type = SC_BULLET_COUNT;
							bullet_packet.id = client_id;
							bullet_packet.bullet_cnt = clients[client_id].bullet;
							clients[client_id].do_send(&bullet_packet);

							clients[client_id].s_lock.unlock();

							// Bullet ����
							bullets_arr[new_bullet_id].setId(new_bullet_id);
							bullets_arr[new_bullet_id].setPos(clients[client_id].pos);
							bullets_arr[new_bullet_id].setPitch(clients[client_id].pitch);
							bullets_arr[new_bullet_id].setYaw(clients[client_id].yaw);
							bullets_arr[new_bullet_id].setRoll(clients[client_id].roll - 20);
							bullets_arr[new_bullet_id].setRightvector(clients[client_id].m_rightvec);
							bullets_arr[new_bullet_id].setUpvector(clients[client_id].m_upvec);
							bullets_arr[new_bullet_id].setLookvector(calcRotate(clients[client_id].m_lookvec
								, bullets_arr[new_bullet_id].getPitch(), 0.f, 0.f));
							bullets_arr[new_bullet_id].setOwner(client_id);
							bullets_arr[new_bullet_id].setInitialPos(bullets_arr[new_bullet_id].getPos());
							bullets_arr[new_bullet_id].setBB_ex(XMFLOAT3{ vulcan_bullet_bbsize_x, vulcan_bullet_bbsize_y, vulcan_bullet_bbsize_z });

							// �������ִ� ��� Ŭ���̾�Ʈ���� ���ο� Bullet������ �����ϴ�.
							SC_ADD_OBJECT_PACKET add_bullet_packet;
							add_bullet_packet.target = TARGET_BULLET;
							add_bullet_packet.id = new_bullet_id;
							strcpy_s(add_bullet_packet.name, "bullet");
							add_bullet_packet.size = sizeof(add_bullet_packet);
							add_bullet_packet.type = SC_ADD_OBJECT;

							add_bullet_packet.x = bullets_arr[new_bullet_id].getPos().x;
							add_bullet_packet.y = bullets_arr[new_bullet_id].getPos().y;
							add_bullet_packet.z = bullets_arr[new_bullet_id].getPos().z;

							add_bullet_packet.right_x = bullets_arr[new_bullet_id].getRightvector().x;
							add_bullet_packet.right_y = bullets_arr[new_bullet_id].getRightvector().y;
							add_bullet_packet.right_z = bullets_arr[new_bullet_id].getRightvector().z;

							add_bullet_packet.up_x = bullets_arr[new_bullet_id].getUpvector().x;
							add_bullet_packet.up_y = bullets_arr[new_bullet_id].getUpvector().y;
							add_bullet_packet.up_z = bullets_arr[new_bullet_id].getUpvector().z;

							add_bullet_packet.look_x = bullets_arr[new_bullet_id].getLookvector().x;
							add_bullet_packet.look_y = bullets_arr[new_bullet_id].getLookvector().y;
							add_bullet_packet.look_z = bullets_arr[new_bullet_id].getLookvector().z;

							for (auto& pl : clients) {
								if (pl.s_state == ST_INGAME)
									pl.do_send(&add_bullet_packet);
							}
						}
					}
					else {	// ���� źȯ�� 0�̶�� reload
						clients[client_id].s_lock.lock();

						clients[client_id].bullet = 100;

						// �߻��� ����ڿ��� �Ѿ� ���������� �˷��ݴϴ�.
						SC_BULLET_COUNT_PACKET bullet_packet;
						bullet_packet.size = sizeof(bullet_packet);
						bullet_packet.type = SC_BULLET_COUNT;
						bullet_packet.id = client_id;
						bullet_packet.bullet_cnt = clients[client_id].bullet;
						clients[client_id].do_send(&bullet_packet);

						clients[client_id].s_lock.unlock();
					}

					// bullet unlock (���� ����)
					break;
				}
			}
		}

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE: {
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->key_val == RT_LBUTTON) {			// ���콺 ��Ŭ�� �巡��
			float rotate_scalar = 0.0f;

			clients[client_id].s_lock.lock();

			if (fabs(rt_p->delta_x) < fabs(rt_p->delta_y)) {	// ���콺 ��,�� �巡��: ����� �����մϴ�. ��ü�� x�� ���� ȸ����ŵ�ϴ�.
				rotate_scalar = -1.f * rt_p->delta_y * PITCH_ROTATE_SCALAR * SENSITIVITY;
				if (fabs(clients[client_id].pitch + rotate_scalar) < PITCH_LIMIT) {		// ���������� ȸ�� ����
					clients[client_id].pitch += rotate_scalar;							// pitch ����
				}
			}
			else {												// ���콺 ��,�� �巡��: ����� ������ �����մϴ�. ��ü�� z�� �������� ȸ����ŵ�ϴ�.
				rotate_scalar = -1.f * rt_p->delta_x * ROLL_ROTATE_SCALAR * SENSITIVITY;
				if (fabs(clients[client_id].roll + rotate_scalar) < ROLL_LIMIT) {		// ���������� ȸ�� ����
					clients[client_id].roll += rotate_scalar;							// roll ����
				}
			}

			// right, up, look ���� ȸ�� & ������Ʈ
			clients[client_id].m_rightvec = calcRotate(basic_coordinate.right, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
			clients[client_id].m_upvec = calcRotate(basic_coordinate.up, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
			clients[client_id].m_lookvec = calcRotate(basic_coordinate.look, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);

			clients[client_id].s_lock.unlock();

			// �۵� ���� ��� Ŭ���̾�Ʈ���� ȸ�� ����� �˷��ݴϴ�.
			for (auto& send_target : clients) {
				lock_guard<mutex> lg{ send_target.s_lock };
				if (send_target.s_state == ST_INGAME) {
					send_target.send_rotate_packet(client_id, TARGET_PLAYER);
				}
			}
		}
		else if (rt_p->key_val == RT_RBUTTON) {		// ���콺 ��Ŭ�� �巡��: ��� ����.
			//clients[client_id].s_lock.lock();
			cout << "���콺 ��Ŭ�� �Էµ�." << endl;// �ӽ��ڵ�
			//clients[client_id].s_lock.unlock();
		}

		break;
	}// CS_INPUT_MOUSE end
	case SS_HEARTBEAT:
	{
		SS_HEARTBEAT_PACKET* heartbeat_pack = reinterpret_cast<SS_HEARTBEAT_PACKET*>(packet);
		int recv_id = heartbeat_pack->sender_id;

		cout << "Server[" << recv_id << "]�� ���� Heartbeat�� �޾ҽ��ϴ�." << endl;
		extended_servers[recv_id].heartbeat_recv_time = chrono::system_clock::now();

		if (recv_id < my_server_id) {	// A->B->A�� heartbeat�� �� ����Ŭ�� ���������ϱ� ����. (��, ������ �����κ��� Heartbeat�� ������ �� ����Ŭ�� ������ �Ǵ�)
			// Heartbeat�� ���� ���� �������� �ڽ��� Heartbeat�� �����մϴ�.
			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[recv_id].do_send(&hb_packet);										// �ڽſ��� Heartbeat�� ���� �������� �����մϴ�.
			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// ������ �ð��� ������Ʈ

			cout << "Heartbeat�� ���� ���� Server[" << recv_id << "]���� �ڽ��� Heartbeat�� �����ϴ�." << endl;
		}
		break;
	}// SS_HEARTBEAT end
	}
}

void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_sc_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_ACCEPT) cout << "Accept Error";
			else {
				//cout << "GQCS Error ( client[" << key << "] )" << endl;
				disconnect(static_cast<int>(key), SESSION_CLIENT);
				if (ex_over->process_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
			int client_id = get_new_client_id();
			if (client_id != -1) {
				clients[client_id].pos = { 0.0f, 0.0f, 0.0f };
				clients[client_id].id = client_id;
				clients[client_id].name[0] = 0;
				clients[client_id].remain_size = 0;
				clients[client_id].socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_sc_iocp, client_id, 0);
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
			break;
		}
		case OP_RECV: {
			if (0 == num_bytes) disconnect(key, SESSION_CLIENT);

			int remain_data = num_bytes + clients[key].remain_size;
			char* p = ex_over->send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key].remain_size = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND: {
			if (0 == num_bytes) disconnect(key, SESSION_CLIENT);
			delete ex_over;
			break;
		}
		}
	}
}

int find_empty_extended_server() {
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
void do_ha_worker() {
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_ss_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_ACCEPT)
				cout << "Accept Error";
			if (ex_over->process_type == OP_CONNECT) {
				cout << "Connect Error" << endl;

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

				OVER_EXP* con_over = new OVER_EXP;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_ss_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - SERIAL_NUM_EXSERVER + HA_PORTNUM_S0;
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
			else {
				//cout << "GQCS Error ( client[" << key << "] )" << endl;
				cout << WSAGetLastError() << endl;
				int server_id = key - SERIAL_NUM_EXSERVER;
				disconnect(static_cast<int>(server_id), SESSION_EXTENDED_SERVER);
				if (ex_over->process_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			SOCKET extended_server_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
			left_ex_server_sock = extended_server_socket;
			int new_id = find_empty_extended_server();
			if (new_id != -1) {
				cout << "Sever[" << new_id << "]�� �����û�� �޾ҽ��ϴ�.\n" << endl;
				extended_servers[new_id].id = new_id;
				extended_servers[new_id].remain_size = 0;
				extended_servers[new_id].socket = extended_server_socket;
				int new_key = new_id + SERIAL_NUM_EXSERVER;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_ss_iocp, new_key, 0);
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
			break;
		}
		case OP_RECV: {
			if (0 == num_bytes) disconnect(key, SESSION_EXTENDED_SERVER);
			int server_id = key - SERIAL_NUM_EXSERVER;

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
		case OP_SEND: {
			int server_id = key - SERIAL_NUM_EXSERVER;
			if (0 == num_bytes) disconnect(server_id, SESSION_EXTENDED_SERVER);
			delete ex_over;
			break;
		}
		case OP_CONNECT: {
			if (FALSE != ret) {
				int server_id = key - SERIAL_NUM_EXSERVER;
				std::cout << "���������� Server[" << server_id << "]�� ����Ǿ����ϴ�.\n" << endl;
				extended_servers[server_id].id = server_id;
				extended_servers[server_id].remain_size = 0;
				extended_servers[server_id].socket = right_ex_server_sock;
				extended_servers[server_id].s_state = ST_ACCEPTED;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_ss_iocp, NULL, 0);
				delete ex_over;
				extended_servers[server_id].do_recv();
			}
		}
		}
	}
}

void timerFunc() {
	while (true) {
		// Helicopter
		for (auto& mv_target : clients) {
			if (mv_target.s_state != ST_INGAME) continue;

			if (mv_target.pl_state == PL_ST_ALIVE) {
				float proj_x = 0.f, proj_z = 0.f;		// proj_x�� UP���͸� x�࿡ ���翵��Ų ���翵����, proj_z�� up���͸� z�࿡ ���翵��Ų ���翵�����Դϴ�.

				// pitch�� 0�� �ƴϸ�(= ��or�ڷ� ������ �ִٸ�) ��⸦ �� �Ǵ� �ڷ� �̵���ŵ�ϴ�.
				if (mv_target.pitch != 0.f) {
					// pitch ������ ���� �̵�: proj_z = ��(y^2 + z^2) cos(90�� - pitch)
					proj_z = sqrtf(powf(mv_target.m_upvec.y, 2) + powf(mv_target.m_upvec.z, 2)) * cos(XMConvertToRadians(90 - mv_target.pitch));
					mv_target.pos = calcMove(mv_target.pos, mv_target.m_lookvec, proj_z * MOVE_SCALAR_FB);
				}

				// roll�� 0�� �ƴϸ�(= ��or��� ������ �ִٸ�) ��⸦ �� �Ǵ� ��� �̵���ŵ�ϴ�.
				if (mv_target.roll != 0.f) {
					// roll ������ ���� �̵�: proj_x = ��(x^2 + y^2) cos(90�� - roll)
					proj_x = sqrtf(powf(mv_target.m_upvec.x, 2) + powf(mv_target.m_upvec.y, 2)) * cos(XMConvertToRadians(90 - mv_target.roll));
					mv_target.pos = calcMove(mv_target.pos, mv_target.m_rightvec, -1.f * proj_x * MOVE_SCALAR_LR);
				}

				// �ٿ�� �ڽ� ������Ʈ
				mv_target.setBB();

				// ���� �̵�
				if (mv_target.pitch != 0.f || mv_target.roll != 0.f) {
					// �۵� ���� ��� Ŭ���̾�Ʈ���� �̵� ����� �˷��ݴϴ�.
					for (auto& send_target : clients) {
						if (send_target.s_state != ST_INGAME) continue;

						lock_guard<mutex> lg{ send_target.s_lock };
						send_target.send_move_packet(mv_target.id, TARGET_PLAYER);
					}
				}

				// �浹üũ
				for (auto& other_pl : clients) {
					if (other_pl.pl_state == PL_ST_DEAD) continue;		// ����� �÷��̾�ʹ� �浹�˻� X
					if (other_pl.s_state != ST_INGAME) continue;		// ���� ���� �ƴ� ����� �浹�˻� X
					if (mv_target.id <= other_pl.id) continue;			// �̹� �˻��� ��󳢸�, �ڱ��ڽŰ��� �浹�˻� X

					float dist = 0;
					float x_difference = pow(other_pl.pos.x - mv_target.pos.x, 2);
					float y_difference = pow(other_pl.pos.y - mv_target.pos.y, 2);
					float z_difference = pow(other_pl.pos.z - mv_target.pos.z, 2);
					dist = sqrtf(x_difference + y_difference + z_difference);
					if (dist > 500.f)	continue;						// �ָ� ������ �÷��̾�� �浹�˻� X

					if (mv_target.m_xoobb.Intersects(other_pl.m_xoobb)) {
						mv_target.s_lock.lock();
						mv_target.hp -= COLLIDE_PLAYER_DAMAGE;

						if (mv_target.hp <= 0) {
							mv_target.pl_state = PL_ST_DEAD;
							mv_target.death_time = chrono::system_clock::now();

							// ����� �÷��̾�� ���ӿ��� ����� �˸��ϴ�.
							SC_PLAYER_STATE_PACKET hpzero_packet;
							hpzero_packet.size = sizeof(SC_PLAYER_STATE_PACKET);
							hpzero_packet.id = mv_target.id;
							hpzero_packet.type = SC_PLAYER_STATE;
							hpzero_packet.state = ST_PACK_DEAD;

							mv_target.do_send(&hpzero_packet);
						}
						else {
							// �浹�� �÷��̾�� �浹 ����� �˸��ϴ�.
							SC_DAMAGED_PACKET damaged_by_player_packet;
							damaged_by_player_packet.size = sizeof(SC_DAMAGED_PACKET);
							damaged_by_player_packet.target = TARGET_PLAYER;
							damaged_by_player_packet.id = mv_target.id;
							damaged_by_player_packet.type = SC_DAMAGED;
							damaged_by_player_packet.dec_hp = COLLIDE_PLAYER_DAMAGE;
							damaged_by_player_packet.col_pos_x = mv_target.pos.x;
							damaged_by_player_packet.col_pos_y = mv_target.pos.y;
							damaged_by_player_packet.col_pos_z = mv_target.pos.z;

							mv_target.do_send(&damaged_by_player_packet);
						}
						mv_target.s_lock.unlock();

						other_pl.s_lock.lock();
						other_pl.hp -= COLLIDE_PLAYER_DAMAGE;

						if (other_pl.hp <= 0) {
							other_pl.pl_state = PL_ST_DEAD;
							other_pl.death_time = chrono::system_clock::now();

							// ����� �÷��̾�� ���ӿ��� ����� �˸��ϴ�.
							SC_PLAYER_STATE_PACKET hpzero_packet;
							hpzero_packet.size = sizeof(SC_PLAYER_STATE_PACKET);
							hpzero_packet.id = other_pl.id;
							hpzero_packet.type = SC_PLAYER_STATE;
							hpzero_packet.state = ST_PACK_DEAD;

							other_pl.do_send(&hpzero_packet);
						}
						else {
							// �浹�� �÷��̾�� �浹 ����� �˸��ϴ�.
							SC_DAMAGED_PACKET damaged_by_player_packet;
							damaged_by_player_packet.size = sizeof(SC_DAMAGED_PACKET);
							damaged_by_player_packet.target = TARGET_PLAYER;
							damaged_by_player_packet.id = other_pl.id;
							damaged_by_player_packet.type = SC_DAMAGED;
							damaged_by_player_packet.dec_hp = COLLIDE_PLAYER_DAMAGE;
							damaged_by_player_packet.col_pos_x = other_pl.pos.x;
							damaged_by_player_packet.col_pos_y = other_pl.pos.y;
							damaged_by_player_packet.col_pos_z = other_pl.pos.z;

							other_pl.do_send(&damaged_by_player_packet);
						}
						other_pl.s_lock.unlock();
					}//if (pl.m_xoobb.Intersects(other_pl.m_xoobb)) end
				}//for (auto& other_pl : clients) end
			}
			else if (mv_target.pl_state == PL_ST_DEAD) {
				if (chrono::system_clock::now() > mv_target.death_time + milliseconds(RESPAWN_TIME)) {	// ������ �ð��� ������
					mv_target.s_lock.lock();

					mv_target.pl_state = PL_ST_ALIVE;
					mv_target.hp = 100;
					mv_target.bullet = 100;
					mv_target.pos.x = 1500 + mv_target.id * 50;
					mv_target.pos.y = 400 + mv_target.id * 20;
					mv_target.pos.z = 1265 - mv_target.id * 50;
					mv_target.pitch = mv_target.yaw = mv_target.roll = 0.0f;
					mv_target.m_rightvec = basic_coordinate.right;
					mv_target.m_upvec = basic_coordinate.up;
					mv_target.m_lookvec = basic_coordinate.look;
					mv_target.setBB();

					// ��Ȱ�� �÷��̾�� ��Ȱ ����� �˸��ϴ�.
					SC_PLAYER_STATE_PACKET revival_packet;
					revival_packet.size = sizeof(SC_PLAYER_STATE_PACKET);
					revival_packet.id = mv_target.id;
					revival_packet.type = SC_PLAYER_STATE;
					revival_packet.state = ST_PACK_REVIVAL;

					mv_target.do_send(&revival_packet);

					// ��Ȱ�������� �̵��� ����� ��� Ŭ���̾�Ʈ�鿡�� �˸��ϴ�.
					for (auto& cl : clients) {
						if (cl.s_state == ST_INGAME) {
							cl.send_move_packet(mv_target.id, TARGET_PLAYER);
							cl.send_rotate_packet(mv_target.id, TARGET_PLAYER);
						}
					}

					mv_target.s_lock.unlock();
				}
				else {	// ���� ������ ������϶�
					mv_target.s_lock.lock();

					if (mv_target.pos.y > 100) {
						// �������鼭 ���ۺ��۵��� ����
						mv_target.pos.y -= 0.0002;

						mv_target.yaw += 0.005f;
						mv_target.m_rightvec = calcRotate(basic_coordinate.right, mv_target.roll, mv_target.yaw, mv_target.pitch);
						mv_target.m_upvec = calcRotate(basic_coordinate.up, mv_target.roll, mv_target.yaw, mv_target.pitch);
						mv_target.m_lookvec = calcRotate(basic_coordinate.look, mv_target.roll, mv_target.yaw, mv_target.pitch);

						for (auto& cl : clients) {
							if (cl.s_state == ST_INGAME) {
								cl.send_move_packet(mv_target.id, TARGET_PLAYER);
								cl.send_rotate_packet(mv_target.id, TARGET_PLAYER);
							}
						}
					}

					mv_target.s_lock.unlock();
				}
			}
		}

		// Bullet
		for (auto& bullet : bullets_arr) {
			if (bullet.getId() == -1) continue;

			if (bullet.calcDistance(bullet.getInitialPos()) > BULLET_RANGE) {	// ���� �Ѿ��� �ʱ� ��ġ�κ��� �ָ� �������ٸ� �����մϴ�.
				// ��ü ������Ŷ�� ��� Ŭ���̾�Ʈ���� �����ϴ�.
				SC_REMOVE_OBJECT_PACKET remove_bullet_packet;
				remove_bullet_packet.target = TARGET_BULLET;
				remove_bullet_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
				remove_bullet_packet.id = bullet.getId();
				remove_bullet_packet.type = SC_REMOVE_OBJECT;

				for (auto& cl : clients) {
					if (cl.s_state == ST_INGAME)
						cl.do_send(&remove_bullet_packet);
				}

				bullet.clear();
			}
			else {
				bullet.moveObj(bullet.getLookvector(), BULLET_MOVE_SCALAR);		// �Ѿ��� ������ �̵���ŵ�ϴ�.
				bullet.setBB_ex(XMFLOAT3(vulcan_bullet_bbsize_x, vulcan_bullet_bbsize_y, vulcan_bullet_bbsize_z));	// �ٿ���ڽ� ������Ʈ

				// �浹�˻�
				for (auto& pl : clients) {
					if (bullet.getOwner() == pl.id) continue;							// ���� �� ����� �浹üũ X
					if (bullet.calcDistance(pl.pos) > BULLET_RANGE)	continue;			// �Ѿ� ��Ÿ����� �ָ� ������ �÷��̾�� �浹üũ X

					if (bullet.intersectsCheck(pl.m_xoobb)) {
						// �켱 �Ѿ� ��ü�� ���ְ�
						SC_REMOVE_OBJECT_PACKET remove_bullet_packet;
						remove_bullet_packet.target = TARGET_BULLET;
						remove_bullet_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
						remove_bullet_packet.id = bullet.getId();
						remove_bullet_packet.type = SC_REMOVE_OBJECT;

						for (auto& cl : clients) {
							if (cl.s_state == ST_INGAME)
								cl.do_send(&remove_bullet_packet);
						}

						// �浹�� �÷��̾��� HP�� ���ҽ�ŵ�ϴ�.
						pl.s_lock.lock();
						pl.hp -= BULLET_DAMAGE;
						pl.s_lock.unlock();

						if (pl.hp <= 0) {
							pl.pl_state = PL_ST_DEAD;
							pl.death_time = chrono::system_clock::now();

							// ����� �÷��̾�� ���ӿ��� ����� �˸��ϴ�.
							SC_PLAYER_STATE_PACKET hpzero_packet;
							hpzero_packet.size = sizeof(SC_PLAYER_STATE_PACKET);
							hpzero_packet.id = pl.id;
							hpzero_packet.type = SC_PLAYER_STATE;
							hpzero_packet.state = ST_PACK_DEAD;

							pl.do_send(&hpzero_packet);
						}
						else {
							// �浹�� �÷��̾�� �浹 ����� �˸��ϴ�.
							SC_DAMAGED_PACKET damaged_by_bullet_packet;
							damaged_by_bullet_packet.size = sizeof(SC_DAMAGED_PACKET);
							damaged_by_bullet_packet.target = TARGET_PLAYER;
							damaged_by_bullet_packet.id = pl.id;
							damaged_by_bullet_packet.type = SC_DAMAGED;
							damaged_by_bullet_packet.dec_hp = BULLET_DAMAGE;
							damaged_by_bullet_packet.col_pos_x = pl.pos.x;
							damaged_by_bullet_packet.col_pos_y = pl.pos.y;
							damaged_by_bullet_packet.col_pos_z = pl.pos.z;

							pl.do_send(&damaged_by_bullet_packet);
						}

						// ���������� �Ѿ��� ������ �ʱ�ȭ�մϴ�.
						bullet.clear();
					}
				}

				// �̵��� �Ѿ��� ��ġ�� ��� Ŭ���̾�Ʈ���� �����մϴ�.
				SC_MOVE_OBJECT_PACKET move_bullet_packet;
				move_bullet_packet.target = TARGET_BULLET;
				move_bullet_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
				move_bullet_packet.id = bullet.getId();
				move_bullet_packet.type = SC_MOVE_OBJECT;
				move_bullet_packet.x = bullet.getPos().x;
				move_bullet_packet.y = bullet.getPos().y;
				move_bullet_packet.z = bullet.getPos().z;

				for (auto& cl : clients) {
					if (cl.s_state == ST_INGAME)
						cl.do_send(&move_bullet_packet);
				}
			}
		}
	}
}

void sendHeartBeat() {	// ������ �������� Heartbeat�� �����ϴ� �Լ�
	while (true) {
		if (my_server_id != MAX_SERVER - 1) {	// ���� ������ ������ ������ �����ϱ� ������ ���� ������ ������ �������� �ʽ��ϴ�.
			if (extended_servers[my_server_id].s_state != ST_ACCEPTED)	continue;
			if (extended_servers[my_server_id + 1].s_state != ST_ACCEPTED) continue;

			if (chrono::system_clock::now() > extended_servers[my_server_id].heartbeat_send_time + chrono::milliseconds(HB_SEND_CYCLE)) {
				cout << "�ڽ��� Heartbeat�� Server[" << my_server_id + 1 << "]���� �����ϴ�." << endl;
				SS_HEARTBEAT_PACKET hb_packet;
				hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
				hb_packet.type = SS_HEARTBEAT;
				hb_packet.sender_id = my_server_id;
				extended_servers[my_server_id + 1].do_send(&hb_packet);	// ������ ������ �����մϴ�.

				extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// ������ �ð��� ������Ʈ
			}
		}
	}
}
void checkHeartbeat() {	// �������ִ� �������������Լ� Heartbeat�� �޾Ҵ� �� Ȯ���ϴ� �Լ�
	while (true) {
		if (my_server_id != 0) {	// ���� ���ʿ� �ִ� ���� �������� ����
			// �������� ���� �˻�
			if (extended_servers[my_server_id - 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id - 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id - 1 << "]���� Heartbeat�� �������� ���� ���߽��ϴ�. ���� �ٿ����� �����մϴ�." << endl;
					disconnect(my_server_id - 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		if (my_server_id != MAX_SERVER - 1) {	// ���� �����ʿ� �ִ� ���� �������� ����
			// ������ ���� �˻�
			if (extended_servers[my_server_id + 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id + 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id + 1 << "]���� Heartbeat�� �������� ���� ���߽��ϴ�. ���� �ٿ����� �����մϴ�." << endl;
					disconnect(my_server_id + 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		Sleep(1000);
	}
}


void init_npc()
{
	for (int i{}; i < MAX_NPCS; i++) {
		int npc_id = i;
		npcs[i].SetID(npc_id);
		npcs[i].SetNpcType(NPC_Helicopter);
		npcs[i].SetRotate(0.0f, 0.0f, 0.0f);
		npcs[i].SetActive(false);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float>AirHigh(350, 650);
		uniform_real_distribution<float>AirPos(2500, 3500);

		npcs[i].SetPosition(AirPos(dre), AirHigh(dre), AirPos(dre));
		npcs[i].SetOrgPosition(npcs[i].GetPosition());

		uniform_real_distribution<float>rTheta(1.2f, 3.0f);
		npcs[i].SetTheta(rTheta(dre));
		npcs[i].SetAcc(npcs[i].GetTheta());

		uniform_int_distribution<int>rRange(15, 30);
		npcs[i].SetRange(rRange(dre));
	}
}
void MoveNPC()
{
	while (true) {
		for (int i = 0; i < MAX_NPCS; i++) {
			npcs[i].MovetoRotate();

			// Move Send
			SC_MOVE_OBJECT_PACKET npc_move_packet;
			npc_move_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
			npc_move_packet.type = SC_MOVE_OBJECT;

			npc_move_packet.target = TARGET_NPC;
			npc_move_packet.id = npcs[i].GetID();
			npc_move_packet.x = npcs[i].GetPosition().x;
			npc_move_packet.y = npcs[i].GetPosition().y;
			npc_move_packet.z = npcs[i].GetPosition().z;

			for (auto& send_target : clients) {
				lock_guard<mutex> lg{ send_target.s_lock };
				if (send_target.s_state == ST_INGAME) {
					send_target.do_send(&npc_move_packet);
				}
			}

			// Rotate Send
			SC_ROTATE_OBJECT_PACKET npc_rotate_packet;
			npc_rotate_packet.size = sizeof(SC_ROTATE_OBJECT_PACKET);
			npc_rotate_packet.type = SC_ROTATE_OBJECT;

			npc_rotate_packet.target = TARGET_NPC;
			npc_rotate_packet.id = npcs[i].GetID();
			npc_rotate_packet.right_x = npcs[i].GetCurr_coordinate().right.x;
			npc_rotate_packet.right_y = npcs[i].GetCurr_coordinate().right.y;
			npc_rotate_packet.right_z = npcs[i].GetCurr_coordinate().right.z;

			npc_rotate_packet.up_x = npcs[i].GetCurr_coordinate().up.x;
			npc_rotate_packet.up_y = npcs[i].GetCurr_coordinate().up.y;
			npc_rotate_packet.up_z = npcs[i].GetCurr_coordinate().up.z;

			npc_rotate_packet.look_x = npcs[i].GetCurr_coordinate().look.x;
			npc_rotate_packet.look_y = npcs[i].GetCurr_coordinate().look.y;
			npc_rotate_packet.look_z = npcs[i].GetCurr_coordinate().look.z;

			for (auto& send_target : clients) {
				lock_guard<mutex> lg{ send_target.s_lock };
				if (send_target.s_state == ST_INGAME) {
					send_target.do_send(&npc_rotate_packet);
				}
			}
		}
	}
}


int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	//======================================================================
	// [ HA - ���� ID, ��Ʈ��ȣ ���� ]
	// � ������ ������ �������� ����� �μ��� �Է¹޾Ƽ� �� ������ ��Ʈ ��ȣ�� �ο��մϴ�.
	my_server_id = 0;		// ���� ������ ���� ������ȣ
	int sc_portnum = -1;	// Ŭ���̾�Ʈ ��ſ� ��Ʈ��ȣ
	int ss_portnum = -1;	// ���� �� ��ſ� ��Ʈ��ȣ
	if (argc > 1) {			// �Էµ� ����� �μ��� �ִٸ�...
		my_server_id = atoi(argv[1]);
	}
	else {				// ���� �Էµ� ����� �μ��� ���ٸ� ����Ʈ��(��Ʈ��ȣ 9000)���� ����˴ϴ�.
		cout << "������ ����: ";
		cin >> my_server_id;
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
	cout << "Server[" << my_server_id << "] �� �����Ǿ����ϴ�. [ S-C PORT: " << sc_portnum << " / S-S PORT: " << ss_portnum << " ]" << endl;

	//======================================================================
	// [ HA - Relay���� ���� ]
	int active_relay_serverid = 0;							// [[[���Ŀ� ���� Active ���¿� �ִ� �����̼����� id�� ��Ʈ��ȣ�� �ٲ� ������!]]]
	int active_relay_portnum = PORTNUM_RELAY2LOGIC_0;		//
	// Connect


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
	h_ss_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ss_listensock), h_ss_iocp, 1999, 0);
	right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP ha_over;
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

		OVER_EXP* con_over = new OVER_EXP;
		con_over->process_type = OP_CONNECT;
		int key_num = SERIAL_NUM_EXSERVER + right_servernum;
		HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_ss_iocp, key_num, 0);
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

	//======================================================================
	// [ Main - Ŭ���̾�Ʈ ���� ]
	// Client Listen Socket (Ŭ���̾�Ʈ-���� ����� ���� Listen����)
	g_sc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM_S0);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_sc_listensock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_sc_listensock, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	// CLient Accept
	h_sc_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_sc_listensock), h_sc_iocp, 999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_sc_listensock, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);		// Ŭ���̾�Ʈ-���� ��ſ� Worker������
	worker_threads.emplace_back(do_ha_worker);		// ���� �� ��ſ� Worker������

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);			// Ŭ���̾�Ʈ ���� Ÿ�̸ӽ�����
	timer_threads.emplace_back(sendHeartBeat);		// ���� �� Heartbeat��ȯ ������
	timer_threads.emplace_back(checkHeartbeat);		// ���� �� �ٿ�� �˻� ������
	timer_threads.emplace_back(MoveNPC);			// NPC ���� ������

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_sc_listensock);
	WSACleanup();
}
