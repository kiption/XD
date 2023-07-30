#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <math.h>
#include <array>
#include <filesystem> // filename
#include <string>
#include "../../MainServer/Server/Protocol.h"
#include "resource.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;
using namespace std::filesystem;

//==================================================
// Network
HANDLE h_iocp;				// IOCP 핸들
SOCKET g_listensock;		// 리슨 소켓

SOCKET lby_socket;	// 현재 로비서버 소켓
SOCKET lgc_socket;	// 현재 로직서버 소켓
SOCKET npc_socket;	// 현재 NPC서버 소켓

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_RUNNING_SERVER, ST_DOWN_SERVER };
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

public:
	SESSION()
	{
		s_state = ST_FREE;
		id = -1;
		socket = 0;
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
			cout << "[Line: 82] WSARecv Error - " << GetLastError() << endl;
		}
	}

	void do_send(void* packet)
	{
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "[Line: 94] WSASend Error - " << GetLastError() << endl;
		}
	}

	void sessionClear() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
	}
};
array<SESSION, 6> servers;

void process_packet(int server_id, char* packet);
void disconnect(int target_id);
int get_new_server_id()	// clients의 비어있는 칸을 찾아서 새로운 client의 아이디를 할당해주는 함수
{
	for (int i = 0; i < MAX_USER; ++i) {
		servers[i].s_lock.lock();
		if (servers[i].s_state == ST_FREE) {
			servers[i].s_state = ST_ACCEPTED;
			servers[i].s_lock.unlock();
			return i;
		}
		servers[i].s_lock.unlock();
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
			else {
				disconnect(static_cast<int>(key));
				if (ex_over->process_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
			int server_id = get_new_server_id();
			if (server_id != -1) {
				// 클라이언트 id, 소켓
				servers[server_id].s_lock.lock();
				servers[server_id].id = server_id;
				servers[server_id].remain_size = 0;
				servers[server_id].s_state = ST_RUNNING_SERVER;
				servers[server_id].socket = c_socket;
				servers[server_id].s_lock.unlock();
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, server_id, 0);
				servers[server_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "어떤 Server의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
			}

			ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
			ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);

			int option = TRUE;//Nagle
			setsockopt(g_listensock, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
			AcceptEx(g_listensock, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);

			break;
		}//OP_ACPT end
		case OP_RECV: {
			int serverkey = static_cast<int>(key);
			if (0 == num_bytes) disconnect(serverkey);

			int remain_data = num_bytes + servers[serverkey].remain_size;
			char* p = ex_over->send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(serverkey), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			servers[key].remain_size = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}
			servers[key].do_recv();

			break;
		}//OP_RECV end
		case OP_SEND: {
			if (0 == num_bytes) disconnect(key);
			delete ex_over;

			break;
		}//OP_SEND end
		}
	}
}

void disconnect(int target_id)
{
	servers[target_id].s_lock.lock();
	if (servers[target_id].s_state == ST_FREE) {
		servers[target_id].s_lock.unlock();
		return;
	}
	closesocket(servers[target_id].socket);
	servers[target_id].s_state = ST_FREE;
	servers[target_id].s_lock.unlock();
}

void process_packet(int server_id, char* packet)
{
	switch (packet[1]) {
	}
}

//==================================================
// Dialog
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM); // 대화상자 프로시저

HWND hServerStatus[6];					// 서버상태
HWND hServerName[6];					// 서버이름
HWND hServerPort[6];					// 서버이름

constexpr int nameMsgSize = 7;			// 윈도우에 띄울 메시지 길이
constexpr int portMsgSize = 6;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	// 원격으로 모든 서버를 실행한다.
	// 서버매니저는 외부 실행(exe 실행)만 고려하여 제작되었다. 제작과정에서의 테스트를 하고싶다면 모든 서버의 실행파일을 Servers 디렉토리로 가져와야한다.
	/*ShellExecute(NULL, L"open", L"Server.exe", L"1", L"./Servers", SW_SHOW);
	ShellExecute(NULL, L"open", L"LobbyServer.exe", L"1", L"./Servers", SW_SHOW);
	ShellExecute(NULL, L"open", L"NpcServer.exe", L"1", L"./Servers", SW_SHOW);*/

	// Dialog 실행
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// [ 서버 Accept 준비 ]
	// Listen Socket
	g_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORTNUM_MANAGER);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_listensock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_listensock, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	// Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_listensock), h_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);

	int option = TRUE;//Nagle
	setsockopt(g_listensock, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	AcceptEx(g_listensock, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	thread worker_thread(do_worker);	// 통신용 Worker스레드
	worker_thread.join();

	WSACleanup();
	closesocket(g_listensock);
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		hServerStatus[0] = GetDlgItem(hDlg, IDC_STATUS0);
		hServerStatus[1] = GetDlgItem(hDlg, IDC_STATUS1);
		hServerStatus[2] = GetDlgItem(hDlg, IDC_STATUS2);
		hServerStatus[3] = GetDlgItem(hDlg, IDC_STATUS3);
		hServerStatus[4] = GetDlgItem(hDlg, IDC_STATUS4);
		hServerStatus[5] = GetDlgItem(hDlg, IDC_STATUS5);

		hServerName[0] = GetDlgItem(hDlg, IDC_NAME0);
		hServerName[1] = GetDlgItem(hDlg, IDC_NAME1);
		hServerName[2] = GetDlgItem(hDlg, IDC_NAME2);
		hServerName[3] = GetDlgItem(hDlg, IDC_NAME3);
		hServerName[4] = GetDlgItem(hDlg, IDC_NAME4);
		hServerName[5] = GetDlgItem(hDlg, IDC_NAME5);

		hServerPort[0] = GetDlgItem(hDlg, IDC_PORT0);
		hServerPort[1] = GetDlgItem(hDlg, IDC_PORT1);
		hServerPort[2] = GetDlgItem(hDlg, IDC_PORT2);
		hServerPort[3] = GetDlgItem(hDlg, IDC_PORT3);
		hServerPort[4] = GetDlgItem(hDlg, IDC_PORT4);
		hServerPort[5] = GetDlgItem(hDlg, IDC_PORT5);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);					// 대화상자 닫기
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}
