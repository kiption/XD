#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <chrono>
#include <random>

#include "global.h"
#include "BulletsMgr.h"
#include "protocol.h"
#include "NPC.h"
#include "Timer.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

Coordinate basic_coordinate;	// 기본(초기) 좌표계

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
	int hp;
	XMFLOAT3 pos;								// Position (x, y, z)
	float pitch, yaw, roll;						// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;	// 현재 Look, Right, Up Vectors
	char name[NAME_SIZE];
	int remain_size;

public:
	SESSION()
	{
		id = -1;
		socket = 0;
		hp = 100;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };
		name[0] = 0;
		s_state = ST_FREE;
		remain_size = 0;
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

		cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void send_login_info_packet();
	void send_move_packet(int client_id, short move_target);
	void send_rotate_packet(int client_id, short rotate_target);
};

array<SESSION, MAX_USER + MAX_NPCS> clients;		// 0 ~ MAX_USER-1: Player,	 MAX_USER ~ MAX_USER+MAX_NPCS: NPC
array<NPC, MAX_NPCS> npcs;

array<Bullets, MAX_BULLET> bullets_arr;
chrono::system_clock::time_point shoot_time;

HANDLE g_h_iocp;
SOCKET g_s_socket;

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

	cout << "[SC_LOGIN_INFO]";
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

	cout << "[SC_MOVE]";
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

	cout << "[SC_ROTATE]";
	do_send(&rotate_pl_packet);
}

void init_npc()
{
	for (int i{}; i < MAX_NPCS; i++) {
		int npc_id = 7001 + i;
		npcs[i].SetID(npc_id);
		npcs[i].SetNpcType(NPC_Helicopter);
		npcs[i].SetRotate(0.0f, 0.0f, 0.0f);
		npcs[i].SetActive(false);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float>AirHigh(1400, 1600);
		uniform_real_distribution<float>AirPos(900, 2000);

		npcs[i].SetPosition(AirPos(dre), AirHigh(dre), AirPos(dre));
		npcs[i].SetOrgPosition(npcs[i].GetPosition());

		uniform_real_distribution<float>rTheta(1.2f, 3.0f);
		npcs[i].SetTheta(rTheta(dre));
		npcs[i].SetAcc(npcs[i].GetTheta());

		uniform_int_distribution<int>rRange(15, 30);
		npcs[i].SetRange(rRange(dre));
	}
}

