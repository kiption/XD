#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <math.h>
#include <filesystem> // filename
#include <string>

#include "resource.h"

using namespace std;
using namespace std::filesystem;

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM); // 대화상자 프로시저

HWND hServerStatus[6];					// 서버상태
HWND hServerName[6];					// 서버이름
HWND hServerPort[6];					// 서버이름
HWND hServerIpaddr[6];					// 서버주소

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 서버 IP 주소를 읽어온다.

	// 원격으로 모든 서버를 실행한다.
	// 서버매니저는 외부 실행(exe 실행)만 고려하여 제작되었다. 제작과정에서의 테스트를 하고싶다면 모든 서버의 실행파일을 Servers 디렉토리로 가져와야한다.
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
			EndDialog(hDlg, IDCANCEL);					// 대화상자 닫기
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}
