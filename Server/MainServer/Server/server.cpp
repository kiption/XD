#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <chrono>
#include <random>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

//======================================================================
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

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

//======================================================================
#include "Protocol.h"
#include "Timer.h"
#include "Constant.h"
#include "CP_KEYS.h"
#include "MapObjects.h"
#include "MathFuncs.h"

//======================================================================
enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };
enum SESSION_TYPE { SESSION_CLIENT, SESSION_EXTENDED_SERVER, SESSION_NPC };

//======================================================================
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
chrono::system_clock::time_point g_s_start_time;	// 서버 시작시간  (단위: ms)
milliseconds g_curr_servertime;
bool b_isfirstplayer;	// 첫 player입장인지. (첫 클라 접속부터 서버시간이 흐르도록 하기 위함)
mutex servertime_lock;	// 서버시간 lock

//======================================================================
class Building : public MapObject
{
public:
	Building() : MapObject() {}
	Building(float px, float py, float pz, float sx, float sy, float sz) : MapObject(px, py, pz, sx, sy, sz) {}

public:
	BoundingOrientedBox m_xoobb;

public:
	void setBB() {
		m_xoobb = BoundingOrientedBox(XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()),
			XMFLOAT3(this->getScaleX(), this->getScaleY(), this->getScaleZ()),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	XMFLOAT3 getPos() { return XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()); }
};
vector<Building> buildings_info;	// Map Buildings CollideBox
MyVector3 exc_XMFtoMyVec(XMFLOAT3 xmf3) { return MyVector3{ xmf3.x, xmf3.y, xmf3.z }; }
XMFLOAT3 exc_MyVectoXMF(MyVector3 mv3) { return XMFLOAT3{ mv3.x, mv3.y, mv3.z }; }

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
	SOCKET socket;
	int remain_size;
	char name[NAME_SIZE];

	short pl_state;
	int hp;
	int remain_bullet;
	XMFLOAT3 pos;									// Position (x, y, z)
	float pitch, yaw, roll;							// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// 현재 Look, Right, Up Vectors
	chrono::system_clock::time_point death_time;

	chrono::system_clock::time_point shoot_time;	// 총을 발사한 시간
	chrono::system_clock::time_point reload_time;	// 총알 장전 시작시간

	short curr_stage;

	BoundingOrientedBox m_xoobb;	// Bounding Box

public:
	SESSION()
	{
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		name[0] = 0;

		pl_state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };
		curr_stage = 0;

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void send_login_packet();
	void send_add_obj_packet(int obj_id, short obj_type);
	void send_move_packet(int obj_id, short obj_type, short dir);
	void send_rotate_packet(int obj_id, short obj_type);
	void send_move_rotate_packet(int obj_id, short obj_type, short dir);

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }

	void sessionClear() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		name[0] = 0;

		pl_state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };
		curr_stage = 0;

		setBB();
	}
};

array<SESSION, MAX_USER> clients;
SESSION npc_server;
array<SESSION, MAX_NPCS> npcs;


void SESSION::send_login_packet() {
	SC_LOGIN_INFO_PACKET login_info_packet;
	login_info_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
	login_info_packet.type = SC_LOGIN_INFO;

	login_info_packet.id = id;
	strcpy_s(login_info_packet.name, name);
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
	login_info_packet.remain_bullet = remain_bullet;

	do_send(&login_info_packet);
}

void SESSION::send_add_obj_packet(int obj_id, short obj_type)
{
	if (curr_stage == 2) return;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.

	switch (obj_type) {
	case TARGET_PLAYER:
		SC_ADD_OBJECT_PACKET add_player_packet;
		add_player_packet.size = sizeof(SC_ADD_OBJECT_PACKET);
		add_player_packet.type = SC_ADD_OBJECT;

		add_player_packet.target = TARGET_PLAYER;
		add_player_packet.id = obj_id;
		strcpy_s(add_player_packet.name, name);

		add_player_packet.x = clients[obj_id].pos.x;
		add_player_packet.y = clients[obj_id].pos.y;
		add_player_packet.z = clients[obj_id].pos.z;

		add_player_packet.right_x = clients[obj_id].m_rightvec.x;
		add_player_packet.right_y = clients[obj_id].m_rightvec.y;
		add_player_packet.right_z = clients[obj_id].m_rightvec.z;

		add_player_packet.up_x = clients[obj_id].m_upvec.x;
		add_player_packet.up_y = clients[obj_id].m_upvec.y;
		add_player_packet.up_z = clients[obj_id].m_upvec.z;

		add_player_packet.look_x = clients[obj_id].m_lookvec.x;
		add_player_packet.look_y = clients[obj_id].m_lookvec.y;
		add_player_packet.look_z = clients[obj_id].m_lookvec.z;

		do_send(&add_player_packet);

		break;

	case TARGET_NPC:
		SC_ADD_OBJECT_PACKET add_npc_packet;
		add_npc_packet.size = sizeof(SC_ADD_OBJECT_PACKET);
		add_npc_packet.type = SC_ADD_OBJECT;

		add_npc_packet.target = TARGET_NPC;
		add_npc_packet.id = obj_id;
		strcpy_s(add_npc_packet.name, name);

		add_npc_packet.x = npcs[obj_id].pos.x;
		add_npc_packet.y = npcs[obj_id].pos.y;
		add_npc_packet.z = npcs[obj_id].pos.z;

		add_npc_packet.right_x = npcs[obj_id].m_rightvec.x;
		add_npc_packet.right_y = npcs[obj_id].m_rightvec.y;
		add_npc_packet.right_z = npcs[obj_id].m_rightvec.z;

		add_npc_packet.up_x = npcs[obj_id].m_upvec.x;
		add_npc_packet.up_y = npcs[obj_id].m_upvec.y;
		add_npc_packet.up_z = npcs[obj_id].m_upvec.z;

		add_npc_packet.look_x = npcs[obj_id].m_lookvec.x;
		add_npc_packet.look_y = npcs[obj_id].m_lookvec.y;
		add_npc_packet.look_z = npcs[obj_id].m_lookvec.z;

		do_send(&add_npc_packet);
		break;
	}
}
void SESSION::send_move_packet(int obj_id, short obj_type, short dir)
{
	SC_MOVE_OBJECT_PACKET move_pl_packet;
	move_pl_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
	move_pl_packet.type = SC_MOVE_OBJECT;
	move_pl_packet.target = obj_type;
	move_pl_packet.id = obj_id;
	move_pl_packet.direction = dir;

	switch (obj_type) {
	case TARGET_PLAYER:
		move_pl_packet.x = clients[obj_id].pos.x;
		move_pl_packet.y = clients[obj_id].pos.y;
		move_pl_packet.z = clients[obj_id].pos.z;
		break;

	case TARGET_NPC:
		move_pl_packet.x = npcs[obj_id].pos.x;
		move_pl_packet.y = npcs[obj_id].pos.y;
		move_pl_packet.z = npcs[obj_id].pos.z;
		break;
	}

	do_send(&move_pl_packet);
}
void SESSION::send_rotate_packet(int obj_id, short obj_type)
{
	SC_ROTATE_OBJECT_PACKET rotate_pl_packet;
	rotate_pl_packet.size = sizeof(SC_ROTATE_OBJECT_PACKET);
	rotate_pl_packet.type = SC_ROTATE_OBJECT;
	rotate_pl_packet.target = obj_type;
	rotate_pl_packet.id = obj_id;

	switch (obj_type) {
	case TARGET_PLAYER:
		rotate_pl_packet.right_x = clients[obj_id].m_rightvec.x;
		rotate_pl_packet.right_y = clients[obj_id].m_rightvec.y;
		rotate_pl_packet.right_z = clients[obj_id].m_rightvec.z;

		rotate_pl_packet.up_x = clients[obj_id].m_upvec.x;
		rotate_pl_packet.up_y = clients[obj_id].m_upvec.y;
		rotate_pl_packet.up_z = clients[obj_id].m_upvec.z;

		rotate_pl_packet.look_x = clients[obj_id].m_lookvec.x;
		rotate_pl_packet.look_y = clients[obj_id].m_lookvec.y;
		rotate_pl_packet.look_z = clients[obj_id].m_lookvec.z;
		break;

	case TARGET_NPC:
		rotate_pl_packet.right_x = npcs[obj_id].m_rightvec.x;
		rotate_pl_packet.right_y = npcs[obj_id].m_rightvec.y;
		rotate_pl_packet.right_z = npcs[obj_id].m_rightvec.z;

		rotate_pl_packet.up_x = npcs[obj_id].m_upvec.x;
		rotate_pl_packet.up_y = npcs[obj_id].m_upvec.y;
		rotate_pl_packet.up_z = npcs[obj_id].m_upvec.z;

		rotate_pl_packet.look_x = npcs[obj_id].m_lookvec.x;
		rotate_pl_packet.look_y = npcs[obj_id].m_lookvec.y;
		rotate_pl_packet.look_z = npcs[obj_id].m_lookvec.z;
		break;
	}
	do_send(&rotate_pl_packet);
}
void SESSION::send_move_rotate_packet(int obj_id, short obj_type, short dir)
{
	SC_MOVE_ROTATE_OBJECT_PACKET update_pl_packet;
	update_pl_packet.size = sizeof(SC_MOVE_ROTATE_OBJECT_PACKET);
	update_pl_packet.type = SC_MOVE_ROTATE_OBJECT;
	update_pl_packet.target = obj_type;
	update_pl_packet.id = obj_id;
	update_pl_packet.direction = dir;

	switch (obj_type) {
	case TARGET_PLAYER:
		update_pl_packet.x = clients[obj_id].pos.x;
		update_pl_packet.y = clients[obj_id].pos.y;
		update_pl_packet.z = clients[obj_id].pos.z;

		update_pl_packet.right_x = clients[obj_id].m_rightvec.x;
		update_pl_packet.right_y = clients[obj_id].m_rightvec.y;
		update_pl_packet.right_z = clients[obj_id].m_rightvec.z;

		update_pl_packet.up_x = clients[obj_id].m_upvec.x;
		update_pl_packet.up_y = clients[obj_id].m_upvec.y;
		update_pl_packet.up_z = clients[obj_id].m_upvec.z;

		update_pl_packet.look_x = clients[obj_id].m_lookvec.x;
		update_pl_packet.look_y = clients[obj_id].m_lookvec.y;
		update_pl_packet.look_z = clients[obj_id].m_lookvec.z;
		break;

	case TARGET_NPC:
		update_pl_packet.x = npcs[obj_id].pos.x;
		update_pl_packet.y = npcs[obj_id].pos.y;
		update_pl_packet.z = npcs[obj_id].pos.z;

		update_pl_packet.right_x = npcs[obj_id].m_rightvec.x;
		update_pl_packet.right_y = npcs[obj_id].m_rightvec.y;
		update_pl_packet.right_z = npcs[obj_id].m_rightvec.z;
		
		update_pl_packet.up_x = npcs[obj_id].m_upvec.x;
		update_pl_packet.up_y = npcs[obj_id].m_upvec.y;
		update_pl_packet.up_z = npcs[obj_id].m_upvec.z;
		
		update_pl_packet.look_x = npcs[obj_id].m_lookvec.x;
		update_pl_packet.look_y = npcs[obj_id].m_lookvec.y;
		update_pl_packet.look_z = npcs[obj_id].m_lookvec.z;
		break;
	}
	do_send(&update_pl_packet);
}

