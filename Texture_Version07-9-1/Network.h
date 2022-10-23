#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
//#include "../Server/Server/protocol.h"
#include "ObjectsInfo.h"
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
		cout << "NUM BYTE ZERO" << endl; //test
		return;
	}
	cout << "RECV CALLBACK " << endl; //test

	processData(g_recv_over.send_buf, num_bytes);

	recvPacket();
}
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	cout << "SEND CALLBACK" << endl; //test
	delete over;
	return;
}


int my_id;
void processPacket(char* ptr)
{
	cout << "[Process Packet] Packet Type: " << (int)ptr[1] << endl;//test
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* recv_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		my_id = recv_packet->id;
		// Player 초기 위치 설정
		//cout << "Init Position - x: " << recv_packet->x << ", y : " << recv_packet->y << ", z : " << recv_packet->z << endl;
		my_info.m_id = recv_packet->id;
		my_info.m_x = recv_packet->x;
		my_info.m_y = recv_packet->y;
		my_info.m_z = recv_packet->z;
		my_info.m_state = OBJ_ST_RUNNING;
		cout << "Init My Info - id: " << my_info.m_id << ", Pos(x: " << my_info.m_x << ", y : " << my_info.m_y << ", z : " << my_info.m_z << ")." << endl;

		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* recv_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int recv_id = recv_packet->id;
		if (recv_id == my_info.m_id) break;

		if (recv_id < MAX_USER) {		// Player 추가
			other_players[recv_id].m_id = recv_id;
			other_players[recv_id].m_x = recv_packet->x;
			other_players[recv_id].m_y = recv_packet->y;
			other_players[recv_id].m_z = recv_packet->z;
			other_players[recv_id].m_state = OBJ_ST_RUNNING;
			cout << "Init New Player's Info - id: " << other_players[recv_id].m_id
				<< ", Pos(x: " << other_players[recv_id].m_x << ", y : " << other_players[recv_id].m_y << ", z : " << other_players[recv_id].m_z << ")." << endl;
		}
		else if (MAX_USER <= recv_id && recv_id < MAX_USER + MAX_NPCS) {	// NPC 추가
			int npc_id = recv_id - MAX_USER;
			npcs_info[npc_id].m_id = recv_id;
			npcs_info[npc_id].m_x = recv_packet->x;
			npcs_info[npc_id].m_y = recv_packet->y;
			npcs_info[npc_id].m_z = recv_packet->z;
			npcs_info[npc_id].m_state = OBJ_ST_RUNNING;
			cout << "Init New NPC's Info - id: " << npcs_info[npc_id].m_id
				<< ", Pos(x: " << npcs_info[npc_id].m_x << ", y : " << npcs_info[npc_id].m_y << ", z : " << npcs_info[npc_id].m_z << ")." << endl;
		}
		else {
			cout << "Exceed Max User." << endl;
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* recv_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int recv_id = recv_packet->id;
		if (recv_id == my_info.m_id) {
			// Player 이동
			my_info.m_x = recv_packet->x;
			my_info.m_y = recv_packet->y;
			my_info.m_z = recv_packet->z;
			cout << "My object moves to (" << my_info.m_x << ", " << my_info.m_y << ", " << my_info.m_z << ")." << endl;
		}
		else {
			// 상대 Object 이동
			other_players[recv_id].m_x = recv_packet->x;
			other_players[recv_id].m_y = recv_packet->y;
			other_players[recv_id].m_z = recv_packet->z;
			cout << "Player[" << recv_id << "]'s object moves to("
				<< other_players[recv_id].m_x << ", " << other_players[recv_id].m_y << ", " << other_players[recv_id].m_z << ")." << endl;
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* recv_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int recv_id = recv_packet->id;
		if (recv_id == my_id) {
			// 자기자신 없애기
		}
		else {
			// 상대 Object 없애기
			other_players[recv_id].m_id = -1;
			other_players[recv_id].m_x = 0;
			other_players[recv_id].m_y = 0;
			other_players[recv_id].m_z = 0;
			other_players[recv_id].m_state = OBJ_ST_EMPTY;

			cout << "Player[" << recv_id << "] is log out" << endl;
		}

		break;
	}
	}
}

void processData(char* net_buf, size_t io_byte)
{
	cout << "Process Data" << endl;//test
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
