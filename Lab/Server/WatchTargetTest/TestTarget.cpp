#include "../MyWatchdog/Common.h"
#include <Windows.h>
#include "../MyWatchdog/Protocol.h"

char* SERVERIP = (char*)"127.0.0.1";
#define BUFSIZE 512

DWORD WINAPI SendHeartbeat(LPVOID arg)
{
	int retval;
	SOCKET watchdog_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];

	addrlen = sizeof(clientaddr);
	getpeername(watchdog_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// Send Heartbeat
		char heartbeat = 1;
		retval = send(watchdog_sock, &heartbeat, sizeof(heartbeat), 0);

		// Delay (1 sec)
		Sleep(1000);
	}
}

int main(int argc, char* argv[])
{
	int retval;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET watchdog_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (watchdog_sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(WATCHDOG_PORT);
	retval = connect(watchdog_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 하트비트 스레드 생성
	HANDLE hHeartbeatThread = CreateThread(NULL, 0, SendHeartbeat, (LPVOID)watchdog_sock, 0, NULL);
	if (hHeartbeatThread == NULL) { closesocket(watchdog_sock); }
	else { CloseHandle(hHeartbeatThread); }

	while (1) {
		Sleep(1000);
	}

	// Networking End
	closesocket(watchdog_sock);

	WSACleanup();
	return 0;
}