//======================================================================
class HA_SERVER {
	OVER_EX recv_over;

public:
	mutex s_lock;
	SESSION_STATE s_state;
	int id;
	SOCKET socket;
	int remain_size;
	chrono::system_clock::time_point heartbeat_recv_time;
	chrono::system_clock::time_point heartbeat_send_time;

public:
	HA_SERVER() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		heartbeat_recv_time = chrono::system_clock::now();
		heartbeat_send_time = chrono::system_clock::now();
	}
	~HA_SERVER() {}

	void do_recv() {
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
	void do_send(void* packet) {
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
};
array<HA_SERVER, MAX_LOGIC_SERVER> extended_servers;	// HA구현을 위해 수평확장된 서버들

//======================================================================
HANDLE h_iocp;				// IOCP 핸들
SOCKET g_sc_listensock;		// 클라이언트 통신 listen소켓
SOCKET g_npc_listensock;	// NPC서버 통신 listen소켓
SOCKET g_ss_listensock;		// 수평확장 서버 간 통신 listen 소켓
SOCKET g_relay_sock;		// 릴레이서버 간 통신 listen 소켓

SOCKET left_ex_server_sock;								// 이전 번호의 서버
SOCKET right_ex_server_sock;							// 다음 번호의 서버

int my_server_id;										// 내 서버 식별번호
bool b_active_server;									// Active 서버인가?
bool b_npcsvr_conn;										// NPC서버가 연결되었는가?

//======================================================================
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

		cout << "Player[ID: " << clients[target_id].id << ", name: " << clients[target_id].name << " is log out\n" << endl;	// server message

		// 남아있는 모든 클라이언트들에게 target_id번 클라이언트가 접속 종료한 사실을 알립니다.
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

			pl.do_send(&remove_pl_packet);
			pl.s_lock.unlock();
		}

		// NPC 서버에게도 target_id번 클라이언트가 접속 종료한 사실을 알립니다.
		if (b_npcsvr_conn) {
			SC_REMOVE_OBJECT_PACKET remove_pl_packet;
			remove_pl_packet.target = TARGET_PLAYER;
			remove_pl_packet.id = target_id;
			remove_pl_packet.size = sizeof(remove_pl_packet);
			remove_pl_packet.type = SC_REMOVE_OBJECT;

			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.do_send(&remove_pl_packet);
		}

		break;

	case SESSION_NPC:
		cout << "NPC 서버가 다운되었습니다." << endl;
		closesocket(npc_server.socket);
		npc_server.socket = 0;
		b_npcsvr_conn = false;
		break;

	case SESSION_EXTENDED_SERVER:
	{
		extended_servers[target_id].s_lock.lock();
		if (extended_servers[target_id].s_state == ST_FREE) {
			extended_servers[target_id].s_lock.unlock();
			return;
		}
		closesocket(extended_servers[target_id].socket);
		extended_servers[target_id].s_state = ST_FREE;
		extended_servers[target_id].s_lock.unlock();

		cout << "Server[" << extended_servers[target_id].id << "]의 다운이 감지되었습니다." << endl;	// server message

		// 서버 재실행
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + target_id);	// 십의자리: Actvie여부(S: 1, A: 2), 일의자리: 서버ID

		// XD폴더 내에서 동작할 때(내부 테스트)와 외부에서 실행할 때를 구분해줍니다.
		string XDFolderKeyword = "XD";
		if (filesystem::current_path().string().find(XDFolderKeyword) != string::npos) {
			ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../../../Execute/Execute_S", SW_SHOW);	// 내부 테스트용
		}
		else {
			ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L".", SW_SHOW);					// 외부 수출용 (exe로 실행될때)
		}

		// 원격 이중화를 위해선 실행되는 PC의 "외부 IP"를 알아야 한다.


		// 클라이언트에게 Active서버가 다운되었다고 알려줌.
		if (!b_active_server) {	// 내가 Active가 아니면 상대가 Active임. (서버가 2개밖에 없기 때문)
			b_active_server = true;
			cout << "현재 Server[" << my_server_id << "] 가 Active 서버로 승격되었습니다. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// 만약 자신의 오른쪽 서버가 다운되었는데, 그 서버가 서버군의 마지막 서버인 경우 재실행된 서버에게 ConnectEx 요청을 보냅니다.
		if (target_id == MAX_LOGIC_SERVER - 1) {
			if (my_server_id < target_id) {
				// ConnectEx
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				int key = CP_KEY_LOGIC2EXLOGIC + MAX_LOGIC_SERVER - 1;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_LOGIC2EXLOGIC + HA_PORTNUM_S0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버
				inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect 재시도 중...\n" << endl;
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
		}
		break;
	}
	}
}

