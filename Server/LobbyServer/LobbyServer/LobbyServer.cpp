#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <filesystem>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#include "Constant.h"

#include "../../MainServer/Server/Protocol.h"
#include "../../MainServer/Server/Constant.h"

using namespace std;
using namespace chrono;

//======================================================================
// Network
HANDLE h_iocp;											// IOCP 핸들
SOCKET g_sc_listensock;									// 클라이언트 통신 listen소켓

//======================================================================
// HA
int my_server_id;
bool b_active_server;
SOCKET g_ss_listensock;									// 수평확장 서버간 통신 listen소켓
SOCKET right_ex_server_sock;							// 자신이 연결할 오른쪽 서버
SOCKET left_ex_server_sock;								// 연결 요청을 보내온 왼쪽 서버

//======================================================================
enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
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
		wsabuf.len = (unsigned char)packet[0];
		wsabuf.buf = send_buf;
		ZeroMemory(&overlapped, sizeof(overlapped));
		process_type = OP_SEND;
		memcpy(send_buf, packet, wsabuf.len);
	}
};

//======================================================================
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_LOGOUT };
enum SESSION_TYPE { SESSION_CLIENT, SESSION_LOBBY };
void disconnect(int target_id, int target);
class SESSION {
public:
	OVER_EX recv_over;
	int remain_size;
	int id;
	char name[NAME_SIZE];
	int curr_room;		// -1 이면 방에 없고 로비에 있다는 뜻. (혹은 s_state가 FREE인데 -1이면 접속중이 아닌것)
	int inroom_state;	// RM_ST_... (EMPTY: 로비, NONREADY: 방에 있지만 준비X, READY: 방에 있고 준비도 O, MANAGER: 방에 있고 그 방의 방장)
	int role;			// ROLE_NOTCHOOSE, ROLE_RIFLE, ROLE_HELI
	SESSION_STATE s_state;
	SOCKET sock;

	mutex s_lock;

public:
	SESSION() { remain_size = 0; id = -1; curr_room = -1; inroom_state = RM_ST_EMPTY; role = ROLE_NOTCHOOSE; sock = 0; s_state = ST_FREE; }

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

public:
	void sessionClear() {
		remain_size = 0;
		id = -1;
		curr_room = -1;
		inroom_state = RM_ST_EMPTY;
		role = ROLE_NOTCHOOSE;
		sock = 0;
		s_state = ST_FREE;
	}
};
array<SESSION, MAX_USER * MAX_ROOM> clients;						// 세션 정보