void disconnect(int client_id)
{
	clients[client_id].s_lock.lock();
	if (clients[client_id].s_state == ST_FREE) {
		clients[client_id].s_lock.unlock();
		return;
	}
	closesocket(clients[client_id].socket);
	clients[client_id].s_state = ST_FREE;
	clients[client_id].s_lock.unlock();

	cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << " is log out" << endl;	// server message

	for (int i = 0; i < MAX_USER; i++) {
		auto& pl = clients[i];

		if (pl.id == client_id) continue;

		pl.s_lock.lock();
		if (pl.s_state != ST_INGAME) {
			pl.s_lock.unlock();
			continue;
		}
		SC_REMOVE_OBJECT_PACKET remove_pl_packet;
		remove_pl_packet.target = TARGET_PLAYER;
		remove_pl_packet.id = client_id;
		remove_pl_packet.size = sizeof(remove_pl_packet);
		remove_pl_packet.type = SC_REMOVE_OBJECT;
		cout << "[SC_REMOVE]";
		pl.do_send(&remove_pl_packet);
		pl.s_lock.unlock();
	}
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].s_lock.lock();
		if (clients[i].s_state == ST_FREE) {
			clients[i].s_state = ST_ACCEPTED;
			clients[i].s_lock.unlock();
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
			disconnect(client_id);
			break;
		}

		// 새로 접속한 플레이어의 초기 정보를 설정합니다.
		clients[client_id].pos.x = 640 + client_id * 50;
		clients[client_id].pos.y = 1400;
		clients[client_id].pos.z = 1165 - client_id * 50;
		cout << "A new object is successfully created! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ")." << endl;

		clients[client_id].pitch = clients[client_id].yaw = clients[client_id].roll = 0.0f;
		clients[client_id].m_rightvec = basic_coordinate.right;
		clients[client_id].m_upvec = basic_coordinate.up;
		clients[client_id].m_lookvec = basic_coordinate.look;

		strcpy_s(clients[client_id].name, login_packet->name);

		clients[client_id].send_login_info_packet();
		clients[client_id].s_state = ST_INGAME;
		clients[client_id].s_lock.unlock();

		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is log in" << endl;	// server message

		for (int i = 0; i < MAX_USER; ++i) {		// 현재 접속해 있는 모든 클라이언트에게 새로운 클라이언트(client_id)의 정보를 전송합니다.
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

			cout << "Send new client's info to client[" << pl.id << "]." << endl;
			cout << "[SC_ADD]";
			pl.do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}

		for (auto& pl : clients) {		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 클라이언트와 NPC들의 정보를 전송합니다.
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

			cout << "Send client[" << pl.id << "]'s info to new client" << endl;
			cout << "[SC_ADD]";
			clients[client_id].do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}
		break;
	}// CS_LOGIN end
	case CS_INPUT_KEYBOARD: {
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		enum InputKey { KEY_Q, KEY_E, KEY_A, KEY_D, KEY_S, KEY_W, KEY_SPACEBAR };

		for (int i = 0; i <= 6; i++) {
			if ((inputkey_p->direction >> i) & 1) {
				float sign = 1.0f;					// right/up/look벡터 방향으로 움직이는지, 반대 방향으로 움직이는지
				switch (i) {
				case KEY_Q:
				case KEY_E:
					clients[client_id].s_lock.lock();
					// 아직 기능 없음.

					// unlock
					clients[client_id].s_lock.unlock();
					break;

				case KEY_A:			// D, A는 기체의 yaw회전 키입니다. 기체를 y축 기준으로 회전시킵니다.
					sign = -1.0f;	// A는 right벡터 반대 방향으로 움직이기 때문에 -1을 곱해줍니다.
					[[fallthrough]]
				case KEY_D:
					clients[client_id].s_lock.lock();
					// yaw 설정
					clients[client_id].yaw += 1.0f * sign * YAW_ROTATE_SCALAR * PI / 360.0f;

					// right, up, look 벡터 업데이트
					clients[client_id].m_rightvec = calcRotate(basic_coordinate.right
						, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
					clients[client_id].m_upvec = calcRotate(basic_coordinate.up
						, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
					clients[client_id].m_lookvec = calcRotate(basic_coordinate.look
						, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);

					// unlock
					clients[client_id].s_lock.unlock();

					// server message
					cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is Rotated. "
						<< "Pitch: " << clients[client_id].pitch << ", Yaw: " << clients[client_id].yaw << ", Roll: " << clients[client_id].roll << endl;

					// 작동 중인 모든 클라이언트에게 회전 결과를 알려줍니다.
					for (int j = 0; j < MAX_USER; j++) {
						auto& pl = clients[j];
						lock_guard<mutex> lg{ pl.s_lock };
						if (pl.s_state == ST_INGAME)
							pl.send_rotate_packet(client_id, TARGET_PLAYER);
					}
					break;

				case KEY_S:			// W, S는 엔진출력 조절 키입니다. 기체를 상승 또는 하강시킵니다.
					sign = -1.0f;	// S는 up벡터 반대 방향으로 움직이기 때문에 -1을 곱해줍니다.
					[[fallthrough]]
				case KEY_W:
					clients[client_id].s_lock.lock();
					// 이동 방향 설정
					XMFLOAT3 move_dir{ 0, 0, 0 };
					move_dir.x = clients[client_id].m_upvec.x * sign;
					move_dir.y = clients[client_id].m_upvec.y * sign;
					move_dir.z = clients[client_id].m_upvec.z * sign;

					// 이동 계산 & 결과 업데이트
					XMFLOAT3 move_result = calcMove(clients[client_id].pos, move_dir, ENGINE_SCALAR);
					clients[client_id].pos = move_result;

					// unlock
					clients[client_id].s_lock.unlock();

					// server message
					cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] moves to ("
						<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.z << ")." << endl;

					// 작동 중인 모든 클라이언트에게 이동 결과를 알려줍니다.
					for (int j = 0; j < MAX_USER; j++) {
						auto& pl = clients[j];
						lock_guard<mutex> lg{ pl.s_lock };
						if (pl.s_state == ST_INGAME)
							pl.send_move_packet(client_id, TARGET_PLAYER);
					}

					break;

				case KEY_SPACEBAR:	// 스페이스바는 공격 키입니다. 바라보는 방향으로 총을 쏩니다.
					// bullet lock (구현 예정)

					// Bullet Cooldown Check
					milliseconds shoot_term = duration_cast<milliseconds>(chrono::system_clock::now() - shoot_time);
					if (shoot_term < milliseconds(SHOOT_COOLDOWN_BULLET)) {	// 쿨타임이 끝나지 않았다면 발사하지 않습니다.
						milliseconds left_cooldown = duration_cast<milliseconds>(milliseconds(SHOOT_COOLDOWN_BULLET) - shoot_term);
						break;
					}

					// empty space check
					int new_bullet_id = -1;
					int arr_cnt = 0;
					while (arr_cnt < MAX_BULLET) {
						if (bullets_arr[arr_cnt].getId() == -1) {
							new_bullet_id = arr_cnt;
							break;
						}
						else {
							arr_cnt++;
						}
					}
					if (new_bullet_id == -1)
						break;

					// shoot time update
					shoot_time = chrono::system_clock::now();

					// Bullet 생성
					bullets_arr[new_bullet_id].setId(new_bullet_id);
					bullets_arr[new_bullet_id].setPos(clients[client_id].pos);
					bullets_arr[new_bullet_id].setPitch(clients[client_id].pitch);
					bullets_arr[new_bullet_id].setYaw(clients[client_id].yaw);
					bullets_arr[new_bullet_id].setRoll(clients[client_id].roll);
					bullets_arr[new_bullet_id].setRightvector(clients[client_id].m_rightvec);
					bullets_arr[new_bullet_id].setUpvector(clients[client_id].m_upvec);
					bullets_arr[new_bullet_id].setLookvector(clients[client_id].m_lookvec);
					bullets_arr[new_bullet_id].setOwner(client_id);
					bullets_arr[new_bullet_id].setInitialPos(bullets_arr[new_bullet_id].getPos());

					// 접속해있는 모든 클라이언트에게 새로운 Bullet정보를 보냅니다.
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

					// server message
					/*
					cout << "Create New Bullet [ID: " << bullets_arr[new_bullet_id].getId()
						<< ", Pos(" << bullets_arr[new_bullet_id].getPos().x
						<< ", " << bullets_arr[new_bullet_id].getPos().y
						<< ", " << bullets_arr[new_bullet_id].getPos().z << ")." << endl;
					*/

					for (auto& pl : clients) {
						if (pl.s_state == ST_INGAME)
							pl.do_send(&add_bullet_packet);
					}

					// bullet unlock (구현 예정)
					break;
				}
			}
		}

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE: {
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		clients[client_id].s_lock.lock();

		if (clients[client_id].s_state == ST_FREE) {
			clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->key_val == RT_LBUTTON) {			// 마우스 좌클릭 드래그

			XMFLOAT3 move_dir{ 0, 0, 0 };
			XMFLOAT3 move_result{ 0, 0, 0 };
			float tmp_scalar = 0.0f;

			if (fabs(rt_p->delta_x) < fabs(rt_p->delta_y)) {	// 마우스 상,하 드래그: 기수를 조절합니다.기체를 x축 기준으로 회전시키고 앞뒤로 이동합니다.
				// 1. pitch 회전
				// pitch 설정
				clients[client_id].pitch += -1.0f * rt_p->delta_y * SENSITIVITY * PI / 360.0f;

				// 비정상적인 회전 방지
				if (clients[client_id].pitch > PITCH_LIMIT * PI / 360.0f)
					clients[client_id].pitch = (PITCH_LIMIT - 1) * PI / 360.0f;
				else if (clients[client_id].pitch < -1.0f * PITCH_LIMIT * PI / 360.0f)
					clients[client_id].pitch = -1.0f * (PITCH_LIMIT - 1) * PI / 360.0f;

				// 2. z축 이동
				// 이동 관련 설정
				move_dir.x = clients[client_id].m_lookvec.x;
				move_dir.y = clients[client_id].m_lookvec.y;
				move_dir.z = clients[client_id].m_lookvec.z;

				tmp_scalar = -1.0f * rt_p->delta_y * SENSITIVITY;
			}
			else {									// 마우스 좌,우 드래그: 기수의 수평을 조절합니다.기체를 z축 기준으로 회전시키고 좌우로 이동합니다.
				// 1. roll 회전
				// roll 설정
				clients[client_id].roll += -1.0f * rt_p->delta_x * SENSITIVITY * PI / 360.0f;

				// 비정상적인 회전 방지
				if (clients[client_id].roll > ROLL_LIMIT * PI / 360.0f)
					clients[client_id].roll = (ROLL_LIMIT - 1) * PI / 360.0f;
				else if (clients[client_id].roll < -1.0f * ROLL_LIMIT * PI / 360.0f)
					clients[client_id].roll = -1.0f * (ROLL_LIMIT - 1) * PI / 360.0f;

				// 2. x축 이동
				// 이동 관련 설정
				move_dir.x = clients[client_id].m_rightvec.x;
				move_dir.y = clients[client_id].m_rightvec.y;
				move_dir.z = clients[client_id].m_rightvec.z;

				tmp_scalar = rt_p->delta_x * SENSITIVITY;
			}

			// right, up, look 벡터 회전 & 업데이트
			clients[client_id].m_rightvec = calcRotate(basic_coordinate.right, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
			clients[client_id].m_upvec = calcRotate(basic_coordinate.up, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
			clients[client_id].m_lookvec = calcRotate(basic_coordinate.look, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);

			// 이동 & 좌표 업데이트
			move_result = calcMove(clients[client_id].pos, move_dir, tmp_scalar);
			clients[client_id].pos = move_result;

		}
		else if (rt_p->key_val == RT_RBUTTON) {		// 마우스 우클릭 드래그: 기능 미정.
			cout << "마우스 우클릭 입력됨." << endl;// 임시코드

		}

		clients[client_id].s_lock.unlock();

		// server message
		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is Rotated. "
			<< "Pitch: " << clients[client_id].pitch << ", Yaw: " << clients[client_id].yaw << ", Roll: " << clients[client_id].roll << endl;
		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] moves to ("
			<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.z << ")." << endl;

		// 작동 중인 모든 클라이언트에게 이동&회전 결과를 알려줍니다.
		for (int i = 0; i < MAX_USER; i++) {
			auto& pl = clients[i];
			lock_guard<mutex> lg{ pl.s_lock };
			if (pl.s_state == ST_INGAME) {
				pl.send_rotate_packet(client_id, TARGET_PLAYER);
				pl.send_move_packet(client_id, TARGET_PLAYER);
			}
		}
		break;
	}// CS_INPUT_MOUSE end
	}
}

void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error ( client[" << key << "] )" << endl;
				disconnect(static_cast<int>(key));
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
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, client_id, 0);
				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Sever is Full" << endl;
			}

			ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
			ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			break;
		}
		case OP_RECV: {
			if (0 == num_bytes) disconnect(key);

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
		case OP_SEND:
			if (0 == num_bytes) disconnect(key);
			delete ex_over;
			break;
		}

	}
}

