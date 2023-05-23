//============================================================
//						  Standard
//============================================================
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <queue>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

//============================================================
//			메인서버에서 사용한 것들을 그대로 사용
//============================================================
#include "../../MainServer/Server/Protocol.h"
#include "../../MainServer/Server/Constant.h"

using namespace std;
using namespace chrono;

//======================================================================
enum PACKET_PROCESS_TYPE { OP_RECV, OP_SEND, OP_CONNECT };

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

//======================================================================
class SESSION {
public:
	OVER_EX recv_over;
	int remain_size;
	int id;
	SOCKET sock;

public:
	SESSION() { remain_size = 0; id = -1; sock = 0; }

public:
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(sock, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void do_send(void* packet)
	{
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		int ret = WSASend(sock, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void send_login_ok_packet(int c_id);
	void send_login_deny_packet(int c_id);
};

HANDLE h_iocp;											// IOCP 핸들
int a_lgcsvr_num;										// Active상태인 메인서버
array<SESSION, MAX_USER> clients;						// 세션 정보

void SESSION::send_login_ok_packet(int npc_id) {
	NPC_FULL_INFO_PACKET npc_init_packet;
	npc_init_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_init_packet.type = NPC_ROTATE;

	clients[a_lgcsvr_num].do_send(&npc_init_packet);
}
void SESSION::send_login_deny_packet(int npc_id) {
	NPC_MOVE_PACKET npc_move_packet;
	npc_move_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_move_packet.type = NPC_MOVE;

	clients[a_lgcsvr_num].do_send(&npc_move_packet);
}


//======================================================================
void process_packet(char* packet)
{
	switch (packet[1]) {
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* login_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet);

	}// SC_ADD_OBJECT end
	}
}

//======================================================================
void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EX* ex_over = reinterpret_cast<OVER_EX*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_CONNECT) {
				// 서버번호를 바꿔가면서 비동기Connect를 재시도합니다.
				if (a_lgcsvr_num == 0)		a_lgcsvr_num = 1;
				else if (a_lgcsvr_num == 1)	a_lgcsvr_num = 0;
				int new_portnum = a_lgcsvr_num + PORTNUM_LGCNPC_0;
				cout << "[ConnectEX Failed] ";
				cout << "Logic Server[" << a_lgcsvr_num << "] (PORTNUM:" << new_portnum << ")로 다시 연결합니다. \n" << endl;

				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN logic_server_addr;
				ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
				logic_server_addr.sin_family = AF_INET;
				clients[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(clients[a_lgcsvr_num].sock, reinterpret_cast<LPSOCKADDR>(&logic_server_addr), sizeof(logic_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(clients[a_lgcsvr_num].sock), h_iocp, new_portnum, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
				logic_server_addr.sin_family = AF_INET;
				logic_server_addr.sin_port = htons(new_portnum);
				inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);
				// 1. 메인서버와 NPC서버가 루프백에서 동작할 때
				//inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

				// 2. 메인서버와 NPC서버가 다른 PC에서 동작할 떄
				if (a_lgcsvr_num == 0) {
					inet_pton(AF_INET, IPADDR_LOGIC0, &logic_server_addr.sin_addr);
				}
				else if (a_lgcsvr_num == 1) {
					inet_pton(AF_INET, IPADDR_LOGIC1, &logic_server_addr.sin_addr);
				}

				BOOL bret = connectExFP(clients[a_lgcsvr_num].sock, reinterpret_cast<sockaddr*>(&logic_server_addr), sizeof(SOCKADDR_IN),
					nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no) {
						//cout << "Server Connect 재시도 중...\n" << endl;
					}
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
			else {
				//cout << "GQCS Error ( client[" << key << "] )" << endl;

				//disconnect(static_cast<int>(key - CP_KEY_LOGIC2CLIENT));
				if (ex_over->process_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->process_type) {
		case OP_RECV: {
			int remain_data = num_bytes + clients[a_lgcsvr_num].remain_size;
			char* p = ex_over->send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[a_lgcsvr_num].remain_size = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}

			clients[a_lgcsvr_num].do_recv();

			break;
		}//OP_RECV end
		case OP_SEND: {
			//if (0 == num_bytes) disconnect(key - CP_KEY_LOGIC2CLIENT, SESSION_CLIENT);
			delete ex_over;
			break;
		}//OP_SEND end
		case OP_CONNECT: {
			if (FALSE != ret) {
				int server_id = key - PORTNUM_LGCNPC_0;
				std::cout << "성공적으로 Logic Server[" << server_id << "]에 연결되었습니다.\n" << endl;
				clients[a_lgcsvr_num].remain_size = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(clients[a_lgcsvr_num].sock), h_iocp, NULL, 0);
				delete ex_over;
				clients[a_lgcsvr_num].do_recv();
			}

		}//OP_CONN end
		}
	}
}


//======================================================================
int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	//					로직서버로 비동기 Connect 요청
	//======================================================================
	int lgvsvr_port = PORTNUM_LGCNPC_0 + a_lgcsvr_num;

	cout << "로직 서버(Server[" << a_lgcsvr_num << "] (PORT: " << lgvsvr_port << ")에 비동기Connect를 요청합니다." << endl;

	// ConnectEx
	SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	GUID guid = WSAID_CONNECTEX;
	DWORD bytes = 0;
	LPFN_CONNECTEX connectExFP;
	::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
	closesocket(temp_s);

	SOCKADDR_IN logic_server_addr;
	ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
	logic_server_addr.sin_family = AF_INET;
	clients[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
	int ret = ::bind(clients[a_lgcsvr_num].sock, reinterpret_cast<LPSOCKADDR>(&logic_server_addr), sizeof(logic_server_addr));
	if (ret != 0) {
		cout << "Bind Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	OVER_EX* con_over = new OVER_EX;
	con_over->process_type = OP_CONNECT;
	HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(clients[a_lgcsvr_num].sock), h_iocp, lgvsvr_port, 0);
	if (NULL == hret) {
		cout << "CreateIoCompletoinPort Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
	logic_server_addr.sin_family = AF_INET;
	logic_server_addr.sin_port = htons(lgvsvr_port);

	// 1. 메인서버와 NPC서버가 루프백에서 동작할 때
	//inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

	// 2. 메인서버와 NPC서버가 다른 PC에서 동작할 떄
	if (a_lgcsvr_num == 0) {
		inet_pton(AF_INET, IPADDR_LOGIC0, &logic_server_addr.sin_addr);
	}
	else if (a_lgcsvr_num == 1) {
		inet_pton(AF_INET, IPADDR_LOGIC1, &logic_server_addr.sin_addr);
	}

	BOOL bret = connectExFP(clients[a_lgcsvr_num].sock, reinterpret_cast<sockaddr*>(&logic_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
	if (FALSE == bret) {
		int err_no = GetLastError();
		if (ERROR_IO_PENDING == err_no)
			cout << "Server Connect 시도 중...\n" << endl;
		else {
			cout << "ConnectEX Error - " << err_no << endl;
			cout << WSAGetLastError() << endl;
		}
	}


	//======================================================================
	//						  Threads Initialize
	//======================================================================
	vector <thread> worker_threads;
	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(do_worker);			// 메인서버-npc서버 통신용 Worker스레드

	for (auto& th : worker_threads)
		th.join();


	//closesocket(g_sc_listensock);
	WSACleanup();
}