#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <random>	// NPC의 초기 위치값 설정할 때 임시로 랜덤값을 부여하기로 함. -> 추후에 정해진 리스폰 지점에 생성되도록 변경해야함.

#include "global.h"
#include "MyVectors.h"
#include "protocol.h"
#include "Func_CalcVectors.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

// NPC를 랜덤한 위치에 생성함.
default_random_engine dre;
uniform_int_distribution<int> uid(1, 10);
// ==== NPC의 리스폰 지점을 만들고나면 없애도 됨.

Coordinate basic_coordinate;	// 기본(초기) 좌표계

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

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
	MyVector3D pos;							// Position (x, y, z)
	float pitch, yaw, roll;					// Rotated Degree
	Coordinate curr_coordinate;				// 현재 Look, Right, Up Vectors
	char name[NAME_SIZE];
	int remain_size;

public:
	SESSION()
	{
		id = -1;
		socket = 0;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		curr_coordinate.x_coordinate = { 1.0f, 0.0f, 0.0f };
		curr_coordinate.y_coordinate = { 0.0f, 1.0f, 0.0f };
		curr_coordinate.z_coordinate = { 0.0f, 0.0f, 1.0f };
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

		cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void send_login_info_packet();
	void send_move_packet(int client_id);
	void send_rotate_packet(int client_id);
};

array<SESSION, MAX_USER + MAX_NPCS> clients;		// 0 ~ MAX_USER-1: Player,	 MAX_USER ~ MAX_USER+MAX_NPCS: NPC

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

	login_info_packet.right_x = curr_coordinate.x_coordinate.x;
	login_info_packet.right_y = curr_coordinate.x_coordinate.y;
	login_info_packet.right_z = curr_coordinate.x_coordinate.z;

	login_info_packet.up_x = curr_coordinate.y_coordinate.x;
	login_info_packet.up_y = curr_coordinate.y_coordinate.y;
	login_info_packet.up_z = curr_coordinate.y_coordinate.z;

	login_info_packet.look_x = curr_coordinate.z_coordinate.x;
	login_info_packet.look_y = curr_coordinate.z_coordinate.y;
	login_info_packet.look_z = curr_coordinate.z_coordinate.z;

	cout << "[SC_LOGIN_INFO]";
	do_send(&login_info_packet);
}
void SESSION::send_move_packet(int client_id)
{
	SC_MOVE_PLAYER_PACKET move_pl_packet;
	move_pl_packet.id = client_id;
	move_pl_packet.size = sizeof(SC_MOVE_PLAYER_PACKET);
	move_pl_packet.type = SC_MOVE_PLAYER;
	move_pl_packet.x = clients[client_id].pos.x;
	move_pl_packet.y = clients[client_id].pos.y;
	move_pl_packet.z = clients[client_id].pos.z;

	cout << "[SC_MOVE]";
	do_send(&move_pl_packet);
}
void SESSION::send_rotate_packet(int client_id)
{
	SC_ROTATE_PLAYER_PACKET rotate_pl_packet;
	rotate_pl_packet.id = client_id;
	rotate_pl_packet.size = sizeof(SC_ROTATE_PLAYER_PACKET);
	rotate_pl_packet.type = SC_ROTATE_PLAYER;

	rotate_pl_packet.right_x = clients[client_id].curr_coordinate.x_coordinate.x;
	rotate_pl_packet.right_y = clients[client_id].curr_coordinate.x_coordinate.y;
	rotate_pl_packet.right_z = clients[client_id].curr_coordinate.x_coordinate.z;

	rotate_pl_packet.up_x = clients[client_id].curr_coordinate.y_coordinate.x;
	rotate_pl_packet.up_y = clients[client_id].curr_coordinate.y_coordinate.y;
	rotate_pl_packet.up_z = clients[client_id].curr_coordinate.y_coordinate.z;

	rotate_pl_packet.look_x = clients[client_id].curr_coordinate.z_coordinate.x;
	rotate_pl_packet.look_y = clients[client_id].curr_coordinate.z_coordinate.y;
	rotate_pl_packet.look_z = clients[client_id].curr_coordinate.z_coordinate.z;

	cout << "[SC_ROTATE]";
	do_send(&rotate_pl_packet);
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
		SC_REMOVE_PLAYER_PACKET remove_pl_packet;
		remove_pl_packet.id = client_id;
		remove_pl_packet.size = sizeof(remove_pl_packet);
		remove_pl_packet.type = SC_REMOVE_PLAYER;
		cout << "[SC_REMOVE]";
		pl.do_send(&remove_pl_packet);
		pl.s_lock.unlock();
	}
}

