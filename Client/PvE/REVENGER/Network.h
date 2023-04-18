#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "ObjectsInfo.h"
#include "GameSound.h"

GameSound gamesound;

#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

const char* SERVER_ADDR = "127.0.0.1";
SOCKET s_socket;
short active_servernum = 1;
array<SOCKET, MAX_SERVER> sockets;
int my_id;

int servertime_ms;
int servertime_sec;

enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };
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

void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
OVER_EX g_recv_over;
void recvPacket(short servernum)
{
	cout << "Do RECV" << endl;
	DWORD recv_flag = 0;

	memset(&g_recv_over.overlapped, 0, sizeof(g_recv_over.overlapped));
	g_recv_over.wsabuf.len = BUF_SIZE;
	g_recv_over.wsabuf.buf = g_recv_over.send_buf;
	if (WSARecv(sockets[servernum], &g_recv_over.wsabuf, 1, 0, &recv_flag, &g_recv_over.overlapped, recvCallback) == SOCKET_ERROR) {
		if (GetLastError() != WSA_IO_PENDING)
			cout << "[WSARecv Error] code: " << GetLastError() << "\n" << endl;
	}
}
void sendPacket(void* packet, short servernum)
{
	cout << "Do SEND" << endl;
	OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
	if (WSASend(sockets[servernum], &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback) == SOCKET_ERROR) {
		int err_no = GetLastError();
		if (err_no == WSAECONNRESET) {	// 서버가 끊어진 상황
			closesocket(sockets[active_servernum]);

			int new_portnum = 0;
			switch (servernum) {
			case 0:	// Active: 0 -> 1
				active_servernum = 1;
				new_portnum = PORT_NUM_S1;
				break;
			case 1:	// Active: 1 -> 0
				active_servernum = 0;
				new_portnum = PORT_NUM_S0;
				break;
			}

			sockets[active_servernum] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			SOCKADDR_IN newserver_addr;
			ZeroMemory(&newserver_addr, sizeof(newserver_addr));
			newserver_addr.sin_family = AF_INET;
			newserver_addr.sin_port = htons(new_portnum);
			inet_pton(AF_INET, SERVER_ADDR, &newserver_addr.sin_addr);
			connect(sockets[active_servernum], reinterpret_cast<sockaddr*>(&newserver_addr), sizeof(newserver_addr));

			CS_RELOGIN_PACKET re_login_pack;
			re_login_pack.size = sizeof(CS_RELOGIN_PACKET);
			re_login_pack.type = CS_RELOGIN;
			re_login_pack.id = my_id;
			sendPacket(&re_login_pack, active_servernum);
			recvPacket(active_servernum);
		}
		cout << "[WSASend Error] code: " << err_no << "\n" << endl;
	}
}

void processPacket(char* ptr);
void processData(char* net_buf, size_t io_byte);
void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	if (num_bytes == 0) {
		return;
	}

	processData(g_recv_over.send_buf, num_bytes);

	recvPacket(active_servernum);
}
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	delete over;
	return;
}


void processPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* recv_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		// Player 초기정보 설정
		my_id = recv_packet->id;
		my_info.m_id = recv_packet->id;
		my_info.m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		my_info.m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		my_info.m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
		my_info.m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };

		my_info.m_state = OBJ_ST_RUNNING;
		break;
	}// SC_LOGIN_INFO case end
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Add Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_info.m_id) break;

			if (recv_id < MAX_USER) {		// Player 추가
				other_players[recv_id].m_id = recv_id;
				other_players[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
				other_players[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				other_players[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				other_players[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
				other_players[recv_id].m_state = OBJ_ST_RUNNING;
			}
			else {
				cout << "Exceed Max User." << endl;
			}
		}
		// 2. Add Bullet
		else if (recv_packet->target == TARGET_BULLET) {
			gamesound.shootingSound();
			bullets_info[recv_id].m_id = recv_id;
			bullets_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			bullets_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			bullets_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			bullets_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			bullets_info[recv_id].m_state = OBJ_ST_RUNNING;
		}
		// 3. Add NPC (Helicopter)
		else if (recv_packet->target == TARGET_NPC) {
			if (npcs_info[recv_id].m_state != OBJ_ST_EMPTY) break;

			npcs_info[recv_id].m_id = recv_id;
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			npcs_info[recv_id].m_state = OBJ_ST_RUNNING;
		}
		else {
			cout << "[ADD ERROR] Unknown Target!" << endl;
		}
		break;
	}// SC_ADD_OBJECT case end
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Move Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_info.m_id) {
				my_info.m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };						// Player Object 이동
			}
			else if (0 <= recv_id && recv_id < MAX_USER && recv_id != my_info.m_id) {
				other_players[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };		// 상대방Players Object 이동
			}
		}
		// 2. Move Bullet
		else if (recv_packet->target == TARGET_BULLET) {
			bullets_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		}
		// 3. Move Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		}
		else {
			cout << "[MOVE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_MOVE_OBJECT case end
	case SC_ROTATE_OBJECT:
	{
		SC_ROTATE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_ROTATE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Rotate Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_info.m_id) {
				// Player 회전
				my_info.m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				my_info.m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				my_info.m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			}
			else if (0 <= recv_id && recv_id < MAX_USER) {
				// 상대 Object 회전
				other_players[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				other_players[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				other_players[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			}
		}
		// 2. Rotate Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_ROTATE_PLAYER case end
	case SC_MOVE_ROTATE_OBJECT:
	{
		SC_MOVE_ROTATE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_MOVE_ROTATE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Rotate Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_info.m_id) {
				// Player 이동
				my_info.m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
				// Player 회전
				my_info.m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				my_info.m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				my_info.m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			}
			else if (0 <= recv_id && recv_id < MAX_USER) {
				// 상대 Object 이동
				other_players[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
				// 상대 Object 회전
				other_players[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				other_players[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				other_players[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			}
		}
		// 2. Rotate Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };

			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[MOVE&ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_MOVE_ROTATE_PLAYER case end
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Remove Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) {
				// 자기자신 없애기
			}
			else if (0 <= recv_id && recv_id < MAX_USER) {
				// 상대 Player 없애기
				other_players[recv_id].m_id = -1;
				other_players[recv_id].m_pos = { 0.f ,0.f ,0.f };
				other_players[recv_id].m_state = OBJ_ST_LOGOUT;
			}
		}
		// 2. Remove Bullet
		else if (recv_packet->target == TARGET_BULLET) {
			bullets_info[recv_id].m_pos = { 0.f , -100.f ,0.f };
			bullets_info[recv_id].m_state = OBJ_ST_LOGOUT;
		}
		// 3. Remove Npc
		else if (recv_packet->target == TARGET_NPC) {
			int npc_id = recv_id - MAX_USER;
			npcs_info[npc_id].m_id = -1;
			npcs_info[npc_id].m_pos = { 0.f ,0.f ,0.f };
			npcs_info[npc_id].m_state = OBJ_ST_LOGOUT;
		}
		else {
			cout << "[REMOVE ERROR] Unknown Target!" << endl;
		}

		break;
	}//SC_REMOVE_PLAYER case end
	case SC_DAMAGED:
	{
		
		SC_DAMAGED_PACKET* recv_packet = reinterpret_cast<SC_DAMAGED_PACKET*>(ptr);

		// Player Damaged
		if (recv_packet->target == TARGET_PLAYER) {
			gamesound.collisionSound();
			if (recv_packet->id == my_id) {
				my_info.m_damaged_effect_on = true;
				my_info.m_hp -= recv_packet->dec_hp;
				if (my_info.m_hp < 0) my_info.m_hp = 0;
			}
			else {
				other_players[recv_packet->id].m_damaged_effect_on = true;
				other_players[recv_packet->id].m_hp -= recv_packet->dec_hp;
				if (other_players[recv_packet->id].m_hp < 0) other_players[recv_packet->id].m_hp = 0;
			}
		}
		// NPC Damaged
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_packet->id].m_damaged_effect_on = true;
			npcs_info[recv_packet->id].m_hp -= recv_packet->dec_hp;
			if (npcs_info[recv_packet->id].m_hp < 0) npcs_info[recv_packet->id].m_hp = 0;
		}

		break;
	}//SC_HP_COUNT case end
	case SC_BULLET_COUNT:
	{
		SC_BULLET_COUNT_PACKET* recv_packet = reinterpret_cast<SC_BULLET_COUNT_PACKET*>(ptr);
		int cnt = recv_packet->bullet_cnt;
		my_info.m_bullet = cnt;

		break;
	}//SC_BULLET_COUNT case end
	case SC_TIME_TICKING:
	{
		SC_TIME_TICKING_PACKET* recv_packet = reinterpret_cast<SC_TIME_TICKING_PACKET*>(ptr);

		servertime_ms = recv_packet->servertime_ms;
		servertime_sec = servertime_ms / 1000;
	}//SC_TIME_TICKING case end
	case SC_ACTIVE_DOWN:
	{
		SC_ACTIVE_DOWN_PACKET* recv_packet = reinterpret_cast<SC_ACTIVE_DOWN_PACKET*>(ptr);

		if (recv_packet->prev_s_id == active_servernum) {
			active_servernum = recv_packet->my_s_id;
		}
		break;
	}//SC_BULLET_COUNT case end
	}
}

void processData(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			processPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}
