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
void recvPacket()
{
	cout << "Do RECV" << endl;
	DWORD recv_flag = 0;

	memset(&g_recv_over.overlapped, 0, sizeof(g_recv_over.overlapped));
	g_recv_over.wsabuf.len = BUF_SIZE;
	g_recv_over.wsabuf.buf = g_recv_over.send_buf;
	if (WSARecv(s_socket, &g_recv_over.wsabuf, 1, 0, &recv_flag, &g_recv_over.overlapped, recvCallback) == SOCKET_ERROR) {
		if (GetLastError() != WSA_IO_PENDING)
			cout << "[WSARecv Error] code: " << GetLastError() << "\n" << endl;
	}
}
void sendPacket(void* packet)
{
	cout << "Do SEND" << endl;
	OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
	if (WSASend(s_socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback) == SOCKET_ERROR) {
		cout << "[WSASend Error] code: " << GetLastError() << "\n" << endl;
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

	recvPacket();
}
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	delete over;
	return;
}


int my_id;
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
			//int npc_id = recv_id - MAX_USER;
			npcs_info[recv_id].m_id = recv_id;
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
			//int npc_id = recv_id - MAX_USER;
			npcs_info[recv_id].m_id = recv_id;
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_ROTATE_PLAYER case end
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

		if (recv_packet->target == TARGET_PLAYER && recv_packet->id == my_id) {
			my_info.m_hp = my_info.m_hp - recv_packet->dec_hp;

			XMFLOAT3 coll_pos = { recv_packet->col_pos_x, recv_packet->col_pos_y, recv_packet->col_pos_z };
			coll_info.push(coll_pos);

			gamesound.collisionSound();
		}

		break;
	}//SC_HP_COUNT case end
	case SC_PLAYER_STATE:
	{
		SC_PLAYER_STATE_PACKET* recv_packet = reinterpret_cast<SC_PLAYER_STATE_PACKET*>(ptr);

		int pl_state = recv_packet->state;
		if (pl_state == ST_PACK_REVIVAL) {
			my_info.m_hp = 100;
		}
		else if (pl_state == ST_PACK_DEAD) {
			my_info.m_hp = 0;
		}

		break;
	}//SC_PLAYER_STATE case end
	case SC_BULLET_COUNT:
	{
		SC_BULLET_COUNT_PACKET* recv_packet = reinterpret_cast<SC_BULLET_COUNT_PACKET*>(ptr);
		int cnt = recv_packet->bullet_cnt;
		my_info.m_bullet = cnt;

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
