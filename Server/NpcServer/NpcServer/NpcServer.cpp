//============================================================
//						  Standard
//============================================================
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <queue>
#include <mutex>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

//============================================================
//						다이렉트X API
//============================================================
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <DirectXCollision.inl>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

//============================================================
//			메인서버에서 사용한 것들을 그대로 사용
//============================================================
#include "../../MainServer/Server/Protocol.h"
#include "../../MainServer/Server/Constant.h"
#include "../../MainServer/Server/MathFuncs.h"
#include "../../MainServer/Server/MapObjects.h"
#include "../../MainServer/Server/CP_KEYS.h"

using namespace std;
using namespace chrono;

using namespace DirectX;
using namespace DirectX::PackedVector;

enum PACKET_PROCESS_TYPE { OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };

system_clock::time_point g_s_start_time;	// 서버 시작시간  (단위: ms)
milliseconds g_curr_servertime;
mutex servertime_lock;	// 서버시간 lock

struct Coordinate {
	XMFLOAT3 right;
	XMFLOAT3 up;
	XMFLOAT3 look;

	Coordinate() {
		right = { 1.0f, 0.0f, 0.0f };
		up = { 0.0f, 1.0f, 0.0f };
		look = { 0.0f, 0.0f, 1.0f };
	}
};
Coordinate basic_coordinate;	// 기본(초기) 좌표계

//======================================================================
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
	OVER_EX recv_over;

public:
	mutex s_lock;

	SESSION_STATE s_state;
	int id;
	char name[NAME_SIZE];

	short pl_state;	// PLAYER_STATE
	int hp;
	int remain_bullet;

	XMFLOAT3 pos;									// Position (x, y, z)
	float pitch, yaw, roll;							// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// 현재 Look, Right, Up Vectors

	chrono::system_clock::time_point shoot_time;	// 총을 발사한 시간
	chrono::system_clock::time_point reload_time;	// 총알 장전 시작시간

	BoundingOrientedBox m_xoobb;	// Bounding Box

public:
	SESSION()
	{
		s_state = ST_FREE;
		id = -1;
		name[0] = 0;

		pl_state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	~SESSION() {}

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
	void sessionClear() {
		s_state = ST_FREE;
		id = -1;
		name[0] = 0;

		pl_state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		setBB();
	}
};
array<SESSION, MAX_USER> playersInfo;
array<SESSION, MAX_NPCS> npcsInfo;

//======================================================================
class LogicServer {
public:
	OVER_EX recv_over;
	int remain_size;
	int id;
	SOCKET sock;

public:
	LogicServer() {	remain_size = 0; id = -1; sock = 0; }

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

	void send_npc_init_packet(int npc_id);
	void send_npc_move_packet(int npc_id);
	void send_npc_rotate_packet(int npc_id);
	void send_npc_move_rotate_packet(int npc_id);
};
HANDLE h_iocp;											// IOCP 핸들
int a_lgcsvr_num;										// Active상태인 메인서버
array<LogicServer, MAX_LOGIC_SERVER> g_logicservers;	// 로직서버 정보