void timerFunc() {
	while (true) {
		// Bullet
		for (auto& bullet : bullets_arr) {
			if (bullet.getId() == -1) continue;

			if (bullet.calcDistance(bullet.getInitialPos()) > BULLET_RANGE) {	// 만약 총알이 초기 위치로부터 멀리 떨어졌다면 제거합니다.
				/*cout << "[" << bullet.getId() << "]번 총알을 제거합니다.";
				cout << "\tDist: " << bullet.calcDistance(bullet.getInitialPos())
					<< " (CurPos:" << bullet.getPos().x << "," << bullet.getPos().y << "," << bullet.getPos().z
					<< " , InitPos:" << bullet.getInitialPos().x << "," << bullet.getInitialPos().y << "," << bullet.getInitialPos().z << ")" << endl;*/
				bullet.clear();

				// 객체 제거패킷을 모든 클라이언트에게 보냅니다.
				SC_REMOVE_OBJECT_PACKET remove_bullet_packet;
				remove_bullet_packet.target = TARGET_BULLET;
				remove_bullet_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
				remove_bullet_packet.id = bullet.getId();
				remove_bullet_packet.type = SC_REMOVE_OBJECT;

				for (auto& cl : clients) {
					if (cl.s_state == ST_INGAME)
						cl.do_send(&remove_bullet_packet);
				}
			}
			else {
				// 총알을 앞으로 이동시킵니다.
				bullet.moveObj(bullet.getLookvector(), BULLET_MOVE_SCALAR);
				/*cout << "[" << bullet.getId() << "]번 총알이 Pos("
					<< bullet.getPos().x << ", " << bullet.getPos().y << ", " << bullet.getPos().z << ")로 이동하였습니다." << endl;*/

				// 이동된 총알의 위치를 모든 클라이언트에게 전달합니다.
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

int main()
{
	init_npc();
	shoot_time = chrono::system_clock::now();

	//int cnt = 0;
	//while (cnt < 100) {
	//	for (int i{}; i < MAX_NPCS; ++i) {
	//		if (i == 0) {
	//			npcs[i].MovetoRotate();
	//			cout << i << "th Pos: " << npcs[i].GetPosition().x << ", " << npcs[i].GetPosition().y << ", " << npcs[i].GetPosition().z << endl;

	//		}
	//	}
	//}

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_s_socket, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	vector <thread> worker_threads;
	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(do_worker);

	thread timer_thread{ timerFunc };
	timer_thread.join();

	for (auto& th : worker_threads)
		th.join();

	closesocket(g_s_socket);
	WSACleanup();
}
