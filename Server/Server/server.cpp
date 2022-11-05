#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <random>	// 플레이어의 초기 위치값 설정할 때 임시로 랜덤값을 부여하기로 함. -> 추후에 정해진 리스폰 지점에 생성되도록 변경해야함.

#include "MyFloat3.h"
#include "protocol.h"
#include "Func_forVectorCalc.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

// 플레이어의 초기 위치값 설정할 때 임시로 랜덤값을 부여하기로 함. -> 추후에 정해진 리스폰 지점에 생성되도록 변경해야함.
default_random_engine dre;
uniform_int_distribution<int> uid(1, 10);
// ==== 리스폰 지점에 생성되도록 변경한 후에 이 부분은 지워도 됨.

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
	MyFloat3 pos;							// Position (x, y, z)
	MyFloat3 right_vec, up_vec, look_vec;	// Vector (x, y ,z)
	char name[NAME_SIZE];
	int remain_size;

public:
	SESSION()
	{
		id = -1;
		socket = 0;
		pos = { 0.0f, 0.0f, 0.0f };
		right_vec = { 1.0f, 0.0f, 0.0f };
		up_vec = { 0.0f, 1.0f, 0.0f };
		look_vec = { 0.0f, 0.0f, 1.0f };
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

	login_info_packet.up_x = up_vec.x, login_info_packet.up_y = up_vec.y, login_info_packet.up_z = up_vec.z;
	login_info_packet.right_x = right_vec.x, login_info_packet.right_y = right_vec.y, login_info_packet.right_z = right_vec.z;
	login_info_packet.look_x = look_vec.x, login_info_packet.look_y = look_vec.y, login_info_packet.look_z = look_vec.z;

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

	rotate_pl_packet.look_x = clients[client_id].look_vec.x;
	rotate_pl_packet.look_y = clients[client_id].look_vec.y;
	rotate_pl_packet.look_z = clients[client_id].look_vec.z;

	rotate_pl_packet.right_x = clients[client_id].right_vec.x;
	rotate_pl_packet.right_y = clients[client_id].right_vec.y;
	rotate_pl_packet.right_z = clients[client_id].right_vec.z;

	rotate_pl_packet.up_x = clients[client_id].up_vec.x;
	rotate_pl_packet.up_y = clients[client_id].up_vec.y;
	rotate_pl_packet.up_z = clients[client_id].up_vec.z;

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
		clients[npc_id].right_vec = { 1.0f, 0.0f, 0.0f };
		clients[npc_id].up_vec = { 0.0f, 1.0f, 0.0f };
		clients[npc_id].look_vec = { 0.0f, 0.0f, 1.0f };
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
		clients[client_id].pos.y = 733;
		clients[client_id].pos.z = 1165 - client_id * 15;
		cout << "A new object is successfully created! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ")." << endl;

		clients[client_id].right_vec = { 1.0f, 0.0f, 0.0f };
		clients[client_id].up_vec = { 0.0f, 1.0f, 0.0f };
		clients[client_id].look_vec = { 0.0f, 0.0f, 1.0f };
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
			add_pl_packet.right_x = clients[client_id].right_vec.x, add_pl_packet.right_y = clients[client_id].right_vec.y, add_pl_packet.right_z = clients[client_id].right_vec.z;
			add_pl_packet.up_x = clients[client_id].up_vec.x, add_pl_packet.up_y = clients[client_id].up_vec.y, add_pl_packet.up_z = clients[client_id].up_vec.z;
			add_pl_packet.look_x = clients[client_id].look_vec.x, add_pl_packet.look_y = clients[client_id].look_vec.y, add_pl_packet.look_z = clients[client_id].look_vec.z;

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
			add_pl_packet.right_x = pl.right_vec.x, add_pl_packet.right_y = pl.right_vec.y, add_pl_packet.right_z = pl.right_vec.z;
			add_pl_packet.up_x = pl.up_vec.x, add_pl_packet.up_y = pl.up_vec.y, add_pl_packet.up_z = pl.up_vec.z;
			add_pl_packet.look_x = pl.look_vec.x, add_pl_packet.look_y = pl.look_vec.y, add_pl_packet.look_z = pl.look_vec.z;

			cout << "Send client[" << pl.id << "]'s info to new client" << endl;
			cout << "[SC_ADD]";
			clients[client_id].do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}
		break;
	}// CS_LOGIN end
	case CS_MOVE: {
		CS_MOVE_PACKET* mv_p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		clients[client_id].s_lock.lock();
		MyFloat3 temp{ 0, };
		float pos_or_neg = 1.0f;
		switch (mv_p->direction) {
		case MV_BACK:
			pos_or_neg = -1.0f;
		case MV_FORWARD:
			temp.x = clients[client_id].look_vec.x * pos_or_neg;
			temp.y = clients[client_id].look_vec.y * pos_or_neg;
			temp.z = clients[client_id].look_vec.z * pos_or_neg;
			break;
		case MV_LEFT:
			pos_or_neg = -1.0f;
		case MV_RIGHT:
			temp.x = clients[client_id].right_vec.x * pos_or_neg;
			temp.y = clients[client_id].right_vec.y * pos_or_neg;
			temp.z = clients[client_id].right_vec.z * pos_or_neg;
			break;
		case MV_DOWN:
			pos_or_neg = -1.0f;
		case MV_UP:
			temp.x = clients[client_id].up_vec.x * pos_or_neg;
			temp.y = clients[client_id].up_vec.y * pos_or_neg;
			temp.z = clients[client_id].up_vec.z * pos_or_neg;
			break;
		}
		MyFloat3 tempPos = calcMove(clients[client_id].pos, temp, 0.6f);// 이동 계산
		clients[client_id].pos = { tempPos.x, tempPos.y, tempPos.z };
		clients[client_id].s_lock.unlock();

		// server message
		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] moves to ("
			<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.z << ")." << endl;

		// send to all of running clients
		for (int i = 0; i < MAX_USER; i++) {
			auto& pl = clients[i];
			lock_guard<mutex> lg{ pl.s_lock };
			if (pl.s_state == ST_INGAME)
				pl.send_move_packet(client_id);
		}
		break;
	}// CS_MOVE end
	case CS_ROTATE: {
		CS_ROTATE_PACKET* rt_p = reinterpret_cast<CS_ROTATE_PACKET*>(packet);
		
		clients[client_id].s_lock.lock();
		if (clients[client_id].s_state == ST_FREE) {
			clients[client_id].s_lock.unlock();
			break;
		}
		clients[client_id].right_vec = calcRotate(clients[client_id].right_vec, rt_p->roll, rt_p->pitch, rt_p->yaw);
		clients[client_id].up_vec = calcRotate(clients[client_id].up_vec, rt_p->roll, rt_p->pitch, rt_p->yaw);
		clients[client_id].look_vec = calcRotate(clients[client_id].look_vec, rt_p->roll, rt_p->pitch, rt_p->yaw);
		clients[client_id].s_lock.unlock();

		// server message
		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is Rotated. LookVec:("
			<< clients[client_id].look_vec.x << ", " << clients[client_id].look_vec.y << ", " << clients[client_id].look_vec.z << ")." << endl;

		// send to all of running clients
		for (int i = 0; i < MAX_USER; i++) {
			auto& pl = clients[i];
			lock_guard<mutex> lg{ pl.s_lock };
			if (pl.s_state == ST_INGAME)
				pl.send_rotate_packet(client_id);
		}
		break;
	}// CS_ROTATE end
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
