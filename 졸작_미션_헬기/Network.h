#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "../Server/Server/protocol.h"
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

int remain_size = 0;
void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
OVER_EX recv_over;	// 09.25
void recv_packet()
{
	cout << "Do RECV" << endl;
	DWORD recv_flag = 0;
	//OVER_EX recv_over;
	memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
	recv_over.wsabuf.len = BUF_SIZE;
	recv_over.wsabuf.buf = recv_over.send_buf;
	//WSARecv(s_socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
	int ret = WSARecv(s_socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, recvCallback);
	//if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
	//	cout << "WSARecv Error - " << ret << endl;
	//	cout << GetLastError() << endl;
	//}
	if (ret != 0) {
		if (GetLastError() == WSA_IO_PENDING)
			SleepEx(100, TRUE);
		else {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
}
void send_packet(void* packet)
{
	cout << "Do SEND" << endl;
	OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
	//WSASend(s_socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback);
	int ret = WSASend(s_socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback);
	if (ret != 0) {
		cout << "WSASend Error - " << ret << endl;
		cout << GetLastError() << endl;
	}
}

void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	if (num_bytes == 0) {
		cout << "NUM BYTE zero" << endl; //test
		return;
	}

	cout << "RECV CALLBACK: " << num_bytes << " bytes." << endl; //test
	recv_packet();
}
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	cout << "SEND CALLBACK" << endl; //test
	delete over;
	return;
}


int my_id;
void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		my_id = packet->id;
		// Player 초기 위치 설정

		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* my_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = my_packet->id;

		if (id < MAX_USER) {
			// Player 초기 위치로 이동
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == my_id) {
			// Player 이동
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == my_id) {
			// 자기자신 없애기
		}

		break;
	}
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
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
