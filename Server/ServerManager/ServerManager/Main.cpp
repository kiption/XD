#include <Windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <string>
using namespace std;

#define START_XPOS 0
#define START_YPOS 0
#define BLOCK_LEN 50
#define BOARD_X_MAX 8
#define BOARD_Y_MAX 8

struct Pos2D {		// 좌표 정보 (0 ~ 8, 몇 번째 칸에 있는지)
	int x, y;
};
struct RGBVal {		// 색상 정보 (0 ~ 255)
	int red, green, blue;
};
class Chesspiece {	// 체스말 객체
private:
	Pos2D m_pos;
	RGBVal m_rgb;

public:
	Chesspiece() {
		m_pos = { 0, 0 };
		m_rgb = { 0, 0, 0 };
	}
	~Chesspiece() {}

public:
	// Accesor Function
	Pos2D getPos() { return m_pos; }
	RGBVal getRGB() { return m_rgb; }
	void setPos(int new_x, int new_y) { m_pos = { new_x, new_y }; }
	void setRGB(int new_red, int new_green, int new_blue) { m_rgb = { new_red, new_green, new_blue }; }

	// Move Object Function
	void moveLeft() { if (m_pos.x > 0) m_pos.x--; }
	void moveUp() { if (m_pos.y > 0) m_pos.y--; }
	void moveRight() { if (m_pos.x < BOARD_X_MAX - 1) m_pos.x++; }
	void moveDown() { if (m_pos.y < BOARD_Y_MAX - 1) m_pos.y++; }
};

HINSTANCE g_hinst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"[Revenger] Server Manager";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevinstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hinst = hinstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hinstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(
		lpszClass,
		lpszWindowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		150,
		100,
		500,
		550,
		NULL,
		(HMENU)NULL,
		hinstance,
		NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	HBRUSH hBrush, oldBrush;

	static Chesspiece player;
	Pos2D draw_pos;

	static RECT rectView;

	switch (iMsg) {
	case WM_CREATE:
	{
		GetClientRect(hwnd, &rectView);
		player.setPos(START_XPOS, START_YPOS);
		player.setRGB(0, 0, 0);

		// 서버 IP 주소를 읽어온다.

		// 원격으로 모든 서버를 실행한다.
		// 서버매니저는 외부 실행(exe 실행)만 고려하여 제작되었다. 제작과정에서의 테스트를 하고싶다면 모든 서버의 실행파일을 Servers 디렉토리로 가져와야한다.
		ShellExecute(NULL, L"open", L"Server.exe", L"1", L"./Servers", SW_SHOW);
		ShellExecute(NULL, L"open", L"LobbyServer.exe", L"1", L"./Servers", SW_SHOW);
		ShellExecute(NULL, L"open", L"NpcServer.exe", L"1", L"./Servers", SW_SHOW);

		break;
	}
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// 보드 그리기
		for (int i = 0; i <= BOARD_X_MAX; i++) {		// 가로
			int startX = 0, endX = BLOCK_LEN * BOARD_X_MAX, y = BLOCK_LEN * i;
			MoveToEx(hdc, startX, y, NULL);
			LineTo(hdc, endX, y);
		}
		for (int i = 0; i <= BOARD_Y_MAX; i++) {		// 세로
			int x = BLOCK_LEN * i, startY = 0, endY = BLOCK_LEN * BOARD_Y_MAX;
			MoveToEx(hdc, x, startY, NULL);
			LineTo(hdc, x, endY);
		}

		// 체스말 그리기
		hBrush = CreateSolidBrush(RGB(player.getRGB().red, player.getRGB().green, player.getRGB().blue));
		oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		draw_pos = { player.getPos().x * BLOCK_LEN, player.getPos().y * BLOCK_LEN };
		Ellipse(hdc, draw_pos.x, draw_pos.y, draw_pos.x + BLOCK_LEN, draw_pos.y + BLOCK_LEN);
		SelectObject(hdc, oldBrush);
		DeleteObject(hBrush);

		EndPaint(hwnd, &ps);
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT:
			player.moveLeft();
			break;
		case VK_RIGHT:
			player.moveRight();
			break;
		case VK_UP:
			player.moveUp();
			break;
		case VK_DOWN:
			player.moveDown();
			break;
		case VK_ESCAPE:
			exit(0);
			break;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case WM_CHAR:
		if (wParam == 'q' or wParam == 'Q')
			exit(0);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}