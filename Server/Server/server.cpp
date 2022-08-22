#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>

#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

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
	float x_pos, y_pos, z_pos;
	char name[NAME_SIZE];
	int remain_size;

public:
	SESSION()
	{
		id = -1;
		socket = 0;
		x_pos = y_pos = z_pos = 0.0;
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
		WSARecv(socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* s_data = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
	}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET login_info_packet;
		login_info_packet.id = id;
		login_info_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
		login_info_packet.type = SC_LOGIN_INFO;
		login_info_packet.x = x_pos;
		login_info_packet.y = y_pos;
		login_info_packet.z = z_pos;

		do_send(&login_info_packet);
	}

	void send_move_packet(int client_id);
};

array<SESSION, MAX_USER> clients;
HANDLE g_h_iocp;
SOCKET g_s_socket;

void SESSION::send_move_packet(int client_id)
{
	SC_MOVE_PLAYER_PACKET move_pl_packet;
	move_pl_packet.id = client_id;
	move_pl_packet.size = sizeof(SC_MOVE_PLAYER_PACKET);
	move_pl_packet.type = SC_MOVE_PLAYER;
	move_pl_packet.x = clients[client_id].x_pos;
	move_pl_packet.y = clients[client_id].y_pos;
	move_pl_packet.z = clients[client_id].z_pos;

	do_send(&move_pl_packet);
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

	for (auto& pl : clients) {
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
		strcpy_s(clients[client_id].name, login_packet->name);
		clients[client_id].send_login_info_packet();
		clients[client_id].s_state = ST_INGAME;
		clients[client_id].s_lock.unlock();

		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] is log in" << endl;	// server message

		for (auto& pl : clients) {
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
			add_pl_packet.x = clients[client_id].x_pos;
			add_pl_packet.y = clients[client_id].y_pos;
			add_pl_packet.z = clients[client_id].z_pos;
			pl.do_send(&add_pl_packet);
			pl.s_lock.unlock();
		}

		for (auto& pl : clients) {
			if (pl.id == client_id) continue;
			lock_guard<mutex> lg{ pl.s_lock };
			if (pl.s_state != ST_INGAME) continue;

			SC_ADD_PLAYER_PACKET add_pl_packet;
			add_pl_packet.id = pl.id;
			strcpy_s(add_pl_packet.name, pl.name);
			add_pl_packet.size = sizeof(add_pl_packet);
			add_pl_packet.type = SC_ADD_PLAYER;
			add_pl_packet.x = pl.x_pos;
			add_pl_packet.y = pl.y_pos;
			add_pl_packet.z = pl.z_pos;

			clients[client_id].do_send(&add_pl_packet);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		float cur_x = clients[client_id].x_pos;
		float cur_y = clients[client_id].y_pos;
		float cur_z = clients[client_id].z_pos;

		switch (p->direction) {	// 0: Forward, 1: Back, 2: Left, 3: Right, 4: Up, 5: Down
		case 0: if (cur_x < WORLD_X_POS - 1) cur_x++; break;
		case 1: if (cur_z < WORLD_Z_POS - 1) cur_z++; break;
		case 2: if (cur_x > 0) cur_x--; break;
		case 3: if (cur_z > 0) cur_z--; break;
		}

		clients[client_id].x_pos = cur_x;
		clients[client_id].y_pos = cur_y;
		clients[client_id].z_pos = cur_z;

		short tempNum = p->direction;
		//cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] moves to ("
		//	<< clients[client_id].x_pos << ", " << clients[client_id].y_pos << ")." << endl;	// server message (추후에 클라이언트 좌표값 받아오면 그때 사용할 예정)
		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "] send <Move Packet - DIR: " << tempNum << " >." << endl;
		// 지금은 임시로 어떤 방향의 이동 패킷을 받았는지만 출력합니다.

		for (auto& pl : clients) {
			lock_guard<mutex> lg{ pl.s_lock };
			if (pl.s_state == ST_INGAME)
				pl.send_move_packet(client_id);
		}
		break;
	}
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
				clients[client_id].x_pos = 0.0;
				clients[client_id].y_pos = 0.0;
				clients[client_id].z_pos = 0.0;
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