//======================================================================
void process_packet(int client_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
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

		// 서버구동 이후 첫번째 클라이언트의 접속이라면 그때부터 서버시간이 흐르기 시작합니다.
		if (b_isfirstplayer) {
			cout << "서버 시간이 흐르기 시작합니다.\n" << endl;
			g_s_start_time = system_clock::now();
			b_isfirstplayer = false;
		}

		// 새로 접속한 플레이어의 초기 정보를 설정합니다.
		clients[client_id].s_state = ST_INGAME;
		clients[client_id].pl_state = PL_ST_IDLE;
		clients[client_id].curr_stage = 0;
		clients[client_id].hp = HELI_MAXHP;
		strcpy_s(clients[client_id].name, login_packet->name);

		clients[client_id].pos.x = RESPAWN_POS_X;
		clients[client_id].pos.y = RESPAWN_POS_Y;
		clients[client_id].pos.z = RESPAWN_POS_Z - 100 * client_id;

		clients[client_id].m_rightvec = basic_coordinate.right;
		clients[client_id].m_upvec = basic_coordinate.up;
		clients[client_id].m_lookvec = basic_coordinate.look;

		clients[client_id].setBB();

		clients[client_id].send_login_packet();
		clients[client_id].s_lock.unlock();

		cout << "Player[ID: " << clients[client_id].id << ", name: " << clients[client_id].name << "]이(가) 접속하였습니다." << endl;	// server message

		if (!b_active_server) {
			cout << "Stand-By서버는 대기 상태를 유지합니다." << endl;
			break;	// Active서버가 아니라면, 클라이언트가 연결되었음을 사용자에게 알리기만 하고 아무일도 하지 않습니다.
		}

		cout << "새로운 오브젝트가 생성되었습니다! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ").\n" << endl;

		//====================
		// 1. Player 객체 정보
		//  1) Clients
		// 현재 접속해 있는 모든 클라이언트에게 새로운 클라이언트(client_id)의 정보를 전송합니다.
		for (auto& pl : clients) {
			if (pl.s_state != ST_INGAME) continue;
			if (pl.id == client_id) continue;
			if (pl.curr_stage != 1) continue;// 로그인을 하면 1스테이지로 넘어가지기 때문

			lock_guard<mutex> lg{ pl.s_lock };
			pl.send_add_obj_packet(client_id, TARGET_PLAYER);
		}

		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 클라이언트의 정보를 전송합니다.
		for (auto& pl : clients) {
			if (pl.s_state != ST_INGAME) continue;
			if (pl.id == client_id) continue;
			if (pl.curr_stage != 1) continue;// 로그인을 하면 1스테이지로 넘어가지기 때문.

			lock_guard<mutex> lg{ pl.s_lock };
			clients[client_id].send_add_obj_packet(pl.id, TARGET_PLAYER);
		}

		//  2) NPCs
		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 NPC의 정보를 전송합니다.
		for (auto& npc : npcs) {
			if (npc.id == -1) continue;

			lock_guard<mutex> lg{ clients[client_id].s_lock};
			clients[client_id].send_add_obj_packet(npc.id, TARGET_NPC);
		}
		
		//  3) NPC서버로 새로 접속한 클라이언트의 정보를 전송합니다.
		if (b_npcsvr_conn) {
			npc_server.send_add_obj_packet(client_id, TARGET_PLAYER);
		}

		//====================
		// 2. 맵 정보
		// 새로 접속한 클라이언트에게 맵 정보를 보내줍니다.
		for (auto& building : buildings_info) {
			SC_MAP_OBJINFO_PACKET building_packet;
			building_packet.type = SC_MAP_OBJINFO;
			building_packet.size = sizeof(SC_MAP_OBJINFO_PACKET);

			building_packet.pos_x = building.getPosX();
			building_packet.pos_y = building.getPosY();
			building_packet.pos_z = building.getPosZ();

			building_packet.scale_x = building.getScaleX();
			building_packet.scale_y = building.getScaleY();
			building_packet.scale_z = building.getScaleZ();

			clients[client_id].do_send(&building_packet);
		}
		break;
	}// CS_LOGIN end
	case CS_MOVE:
	{
		if (!b_active_server) break;
		CS_MOVE_PACKET* cl_move_packet = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		// 1. 충돌체크를 한다. -> 플레이어가 이동할 수 없는 곳으로 이동했다면 RollBack패킷을 보내 이전 위치로 돌아가도록 한다.
		bool b_isCollide = false;
		//  1) 맵 오브젝트

		//  2) 다른 플레이어

		//  3) NPC
		
		//if (b_isCollide) break;

		// 2. 충돌이 아니라면 세션 정보를 업데이트 한다.
		clients[client_id].s_lock.lock();
		clients[client_id].pos = { cl_move_packet->x, cl_move_packet->y, cl_move_packet->z };
		clients[client_id].setBB();
		clients[client_id].pl_state = cl_move_packet->direction + 1;	// MV_FRONT = 0, MV_BACK = 1, MV_SIDE = 2; PL_ST_MOVE_FRONT = 1, PL_ST_MOVE_BACK = 2, PL_ST_MOVE_SIDE = 3;
		clients[client_id].s_lock.unlock();

		cout << "[Move TEST] Player[" << client_id << "]가 " << cl_move_packet->direction << " 방향으로 이동하여 POS가 ("
			<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.x << ")가 되었음.\n" << endl;

		// 3. 다른 클라이언트에게 플레이어가 이동한 위치를 알려준다.
		for (auto& other_pl : clients) {
			if (other_pl.id == client_id) continue;
			if (other_pl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ other_pl.s_lock };
			other_pl.send_move_packet(client_id, TARGET_PLAYER, cl_move_packet->direction);
		}

		// 4. NPC 서버에게도 플레이어가 이동한 위치를 알려준다.
		lock_guard<mutex> lg{ npc_server.s_lock };
		npc_server.send_move_packet(client_id, TARGET_PLAYER, cl_move_packet->direction);

		break;
	}// CS_MOVE end
	case CS_ROTATE:
	{
		if (!b_active_server) break;
		CS_ROTATE_PACKET* cl_rotate_packet = reinterpret_cast<CS_ROTATE_PACKET*>(packet);

		// 1. 세션 정보를 업데이트 한다.
		clients[client_id].s_lock.lock();
		clients[client_id].m_rightvec = { cl_rotate_packet->right_x, cl_rotate_packet->right_y, cl_rotate_packet->right_z };
		clients[client_id].m_upvec = { cl_rotate_packet->up_x, cl_rotate_packet->up_y, cl_rotate_packet->up_z };
		clients[client_id].m_lookvec = { cl_rotate_packet->look_x, cl_rotate_packet->look_y, cl_rotate_packet->look_z };
		clients[client_id].setBB();
		clients[client_id].s_lock.unlock();

		// 2. 다른 클라이언트에게 플레이어가 회전한 방향을 알려준다.
		for (auto& other_pl : clients) {
			if (other_pl.id == client_id) continue;
			if (other_pl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ other_pl.s_lock };
			other_pl.send_rotate_packet(client_id, TARGET_PLAYER);
		}

		// 4. NPC 서버에게도 플레이어가 회전한 방향를 알려준다.
		lock_guard<mutex> lg{ npc_server.s_lock };
		npc_server.send_rotate_packet(client_id, TARGET_PLAYER);

		break;
	}// CS_ROTATE end
	case CS_ATTACK:
	{
		if (!b_active_server) break;
		CS_ATTACK_PACKET* cl_attack_packet = reinterpret_cast<CS_ATTACK_PACKET*>(packet);

		//==============================
		// 1. 총알
		// Bullet 쿨타임 체크
		milliseconds shoot_term = duration_cast<milliseconds>(chrono::system_clock::now() - clients[client_id].shoot_time);
		if (shoot_term < milliseconds(SHOOT_COOLDOWN_BULLET)) break;	// 쿨타임이 끝나지 않았다면 발사하지 않습니다.

		// Bullet 개수 체크
		bool enough_bullet = true;
		if (clients[client_id].remain_bullet <= 0) {
			// Bullet 장전 중 여부 체크
			milliseconds reload_term = duration_cast<milliseconds>(chrono::system_clock::now() - clients[client_id].reload_time);
			if (reload_term < milliseconds(RELOAD_TIME)) {
				enough_bullet = false;
			}
			else {
				clients[client_id].s_lock.lock();
				clients[client_id].remain_bullet = MAX_BULLET;
				clients[client_id].s_lock.unlock();

				if (clients[client_id].s_state == ST_INGAME) {
					SC_BULLET_COUNT_PACKET reload_done_pack;
					reload_done_pack.type = SC_BULLET_COUNT;
					reload_done_pack.size = sizeof(SC_BULLET_COUNT_PACKET);
					reload_done_pack.bullet_cnt = MAX_BULLET;

					lock_guard<mutex> lg{ clients[client_id].s_lock };
					clients[client_id].do_send(&reload_done_pack);
				}
			}
		}
		if (!enough_bullet) break;

		// 세션정보 업데이트
		clients[client_id].s_lock.lock();
		clients[client_id].shoot_time = chrono::system_clock::now(); // 발사 시간 업데이트
		clients[client_id].remain_bullet -= 1;
		if (clients[client_id].remain_bullet <= 0) { // 장전 시작
			clients[client_id].remain_bullet = 0;
			clients[client_id].reload_time = chrono::system_clock::now();
		}
		clients[client_id].s_lock.unlock();

		if (clients[client_id].s_state == ST_INGAME) {
			SC_BULLET_COUNT_PACKET bullet_update_pack;
			bullet_update_pack.type = SC_BULLET_COUNT;
			bullet_update_pack.size = sizeof(SC_BULLET_COUNT_PACKET);
			bullet_update_pack.bullet_cnt = clients[client_id].remain_bullet;

			lock_guard<mutex> lg{ clients[client_id].s_lock };
			clients[client_id].do_send(&bullet_update_pack);
		}

		// 우선 이 플레이어가 총알을 발사했다는 정보를 "게임에 접속 중인 모든" 클라이언트에게 알려줍니다. (총알 나가는 모션 동기화를 위함)
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_stage == 0) continue;

			SC_OBJECT_STATE_PACKET atk_pack;
			atk_pack.type = SC_OBJECT_STATE;
			atk_pack.size = sizeof(SC_OBJECT_STATE_PACKET);
			atk_pack.target = TARGET_PLAYER;
			atk_pack.id = client_id;
			atk_pack.state = PL_ST_ATTACK;
			cl.do_send(&atk_pack);
		}

		//==============================
		// 2. 충돌검사
		if (clients[client_id].curr_stage == 1) {
			MyVector3 intersection_result;

			bool b_collide = false;

			enum CollideTarget {CT_PLAYER, CT_NPC, CT_BUILDING};
			int collide_target = -1;

			int collide_id = -1;

			float min_dist = FLT_MAX;

			// 1) 클라이언트와 충돌검사
			for (auto& cl : clients) {
				if (cl.id == client_id) continue;		// 자기자신은 검사하면 안됨.
				if (cl.s_state != ST_INGAME) continue;	// 게임중이 아닌 세션은 검사할 필요X
				if (cl.curr_stage == 0) continue;		// 아직 게임 진입 중인 세션도 검사 X

				// 플레이어의 좌표와 룩벡터를 갖고 레이캐스트를 합니다.
				Cube pl_obj{ exc_XMFtoMyVec(cl.pos), HUMAN_BOXSIZE_X, HUMAN_BOXSIZE_Y, HUMAN_BOXSIZE_Z };
				MyVector3 tmp_intersection = MyRaycast_LimitDistance(
					MyVector3{ clients[client_id].pos.x, clients[client_id].pos.y, clients[client_id].pos.z }
					, exc_XMFtoMyVec(clients[client_id].m_lookvec), pl_obj, BULLET_RANGE);

				// 충돌했다면
				if (tmp_intersection != defaultVec) {
					b_collide = true;

					// 쏜 사람과의 충돌점과의 거리를 잽니다.
					float cur_dist = calcDistance(exc_XMFtoMyVec(clients[client_id].pos), tmp_intersection);

					// 그 거리가 최소거리보다 작다면 업데이트해줍니다.
					if (cur_dist < min_dist) {
						intersection_result = tmp_intersection;
						collide_target = CT_PLAYER;
						collide_id = cl.id;
						min_dist = cur_dist;
					}
				}
			}

			// 2) NPC와 충돌검사
			for (auto& npc : npcs) {
				// 플레이어의 좌표와 룩벡터를 갖고 레이캐스트를 합니다.
				Cube npc_obj{ exc_XMFtoMyVec(npc.pos), HELI_BOXSIZE_X, HELI_BOXSIZE_Y, HELI_BOXSIZE_Z };
				MyVector3 tmp_intersection = MyRaycast_LimitDistance(
					MyVector3{ clients[client_id].pos.x, clients[client_id].pos.y, clients[client_id].pos.z }
					, exc_XMFtoMyVec(clients[client_id].m_lookvec), npc_obj, BULLET_RANGE);

				// 충돌했다면
				if (tmp_intersection != defaultVec) {
					b_collide = true;

					// 쏜 사람과의 충돌점과의 거리를 잽니다.
					float cur_dist = calcDistance(exc_XMFtoMyVec(clients[client_id].pos), tmp_intersection);

					// 그 거리가 최소거리보다 작다면 업데이트해줍니다.
					if (cur_dist < min_dist) {
						intersection_result = tmp_intersection;
						collide_target = CT_NPC;
						collide_id = npc.id;
						min_dist = cur_dist;
					}
				}
			}

			// 3) 건물과 충돌검사
			for (auto& building : buildings_info) {
				Cube bd_obj{ exc_XMFtoMyVec(building.getPos()), building.getScaleX(), building.getScaleY(), building.getScaleZ() };
				MyVector3 bd_intersection = MyRaycast_LimitDistance(
					MyVector3{ clients[client_id].pos.x, clients[client_id].pos.y, clients[client_id].pos.z }
					, exc_XMFtoMyVec(clients[client_id].m_lookvec), bd_obj, BULLET_RANGE);

				// 충돌했다면
				if (bd_intersection != defaultVec) {
					b_collide = true;

					// 쏜 사람과의 충돌점과의 거리를 잽니다.
					float cur_dist = calcDistance(exc_XMFtoMyVec(clients[client_id].pos), bd_intersection);

					// 그 거리가 최소거리보다 작다면 업데이트해줍니다.
					if (cur_dist < min_dist) {
						intersection_result = bd_intersection;
						collide_target = CT_BUILDING;
						collide_id = 0;
						min_dist = cur_dist;
					}
				}
			}

			// 4) 최종
			// 레이캐스트에서 충돌으로 판단되었다면
			if (b_collide) {
				switch (collide_target) {
				case CT_PLAYER:
					cout << "Player[" << collide_id << "]가 피격당했습니다. (공격자: Player[" << client_id << "])" << endl;

					// 피격된 플레이어의 HP를 감소시킵니다.
					clients[collide_id].s_lock.lock();
					clients[collide_id].hp -= BULLET_DAMAGE;
					clients[collide_id].s_lock.unlock();

					if (clients[collide_id].hp <= 0) {
						cout << "Player[" << collide_id << "]가 사망하였습니다.\n" << endl;

						// 사망 정보를 게임에 접속한 모든 클라이언트들에게 보냅니다. (자신 포함)
						for (auto& cl : clients) {
							if (cl.s_state != ST_INGAME) continue;
							if (cl.curr_stage == 0) continue;

							SC_OBJECT_STATE_PACKET death_pack;
							death_pack.size = sizeof(SC_OBJECT_STATE_PACKET);
							death_pack.type = SC_OBJECT_STATE;
							death_pack.target = TARGET_PLAYER;
							death_pack.id = clients[collide_id].id;
							death_pack.state = PL_ST_DEAD;
							cl.do_send(&death_pack);
						}
					}
					else {
						cout << "Player[" << clients[collide_id].id << "]의 HP가 " << BULLET_DAMAGE << "만큼 감소하였습니다. \n" << endl;

						// 피격 정보를 게임에 접속한 모든 클라이언트들에게 보냅니다. (자신 포함)
						for (auto& cl : clients) {
							if (cl.s_state != ST_INGAME) continue;
							if (cl.curr_stage == 0) continue;

							SC_DAMAGED_PACKET dmg_bullet_packet;
							dmg_bullet_packet.size = sizeof(SC_DAMAGED_PACKET);
							dmg_bullet_packet.type = SC_DAMAGED;
							dmg_bullet_packet.target = TARGET_PLAYER;
							dmg_bullet_packet.id = clients[collide_id].id;
							dmg_bullet_packet.damage = BULLET_DAMAGE;
							cl.do_send(&dmg_bullet_packet);
						}
					}
					break;

				case CT_NPC:
					cout << "NPC[" << collide_id << "]가 피격당했습니다. (공격자: Player[" << client_id << "])\n" << endl;

					// 피격된 npc의 HP를 감소시킵니다.
					npcs[collide_id].s_lock.lock();
					npcs[collide_id].hp -= BULLET_DAMAGE;
					npcs[collide_id].s_lock.unlock();

					if (npcs[collide_id].hp <= 0) {
						cout << "Npc[" << collide_id << "]가 사망하였습니다.\n" << endl;

						// 사망 정보를 게임에 접속한 모든 클라이언트들에게 보냅니다. (자신 포함)
						for (auto& cl : clients) {
							if (cl.s_state != ST_INGAME) continue;
							if (cl.curr_stage == 0) continue;

							SC_OBJECT_STATE_PACKET death_pack;
							death_pack.size = sizeof(SC_OBJECT_STATE_PACKET);
							death_pack.type = SC_OBJECT_STATE;
							death_pack.target = TARGET_NPC;
							death_pack.id = npcs[collide_id].id;
							death_pack.state = PL_ST_DEAD;
							cl.do_send(&death_pack);
						}

						// 사망 정보를 npc서버에도 보냅니다.
						SC_OBJECT_STATE_PACKET npc_death_pack;
						npc_death_pack.size = sizeof(SC_OBJECT_STATE_PACKET);
						npc_death_pack.type = SC_OBJECT_STATE;
						npc_death_pack.target = TARGET_NPC;
						npc_death_pack.id = npcs[collide_id].id;
						npc_death_pack.state = PL_ST_DEAD;
						npc_server.do_send(&npc_death_pack);
					}
					else {
						cout << "Npc[" << npcs[collide_id].id << "]의 HP가 " << BULLET_DAMAGE << "만큼 감소하였습니다. \n" << endl;

						// 피격 정보를 게임에 접속한 모든 클라이언트들에게 보냅니다. (자신 포함)
						for (auto& cl : clients) {
							if (cl.s_state != ST_INGAME) continue;
							if (cl.curr_stage == 0) continue;

							SC_DAMAGED_PACKET dmg_bullet_packet;
							dmg_bullet_packet.size = sizeof(SC_DAMAGED_PACKET);
							dmg_bullet_packet.type = SC_DAMAGED;
							dmg_bullet_packet.target = TARGET_NPC;
							dmg_bullet_packet.id = npcs[collide_id].id;
							dmg_bullet_packet.damage = BULLET_DAMAGE;
							cl.do_send(&dmg_bullet_packet);
						}

						// 피격 정보를 npc서버에도 보냅니다.
						SC_DAMAGED_PACKET npc_damaged_packet;
						npc_damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
						npc_damaged_packet.type = SC_DAMAGED;
						npc_damaged_packet.target = TARGET_NPC;
						npc_damaged_packet.id = npcs[collide_id].id;
						npc_damaged_packet.damage = BULLET_DAMAGE;
						npc_server.do_send(&npc_damaged_packet);
					}
					break;

				case CT_BUILDING:
					cout << "총알이 건물에 막혔다. (공격자: Player[" << client_id << "])\n" << endl;
					// 지금은 딱히 할 기능이 없다.
					// 나중에 건물에 파편이 튀기는 연출을 하고싶으면
					// 클라한테 충돌한 지점(좌표)를 보내주면 된다.
					break;

				default:
					cout << "[MyRayCast Error] Unknown Interserction Type.\n" << endl;
				}


			}

		}
		// 2. 스테이지 2 로직
		else if (clients[client_id].curr_stage == 2) {

		}

		break;
	}// CS_ATTACK end
	case CS_INPUT_KEYBOARD:
	{
		if (!b_active_server) break;		// Active 서버만 패킷을 처리합니다.
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		float sign = 1.0f;					// right/up/look벡터 방향으로 움직이는지, 반대 방향으로 움직이는지
		switch (inputkey_p->keytype) {
		case PACKET_KEY_NUM1:
			if (clients[client_id].curr_stage == 1) break;

			clients[client_id].s_lock.lock();
			clients[client_id].curr_stage = 1;
			cout << "Client[" << client_id << "] Stage1로 전환." << endl;
			clients[client_id].s_lock.unlock();

			SC_CHANGE_SCENE_PACKET chg_scene1_pack;
			chg_scene1_pack.type = SC_CHANGE_SCENE;
			chg_scene1_pack.size = sizeof(SC_CHANGE_SCENE_PACKET);
			chg_scene1_pack.id = client_id;
			chg_scene1_pack.scene_num = 1;
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;

				lock_guard<mutex> lg{ cl.s_lock };
				cl.do_send(&chg_scene1_pack);
			}
			break;
		case PACKET_KEY_NUM2:
			if (clients[client_id].curr_stage == 2) break;

			clients[client_id].s_lock.lock();
			clients[client_id].curr_stage = 2;
			cout << "Client[" << client_id << "] Stage2로 전환." << endl;
			clients[client_id].s_lock.unlock();

			SC_CHANGE_SCENE_PACKET chg_scene2_pack;
			chg_scene2_pack.type = SC_CHANGE_SCENE;
			chg_scene2_pack.size = sizeof(SC_CHANGE_SCENE_PACKET);
			chg_scene2_pack.id = client_id;
			chg_scene2_pack.scene_num = 2;
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;

				lock_guard<mutex> lg{ cl.s_lock };
				cl.do_send(&chg_scene2_pack);
			}
			break;
		case PACKET_KEYUP_MOVEKEY:
			if (PL_ST_MOVE_FRONT <= clients[client_id].pl_state && clients[client_id].pl_state <= PL_ST_MOVE_SIDE) {
				clients[client_id].s_lock.lock();
				clients[client_id].pl_state = PL_ST_IDLE;
				clients[client_id].s_lock.unlock();

				SC_OBJECT_STATE_PACKET change2idle_pack;
				change2idle_pack.type = SC_OBJECT_STATE;
				change2idle_pack.size = sizeof(SC_OBJECT_STATE_PACKET);
				change2idle_pack.target = TARGET_PLAYER;
				change2idle_pack.id = client_id;
				change2idle_pack.state = PL_ST_IDLE;

				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage == 0) continue;
					//if (cl.id == client_id) continue;

					lock_guard<mutex> lg{ cl.s_lock };
					cl.do_send(&change2idle_pack);
				}
			}
			break;
		}

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE:
	{
		if (!b_active_server) break;		// Active 서버만 패킷을 처리합니다.
		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->buttontype == PACKET_NONCLICK) {			// 버튼 안누름 (그냥 이동만 한 경우)
		}
		else if (rt_p->buttontype == PACKET_BUTTON_L) {			// 마우스 좌클릭
			// 1스테이지 로직
			if (clients[client_id].curr_stage == 1) {

			}
			// 2스테이지에선 ATTACK_PACKET을 보냄.
		}
		else if (rt_p->buttontype == PACKET_BUTTON_R) {		// 마우스 우클릭 드래그: 기능 미정.
			// 1스테이지 로직
			if (clients[client_id].curr_stage == 1) {

			}
			// 2스테이지 로직
			else if (clients[client_id].curr_stage == 2) {

			}
		}

		break;
	}// CS_INPUT_MOUSE end
	case CS_PING:
	{
		CS_PING_PACKET* re_login_pack = reinterpret_cast<CS_PING_PACKET*>(packet);

		SC_PING_RETURN_PACKET ping_ret_pack;
		ping_ret_pack.type = SC_PING_RETURN;
		ping_ret_pack.size = sizeof(SC_PING_RETURN_PACKET);
		ping_ret_pack.ping_sender_id = client_id;
		clients[client_id].do_send(&ping_ret_pack);

		break;
	}// CS_PING end
	case CS_RELOGIN:
	{
		CS_RELOGIN_PACKET* re_login_pack = reinterpret_cast<CS_RELOGIN_PACKET*>(packet);

		int re_login_id = re_login_pack->id;
		clients[re_login_id].s_lock.lock();
		clients[re_login_id].s_state = ST_INGAME;
		clients[re_login_id].setBB();
		clients[re_login_id].s_lock.unlock();

		cout << "[HA] Clients[" << re_login_id << "]와 다시 연결되었습니다.\n" << endl;

		break;
	}// CS_RELOGIN end
	case SS_HEARTBEAT:
	{
		SS_HEARTBEAT_PACKET* heartbeat_pack = reinterpret_cast<SS_HEARTBEAT_PACKET*>(packet);
		int recv_id = heartbeat_pack->sender_id;

		extended_servers[recv_id].heartbeat_recv_time = chrono::system_clock::now();
		if (recv_id < my_server_id) {	// A->B->A로 heartbeat의 한 사이클이 끝나도록하기 위함. (즉, 오른쪽 서버로부터 Heartbeat를 받으면 한 사이클의 끝으로 판단)
			// Heartbeat를 먼저 보낸 서버에게 자신의 Heartbeat를 전송합니다.
			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[recv_id].do_send(&hb_packet);										// 자신에게 Heartbeat를 보낸 서버에게 전송합니다.
			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// 전송한 시간을 업데이트
		}
		break;
	}// SS_HEARTBEAT end
	case SS_DATA_REPLICA:
	{
		SS_DATA_REPLICA_PACKET* replica_pack = reinterpret_cast<SS_DATA_REPLICA_PACKET*>(packet);

		int replica_id = replica_pack->id;
		clients[replica_id].s_lock.lock();
		clients[replica_id].id = replica_id;
		clients[replica_id].s_state = ST_FREE;
		strcpy_s(clients[replica_id].name, replica_pack->name);

		clients[replica_id].pos = { replica_pack->x, replica_pack->y, replica_pack->z };

		clients[replica_id].m_rightvec = { replica_pack->right_x, replica_pack->right_y, replica_pack->right_z };
		clients[replica_id].m_upvec = { replica_pack->up_x, replica_pack->up_y, replica_pack->up_z };
		clients[replica_id].m_lookvec = { replica_pack->look_x, replica_pack->look_y, replica_pack->look_z };

		clients[replica_id].pl_state = replica_pack->state;
		clients[replica_id].hp = replica_pack->hp;
		clients[replica_id].remain_bullet = replica_pack->bullet_cnt;
		clients[replica_id].curr_stage = replica_pack->curr_stage;

		clients[replica_id].s_lock.unlock();

		//cout << "Client[" << replica_id << "]의 데이터가 복제되었습니다." << endl;
		//cout << "===================================" << endl;
		//cout << "Name: " << clients[replica_id].name << endl;
		//cout << "Stage: " << clients[replica_id].curr_stage << endl;
		//cout << "State: " << clients[replica_id].pl_state << endl;
		//cout << "Pos: " << clients[replica_id].pos.x << ", " << clients[replica_id].pos.y << ", " << clients[replica_id].pos.z << endl;
		//cout << "LookVec: " << clients[replica_id].m_lookvec.x << ", " << clients[replica_id].m_lookvec.y << ", " << clients[replica_id].m_lookvec.z << endl;
		//cout << "STime: " << replica_pack->curr_stage << "ms." << endl;
		//cout << "===================================\n" << endl;
		break;
	}// SS_DATA_REPLICA end
	case NPC_FULL_INFO:
	{
		NPC_FULL_INFO_PACKET* npc_info_pack = reinterpret_cast<NPC_FULL_INFO_PACKET*>(packet);

		short npc_id = npc_info_pack->n_id;
		
		npcs[npc_id].s_lock.lock();
		npcs[npc_id].hp = npc_info_pack->hp;
		strcpy_s(npcs[npc_id].name, npc_info_pack->name);
		npcs[npc_id].id = npc_info_pack->n_id;
		npcs[npc_id].pos = { npc_info_pack->x, npc_info_pack->y, npc_info_pack->z };
		npcs[npc_id].m_rightvec = { npc_info_pack->right_x, npc_info_pack->right_y, npc_info_pack->right_z };
		npcs[npc_id].m_upvec = { npc_info_pack->up_x, npc_info_pack->up_y, npc_info_pack->up_z };
		npcs[npc_id].m_lookvec = { npc_info_pack->look_x, npc_info_pack->look_y, npc_info_pack->look_z };
		npcs[npc_id].s_lock.unlock();

		cout << "NPC[" << npc_id << "]의 모든 정보를 받았습니다.\n" << endl;

		break;
	}// NPC_FULL_INFO end
	case NPC_MOVE:
	{
		NPC_MOVE_PACKET* npc_move_pack = reinterpret_cast<NPC_MOVE_PACKET*>(packet);

		short npc_id = npc_move_pack->n_id;

		npcs[npc_id].s_lock.lock();
		npcs[npc_id].pos = { npc_move_pack->x, npc_move_pack->y, npc_move_pack->z };
		npcs[npc_id].s_lock.unlock();
		//cout << "NPC[" << npc_id << "]가 POS(" << npcs[npc_id].pos.x << ", " << npcs[npc_id].pos.y << ", " << npcs[npc_id].pos.z << ")로 이동하였습니다.\n" << endl;

		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 NPC 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.send_move_packet(npc_id, TARGET_NPC, MV_FRONT);//[임시코드] 나중에 npc도 이동방향을 받도록 해줘야함
		}

		break;
	}// NPC_MOVE end
	case NPC_ROTATE:
	{
		NPC_ROTATE_PACKET* npc_rotate_pack = reinterpret_cast<NPC_ROTATE_PACKET*>(packet);

		short npc_id = npc_rotate_pack->n_id;

		npcs[npc_id].s_lock.lock();
		npcs[npc_id].m_rightvec = { npc_rotate_pack->right_x, npc_rotate_pack->right_y, npc_rotate_pack->right_z };
		npcs[npc_id].m_upvec = { npc_rotate_pack->up_x, npc_rotate_pack->up_y, npc_rotate_pack->up_z };
		npcs[npc_id].m_lookvec = { npc_rotate_pack->look_x, npc_rotate_pack->look_y, npc_rotate_pack->look_z };
		npcs[npc_id].s_lock.unlock();
		/*cout << "NPC[" << npc_id << "]가 Look(" << npcs[npc_id].m_lookvec.x << ", " << npcs[npc_id].m_lookvec.y << ", " << npcs[npc_id].m_lookvec.z
			<< ") 방향으로 회전하였습니다.\n" << endl;*/

		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 NPC 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.send_rotate_packet(npc_id, TARGET_NPC);
		}

		break;
	}// NPC_ROTATE end
	case NPC_MOVE_ROTATE:
	{
		NPC_MOVE_ROTATE_PACKET* npc_mvrt_pack = reinterpret_cast<NPC_MOVE_ROTATE_PACKET*>(packet);

		short npc_id = npc_mvrt_pack->n_id;

		npcs[npc_id].s_lock.lock();
		npcs[npc_id].pos = { npc_mvrt_pack->x, npc_mvrt_pack->y, npc_mvrt_pack->z };
		npcs[npc_id].m_rightvec = { npc_mvrt_pack->right_x, npc_mvrt_pack->right_y, npc_mvrt_pack->right_z };
		npcs[npc_id].m_upvec = { npc_mvrt_pack->up_x, npc_mvrt_pack->up_y, npc_mvrt_pack->up_z };
		npcs[npc_id].m_lookvec = { npc_mvrt_pack->look_x, npc_mvrt_pack->look_y, npc_mvrt_pack->look_z };
		npcs[npc_id].s_lock.unlock();
		/*cout << "NPC[" << npc_id << "]가 POS(" << npcs[npc_id].pos.x << ", " << npcs[npc_id].pos.y << ", " << npcs[npc_id].pos.z << ")로 이동하였습니다.\n" << endl;
		cout << "NPC[" << npc_id << "]가 Look(" << npcs[npc_id].m_lookvec.x << ", " << npcs[npc_id].m_lookvec.y << ", " << npcs[npc_id].m_lookvec.z
			<< ") 방향으로 회전하였습니다.\n" << endl;*/

		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 NPC 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.send_move_rotate_packet(npc_id, TARGET_NPC, MV_FRONT);//[임시코드] 나중에 npc 방향을 넣어줘야함.
		}

		break;
	}// NPC_MOVE_ROTATE end
	case NPC_REMOVE:
	{
		NPC_REMOVE_PACKET* npc_remove_pack = reinterpret_cast<NPC_REMOVE_PACKET*>(packet);

		short npc_id = npc_remove_pack->n_id;

		npcs[npc_id].s_lock.lock();
		npcs[npc_id].sessionClear();
		npcs[npc_id].s_lock.unlock();
		//cout << "NPC[" << npc_id << "]가 삭제되었습니다.\n" << endl;

		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 NPC 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			SC_REMOVE_OBJECT_PACKET rm_npc_pack;
			rm_npc_pack.size = sizeof(SC_REMOVE_OBJECT_PACKET);
			rm_npc_pack.type = SC_REMOVE_OBJECT;
			rm_npc_pack.target = TARGET_NPC;
			rm_npc_pack.id = npc_id;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.do_send(&rm_npc_pack);
		}

		break;
	}// NPC_REMOVE end
	case NPC_CHANGE_STATE:
	{
		NPC_CHANGE_STATE_PACKET* npc_chgstate_pack = reinterpret_cast<NPC_CHANGE_STATE_PACKET*>(packet);

		break;
	}// NPC_CHANGE_STATE_PACKET end
	}
}