int get_new_client_id()	// clients의 비어있는 칸을 찾아서 새로운 client의 아이디를 할당해주는 함수
{
	for (int i = 0; i < MAX_USER * MAX_ROOM; ++i) {
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
// HA
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
array<HA_SERVER, MAX_LOBBY_SERVER> extended_servers;	// HA구현을 위해 수평확장된 서버들

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
class Game_Room {
public:
	array<int, MAX_USER> users;
	
public:
	int room_id;
	char room_name[ROOM_NAME_SIZE];
	int room_state;
	int user_count;

public:
	Game_Room() {
		for (int i = 0; i < MAX_USER; ++i) { users[i] = -1; }
		room_id = -1;
		strcpy_s(room_name, "\0");
		room_state = R_ST_WAIT;
		user_count = 0;
	}

public:
	int user_join(int c_id);
	int user_leave(int c_id);
};
vector<Game_Room> game_rooms;
mutex r_lock;
int room_count = 0;	// 0부터 시작해서 새로운 방을 만들때마다 계속해서 1씩 증가한다. (방 ID에 사용됨)

int Game_Room::user_join(int c_id) {
	if (room_state == R_ST_FULL) return -1;

	for (int i = 0; i < MAX_USER; ++i) {
		if (users[i] == -1) {
			r_lock.lock();

			users[i] = c_id;
			user_count++;
			cout << "Room[" << room_id << "]에 Client[" << c_id << "]가 입장하였습니다." << endl;
			cout << "Room[" << room_id << "]의 현재 참가인원: " << user_count << "(명)" << endl;
			if (user_count == MAX_USER) {
				room_state = R_ST_FULL;
				cout << "Room[" << room_id << "]의 인원이 가득찼습니다." << endl;
			}
			r_lock.unlock();

			clients[c_id].s_lock.lock();
			clients[c_id].curr_room = room_id;
			cout << "Client[" << c_id << "]는 Room[" << room_id << "]에서의 인덱스는 " << i << "입니다." << endl;
			if (user_count == 1)
				clients[c_id].inroom_state = RM_ST_MANAGER;
			else
				clients[c_id].inroom_state = RM_ST_NONREADY;
			clients[c_id].s_lock.unlock();

			cout << "\n";

			return 0;
		}
	}
	return -1;
}
int Game_Room::user_leave(int c_id) {
	if (user_count == 0) return -1;

	for (int i = 0; i < MAX_USER; ++i) {
		if (users[i] == c_id) {
			r_lock.lock();

			users[i] = -1;
			user_count--;
			cout << "Room[" << room_id << "]에서 Client[" << c_id << "]가 퇴장하였습니다." << endl;
			cout << "Room[" << room_id << "]의 현재 참가인원: " << user_count << "(명)\n" << endl;
			if (room_state == R_ST_FULL) {
				room_state = R_ST_WAIT;
			}
			cout << "\n";

			r_lock.unlock();

			clients[c_id].s_lock.lock();
			clients[c_id].curr_room = -1;
			clients[c_id].inroom_state = RM_ST_EMPTY;
			clients[c_id].s_lock.unlock();

			return 0;
		}
	}
	return -1;
}

int create_new_room(string new_name) {
	Game_Room new_room;
	new_room.room_id = room_count++;
	new_room.user_count = 0;
	strcpy_s(new_room.room_name, new_name.c_str());
	game_rooms.push_back(new_room);

	cout << "Room[ID: " << new_room.room_id << ", Name: " << new_room.room_name << "]이 생성되었습니다." << endl;
	return new_room.room_id;
}
int find_joinable_room() {
	for (auto& room : game_rooms) {
		if (room.user_count < MAX_USER) return room.room_id;
	}

	return -1;
}

//======================================================================
void process_packet(int client_id, char* packet)
{
	switch (packet[1]) {
	case CLBY_CONNECT:
	{
		CLBY_CONNECT_PACKET* recv_packet = reinterpret_cast<CLBY_CONNECT_PACKET*>(packet);
		strcpy_s(clients[client_id].name, recv_packet->name);
		cout << "새로운 클라이언트가 접속하였습니다. (ID: " << client_id << ", Name: " << clients[client_id].name << " )\n" << endl;

		break;
	}// CLBY_CONNECT case end
	case CLBY_CREATE_ROOM:
	{
		CLBY_CREATE_ROOM_PACKET* recv_packet = reinterpret_cast<CLBY_CREATE_ROOM_PACKET*>(packet);

		// 새로운 방을 생성
		r_lock.lock();
		int new_room_id = create_new_room(recv_packet->room_name);
		r_lock.unlock();

		// 생성된 방에 플레이어 입장
		int ret = game_rooms[new_room_id].user_join(client_id);
		if (ret != 0) {
			cout << "[Error] 생성된 방에 입장하지 못했습니다." << endl;
			break;
		}

		// 방 생성 요청을 보낸 클라이언트에게 '방 참가 패킷'을 보냄.
		LBYC_ROOM_JOIN_PACKET join_my_room;
		join_my_room.size = sizeof(LBYC_ROOM_JOIN_PACKET);
		join_my_room.type = LBYC_ROOM_JOIN;
		join_my_room.room_id = new_room_id;
		strcpy_s(join_my_room.room_name, game_rooms[new_room_id].room_name);
		join_my_room.member_count = 1;
		// 방 유저의 0번째 인덱스는 방 생성을 요청한 클라이언트이고, 그는 방장이다.
		strcpy_s(join_my_room.member_name[0], clients[client_id].name);
		join_my_room.member_state[0] = RM_ST_MANAGER;
		// 막 만들어진 방이므로 요청한 클라이언트 외에 아무도 없는 것이 정상임.
		for (int i = 1; i < MAX_USER; ++i) { strcpy_s(join_my_room.member_name[i], "\0"); }
		for (int i = 1; i < MAX_USER; ++i) { join_my_room.member_state[i] = RM_ST_EMPTY; }
		join_my_room.your_roomindex = 0;
		join_my_room.b_manager = b_TRUE;
		clients[client_id].do_send(&join_my_room);
		cout << "Client[" << client_id << "]에게 Room[" << new_room_id << "] 참가 패킷을 보냈습니다.\n" << endl;

		// 로비에 접속 중인 다른 클라이언트들에게는 '방 추가 패킷'을 보냄.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_room != -1) continue; // 방에 있지않고 로비에 있는 유저에게만 보냅니다.

			LBYC_ADD_ROOM_PACKET add_room_pack;
			add_room_pack.size = sizeof(LBYC_ADD_ROOM_PACKET);
			add_room_pack.type = LBYC_ADD_ROOM;
			add_room_pack.room_id = new_room_id;
			strcpy_s(add_room_pack.room_name, game_rooms[new_room_id].room_name);
			cl.do_send(&add_room_pack);
			cout << "로비에 있는 Client[" << cl.id << "]에게 Room[" << new_room_id << "] 추가 패킷을 보냈습니다.\n" << endl;
		}

		break;
	}// CLBY_CREATE_ROOM case end
	case CLBY_QUICK_MATCH:
	{
		CLBY_QUICK_MATCH_PACKET* recv_packet = reinterpret_cast<CLBY_QUICK_MATCH_PACKET*>(packet);

		// 입장가능한 방 찾기
		int matched_room_id = -1;
		for (auto& room : game_rooms) {
			if (room.user_count < MAX_USER) {
				matched_room_id = room.room_id;
				break;
			}
		}
		if (matched_room_id == -1) {
			cout << "비어있는 방이 없어 매칭에 실패하였습니다." << endl;
			LBYC_MATCH_FAIL_PACKET match_fail_packet;
			match_fail_packet.size = sizeof(LBYC_MATCH_FAIL_PACKET);
			match_fail_packet.type = LBYC_MATCH_FAIL;
			match_fail_packet.fail_reason = MATCH_FAIL_NOEMPTYROOM;
			clients[client_id].do_send(&match_fail_packet);
			break;
		}

		// 매칭된 방에 입장
		int ret = game_rooms[matched_room_id].user_join(client_id);
		if (ret != 0) {
			cout << "[Error] 생성된 방에 입장하지 못했습니다." << endl;
			LBYC_MATCH_FAIL_PACKET match_fail_packet;
			match_fail_packet.size = sizeof(LBYC_MATCH_FAIL_PACKET);
			match_fail_packet.type = LBYC_MATCH_FAIL;
			match_fail_packet.fail_reason = MATCH_FAIL_UNKNOWN;
			clients[client_id].do_send(&match_fail_packet);
			break;
		}

		// 빠른시작 요청을 보낸 클라이언트에게 '방 참가 패킷'을 보냄.
		LBYC_ROOM_JOIN_PACKET room_join_packet;
		room_join_packet.size = sizeof(LBYC_ROOM_JOIN_PACKET);
		room_join_packet.type = LBYC_ROOM_JOIN;
		room_join_packet.room_id = matched_room_id;
		strcpy_s(room_join_packet.room_name, game_rooms[matched_room_id].room_name);
		room_join_packet.member_count = game_rooms[matched_room_id].user_count;
		for (int i = 0; i < MAX_USER; ++i) {
			int member_id = game_rooms[matched_room_id].users[i];

			strcpy_s(room_join_packet.member_name[i], clients[member_id].name);
			room_join_packet.member_state[i] = clients[member_id].inroom_state;

			if (member_id == client_id) {
				room_join_packet.your_roomindex = i;
			}
		}
		room_join_packet.b_manager = b_FALSE;
		clients[client_id].do_send(&room_join_packet);
		cout << "Client[" << client_id << "]에게 Room[" << matched_room_id << "] 참가 패킷을 보냈습니다.\n" << endl;


		// 그 방에 이전부터 있었던 클라이언트에게 새로운 클라이언트가 참가했음을 알려줍니다.
		for (int i = 0; i < MAX_USER; ++i) {
			int member_id = game_rooms[matched_room_id].users[i];
			if (member_id == -1) continue;
			if (member_id == client_id) continue;

			LBYC_ROOM_NEW_MEMBER_PACKET new_member_packet;
			new_member_packet.size = sizeof(LBYC_ROOM_NEW_MEMBER_PACKET);
			new_member_packet.type = LBYC_ROOM_NEW_MEMBER;
			strcpy_s(new_member_packet.new_member_name, clients[client_id].name);
			new_member_packet.new_member_roomindex = room_join_packet.your_roomindex;

			clients[member_id].do_send(&new_member_packet);
			cout << "Client[" << member_id << "]에게 새로운 유저[" << client_id << "]의 합류 패킷을 보냈습니다.\n" << endl;
		}

		// 로비에 있는 클라이언트들에게 해당 방 유저 수가 1 증가하였음을 알려줍니다.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_room != -1) continue; // 방에 있지않고 로비에 있는 유저에게만 보냅니다.

			LBYC_ROOM_USERCOUNT_PACKET user_increase_pack;
			user_increase_pack.size = sizeof(LBYC_ROOM_USERCOUNT_PACKET);
			user_increase_pack.type = LBYC_ROOM_USERCOUNT;
			user_increase_pack.room_id = matched_room_id;
			user_increase_pack.user_count = game_rooms[matched_room_id].user_count;
			cl.do_send(&user_increase_pack);
			cout << "로비에 있는 Client[" << cl.id << "]에게 Room[" << matched_room_id << "]의 인원이 "
				<< game_rooms[matched_room_id].user_count << "명이 되었다고 알려줍니다.\n" << endl;
		}

		break;
	}// CLBY_QUICK_MATCH case end
	case CLBY_LEAVE_ROOM:
	{
		CLBY_LEAVE_ROOM_PACKET* recv_packet = reinterpret_cast<CLBY_LEAVE_ROOM_PACKET*>(packet);
		
		int room_id = clients[client_id].curr_room;

		// 원래 있던 방에서의 인덱스를 알아냄
		int inroom_index = -1;
		for (int i = 0; i < MAX_USER; ++i) {
			if (game_rooms[room_id].users[i] == client_id) {
				inroom_index = i;
				break;
			}
		}
		if (inroom_index == -1) {
			cout << "[Error] Unknown Error (Line:388)" << endl;
			break;
		}

		// 원래 있던 방에서 퇴장
		int ret = game_rooms[room_id].user_leave(client_id);
		if (ret != 0) {
			cout << "[Error] 방에서 나오지 못했습니다." << endl;
			break;
		}

		// 우선 방 퇴장 요청을 보낸 클라이언트에게 로비에 있는 방 데이터 초기화를 명령합니다.
		LBYC_LOBBY_CLEAR_PACKET lobby_clear_pack;
		lobby_clear_pack.size = sizeof(LBYC_LOBBY_CLEAR_PACKET);
		lobby_clear_pack.type = LBYC_LOBBY_CLEAR;
		clients[client_id].do_send(&lobby_clear_pack);

		// 방 퇴장 요청을 보낸 클라이언트에게 로비에 있는 모든 방에 대한 간략한 정보를 보냅니다.
		for (auto& room : game_rooms) {
			LBYC_ADD_ROOM_PACKET room_info_pack;
			room_info_pack.size = sizeof(LBYC_ADD_ROOM_PACKET);
			room_info_pack.type = LBYC_ADD_ROOM;
			room_info_pack.room_id = room.room_id;
			strcpy_s(room_info_pack.room_name, room.room_name);

			clients[client_id].do_send(&room_info_pack);
			//cout << "Client[" << client_id << "]에게 로비에 보일 Room[" << room.room_id << "]의 정보를 보냈습니다." << endl;
		}
		cout << "Client[" << client_id << "]에게 존재하는 모든 방들의 정보를 보냈습니다.\n" << endl;

		// 방에 남아있는 클라이언트에게 방 퇴장 요청을 보냈던 클라이언트가 방에서 나왔음을 알려줍니다.
		for (int i = 0; i < MAX_USER; ++i) {
			int member_id = game_rooms[room_id].users[i];
			if (member_id == -1) continue;
			if (member_id == client_id) continue;

			LBYC_ROOM_LEFT_MEMBER_PACKET new_member_packet;
			new_member_packet.size = sizeof(LBYC_ROOM_LEFT_MEMBER_PACKET);
			new_member_packet.type = LBYC_ROOM_LEFT_MEMBER;
			strcpy_s(new_member_packet.left_member_name, clients[client_id].name);
			new_member_packet.left_member_roomindex = inroom_index;

			clients[member_id].do_send(&new_member_packet);
			cout << "Client[" << member_id << "]에게 유저[" << client_id << "]의 퇴장 패킷을 보냈습니다.\n" << endl;
		}

		// 로비에 있는 클라이언트들에게 해당 방 유저 수가 1 감소하였음을 알려줍니다.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_room != -1) continue; // 방에 있지않고 로비에 있는 유저에게만 보냅니다.
			if (cl.id == client_id) continue; // 얘는 이미 위에서 로비 전체 정보 패킷을 보냈음.

			LBYC_ROOM_USERCOUNT_PACKET user_decrease_pack;
			user_decrease_pack.size = sizeof(LBYC_ROOM_USERCOUNT_PACKET);
			user_decrease_pack.type = LBYC_ROOM_USERCOUNT;
			user_decrease_pack.room_id = room_id;
			user_decrease_pack.user_count = game_rooms[room_id].user_count;
			cl.do_send(&user_decrease_pack);
			cout << "로비에 있는 Client[" << cl.id << "]에게 Room[" << room_id << "]의 인원이 " << game_rooms[room_id].user_count << "명이 되었다고 알려줍니다.\n" << endl;
		}

		break;
	}// CLBY_LEAVE_ROOM case end
	case CLBY_ROLE_CHANGE:
	{
		CLBY_ROLE_CHANGE_PACKET* recv_packet = reinterpret_cast<CLBY_ROLE_CHANGE_PACKET*>(packet);

		int before_role = clients[client_id].role;
		int after_role = recv_packet->role;
		cout << "Before: " << before_role << ", After: " << after_role << endl;
		if (before_role == after_role) break;	// 이미 같은 역할임.

		int cur_room = clients[client_id].curr_room;
		if (cur_room == -1) break;	// 비정상 요청 (나중에 접속을 끊어버리던가 하자)

		// 그 방에 있는 역할 수 파악
		int rifle_num = 0;
		int heli_num = 0;
		if (after_role == ROLE_NOTCHOOSE) {
			// 역할을 바꿉니다.
			cout << "Room[" << cur_room << "에 있는 Client[" << client_id << "]의 역할이 [선택 안함]으로 변경되었습니다.\n" << endl;
			clients[client_id].role = ROLE_NOTCHOOSE;
		}
		else {
			for (auto& room : game_rooms) {
				if (room.room_id == cur_room) {
					for (int i = 0; i < MAX_USER; ++i) {
						int member_id = room.users[i];
						if (member_id == -1) continue;
						if (clients[member_id].curr_room != cur_room) continue;

						if (clients[member_id].role == ROLE_RIFLE)
							rifle_num++;
						else if (clients[member_id].role == ROLE_HELI)
							heli_num++;
					}
				}
			}

			// 역할을 바꿉니다.
			if (after_role == ROLE_RIFLE) {
				if (rifle_num >= 2) {
					cout << "Room[" << cur_room << "]에는 RIFLE유저가 이미 " << rifle_num << "명이어서 RIFLE은 선택할 수 없습니다.\n" << endl;
					break;
				}
				else {
					cout << "Room[" << cur_room << "에 있는 Client[" << client_id << "]의 역할이 [RIFLE]로 변경되었습니다.\n" << endl;
					clients[client_id].role = ROLE_RIFLE;
				}
			}
			else if (after_role == ROLE_HELI) {
				if (heli_num >= 1) {
					cout << "Room[" << cur_room << "]에는 HELI유저가 이미 " << heli_num << "명이어서 HELI은 선택할 수 없습니다.\n" << endl;
					break;
				}
				else {
					cout << "Room[" << cur_room << "에 있는 Client[" << client_id << "]의 역할이 [HELI]로 변경되었습니다.\n" << endl;
					clients[client_id].role = ROLE_HELI;
				}
			}
		}

		// 요청한 클라이언트가 방에서의 몇번째 인덱스인지 알아냄
		int inroom_index = -1;
		for (int i = 0; i < MAX_USER; ++i) {
			if (game_rooms[cur_room].users[i] == client_id) {
				inroom_index = i;
				break;
			}
		}
		if (inroom_index == -1) {
			cout << "[Error] Unknown Error (Line:524)" << endl;
			break;
		}

		// 역할이 바뀌었다고 그 방에 있는 유저들에게 알려줍니다.
		for (auto& room : game_rooms) {
			if (room.room_id == cur_room) {
				for (int i = 0; i < MAX_USER; ++i) {
					int member_id = room.users[i];
					if (member_id == -1) continue;
					if (clients[member_id].curr_room != cur_room) continue;

					LBYC_ROLE_CHANGE_PACKET role_change_packet;
					role_change_packet.size = sizeof(LBYC_ROLE_CHANGE_PACKET);
					role_change_packet.type = LBYC_ROLE_CHANGE;
					role_change_packet.member_id = inroom_index;
					role_change_packet.role = clients[client_id].role;
					clients[member_id].do_send(&role_change_packet);
				}
			}
		}

		break;
	}// CLBY_ROLE_CHANGE case end
	case CLBY_GAME_READY:
	{
		CLBY_GAME_READY_PACKET* recv_packet = reinterpret_cast<CLBY_GAME_READY_PACKET*>(packet);

		int cur_room = clients[client_id].curr_room;
		if (cur_room == -1) break;	// 비정상 요청 (나중에 접속을 끊어버리던가 하자)

		// 요청한 클라이언트가 방에서의 몇번째 인덱스인지 알아냄
		int inroom_index = -1;
		for (int i = 0; i < MAX_USER; ++i) {
			if (game_rooms[cur_room].users[i] == client_id) {
				inroom_index = i;
				break;
			}
		}
		if (inroom_index == -1) {
			cout << "[Error] Unknown Error (Line:564)" << endl;
			break;
		}

		// 요청한 클라이언트가 역할을 선택했는지 확인함.
		if (clients[client_id].role == ROLE_NOTCHOOSE) {
			cout << "Client[" << client_id << "]가 준비 요청을 하였으나, 아직 역할을 선택하지 않아 요청을 거절하였습니다.\n" << endl;
			break;
		}

		// 준비 상태로 바꿈
		if (clients[client_id].inroom_state == RM_ST_NONREADY) {
			clients[client_id].inroom_state = RM_ST_READY;
			cout << "Client[" << client_id << "] 준비 완료." << endl;

			for (auto& room : game_rooms) {
				if (room.room_id == clients[client_id].curr_room) {
					// 방에 있는 모든 클라이언트에게 준비 완료 패킷을 보냄.
					for (int i = 0; i < MAX_USER; ++i) {
						int member_id = room.users[i];
						if (member_id == -1) continue;

						LBYC_MEMBER_STATE_PACKET ready_packet;
						ready_packet.size = sizeof(LBYC_MEMBER_STATE_PACKET);
						ready_packet.type = LBYC_MEMBER_STATE;
						ready_packet.member_id = inroom_index;
						ready_packet.member_state = RM_ST_READY;
						clients[member_id].do_send(&ready_packet);
						cout << "Client[" << member_id << "]에게 Client[" << client_id << "]가 준비되었다고 알림." << endl;
					}
				}
			}
			cout << "\n";
		}
		else if (clients[client_id].inroom_state == RM_ST_READY) {
			clients[client_id].inroom_state = RM_ST_NONREADY;
			cout << "Client[" << client_id << "] 준비 해제." << endl;

			for (auto& room : game_rooms) {
				if (room.room_id == clients[client_id].curr_room) {
					// 방에 있는 모든 클라이언트에게 준비 완료 패킷을 보냄.
					for (int i = 0; i < MAX_USER; ++i) {
						int member_id = room.users[i];
						if (member_id == -1) continue;

						LBYC_MEMBER_STATE_PACKET ready_packet;
						ready_packet.size = sizeof(LBYC_MEMBER_STATE_PACKET);
						ready_packet.type = LBYC_MEMBER_STATE;
						ready_packet.member_id = inroom_index;
						ready_packet.member_state = RM_ST_NONREADY;
						clients[member_id].do_send(&ready_packet);
						cout << "Client[" << member_id << "]에게 Client[" << client_id << "]가 준비를 해제하였다고 알림." << endl;
					}
				}
			}
			cout << "\n";
		}
		else if (clients[client_id].inroom_state == RM_ST_MANAGER) {
			cout << "Client[" << client_id << "]은 방장입니다.\n" << endl;
			// 방장은 팀원 모두가 준비 완료일때 게임시작 버튼을 눌러 게임을 시작하는 역할이다. 따라서 준비 라는 상태가 될 수 없다.
		}

		break;
	}// CLBY_GAME_READY case end
	case CLBY_GAME_START:
	{
		CLBY_GAME_START_PACKET* recv_packet = reinterpret_cast<CLBY_GAME_START_PACKET*>(packet);

		cout << "게임 시작.\n" << endl;
		// 클라에서 검사하고 보냈지만, 서버에서 한번더 검사를 합니다.
		bool gamestart = true;

		// 1. 패킷을 보낸 사람이 방장인가?
		if (clients[client_id].inroom_state != RM_ST_MANAGER) {
			gamestart = false;
			break;
		}
		
		int room_id = clients[client_id].curr_room;
		
		/* [치트키] 작업 편의성을 위해 1명만 접속해도 게임 시작이 가능하도록 주석처리하였음.
		for (auto& room : game_rooms) {
			if (room.room_id == room_id) {
				
				// 2. 인원이 3명인가
				if (room.user_count < MAX_USER) {
					gamestart = false;
					break;
				}

				for (int i = 0; i < MAX_USER; ++i) {
					int member_id = room.users[i];
					// 3. 3명 모두 역할을 선택하였는가?
					if (clients[member_id].role == ROLE_NOTCHOOSE) {
						gamestart = false;
						cout << "Client[" << client_id << "]가 아직 역할을 선택하지 않아 시작할 수 없습니다.\n" << endl;
						break;
					}
					// 4. 3명 모두 준비 상태인가
					if (clients[member_id].inroom_state == RM_ST_MANAGER) continue;	// 방장은 준비 X
					if (clients[member_id].inroom_state == RM_ST_NONREADY) {
						gamestart = false;
						break;
					}
				}
			}
		}
		*/
		if (!gamestart) break;

		// 검사를 모두 통과했다면 게임을 시작한다.
		for (auto& room : game_rooms) {
			if (room.room_id == room_id) {
				// 방 상태를 '게임중'으로 바꾼다.
				room.room_state = R_ST_INGAME;
				cout << "Room[" << room.room_id << "]가 게임을 시작했습니다.\n" << endl;

				// 그 방에 있는 사람 모두에게 게임 시작 패킷을 보낸다.
				for (int i = 0; i < MAX_USER; ++i) {
					int member_id = room.users[i];
					if (member_id == -1) continue;
					if (clients[member_id].s_state != ST_INGAME) continue;

					LBYC_GAME_START_PACKET gamestart_pack;
					gamestart_pack.size = sizeof(LBYC_GAME_START_PACKET);
					gamestart_pack.type = LBYC_GAME_START;
					clients[member_id].do_send(&gamestart_pack);
				}
				
			}
		}
		// 로비에 있는 클라이언트들에게 그 방의 상태를 게임중으로 바꾸도록 한다.
		
		break;
	}// CLBY_GAME_START case end
	}// switch end
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

				int target_portnum = key - CP_KEY_EX_LBY + HA_PORTNUM_LBY0;
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
				if (0 <= key && key < MAX_USER * MAX_ROOM) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key), SESSION_CLIENT);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				// 2. Ex_Server Error
				else if (CP_KEY_EX_LBY <= key && key < CP_KEY_EX_LBY_LISTEN) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key), SESSION_LOBBY);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}


			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			if (key == CP_KEY_CLIENT) {
				SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				int client_id = get_new_client_id();
				if (client_id != -1) {
					// 클라이언트 id, 소켓
					clients[client_id].s_lock.lock();
					clients[client_id].id = client_id;
					clients[client_id].s_state = ST_INGAME;
					clients[client_id].remain_size = 0;
					clients[client_id].sock = c_socket;
					clients[client_id].s_lock.unlock();
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, client_id, 0);
					clients[client_id].do_recv();
					c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "어떤 Client의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
				int addr_size = sizeof(SOCKADDR_IN);

				int option = TRUE;//Nagle
				setsockopt(g_sc_listensock, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
				AcceptEx(g_sc_listensock, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			else if (key == CP_KEY_EX_LBY_LISTEN) {
				SOCKET extended_server_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				left_ex_server_sock = extended_server_socket;
				int new_id = find_empty_extended_server();
				if (new_id != -1) {
					//cout << "Sever[" << new_id << "]의 연결요청을 받았습니다.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_EX_LBY;
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
			if (0 <= key && key < MAX_USER * MAX_ROOM) {
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
			}
			else if (CP_KEY_EX_LBY <= key && key < CP_KEY_EX_LBY_LISTEN) {
				if (0 == num_bytes) disconnect(key, SESSION_LOBBY);
				int server_id = key - CP_KEY_EX_LBY;

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
			if ((0 <= key && key < MAX_USER * MAX_ROOM ) || key == CP_KEY_CLIENT) {
				if (0 == num_bytes) disconnect(static_cast<int>(key), SESSION_CLIENT);
				delete ex_over;
			}
			else if (CP_KEY_EX_LBY <= key && key < CP_KEY_EX_LBY_LISTEN) {
				if (0 == num_bytes) disconnect(static_cast<int>(key), SESSION_LOBBY);
				delete ex_over;
			}
			break;
		}//OP_SEND end
		case OP_CONNECT: {
			if (CP_KEY_EX_LBY <= key && key < CP_KEY_EX_LBY_LISTEN) {
				if (FALSE != ret) {
					int server_id = key - CP_KEY_EX_LBY;
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
			break;
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
	// [ HA - 서버 ID, 포트번호 지정 ]
	// 어떤 서버를 가동할 것인지를 명령행 인수로 입력받아서 그 서버에 포트 번호를 부여합니다.
	my_server_id = 0;		// 서버 구분을 위한 지정번호
	int sc_portnum = -1;	// 클라이언트 통신용 포트번호
	int ss_portnum = -1;	// 서버 간 통신용 포트번호
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
		sc_portnum = PORTNUM_LOBBY_0;
		ss_portnum = HA_PORTNUM_LBY0;
		break;
	case 1:	// 1번 서버
		sc_portnum = PORTNUM_LOBBY_1;
		ss_portnum = HA_PORTNUM_LBY1;
		break;
	default:
		cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
		return 0;
	}
	cout << "Lobby Server[" << my_server_id << "] 가 가동되었습니다. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else				 cout << "Stand-By";
	cout << " / S - C PORT : " << sc_portnum << " / S - S PORT : " << ss_portnum << " ]" << endl;

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
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ss_listensock), h_iocp, CP_KEY_EX_LBY_LISTEN, 0);
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
		int key_num = CP_KEY_EX_LBY + right_servernum;
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
		inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

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
	// [ Main - 클라이언트 연결 ]
	// Client Listen Socket (클라이언트-서버 통신을 위한 Listen소켓)
	g_sc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORTNUM_LOBBY_0);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_sc_listensock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_sc_listensock, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	// Client Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_sc_listensock), h_iocp, CP_KEY_CLIENT, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);

	int option = TRUE;//Nagle
	setsockopt(g_sc_listensock, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	AcceptEx(g_sc_listensock, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);


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

//======================================================================
void disconnect(int target_id, int target)
{
	switch (target) {
	case SESSION_CLIENT:
	{
		// 1. 클라이언트가 게임룸에 있었다면 내보냅니다.
		if (clients[target_id].curr_room != -1) {
			int room_id = clients[target_id].curr_room;
			for (auto& room : game_rooms) {
				if (room.room_id == room_id) {
					// 원래 있던 방에서의 인덱스를 알아냄
					int inroom_index = -1;
					for (int i = 0; i < MAX_USER; ++i) {
						if (game_rooms[room_id].users[i] == target_id) {
							inroom_index = i;
							break;
						}
					}
					if (inroom_index == -1) {
						cout << "[Error] Unknown Error (Line:133)" << endl;
					}

					// 방에서 내보냄
					int ret = room.user_leave(target_id);
					if (ret != 0) {//error
						cout << "[Error] Disconnect - Room Leave Error.\n" << endl;
					}

					// 방에 남아있는 클라이언트에게 방 퇴장 요청을 보냈던 클라이언트가 방에서 나왔음을 알려줍니다.
					for (int i = 0; i < MAX_USER; ++i) {
						int member_id = game_rooms[room_id].users[i];
						if (member_id == -1) continue;
						if (member_id == target_id) continue; // 얘는 애초에 지금 접속중이 아님.

						LBYC_ROOM_LEFT_MEMBER_PACKET new_member_packet;
						new_member_packet.size = sizeof(LBYC_ROOM_LEFT_MEMBER_PACKET);
						new_member_packet.type = LBYC_ROOM_LEFT_MEMBER;
						strcpy_s(new_member_packet.left_member_name, clients[target_id].name);
						new_member_packet.left_member_roomindex = inroom_index;

						clients[member_id].do_send(&new_member_packet);
						cout << "Client[" << member_id << "]에게 유저[" << target_id << "]의 퇴장 패킷을 보냈습니다.\n" << endl;
					}

					// 로비에 있는 클라이언트들에게 해당 방 유저 수가 1 감소하였음을 알려줍니다.
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_room != -1) continue; // 방에 있지않고 로비에 있는 유저에게만 보냅니다.
						if (cl.id == target_id) continue; // 얘는 이미 위에서 로비 전체 정보 패킷을 보냈음.

						LBYC_ROOM_USERCOUNT_PACKET user_decrease_pack;
						user_decrease_pack.size = sizeof(LBYC_ROOM_USERCOUNT_PACKET);
						user_decrease_pack.type = LBYC_ROOM_USERCOUNT;
						user_decrease_pack.room_id = room_id;
						user_decrease_pack.user_count = game_rooms[room_id].user_count;
						cl.do_send(&user_decrease_pack);
						cout << "로비에 있는 Client[" << cl.id << "]에게 Room[" << room_id << "]의 인원이 " << game_rooms[room_id].user_count << "명이 되었다고 알려줍니다.\n" << endl;
					}
				}
			}
		}

		// Server Message
		cout << "Player[ID: " << clients[target_id].id << ", name: " << clients[target_id].name << "] is log out\n" << endl;	// server message

		// 2. 세션 초기화
		clients[target_id].s_lock.lock();
		if (clients[target_id].s_state == ST_FREE) {
			clients[target_id].s_lock.unlock();
			return;
		}
		closesocket(clients[target_id].sock);
		clients[target_id].sessionClear();
		clients[target_id].s_lock.unlock();

		break;
	}
	case SESSION_LOBBY:
	{
		int session_id = target_id - CP_KEY_EX_LBY;
		extended_servers[session_id].s_lock.lock();
		if (extended_servers[session_id].s_state == ST_FREE) {
			extended_servers[session_id].s_lock.unlock();
			return;
		}
		closesocket(extended_servers[session_id].socket);
		extended_servers[session_id].s_state = ST_FREE;
		extended_servers[session_id].s_lock.unlock();

		cout << "Server[" << extended_servers[session_id].id << "]의 다운이 감지되었습니다." << endl;	// server message

		// 서버 재실행
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + session_id);	// 십의자리: Actvie여부(S: 1, A: 2), 일의자리: 서버ID

		// XD폴더 내에서 동작할 때(내부 테스트)와 외부에서 실행할 때를 구분해줍니다.
		ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../../../Execute/Execute_S", SW_SHOW);	// 내부 테스트용 (컴파일러로 실행할때)
		//ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L".", SW_SHOW);							// 외부 수출용 (exe로 실행할때

		// 클라이언트에게 Active서버가 다운되었다고 알려줌.
		if (!b_active_server) {	// 내가 Active가 아니면 상대가 Active임. (서버가 2개밖에 없기 때문)
			b_active_server = true;
			cout << "현재 Server[" << my_server_id << "] 가 Active 서버로 승격되었습니다. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// 만약 자신의 오른쪽 서버가 다운되었는데, 그 서버가 서버군의 마지막 서버인 경우 재실행된 서버에게 ConnectEx 요청을 보냅니다.
		if (session_id == MAX_LOBBY_SERVER - 1) {
			if (my_server_id < session_id) {
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
				int key = CP_KEY_EX_LBY + MAX_LOBBY_SERVER - 1;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_EX_LBY + HA_PORTNUM_LBY0;
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