void init_npc()
{
	for (int i = 0; i < MAX_USER + MAX_NPCS; ++i)
		clients[i].id = i;
	for (int i = 0; i < MAX_NPCS; i++) {
		int npc_id = i + MAX_USER;
		clients[npc_id].s_state = ST_INGAME;
		clients[npc_id].pos.x = uid(dre) * 150;
		clients[npc_id].pos.y = 800 + uid(dre) * 50;
		clients[npc_id].pos.z = uid(dre) * 150;
		clients[npc_id].pitch = clients[npc_id].yaw = clients[npc_id].roll = 0.0f;
		clients[npc_id].curr_coordinate = basic_coordinate;
		sprintf_s(clients[npc_id].name, "NPC-No.%d", i);
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

		// 새로 접속한 플레이어의 초기 위치정보를 설정합니다.
		clients[client_id].pos.x = 640 + client_id * 15;
		clients[client_id].pos.y = 961 + client_id;
		clients[client_id].pos.z = 1165 - client_id * 15;
		cout << "A new object is successfully created! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ")." << endl;

		clients[client_id].pitch = clients[client_id].yaw = clients[client_id].roll = 0.0f;
		clients[client_id].curr_coordinate = basic_coordinate;

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
			SC_ADD_PLAYER_PACKET add_pl_packet;
			add_pl_packet.id = client_id;
			strcpy_s(add_pl_packet.name, login_packet->name);
			add_pl_packet.size = sizeof(add_pl_packet);
			add_pl_packet.type = SC_ADD_PLAYER;

			add_pl_packet.x = clients[client_id].pos.x;
			add_pl_packet.y = clients[client_id].pos.y;
			add_pl_packet.z = clients[client_id].pos.z;

			add_pl_packet.right_x = clients[client_id].curr_coordinate.x_coordinate.x;
			add_pl_packet.right_y = clients[client_id].curr_coordinate.x_coordinate.y;
			add_pl_packet.right_z = clients[client_id].curr_coordinate.x_coordinate.z;

			add_pl_packet.up_x = clients[client_id].curr_coordinate.y_coordinate.x;
			add_pl_packet.up_y = clients[client_id].curr_coordinate.y_coordinate.y;
			add_pl_packet.up_z = clients[client_id].curr_coordinate.y_coordinate.z;

			add_pl_packet.look_x = clients[client_id].curr_coordinate.z_coordinate.x;
			add_pl_packet.look_y = clients[client_id].curr_coordinate.z_coordinate.y;
			add_pl_packet.look_z = clients[client_id].curr_coordinate.z_coordinate.z;

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

			SC_ADD_PLAYER_PACKET add_pl_packet;
			add_pl_packet.id = pl.id;
			strcpy_s(add_pl_packet.name, pl.name);
			add_pl_packet.size = sizeof(add_pl_packet);
			add_pl_packet.type = SC_ADD_PLAYER;

			add_pl_packet.x = pl.pos.x;
			add_pl_packet.y = pl.pos.y;
			add_pl_packet.z = pl.pos.z;

			add_pl_packet.right_x = pl.curr_coordinate.x_coordinate.x;
			add_pl_packet.right_y = pl.curr_coordinate.x_coordinate.y;
			add_pl_packet.right_z = pl.curr_coordinate.x_coordinate.z;

			add_pl_packet.up_x = pl.curr_coordinate.y_coordinate.x;
			add_pl_packet.up_y = pl.curr_coordinate.y_coordinate.y;
			add_pl_packet.up_z = pl.curr_coordinate.y_coordinate.z;

			add_pl_packet.look_x = pl.curr_coordinate.z_coordinate.x;
			add_pl_packet.look_y = pl.curr_coordinate.z_coordinate.y;
			add_pl_packet.look_z = pl.curr_coordinate.z_coordinate.z;

			cout << "Send client[" << pl.id << "]'s info to new client" << endl;
			cout << "[SC_ADD]";
			clients[client_id].do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}
		break;
	}// CS_LOGIN end
	case CS_INPUT_KEYBOARD: {
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		enum { KEY_QE, KEY_DA, KEY_WS };

		for (int i = 0; i <= 5; i++) {
			if ((inputkey_p->direction >> i) & 1) {
				clients[client_id].s_lock.lock();

				float sign = 1.0f;					// 양, 음 부호
				if (i % 2 == 0) sign = -1.0f;		// A, S, E key

				switch (i/2) {
				case KEY_QE:
					// 아직 기능 없음.
					
					// unlock
					clients[client_id].s_lock.unlock();

					break;

				case KEY_DA:	// D, A는 기체의 yaw회전 키입니다. 기체를 y축 기준으로 회전시킵니다.
					// yaw 설정
					clients[client_id].yaw += 1.0f * sign * YAW_ROTATE_SCALAR * PI / 360.0f;

					// right, up, look 벡터 업데이트
					clients[client_id].curr_coordinate.x_coordinate = calcRotate(basic_coordinate.x_coordinate
						, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
					clients[client_id].curr_coordinate.y_coordinate = calcRotate(basic_coordinate.y_coordinate
						, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
					clients[client_id].curr_coordinate.z_coordinate = calcRotate(basic_coordinate.z_coordinate
						, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);

					// unlock
					clients[client_id].s_lock.unlock();

					// server message
					cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is Rotated. "
						<< "Pitch: " << clients[client_id].pitch << ", Yaw: " << clients[client_id].yaw << ", Roll: " << clients[client_id].roll << endl;

					// 작동 중인 모든 클라이언트에게 회전 결과를 알려줍니다.
					for (int i = 0; i < MAX_USER; i++) {
						auto& pl = clients[i];
						lock_guard<mutex> lg{ pl.s_lock };
						if (pl.s_state == ST_INGAME)
							pl.send_rotate_packet(client_id);
					}

					break;

				case KEY_WS:	// W, S는 엔진출력 조절 키입니다. 기체를 상승 또는 하강시킵니다.
					// 이동 방향 설정
					MyVector3D move_dir{ 0, 0, 0 };
					move_dir.x = clients[client_id].curr_coordinate.y_coordinate.x * sign;
					move_dir.y = clients[client_id].curr_coordinate.y_coordinate.y * sign;
					move_dir.z = clients[client_id].curr_coordinate.y_coordinate.z * sign;

					// 이동 계산 & 결과 업데이트
					MyVector3D move_result = calcMove(clients[client_id].pos, move_dir, ENGINE_SCALAR);
					clients[client_id].pos = move_result;

					// unlock
					clients[client_id].s_lock.unlock();

					// server message
					cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] moves to ("
						<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.z << ")." << endl;

					// 작동 중인 모든 클라이언트에게 이동 결과를 알려줍니다.
					for (int i = 0; i < MAX_USER; i++) {
						auto& pl = clients[i];
						lock_guard<mutex> lg{ pl.s_lock };
						if (pl.s_state == ST_INGAME)
							pl.send_move_packet(client_id);
					}

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

			MyVector3D move_dir{ 0, 0, 0 };
			MyVector3D move_result{ 0, 0, 0 };
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
				move_dir.x = clients[client_id].curr_coordinate.z_coordinate.x;
				move_dir.y = clients[client_id].curr_coordinate.z_coordinate.y;
				move_dir.z = clients[client_id].curr_coordinate.z_coordinate.z;

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
				move_dir.x = clients[client_id].curr_coordinate.x_coordinate.x;
				move_dir.y = clients[client_id].curr_coordinate.x_coordinate.y;
				move_dir.z = clients[client_id].curr_coordinate.x_coordinate.z;

				tmp_scalar = rt_p->delta_x * SENSITIVITY;
			}

			// right, up, look 벡터 회전 & 업데이트
			clients[client_id].curr_coordinate.x_coordinate = calcRotate(basic_coordinate.x_coordinate
				, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
			clients[client_id].curr_coordinate.y_coordinate = calcRotate(basic_coordinate.y_coordinate
				, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);
			clients[client_id].curr_coordinate.z_coordinate = calcRotate(basic_coordinate.z_coordinate
				, clients[client_id].roll, clients[client_id].pitch, clients[client_id].yaw);

			// 이동 & 좌표 업데이트
			move_result = calcMove(clients[client_id].pos, move_dir, tmp_scalar);
			clients[client_id].pos = move_result;

		}
		else if (rt_p->key_val == RT_RBUTTON) {		// 마우스 우클릭 드래그: 기능 미정.
			

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
				pl.send_rotate_packet(client_id);
				pl.send_move_packet(client_id);
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

int main()
{
	init_npc();

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
	for (auto& th : worker_threads)
		th.join();

	closesocket(g_s_socket);
	WSACleanup();
}
