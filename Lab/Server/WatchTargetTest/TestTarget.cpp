#include "../MyWatchdog/Common.h"
#include <Windows.h>
#include "../MyWatchdog/Protocol.h"

char* SERVERIP = (char*)"127.0.0.1";

int main(int argc, char* argv[])
{
	int retval;
	// 扩加 檬扁拳
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 家南 积己
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	while (1) {
		// Send Heartbeat
		char heartbeat = 1;
		retval = send(sock, &heartbeat, sizeof(heartbeat), 0);

		// Delay (1 sec)
		Sleep(1000);
	}

	// Networking End
	closesocket(sock);

	WSACleanup();
	return 0;
}