//======================================================================
int get_new_client_id()	// clients의 비어있는 칸을 찾아서 새로운 client의 아이디를 할당해주는 함수
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

//======================================================================
int find_empty_extended_server() {	// ex_servers의 비어있는 칸을 찾아서 새로운 Server_ex의 아이디를 할당해주는 함수
	for (int i = 0; i < MAX_USER; ++i) {
		extended_servers[i].s_lock.lock();
		if (extended_servers[i].s_state == ST_FREE) {
			extended_servers[i].s_state = ST_ACCEPTED;
			extended_servers[i].heartbeat_recv_time = chrono::system_clock::now();
			extended_servers[i].heartbeat_send_time = chrono::system_clock::now();
			extended_servers[i].s_lock.unlock();
			return i;
		}
		extended_servers[i].s_lock.unlock();
	}
	return -1;
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
			if (ex_over->process_type == OP_ACCEPT) {
				cout << "Accept Error";
			}
			else if (ex_over->process_type == OP_CONNECT) {
				//cout << "Connect Error" << endl;

				// 비동기Conn를 다시 시도합니다.
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_LOGIC2EXLOGIC + HA_PORTNUM_S0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);
				inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
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
				// 1. Client Error
				if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key - CP_KEY_LOGIC2CLIENT), SESSION_CLIENT);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				// 2. NPC Server Error
				else if (key >= CP_KEY_LOGIC2NPC && key <= CP_KEY_LISTEN_NPC) {
					disconnect(0, SESSION_NPC);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				// 3. Ex_Server Error
				else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
					//cout << WSAGetLastError() << endl;
					disconnect(static_cast<int>(key - CP_KEY_LOGIC2EXLOGIC), SESSION_EXTENDED_SERVER);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			// 1. Client Accept
			if (key == CP_KEY_LISTEN_CLIENT) {
				SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				int client_id = get_new_client_id();
				if (client_id != -1) {
					// 클라이언트 id, 소켓
					clients[client_id].s_lock.lock();
					clients[client_id].id = client_id;
					clients[client_id].remain_size = 0;
					clients[client_id].socket = c_socket;
					clients[client_id].s_lock.unlock();
					int new_key = client_id + CP_KEY_LOGIC2CLIENT;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, new_key, 0);
					clients[client_id].do_recv();
					c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "어떤 Client의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_sc_listensock, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			// 2. Npc Server Accept
			else if (key == CP_KEY_LISTEN_NPC) {
				SOCKET npc_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				npc_server.socket = npc_socket;
				cout << "NPC 서버의 연결요청을 받았습니다.\n" << endl;
				b_npcsvr_conn = true;
				int new_key = CP_KEY_LOGIC2NPC;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(npc_socket), h_iocp, new_key, 0);
				npc_server.do_recv();
				npc_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(npc_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_npc_listensock, npc_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			// 3. Ex_Server Accept
			else if (key == CP_KEY_LISTEN_EXLOGIC) {
				SOCKET extended_server_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				left_ex_server_sock = extended_server_socket;
				int new_id = find_empty_extended_server();
				if (new_id != -1) {
					//cout << "Sever[" << new_id << "]의 연결요청을 받았습니다.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_LOGIC2EXLOGIC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_iocp, new_key, 0);
					extended_servers[new_id].do_recv();
					extended_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "다른 Sever의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(extended_server_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_ss_listensock, extended_server_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}

			break;
		}//OP_ACPT end
		case OP_RECV: {
			// 1. Client Recv
			if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
				int recved_id = key - CP_KEY_LOGIC2CLIENT;
				if (0 == num_bytes) disconnect(recved_id, SESSION_CLIENT);

				int remain_data = num_bytes + clients[recved_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(recved_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				clients[recved_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				clients[recved_id].do_recv();
			}
			// 2. NPC Server Recv
			else if (key >= CP_KEY_LOGIC2NPC && key < CP_KEY_LISTEN_NPC) {
				if (0 == num_bytes) disconnect(key, SESSION_NPC);

				int remain_data = num_bytes + npc_server.remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(0, p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				npc_server.remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				npc_server.do_recv();
			}
			// 3. Ex_Server Recv
			else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				if (0 == num_bytes) disconnect(key, SESSION_EXTENDED_SERVER);
				int server_id = key - CP_KEY_LOGIC2EXLOGIC;

				int remain_data = num_bytes + extended_servers[server_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(server_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				extended_servers[server_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				extended_servers[server_id].do_recv();
				break;
			}

			break;
		}//OP_RECV end
		case OP_SEND: {
			// 1. Client Send
			if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
				if (0 == num_bytes) disconnect(key - CP_KEY_LOGIC2CLIENT, SESSION_CLIENT);
				delete ex_over;
			}
			// 2. NPC Server Send
			else if (key >= CP_KEY_LOGIC2NPC && key <= CP_KEY_LISTEN_NPC) {
				if (0 == num_bytes) disconnect(0, SESSION_NPC);
				delete ex_over;
			}
			// 3. Ex_Server Send
			else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				int server_id = key - CP_KEY_LOGIC2EXLOGIC;
				if (0 == num_bytes) disconnect(server_id, SESSION_EXTENDED_SERVER);
				delete ex_over;
			}

			break;
		}//OP_SEND end
		case OP_CONNECT: {
			// 1. Ex_Server Conn_Ex
			if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				if (FALSE != ret) {
					int server_id = key - CP_KEY_LOGIC2EXLOGIC;
					std::cout << "성공적으로 Server[" << server_id << "]에 연결되었습니다.\n" << endl;
					extended_servers[server_id].id = server_id;
					extended_servers[server_id].remain_size = 0;
					extended_servers[server_id].socket = right_ex_server_sock;
					extended_servers[server_id].s_state = ST_ACCEPTED;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, NULL, 0);
					delete ex_over;
					extended_servers[server_id].do_recv();
				}
			}
		}//OP_CONN end
		}
	}
}

//======================================================================
void timerFunc() {
	while (true) {
		auto start_t = system_clock::now();
		// ================================
		// 서버 시간 업데이트
		if (b_active_server && !b_isfirstplayer) {
			servertime_lock.lock();
			g_curr_servertime = duration_cast<milliseconds>(start_t - g_s_start_time);
			servertime_lock.unlock();
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
				SC_TIME_TICKING_PACKET ticking_packet;
				ticking_packet.size = sizeof(SC_TIME_TICKING_PACKET);
				ticking_packet.type = SC_TIME_TICKING;
				int left_time = STAGE1_TIMELIMIT * 1000 - static_cast<int>(g_curr_servertime.count());
				if (left_time < 0) left_time = 0;
				ticking_packet.servertime_ms = left_time;
				cl.do_send(&ticking_packet);
			}
		}

		// ================================
		// --- 업데이트


		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}

//======================================================================
void heartBeatFunc() {	// Heartbeat관련 스레드 함수
	while (true) {
		auto start_t = system_clock::now();

		// ================================
		// 1. Heartbeat 전송
		// : 오른쪽 서버로 Heartbeat를 보냅니다. (왼쪽 서버가 오른쪽 서버로 전송하기 때문에 가장 마지막 서버는 보내지 않습니다.)
		if (my_server_id != MAX_LOGIC_SERVER - 1) {
			if (extended_servers[my_server_id].s_state != ST_ACCEPTED)	continue;
			if (extended_servers[my_server_id + 1].s_state != ST_ACCEPTED) continue;

			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[my_server_id + 1].do_send(&hb_packet);	// 오른쪽 서버에 전송합니다.

			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// 전송한 시간을 업데이트
		}

		// ================================
		// 2. Heartbeat 수신검사
		// 오랫동안 Heartbeat를 받지 못한 서버구성원이 있는지 확인합니다.
		//   1) 오른쪽 서버 검사 (가장 왼쪽에 있는 서버 구성원은 검사를 하지 않습니다.)
		if (my_server_id != 0) {
			if (extended_servers[my_server_id - 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id - 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id - 1 << "]에게 Heartbeat를 오랫동안 받지 못했습니다. 서버 다운으로 간주합니다." << endl;
					disconnect(my_server_id - 1, SESSION_EXTENDED_SERVER);
				}
			}
		}
		//   2) 왼쪽 서버 검사 (가장 오른쪽에 있는 서버 구성원은 검사를 하지 않습니다.)
		if (my_server_id != MAX_LOGIC_SERVER - 1) {
			if (extended_servers[my_server_id + 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id + 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id + 1 << "]에게 Heartbeat를 오랫동안 받지 못했습니다. 서버 다운으로 간주합니다." << endl;
					disconnect(my_server_id + 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		// ================================
		// 3. Data Replica 전송 (자신이 Active서버 일때에만)
		if (b_active_server) {
			// 데이터복제 패킷을 받게될 Standby서버의 id를 알아냅니다.
			int standby_id = -1;
			if (my_server_id == 0)
				standby_id = 1;
			else
				standby_id = 0;

			if (extended_servers[standby_id].s_state == ST_ACCEPTED) {
				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage == 0) continue;

					SS_DATA_REPLICA_PACKET replica_pack;
					replica_pack.type = SS_DATA_REPLICA;
					replica_pack.size = sizeof(SS_DATA_REPLICA_PACKET);

					replica_pack.target = TARGET_PLAYER;
					replica_pack.id = cl.id;
					strcpy_s(replica_pack.name, cl.name);

					replica_pack.x = cl.pos.x;
					replica_pack.y = cl.pos.y;
					replica_pack.z = cl.pos.z;

					replica_pack.right_x = cl.m_rightvec.x;
					replica_pack.right_y = cl.m_rightvec.y;
					replica_pack.right_z = cl.m_rightvec.z;

					replica_pack.up_x = cl.m_upvec.x;
					replica_pack.up_y = cl.m_upvec.y;
					replica_pack.up_z = cl.m_upvec.z;

					replica_pack.look_x = cl.m_lookvec.x;
					replica_pack.look_y = cl.m_lookvec.y;
					replica_pack.look_z = cl.m_lookvec.z;

					replica_pack.state = cl.pl_state;
					replica_pack.hp = cl.hp;
					replica_pack.bullet_cnt = cl.remain_bullet;
					replica_pack.curr_stage = cl.curr_stage;

					extended_servers[standby_id].do_send(&replica_pack);

					//cout << "[REPLICA TEST] Client[" << cl.id << "]의 정보를 Sever[" << standby_id << "]에게 전달합니다. - line: 1413" << endl;
					//cout << "Stage: " << replica_pack.curr_stage << ", State: " << replica_pack.state
					//	<< ", Pos: " << replica_pack.x << ", " << replica_pack.y << ", " << replica_pack.z
					//	<< ", Look: " << replica_pack.look_x << ", " << replica_pack.look_y << ", " << replica_pack.look_z << "\n" << endl;
				}
			}
		}

		// ================================
		// 4. 스레드 대기 (과부하 방지)
		auto curr_t = system_clock::now();
		if (curr_t - start_t < static_cast<milliseconds>(HB_SEND_CYCLE)) {
			this_thread::sleep_for(static_cast<milliseconds>(HB_SEND_CYCLE) - (curr_t - start_t));
		}
	}
}

//======================================================================
int main(int argc, char* argv[])
{
	b_isfirstplayer = true;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	// [ HA - 서버 ID, 포트번호 지정 ]
	// 어떤 서버를 가동할 것인지를 명령행 인수로 입력받아서 그 서버에 포트 번호를 부여합니다.
	my_server_id = 0;		// 서버 구분을 위한 지정번호
	int sc_portnum = -1;	// 클라이언트 통신용 포트번호
	int ss_portnum = -1;	// 서버 간 통신용 포트번호
	int snpc_portnum = -1;	// npc서버 통신용 포트번호
	if (argc > 1) {			// 입력된 명령행 인수가 있을 때 (주로 서버다운으로 인한 서버 재실행때 사용됨)
		// Serve ID지정
		my_server_id = atoi(argv[1]) % 10;

		// Active 여부 결정
		int is_active = atoi(argv[1]) / 10;	// 십의자리가 1: Standby, 2: Active
		if (is_active == 0) {	// 서버의 첫 실행은 ID에 따라 구분
			if (my_server_id == 0) {
				b_active_server = false;
			}
			else if (my_server_id == 1) {
				b_active_server = true;
			}
		}
		else if (is_active == 1) {	// 강제 Standby모드 실행
			b_active_server = false;
		}
		else if (is_active == 2) {	// 강제 Active모드 실행
			b_active_server = true;
		}
		else {
			cout << "[Server ID Error] Unknown ID.\n" << endl;
			return -1;
		}
	}
	else {				// 만약 입력된 명령행 인수가 없다면 입력을 받습니다.
		cout << "실행할 서버: ";
		cin >> my_server_id;

		// Active 여부 결정
		switch (my_server_id) {
		case 0:	// 0번 서버
			b_active_server = false;
			break;
		case 1:	// 1번 서버
			b_active_server = true;
			break;
		case 10:	// 0번 서버 (강제 Standby)
		case 11:	// 1번 서버 (강제 Standby)
			b_active_server = false;
			break;
		case 20:	// 0번 서버 (강제 Active)
		case 21:	// 1번 서버 (강제 Active)
			b_active_server = true;
			break;
		default:
			cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
			return 0;
		}
	}

	// 서버번호에 따라 포트번호를 지정해줍니다.
	switch (my_server_id % 10) {
	case 0:	// 0번 서버
		sc_portnum = PORTNUM_LOGIC_0;
		ss_portnum = HA_PORTNUM_S0;
		snpc_portnum = PORTNUM_LGCNPC_0;
		break;
	case 1:	// 1번 서버
		sc_portnum = PORTNUM_LOGIC_1;
		ss_portnum = HA_PORTNUM_S1;
		snpc_portnum = PORTNUM_LGCNPC_1;
		break;
	default:
		cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
		return 0;
	}
	cout << "Server[" << my_server_id << "] 가 가동되었습니다. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else cout << "Stand-By";
	cout << " / S - C PORT : " << sc_portnum << " / S - S PORT : " << ss_portnum << " / S - NPC PORT : " << snpc_portnum << " ]" << endl;


	//======================================================================
	// [ HA - 서버 수평확장 ]
	// HA Listen Socket (서버 간 통신을 위한 Listen소켓)
	g_ss_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN ha_server_addr;
	memset(&ha_server_addr, 0, sizeof(ha_server_addr));
	ha_server_addr.sin_family = AF_INET;
	ha_server_addr.sin_port = htons(ss_portnum);
	ha_server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_ss_listensock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(ha_server_addr));
	listen(g_ss_listensock, SOMAXCONN);
	SOCKADDR_IN ha_addr;
	int ha_addr_size = sizeof(ha_addr);

	// HA Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ss_listensock), h_iocp, CP_KEY_LISTEN_EXLOGIC, 0);
	right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX ha_over;
	ha_over.process_type = OP_ACCEPT;
	ha_over.wsabuf.buf = reinterpret_cast<CHAR*>(right_ex_server_sock);
	AcceptEx(g_ss_listensock, right_ex_server_sock, ha_over.send_buf, 0, ha_addr_size + 16, ha_addr_size + 16, 0, &ha_over.overlapped);

	// 수평확장된 서버의 마지막 구성원이 아니라면, 오른쪽에 있는 서버에 비동기connect 요청을 보냅니다.
	if (my_server_id < MAX_LOGIC_SERVER - 1) {
		int right_servernum = my_server_id + 1;
		int right_portnum = ss_portnum + 1;
		cout << "다른 이중화 서버(Server[" << right_servernum << "] (S-S PORT: " << right_portnum << ")에 비동기Connect를 요청합니다." << endl;

		// ConnectEx
		SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		GUID guid = WSAID_CONNECTEX;
		DWORD bytes = 0;
		LPFN_CONNECTEX connectExFP;
		::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
		closesocket(temp_s);

		SOCKADDR_IN ha_server_addr;
		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
		int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
		if (ret != 0) {
			cout << "Bind Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		OVER_EX* con_over = new OVER_EX;
		con_over->process_type = OP_CONNECT;
		int key_num = CP_KEY_LOGIC2EXLOGIC + right_servernum;
		HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key_num, 0);
		if (NULL == hret) {
			cout << "CreateIoCompletoinPort Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		ha_server_addr.sin_port = htons(right_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버

		// 루프백
		//inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

		// 원격
		if (my_server_id == 0) {
			inet_pton(AF_INET, IPADDR_LOGIC1, &ha_server_addr.sin_addr);
		}
		else if (my_server_id == 1) {
			inet_pton(AF_INET, IPADDR_LOGIC0, &ha_server_addr.sin_addr);
		}

		BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
		if (FALSE == bret) {
			int err_no = GetLastError();
			if (ERROR_IO_PENDING == err_no)
				cout << "Server Connect 시도 중...\n" << endl;
			else {
				cout << "ConnectEX Error - " << err_no << endl;
				cout << WSAGetLastError() << endl;
			}
		}
	}
	else {
		cout << "마지막 서버구성원이므로 Connect를 수행하지않습니다.\n" << endl;
	}
	extended_servers[my_server_id].id = my_server_id;
	extended_servers[my_server_id].s_state = ST_ACCEPTED;

	//======================================================================
	// [ Main ]

	// [ Main - 맵 정보 로드 ]
	// 1. 디렉토리 검색
	string filename;
	vector<string> readTargets;

	filesystem::path collidebox_path(".\\Collideboxes");
	if (filesystem::exists(collidebox_path)) {
		filesystem::recursive_directory_iterator itr(collidebox_path);
		while (itr != filesystem::end(itr)) {
			const filesystem::directory_entry& entry = *itr;
			//cout << entry.path().string() << endl;
			string path_name = entry.path().string();
			if (path_name.find(".txt") != string::npos) {	// .txt 가 들어간 파일만 저장합니다. (디렉토리 이름만 있는 path 배제)
				readTargets.push_back(path_name);
			}
			itr++;
		}
	}
	else {
		cout << "[Directory Search Error] Unknown Directory." << endl;
	}

	// 2. 파일 읽기
	for (auto& fname : readTargets) {
		cout << "[Map Loading...] " << fname;
		//string fname = readTargets[0];
		ifstream txtfile(fname);

		string line;

		int line_cnt = 0;

		char b_pos = 0;
		int pos_count = 0;

		char b_scale = 0;
		int scale_count = 0;

		float tmp_pos[3] = { 0.f, 0.f, 0.f }; // 뽑은 좌표정보를 임시 저장할 공간, 3개 꽉차면 벡터에 넣어주고 비워두자.
		float tmp_scale[3] = { 0.f, 0.f, 0.f }; // 뽑은 크기정보를 임시 저장할 공간, 3개 꽉차면 벡터에 넣어주고 비워두자.
		if (txtfile.is_open()) {
			while (txtfile >> line) {
				if (line == "Position:") {
					b_pos = 1;
					pos_count = 0;
				}
				else if (line == "Size:") {
					b_scale = 1;
					scale_count = 0;
				}
				else {
					if (b_pos == 1) {
						tmp_pos[pos_count] = string2data(line);

						if (pos_count == 2) {
							tmp_pos[pos_count] = string2data(line);

							b_pos = 0;
							pos_count = 0;
						}
						else {
							pos_count += 1;
						}
					}
					else if (b_scale == 1) {
						tmp_scale[scale_count] = string2data(line);

						if (scale_count == 2) {
							tmp_scale[scale_count] = string2data(line);
							b_scale = 0;
							scale_count = 0;

							Building tmp_mapobj(tmp_pos[0], tmp_pos[1], tmp_pos[2], tmp_scale[0], tmp_scale[1], tmp_scale[2]);
							tmp_mapobj.setBB();
							buildings_info.push_back(tmp_mapobj);
							memset(tmp_pos, 0, sizeof(tmp_pos));
							memset(tmp_scale, 0, sizeof(tmp_scale));
						}
						else {
							scale_count += 1;
						}
					}
				}
				line_cnt++;
			}
			cout << " ---- OK." << endl;
		}
		else {
			cout << "[Error] Unknown File." << endl;
		}
		txtfile.close();
	}
	cout << "\n";

	//======================================================================
	// [ Main - 클라이언트 연결 ]
	// Client Listen Socket (클라이언트-서버 통신을 위한 Listen소켓)
	g_sc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(sc_portnum);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_sc_listensock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_sc_listensock, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	// Client Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_sc_listensock), h_iocp, CP_KEY_LISTEN_CLIENT, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(g_sc_listensock, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	//======================================================================
	// [ Main - NPC서버 연결 ]
	// NPC Listen Socket (로직서버-NPC서버 통신을 위한 Listen소켓)
	g_npc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr2;
	memset(&server_addr, 0, sizeof(server_addr2));
	server_addr2.sin_family = AF_INET;
	server_addr2.sin_port = htons(snpc_portnum);
	server_addr2.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_npc_listensock, reinterpret_cast<sockaddr*>(&server_addr2), sizeof(server_addr2));
	listen(g_npc_listensock, SOMAXCONN);
	SOCKADDR_IN npc_addr;
	int addr_size2 = sizeof(npc_addr);
	int npc_id = 0;

	// NPC Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_npc_listensock), h_iocp, CP_KEY_LISTEN_NPC, 0);
	SOCKET npc_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over2;
	a_over2.process_type = OP_ACCEPT;
	a_over2.wsabuf.buf = reinterpret_cast<CHAR*>(npc_socket);
	AcceptEx(g_npc_listensock, npc_socket, a_over2.send_buf, 0, addr_size2 + 16, addr_size2 + 16, 0, &a_over2.overlapped);

	//======================================================================
	// [ Main - 스레드 생성 ]
	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);			// 클라이언트-서버 통신용 Worker스레드

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);				// 클라이언트 로직 타이머스레드
	timer_threads.emplace_back(heartBeatFunc);			// 서버 간 Heartbeat교환 스레드

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_sc_listensock);
	WSACleanup();
}