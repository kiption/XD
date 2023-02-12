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
enum PLAYER_STATE { PL_ST_ALIVE, PL_ST_DEAD };

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
	int remain_size;
	char name[NAME_SIZE];

	PLAYER_STATE pl_state;
	int hp;
	int bullet;
	XMFLOAT3 pos;								// Position (x, y, z)
	float pitch, yaw, roll;						// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;	// 현재 Look, Right, Up Vectors
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

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(1.5f, 1.5f, 1.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(1.5f, 1.5f, 1.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
};

int online_player_cnt = 0;
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

	login_info_packet.hp = hp;
	login_info_packet.remain_bullet = bullet;

	//cout << "[SC_LOGIN_INFO]";
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

	//cout << "[SC_MOVE]";
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

	//cout << "[SC_ROTATE]";
	do_send(&rotate_pl_packet);
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

	online_player_cnt--;
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
			disconnect(client_id);
			break;
		}

		// 새로 접속한 플레이어의 초기 정보를 설정합니다.
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
			//cout << "[SC_ADD]";
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
			//cout << "[SC_ADD]";
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
					clients[client_id].yaw += sign * YAW_ROTATE_SCALAR;

					// right, up, look 벡터 업데이트
					clients[client_id].m_rightvec = calcRotate(basic_coordinate.right
						, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
					clients[client_id].m_upvec = calcRotate(basic_coordinate.up
						, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
					clients[client_id].m_lookvec = calcRotate(basic_coordinate.look
						, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);

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

					// 이동 계산 & 결과 업데이트
					XMFLOAT3 move_result = calcMove(clients[client_id].pos, clients[client_id].m_upvec, ENGINE_SCALAR * sign);
					clients[client_id].pos = move_result;

					// 바운딩 박스 업데이트
					clients[client_id].setBB();

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
					if (clients[client_id].bullet > 0) {		// 남은 총알이 있을 때에만
						while (arr_cnt < MAX_BULLET) {
							if (bullets_arr[arr_cnt].getId() == -1) {
								new_bullet_id = arr_cnt;

								clients[client_id].s_lock.lock();
								// 총알 하나 사용
								clients[client_id].bullet -= 1;

								// 발사한 사용자에게 총알 사용했음을 알려줍니다.
								SC_BULLET_COUNT_PACKET bullet_packet;
								bullet_packet.size = sizeof(bullet_packet);
								bullet_packet.type = SC_BULLET_COUNT;
								bullet_packet.id = client_id;
								bullet_packet.bullet_cnt = clients[client_id].bullet;
								clients[client_id].do_send(&bullet_packet);

								clients[client_id].s_lock.unlock();
								break;
							}
							else {
								arr_cnt++;
							}
						}

						// 벡터에 남아있는 공간이 있을 때에만 발사합니다.
						if (0 <= new_bullet_id && new_bullet_id < MAX_BULLET) {
							// shoot time update
							shoot_time = chrono::system_clock::now();

							// Bullet 생성
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

							for (auto& pl : clients) {
								if (pl.s_state == ST_INGAME)
									pl.do_send(&add_bullet_packet);
							}
						}
					}
					else {	// 남은 탄환이 0이라면 reload
						clients[client_id].s_lock.lock();

						clients[client_id].bullet = 100;

						// 발사한 사용자에게 총알 장전했음을 알려줍니다.
						SC_BULLET_COUNT_PACKET bullet_packet;
						bullet_packet.size = sizeof(bullet_packet);
						bullet_packet.type = SC_BULLET_COUNT;
						bullet_packet.id = client_id;
						bullet_packet.bullet_cnt = clients[client_id].bullet;
						clients[client_id].do_send(&bullet_packet);

						clients[client_id].s_lock.unlock();
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

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->key_val == RT_LBUTTON) {			// 마우스 좌클릭 드래그
			float rotate_scalar = 0.0f;

			clients[client_id].s_lock.lock();

			if (fabs(rt_p->delta_x) < fabs(rt_p->delta_y)) {	// 마우스 상,하 드래그: 기수를 조절합니다. 기체를 x축 기준 회전시킵니다.
				rotate_scalar = -1.f * rt_p->delta_y * PITCH_ROTATE_SCALAR * SENSITIVITY;
				if (fabs(clients[client_id].pitch + rotate_scalar) < PITCH_LIMIT) {		// 비정상적인 회전 방지
					clients[client_id].pitch += rotate_scalar;							// pitch 설정
				}
			}
			else {												// 마우스 좌,우 드래그: 기수의 수평을 조절합니다. 기체를 z축 기준으로 회전시킵니다.
				rotate_scalar = -1.f * rt_p->delta_x * ROLL_ROTATE_SCALAR * SENSITIVITY;
				if (fabs(clients[client_id].roll + rotate_scalar) < ROLL_LIMIT) {		// 비정상적인 회전 방지
					clients[client_id].roll += rotate_scalar;							// roll 설정
				}
			}

			// right, up, look 벡터 회전 & 업데이트
			clients[client_id].m_rightvec = calcRotate(basic_coordinate.right, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
			clients[client_id].m_upvec = calcRotate(basic_coordinate.up, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);
			clients[client_id].m_lookvec = calcRotate(basic_coordinate.look, clients[client_id].pitch, clients[client_id].yaw, clients[client_id].roll);

			clients[client_id].s_lock.unlock();

			// server message
			cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is Rotated. "
				<< "Pitch: " << clients[client_id].pitch << ", Yaw: " << clients[client_id].yaw << ", Roll: " << clients[client_id].roll << endl;

			// 작동 중인 모든 클라이언트에게 회전 결과를 알려줍니다.
			for (auto& send_target : clients) {
				lock_guard<mutex> lg{ send_target.s_lock };
				if (send_target.s_state == ST_INGAME) {
					send_target.send_rotate_packet(client_id, TARGET_PLAYER);
				}
			}
		}
		else if (rt_p->key_val == RT_RBUTTON) {		// 마우스 우클릭 드래그: 기능 미정.
			//clients[client_id].s_lock.lock();
			cout << "마우스 우클릭 입력됨." << endl;// 임시코드
			//clients[client_id].s_lock.unlock();
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
				//cout << "GQCS Error ( client[" << key << "] )" << endl;
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
		// Helicopter
		for (auto& mv_target : clients) {
			if (mv_target.s_state != ST_INGAME) continue;
			if (mv_target.pl_state == PL_ST_DEAD) continue;

			float proj_x = 0.f, proj_z = 0.f;		// proj_x는 UP벡터를 x축에 정사영시킨 정사영벡터, proj_z는 up벡터를 z축에 정사영시킨 정사영벡터입니다.

			// pitch가 0이 아니면(= 앞or뒤로 기울어져 있다면) 헬기를 앞 또는 뒤로 이동시킵니다.
			if (mv_target.pitch != 0.f) {
				// pitch 각도에 따른 이동: proj_z = √(y^2 + z^2) cos(90˚ - pitch)
				proj_z = sqrtf(powf(mv_target.m_upvec.y, 2) + powf(mv_target.m_upvec.z, 2)) * cos(XMConvertToRadians(90 - mv_target.pitch));
				mv_target.pos = calcMove(mv_target.pos, mv_target.m_lookvec, proj_z * MOVE_SCALAR_FB);
			}

			// roll이 0이 아니면(= 좌or우로 기울어져 있다면) 헬기를 좌 또는 우로 이동시킵니다.
			if (mv_target.roll != 0.f) {
				// roll 각도에 따른 이동: proj_x = √(x^2 + y^2) cos(90˚ - roll)
				proj_x = sqrtf(powf(mv_target.m_upvec.x, 2) + powf(mv_target.m_upvec.y, 2)) * cos(XMConvertToRadians(90 - mv_target.roll));
				mv_target.pos = calcMove(mv_target.pos, mv_target.m_rightvec, -1.f * proj_x * MOVE_SCALAR_LR);
			}

			// 바운딩 박스 업데이트
			mv_target.setBB();

			// 최종 이동
			if (mv_target.pitch != 0.f || mv_target.roll != 0.f) {
				//cout << "Player[" << mv_target.id << "] moves to pos(" << mv_target.pos.x << ", " << mv_target.pos.y << ", " << mv_target.pos.z << ").";

				// 작동 중인 모든 클라이언트에게 이동 결과를 알려줍니다.
				for (auto& send_target : clients) {
					if (send_target.s_state != ST_INGAME) continue;

					lock_guard<mutex> lg{ send_target.s_lock };
					send_target.send_move_packet(mv_target.id, TARGET_PLAYER);
				}
			}

			// 충돌검사
			for (auto& pl : clients) {
				if (pl.s_state != ST_INGAME) continue;
				if (pl.pl_state == PL_ST_DEAD) continue;

				for (auto& other_pl : clients) {
					// 사망한 플레이어와는 충돌검사 X
					if (other_pl.pl_state == PL_ST_DEAD) continue;
					// 접속 중이 아닌 대상을 충돌검사 X
					if (other_pl.s_state != ST_INGAME) continue;
					// 이미 검사한 대상끼리, 자기자신과는 충돌검사 X
					if (pl.id <= other_pl.id) continue;
					// 멀리 떨어진 플레이어는 충돌검사 X
					float dist = 0;
					float x_difference = pow(other_pl.pos.x - pl.pos.x, 2);
					float y_difference = pow(other_pl.pos.y - pl.pos.y, 2);
					float z_difference = pow(other_pl.pos.z - pl.pos.z, 2);
					dist = sqrtf(x_difference + y_difference + z_difference);
					if (dist > 500.f)	continue;

					if (pl.m_xoobb.Intersects(other_pl.m_xoobb)) {
						pl.s_lock.lock();
						pl.hp -= COLLIDE_PLAYER_DAMAGE;

						if (pl.hp <= 0) {
							pl.pl_state = PL_ST_DEAD;
							pl.death_time = chrono::system_clock::now();

							cout << "Player[" << pl.id << "] is Killed by Player[" << other_pl.id << "]!" << endl; //server message

							// 사망한 플레이어에게 게임오버 사실을 알립니다.
							SC_PLAYER_STATE_PACKET hpzero_packet;
							hpzero_packet.size = sizeof(SC_PLAYER_STATE_PACKET);
							hpzero_packet.id = pl.id;
							hpzero_packet.type = SC_PLAYER_STATE;
							hpzero_packet.state = ST_PACK_DEAD;

							pl.do_send(&hpzero_packet);
						}
						else {
							cout << "Player[" << pl.id << "] is Damaged by Player[" << other_pl.id << "]!" << endl; //server message

							// 충돌한 플레이어에게 충돌 사실을 알립니다.
							SC_HP_COUNT_PACKET damaged_packet;
							damaged_packet.size = sizeof(SC_HP_COUNT_PACKET);
							damaged_packet.id = pl.id;
							damaged_packet.type = SC_HP_COUNT;
							damaged_packet.hp = pl.hp;
							damaged_packet.change_cause = CAUSE_DAMAGED_BY_PLAYER;

							pl.do_send(&damaged_packet);
						}

						pl.s_lock.unlock();
					}//if (pl.m_xoobb.Intersects(other_pl.m_xoobb)) end
				}
			}
		}

		// Bullet
		for (auto& bullet : bullets_arr) {
			if (bullet.getId() == -1) continue;

			if (bullet.calcDistance(bullet.getInitialPos()) > BULLET_RANGE) {	// 만약 총알이 초기 위치로부터 멀리 떨어졌다면 제거합니다.
				//cout << "[" << bullet.getId() << "]번 총알을 제거합니다.";
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

				// 충돌검사
				for (auto& pl : clients) {
					if (bullet.calcDistance(pl.pos) > BULLET_MOVE_SCALAR)	continue;	// 총알 사거리보다 멀리 떨어진 플레이어는 충돌체크 X

					if (bullet.intersectsCheck(pl.m_xoobb)) {
						pl.s_lock.lock();
						pl.hp -= BULLET_DAMAGE;
						pl.s_lock.unlock();

						if (pl.hp <= 0) {
							pl.pl_state = PL_ST_DEAD;
							pl.death_time = chrono::system_clock::now();

							cout << "Player[" << pl.id << "] is Killed by Player[" << bullet.getOwner() << "]'s Bullet!" << endl; //server message

							// 사망한 플레이어에게 게임오버 사실을 알립니다.
							SC_PLAYER_STATE_PACKET hpzero_packet;
							hpzero_packet.size = sizeof(SC_PLAYER_STATE_PACKET);
							hpzero_packet.id = pl.id;
							hpzero_packet.type = SC_PLAYER_STATE;
							hpzero_packet.state = ST_PACK_DEAD;

							pl.do_send(&hpzero_packet);
						}
						else {
							cout << "Player[" << pl.id << "] is Damaged by Bullet!" << endl; //server message

							// 충돌한 플레이어에게 충돌 사실을 알립니다.
							SC_HP_COUNT_PACKET damaged_packet;
							damaged_packet.size = sizeof(SC_HP_COUNT_PACKET);
							damaged_packet.id = pl.id;
							damaged_packet.type = SC_HP_COUNT;
							damaged_packet.hp = pl.hp;
							damaged_packet.change_cause = CAUSE_DAMAGED_BY_BULLET;

							pl.do_send(&damaged_packet);
						}
					}
				}

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

		/*
		// NPC
		for (int i = 0; i < MAX_NPCS; i++) {
			npcs[i].MovetoRotate();

			// Move Send
			SC_MOVE_OBJECT_PACKET npc_move_packet;
			npc_move_packet.target = TARGET_NPC;
			npc_move_packet.id = npcs[i].GetID();
			npc_move_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
			npc_move_packet.type = SC_MOVE_OBJECT;

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
			npc_rotate_packet.target = TARGET_NPC;
			npc_rotate_packet.id = npcs[i].GetID();
			npc_rotate_packet.size = sizeof(SC_ROTATE_OBJECT_PACKET);
			npc_rotate_packet.type = SC_ROTATE_OBJECT;

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

			cout << "NPC[" << npcs[i].GetID() << "] moves to POS("
				<< npcs[i].GetPosition().x << ", " << npcs[i].GetPosition().y << ", " << npcs[i].GetPosition().z << ")" << endl;
		}
		*/
	}
}

void MoveNPC()
{
	while (true) {
		for (int i = 0; i < MAX_NPCS; i++) {
			npcs[i].MovetoRotate();

			// Move Send
			SC_MOVE_OBJECT_PACKET npc_move_packet;
			npc_move_packet.target = TARGET_NPC;
			npc_move_packet.id = npcs[i].GetID();
			npc_move_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
			npc_move_packet.type = SC_MOVE_OBJECT;

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
			npc_rotate_packet.target = TARGET_NPC;
			npc_rotate_packet.id = npcs[i].GetID();
			npc_rotate_packet.size = sizeof(SC_ROTATE_OBJECT_PACKET);
			npc_rotate_packet.type = SC_ROTATE_OBJECT;

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

			cout << "NPC[" << npcs[i].GetID() << "] moves to POS("
				<< npcs[i].GetPosition().x << ", " << npcs[i].GetPosition().y << ", " << npcs[i].GetPosition().z << ")" << endl;
		}
	}
}


int main()
{
	init_npc();
	shoot_time = chrono::system_clock::now();

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

	thread NPC_thread{ MoveNPC };
	NPC_thread.join();

	for (auto& th : worker_threads)
		th.join();

	closesocket(g_s_socket);
	WSACleanup();
}
