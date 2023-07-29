#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <math.h>
#include <filesystem> // filename
#include <string>

#include "resource.h"

using namespace std;
using namespace std::filesystem;

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM); // ��ȭ���� ���ν���

HWND hServerStatus[6];					// ��������
HWND hServerName[6];					// �����̸�
HWND hServerPort[6];					// �����̸�
HWND hServerIpaddr[6];					// �����ּ�

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ���� IP �ּҸ� �о�´�.

	// �������� ��� ������ �����Ѵ�.
	// �����Ŵ����� �ܺ� ����(exe ����)�� ����Ͽ� ���۵Ǿ���. ���۰��������� �׽�Ʈ�� �ϰ�ʹٸ� ��� ������ ���������� Servers ���丮�� �����;��Ѵ�.
	ShellExecute(NULL, L"open", L"Server.exe", L"1", L"./Servers", SW_SHOW);
	ShellExecute(NULL, L"open", L"LobbyServer.exe", L"1", L"./Servers", SW_SHOW);
	ShellExecute(NULL, L"open", L"NpcServer.exe", L"1", L"./Servers", SW_SHOW);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);					// ��ȭ���� �ݱ�
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}
