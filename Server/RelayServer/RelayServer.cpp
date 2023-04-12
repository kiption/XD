#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <chrono>

#include "Constant.h"
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };
enum SESSION_TYPE { SESSION_CLIENT, SESSION_EXTENDED_SERVER };

int online_user_cnt = 0;				// 접속중인 유저 수
HANDLE h_iocp_relay2client;				// 클라이언트-릴레이서버 통신 IOCP 핸들
SOCKET g_listensock_relay2client;		// 클라이언트-릴레이서버 통신 listen소켓
HANDLE h_iocp_relay2logic;				// 릴레이서버-로직서버 통신 IOCP 핸들
SOCKET g_listensock_relay2logic;		// 릴레이서버-로직서버 통신 listen소켓

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

public:
	SESSION()
	{
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		name[0] = 0;
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
};
array<SESSION, MAX_USER> clients;		// 0 ~ MAX_USER-1: Player,	 MAX_USER ~ MAX_USER+MAX_NPCS: NPC


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

		online_user_cnt--;
		cout << "Player[ID: " << clients[target_id].id << ", name: " << clients[target_id].name << "] is log out" << endl;	// server message

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
			cout << "[SC_REMOVE]";
			pl.do_send(&remove_pl_packet);
			pl.s_lock.unlock();
		}
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
			online_user_cnt++;
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
		clients[client_id].s_lock.unlock();

		cout << "Client[" << client_id << "] is Connected." << endl;
		break;
	}// CS_LOGIN end
	case CS_INPUT_KEYBOARD: {
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		enum InputKey { KEY_Q, KEY_E, KEY_A, KEY_D, KEY_S, KEY_W, KEY_SPACEBAR };

		for (int i = 0; i <= 6; i++) {
			if ((inputkey_p->direction >> i) & 1) {
				switch (i) {
				case KEY_Q:
					cout << "Q" << endl;
					break;
				case KEY_E:
					cout << "E" << endl;
					break;
				case KEY_A:
					cout << "E" << endl;
					break;
				case KEY_D:
					cout << "D" << endl;
					break;
				case KEY_S:
					cout << "S" << endl;
					break;
				case KEY_W:
					cout << "W" << endl;
					break;
				case KEY_SPACEBAR:
					cout << "SPACEBAR" << endl;
					break;
				default:
					cout << "UNKNOWN KEY" << endl;
					break;
				}
			}
		}
		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE: {
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE)	break;

		if (rt_p->key_val == RT_LBUTTON)
			cout << "LEFT BUTTON" << endl;
		else if (rt_p->key_val == RT_RBUTTON)
			cout << "RIGHT BUTTON" << endl;
		else
			cout << "UNKNOWN BUTTON" << endl;

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
		BOOL ret = GetQueuedCompletionStatus(h_iocp_relay2client, &num_bytes, &key, &over, INFINITE);
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
				clients[client_id].id = client_id;
				clients[client_id].name[0] = 0;
				clients[client_id].remain_size = 0;
				clients[client_id].socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp_relay2client, client_id, 0);
				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Sever is Full" << endl;
			}

			ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
			ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_listensock_relay2client, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
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

void timerFunc() {
	while (true) {
		cout << "TIMER" << endl;
		Sleep(1000);
	}
}

int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	//======================================================================
	// [ 하위 Server 실행 ]
	// 1. 인증 서버

	// 2. 로비 서버

	// 3. 로직 서버
	for (int i = 0; i < MAX_SERVER; i++) {
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", i);
		ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../x64/Release", SW_SHOW);
	}

	// [ 하위 Server 연결 ]
	// LogicServer Listen Socket (릴레이-로직서버 통신을 위한 Listen소켓)
	g_listensock_relay2logic = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN logic_sockaddr;
	memset(&logic_sockaddr, 0, sizeof(logic_sockaddr));
	logic_sockaddr.sin_family = AF_INET;
	logic_sockaddr.sin_port = htons(PORTNUM_RELAY2LOGIC_0);
	logic_sockaddr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_listensock_relay2logic, reinterpret_cast<sockaddr*>(&logic_sockaddr), sizeof(logic_sockaddr));
	listen(g_listensock_relay2logic, SOMAXCONN);
	int logic_addrsize = sizeof(logic_sockaddr);

	// LogicServer Accept
	h_iocp_relay2logic = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_listensock_relay2logic), h_iocp_relay2logic, CP_KEY_RELAY2LOGIC, 0);
	SOCKET logic_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP logic_over;
	logic_over.process_type = OP_ACCEPT;
	logic_over.wsabuf.buf = reinterpret_cast<CHAR*>(logic_socket);
	AcceptEx(g_listensock_relay2logic, logic_socket, logic_over.send_buf, 0, logic_addrsize + 16, logic_addrsize + 16, 0, &logic_over.overlapped);

	//======================================================================
	// [ Client 연결 ]
	// Client Listen Socket (클라이언트-릴레이서버 통신을 위한 Listen소켓)
	g_listensock_relay2client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORTNUM_RELAY2CLIENT_0);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_listensock_relay2client, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_listensock_relay2client, SOMAXCONN);
	int addr_size = sizeof(server_addr);
	int client_id = 0;

	// CLient Accept
	h_iocp_relay2client = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_listensock_relay2client), h_iocp_relay2client, CP_KEY_RELAY2CLIENT, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_listensock_relay2client, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);		// 클라이언트-서버 통신용 Worker스레드

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);			// 클라이언트 로직 타이머스레드

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_listensock_relay2client);
	WSACleanup();
}
