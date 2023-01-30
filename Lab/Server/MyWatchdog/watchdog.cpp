#include "Common.h"
#include <iostream>
#include "Protocol.h"
using namespace std;

// =============================================
// ServerWatch: ���� ����(watch) ������ ����ϴ� �������Լ�
// ���� ��� �����κ��� Heartbeat Signal�� �ް� 3ȸ �̻� �ñ׳��� ���� ���� ������ �ִٸ� ���� �ٿ����� �����ϰ� ������ ���̰� �ٽ� �����մϴ�.
DWORD WINAPI ServerWatch(LPVOID arg)
{
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// Recv
		char heartbeat = 0;
		retval = recv(client_sock, &heartbeat, sizeof(heartbeat), 0);

		if (heartbeat) {
			cout << "���ô���� ���� �۵� ���Դϴ�." << endl;
		}
		else {
			cout << "���ô���� ���� �ߴ��� �����Ǿ����ϴ�." << endl;
			cout << "����� ������մϴ�." << endl;
			ShellExecute(NULL, "open", "WatchTargetTest", NULL, "../WatchTargetTest/x64/Release", SW_SHOW);
			return 0;
		}
	}

	return 0;
}
// =============================================

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(WATCHDOG_PORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	SOCKET target_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	HANDLE hServerWatchThread[WATCH_TARGET_NUM];
	int cur_target_num = 0;

	while (1) {
		addrlen = sizeof(clientaddr);
		target_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (target_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n���� ����� Connect �Ϸ�: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", addr, ntohs(clientaddr.sin_port));

		// ������ ����
		hServerWatchThread[cur_target_num] = CreateThread(NULL, 0, ServerWatch, (LPVOID)target_sock, 0, NULL);
		if (hServerWatchThread[cur_target_num] == NULL)
			closesocket(target_sock);
		else
			CloseHandle(hServerWatchThread[cur_target_num]);
		cur_target_num++;
	}

	closesocket(listen_sock);

	WSACleanup();
	return 0;
}