void LogicServer::send_npc_init_packet(int npc_id) {
	NPC_FULL_INFO_PACKET npc_init_packet;
	npc_init_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_init_packet.type = NPC_ROTATE;
	npc_init_packet.hp = npcsInfo[npc_id].hp;
	npc_init_packet.x = npcsInfo[npc_id].pos.x;
	npc_init_packet.y = npcsInfo[npc_id].pos.y;
	npc_init_packet.z = npcsInfo[npc_id].pos.z;
	npc_init_packet.right_x = npcsInfo[npc_id].m_rightvec.x;
	npc_init_packet.right_y = npcsInfo[npc_id].m_rightvec.y;
	npc_init_packet.right_z = npcsInfo[npc_id].m_rightvec.z;
	npc_init_packet.up_x = npcsInfo[npc_id].m_upvec.x;
	npc_init_packet.up_y = npcsInfo[npc_id].m_upvec.y;
	npc_init_packet.up_z = npcsInfo[npc_id].m_upvec.z;
	npc_init_packet.look_x = npcsInfo[npc_id].m_lookvec.x;
	npc_init_packet.look_y = npcsInfo[npc_id].m_lookvec.y;
	npc_init_packet.look_z = npcsInfo[npc_id].m_lookvec.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_init_packet);
}
void LogicServer::send_npc_move_packet(int npc_id) {
	NPC_MOVE_PACKET npc_move_packet;
	npc_move_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_move_packet.type = NPC_MOVE;
	npc_move_packet.x = npcsInfo[npc_id].pos.x;
	npc_move_packet.y = npcsInfo[npc_id].pos.y;
	npc_move_packet.z = npcsInfo[npc_id].pos.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_move_packet);
}
void LogicServer::send_npc_rotate_packet(int npc_id) {
	NPC_ROTATE_PACKET npc_rotate_packet;
	npc_rotate_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_rotate_packet.type = NPC_ROTATE;
	npc_rotate_packet.right_x = npcsInfo[npc_id].m_rightvec.x;
	npc_rotate_packet.right_y = npcsInfo[npc_id].m_rightvec.y;
	npc_rotate_packet.right_z = npcsInfo[npc_id].m_rightvec.z;
	npc_rotate_packet.up_x = npcsInfo[npc_id].m_upvec.x;
	npc_rotate_packet.up_y = npcsInfo[npc_id].m_upvec.y;
	npc_rotate_packet.up_z = npcsInfo[npc_id].m_upvec.z;
	npc_rotate_packet.look_x = npcsInfo[npc_id].m_lookvec.x;
	npc_rotate_packet.look_y = npcsInfo[npc_id].m_lookvec.y;
	npc_rotate_packet.look_z = npcsInfo[npc_id].m_lookvec.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_rotate_packet);
}
void LogicServer::send_npc_move_rotate_packet(int npc_id) {
	NPC_MOVE_PACKET npc_mv_packet;
	npc_mv_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_mv_packet.type = NPC_MOVE;
	npc_mv_packet.x = npcsInfo[npc_id].pos.x;
	npc_mv_packet.y = npcsInfo[npc_id].pos.y;
	npc_mv_packet.z = npcsInfo[npc_id].pos.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_mv_packet);

	NPC_ROTATE_PACKET npc_rt_packet;
	npc_rt_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_rt_packet.type = NPC_ROTATE;
	npc_rt_packet.right_x = npcsInfo[npc_id].m_rightvec.x;
	npc_rt_packet.right_y = npcsInfo[npc_id].m_rightvec.y;
	npc_rt_packet.right_z = npcsInfo[npc_id].m_rightvec.z;
	npc_rt_packet.up_x = npcsInfo[npc_id].m_upvec.x;
	npc_rt_packet.up_y = npcsInfo[npc_id].m_upvec.y;
	npc_rt_packet.up_z = npcsInfo[npc_id].m_upvec.z;
	npc_rt_packet.look_x = npcsInfo[npc_id].m_lookvec.x;
	npc_rt_packet.look_y = npcsInfo[npc_id].m_lookvec.y;
	npc_rt_packet.look_z = npcsInfo[npc_id].m_lookvec.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_rt_packet);
}

