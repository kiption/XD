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
enum SESSION_TYPE { SESSION_CLIENT, SESSION_LOGIN, SESSION_LOGIC };

HANDLE h_iocp;							// IOCP 핸들
SOCKET g_listensock_relay2client;		// 클라이언트-릴레이서버 통신 listen소켓
SOCKET g_listensock_relay2logic;		// 릴레이서버-로직서버 통신 listen소켓

int active_svrid_logic;					// 현재 Active상태인 로직서버 ID

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
array<SESSION, MAX_USER> clients;
array<SESSION, MAX_LOGIN_SERVER> login_servers;
array<SESSION, MAX_LOGIC_SERVER> logic_servers;


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

		cout << "Client[" << clients[target_id].id << "]가 나갔습니다.\n" << endl;	// server message
		break;

	case SESSION_LOGIN:
		break;

	case SESSION_LOGIC:
		logic_servers[target_id].s_lock.lock();
		if (logic_servers[target_id].s_state == ST_FREE) {
			logic_servers[target_id].s_lock.unlock();
			return;
		}
		closesocket(logic_servers[target_id].socket);
		logic_servers[target_id].s_state = ST_FREE;
		logic_servers[target_id].s_lock.unlock();

		cout << "로직서버[" << logic_servers[target_id].id << "]와의 연결이 끊어졌습니다.\n" << endl;	// server message
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
		clients[client_id].s_state = ST_INGAME;
		clients[client_id].s_lock.unlock();

		cout << "Client[" << client_id << "]가 연결되었습니다.\n" << endl;

		// 로직서버로 패킷 전달 (나중에는 여기서 보내는게 아니라, 매칭을 돌리고 매칭이 잡혔을때 로직서버로 전달하도록 수정해야함)
		logic_servers[active_svrid_logic].do_send(login_packet);


		break;
	}// CS_LOGIN end
	case CS_INPUT_KEYBOARD: {
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE: {
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE)	break;

		if (rt_p->buttontype == PACKET_BUTTON_L)
			cout << "LEFT BUTTON" << endl;
		else if (rt_p->buttontype == PACKET_BUTTON_R)
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
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_ACCEPT) cout << "Accept Error";
			else {
				if (key == CP_KEY_RELAY2CLIENT) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key - CP_KEY_RELAY2CLIENT), SESSION_CLIENT);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				else if (key == CP_KEY_RELAY2LOGIN) {

				}
				else if (key == CP_KEY_RELAY2LOGIC) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key - CP_KEY_RELAY2LOGIC), SESSION_LOGIC);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			if (key == CP_KEY_RELAY2CLIENT) {	// 클라이언트의 연결 요청
				SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				int client_id = get_new_client_id();
				if (client_id != -1) {
					clients[client_id].s_lock.lock();
					clients[client_id].id = client_id;
					clients[client_id].name[0] = 0;
					clients[client_id].remain_size = 0;
					clients[client_id].socket = c_socket;
					int new_key = client_id + CP_KEY_RELAY2CLIENT;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, new_key, 0);
					clients[client_id].do_recv();
					clients[client_id].s_lock.unlock();
					c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "[Client Accept] Sever is Full" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_listensock_relay2client, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			else if (key == CP_KEY_RELAY2LOGIN) {	// 인증서버의 연결 요청
				
			}
			else if (key == CP_KEY_RELAY2LOGIC) {	// 로직서버의 연결 요청
				SOCKET lgc_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				int new_lgcsvr_id = -1;
				for (int i = 0; i < MAX_LOGIC_SERVER; ++i) {
					logic_servers[i].s_lock.lock();
					if (logic_servers[i].s_state == ST_FREE) {
						logic_servers[i].s_state = ST_ACCEPTED;
						new_lgcsvr_id = i;
						logic_servers[i].s_lock.unlock();
						break;
					}
					logic_servers[i].s_lock.unlock();
				}

				if (new_lgcsvr_id != -1) {
					logic_servers[new_lgcsvr_id].s_lock.lock();
					logic_servers[new_lgcsvr_id].id = new_lgcsvr_id;
					logic_servers[new_lgcsvr_id].name[0] = 0;
					logic_servers[new_lgcsvr_id].remain_size = 0;
					logic_servers[new_lgcsvr_id].socket = lgc_socket;
					int new_key = new_lgcsvr_id + CP_KEY_RELAY2LOGIC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(lgc_socket), h_iocp, new_key, 0);
					logic_servers[new_lgcsvr_id].do_recv();
					logic_servers[new_lgcsvr_id].s_lock.unlock();
					lgc_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
					cout << "로직서버[" << new_lgcsvr_id << "]의 연결요청을 받았습니다.\n" << endl;//server message
				}
				else {
					cout << "[LogicServer Accept] Sever is Full" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(lgc_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_listensock_relay2logic, lgc_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			break;
		}
		case OP_RECV: {
			if (key == CP_KEY_RELAY2CLIENT) {		// 클라이언트 RECV
				int cl_id = key - CP_KEY_RELAY2CLIENT;
				if (0 == num_bytes) disconnect(cl_id, SESSION_CLIENT);

				int remain_data = num_bytes + clients[cl_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(cl_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				clients[cl_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				clients[cl_id].do_recv();
			}
			else if (key == CP_KEY_RELAY2LOGIN) {	// 인증서버의 RECV

			}
			else if (key == CP_KEY_RELAY2LOGIC) {	// 로직서버의 RECV
				int lgcsrv_id = key - CP_KEY_RELAY2LOGIC;
				if (0 == num_bytes) disconnect(lgcsrv_id, SESSION_LOGIC);

				int remain_data = num_bytes + logic_servers[lgcsrv_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(lgcsrv_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				logic_servers[lgcsrv_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				logic_servers[lgcsrv_id].do_recv();
			}

			break;
		}
		case OP_SEND: {
			if (key == CP_KEY_RELAY2CLIENT) {		// 클라이언트 SEND
				if (0 == num_bytes) disconnect(key - CP_KEY_RELAY2CLIENT, SESSION_CLIENT);
			}
			else if (key == CP_KEY_RELAY2LOGIN) {	// 인증서버의 SEND

			}
			else if (key == CP_KEY_RELAY2LOGIC) {	// 로직서버의 SEND
				if (0 == num_bytes) disconnect(key - CP_KEY_RELAY2LOGIC, SESSION_LOGIC);
			}
			delete ex_over;
			break;
		}
		}
	}
}

void timerFunc() {
	while (true) {
		cout << "TIMER TEST" << endl;
		Sleep(1000);
	}
}

int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	// [ 하위 Server 실행 ]
	// 1. 인증 서버

	// 2. 로비 서버

	// 3. 로직 서버
	active_svrid_logic = MAX_SERVER - 1;
	for (int i = 0; i < MAX_SERVER; i++) {
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", i);
		ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../x64/Release", SW_SHOW);
		Sleep(300);//제대로 0번, 1번 순서대로 logic_servers 배열에 들어가도록 하기 위함.
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
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_listensock_relay2logic), h_iocp, CP_KEY_RELAY2LOGIC, 0);
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

	// Client Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_listensock_relay2client), h_iocp, CP_KEY_RELAY2CLIENT, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_listensock_relay2client, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);		// 클라이언트-서버 통신용 Worker스레드

	//vector<thread> timer_threads;
	//timer_threads.emplace_back(timerFunc);			// 클라이언트 로직 타이머스레드

	for (auto& th : worker_threads)
		th.join();
	//for (auto& timer_th : timer_threads)
	//	timer_th.join();

	closesocket(g_listensock_relay2client);
	WSACleanup();
}