//======================================================================
void process_packet(char* packet)
{
	switch (packet[1]) {
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* login_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet);

		int client_id = login_packet->id;

		playersInfo[client_id].id = login_packet->id;
		strcpy_s(playersInfo[client_id].name, login_packet->name);

		playersInfo[client_id].pos.x = login_packet->x;
		playersInfo[client_id].pos.y = login_packet->y;
		playersInfo[client_id].pos.z = login_packet->z;

		playersInfo[client_id].m_rightvec.x = login_packet->right_x;
		playersInfo[client_id].m_rightvec.y = login_packet->right_y;
		playersInfo[client_id].m_rightvec.z = login_packet->right_z;

		playersInfo[client_id].m_upvec.x = login_packet->up_x;
		playersInfo[client_id].m_upvec.y = login_packet->up_y;
		playersInfo[client_id].m_upvec.z = login_packet->up_z;

		playersInfo[client_id].m_lookvec.x = login_packet->look_x;
		playersInfo[client_id].m_lookvec.y = login_packet->look_y;
		playersInfo[client_id].m_lookvec.z = login_packet->look_z;

		break;
	}// SC_ADD_OBJECT end
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* move_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(packet);

		int client_id = move_packet->id;

		playersInfo[client_id].pos.x = move_packet->x;
		playersInfo[client_id].pos.y = move_packet->y;
		playersInfo[client_id].pos.z = move_packet->z;

		break;
	}// SC_MOVE_OBJECT end
	case SC_ROTATE_OBJECT:
	{
		SC_ROTATE_OBJECT_PACKET* rotate_packet = reinterpret_cast<SC_ROTATE_OBJECT_PACKET*>(packet);

		int client_id = rotate_packet->id;

		playersInfo[client_id].m_rightvec.x = rotate_packet->right_x;
		playersInfo[client_id].m_rightvec.y = rotate_packet->right_y;
		playersInfo[client_id].m_rightvec.z = rotate_packet->right_z;

		playersInfo[client_id].m_upvec.x = rotate_packet->up_x;
		playersInfo[client_id].m_upvec.y = rotate_packet->up_y;
		playersInfo[client_id].m_upvec.z = rotate_packet->up_z;

		playersInfo[client_id].m_lookvec.x = rotate_packet->look_x;
		playersInfo[client_id].m_lookvec.y = rotate_packet->look_y;
		playersInfo[client_id].m_lookvec.z = rotate_packet->look_z;

		break;
	}// SC_ROTATE_OBJECT end
	case SC_MOVE_ROTATE_OBJECT:
	{
		SC_MOVE_ROTATE_OBJECT_PACKET* mvrt_packet = reinterpret_cast<SC_MOVE_ROTATE_OBJECT_PACKET*>(packet);

		int client_id = mvrt_packet->id;

		playersInfo[client_id].pos.x = mvrt_packet->x;
		playersInfo[client_id].pos.y = mvrt_packet->y;
		playersInfo[client_id].pos.z = mvrt_packet->z;

		playersInfo[client_id].m_rightvec.x = mvrt_packet->right_x;
		playersInfo[client_id].m_rightvec.y = mvrt_packet->right_y;
		playersInfo[client_id].m_rightvec.z = mvrt_packet->right_z;

		playersInfo[client_id].m_upvec.x = mvrt_packet->up_x;
		playersInfo[client_id].m_upvec.y = mvrt_packet->up_y;
		playersInfo[client_id].m_upvec.z = mvrt_packet->up_z;

		playersInfo[client_id].m_lookvec.x = mvrt_packet->look_x;
		playersInfo[client_id].m_lookvec.y = mvrt_packet->look_y;
		playersInfo[client_id].m_lookvec.z = mvrt_packet->look_z;

		break;
	}// SC_MOVE_ROTATE_OBJECT end
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* remove_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(packet);
		int client_id = remove_packet->id;
		playersInfo[client_id].sessionClear();

		break;
	}// SC_REMOVE_OBJECT end
	case SC_DAMAGED:
	{
		SC_DAMAGED_PACKET* damaged_packet = reinterpret_cast<SC_DAMAGED_PACKET*>(packet);
		int client_id = damaged_packet->id;
		playersInfo[client_id].hp -= damaged_packet->damage;

		break;
	}// SC_DAMAGED end
	case SC_OBJECT_STATE:
	{
		SC_OBJECT_STATE_PACKET* chgstate_packet = reinterpret_cast<SC_OBJECT_STATE_PACKET*>(packet);
		int client_id = chgstate_packet->id;
		playersInfo[client_id].pl_state = chgstate_packet->state;

		break;
	}// SC_OBJECT_STATE end
	}
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
				g_logicservers[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<LPSOCKADDR>(&logic_server_addr), sizeof(logic_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, new_portnum, 0);
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

				BOOL bret = connectExFP(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<sockaddr*>(&logic_server_addr), sizeof(SOCKADDR_IN),
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
			int remain_data = num_bytes + g_logicservers[a_lgcsvr_num].remain_size;
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
			g_logicservers[a_lgcsvr_num].remain_size = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}

			g_logicservers[a_lgcsvr_num].do_recv();

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
				g_logicservers[a_lgcsvr_num].remain_size = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, NULL, 0);
				delete ex_over;
				g_logicservers[a_lgcsvr_num].do_recv();
			}

		}//OP_CONN end
		}
	}
}


void timerFunc() {
	while (true) {
		auto start_t = system_clock::now();


		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}


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
	g_logicservers[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
	int ret = ::bind(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<LPSOCKADDR>(&logic_server_addr), sizeof(logic_server_addr));
	if (ret != 0) {
		cout << "Bind Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	OVER_EX* con_over = new OVER_EX;
	con_over->process_type = OP_CONNECT;
	HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, lgvsvr_port, 0);
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

	BOOL bret = connectExFP(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<sockaddr*>(&logic_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
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
	//							NPC Initialize
	//======================================================================
	//init_npc();


	//======================================================================
	//						  Threads Initialize
	//======================================================================
	vector <thread> worker_threads;
	for (int i = 0; i < 5; ++i)
		worker_threads.emplace_back(do_worker);			// 메인서버-npc서버 통신용 Worker스레드

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);				// npc 로직 타이머스레드

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();


	//closesocket(g_sc_listensock);
	WSACleanup();
}