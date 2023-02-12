#pragma once


#include "Timer.h"
#include "Player.h"
#include "HumanPlayer.h"
#include "HelicopterPlayer.h"
#include "Scene.h"
#include "Stage1.h"
#include "Stage2.h"
#include <queue> //S
enum MButton { L_BUTTON, R_BUTTON };

struct MouseInputVal {
	char button;
	float delX, delY;
};
//S
#define KEY_A         0x41
#define KEY_D         0x44
#define KEY_S         0x53
#define KEY_W         0x57
#define KEY_Q         0x51
#define KEY_E		  0x45
class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ChangeScene(DWORD nMode);
	DWORD						m_nMode = SCENE1STAGE;

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;


#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	SceneManager						*m_pScene = NULL;
	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];



//==================================================
//			  ���� ����� ���� �͵�...
//==================================================
public:
	// ������ ���� Ű���� �Է°�
	queue<short> q_keyboardInput;

	// ������ ���� ���콺 �Է°�
	queue<MouseInputVal> q_mouseInput;
	//==================================================

	//==================================================
	//			  ���� ��ſ� �ʿ��� �Լ���
	//==================================================
public:
	// ���ο� �Է��� �ִ��� Ȯ���ϴ� �Լ��Դϴ�.
	bool CheckNewInputExist_Keyboard();
	bool CheckNewInputExist_Mouse();

	// Ű�Է� ť���� ���� �ϳ� �����ɴϴ�.
	short PopInputVal_Keyboard();
	MouseInputVal PopInputVal_Mouse();

	// ��ü �� �ֽ�ȭ �Լ��Դϴ�.
	void SetPosition_PlayerObj(XMFLOAT3 pos);
	void SetVectors_PlayerObj(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);

	void SetPosition_OtherPlayerObj(int id, XMFLOAT3 pos);
	void SetVectors_OtherPlayerObj(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void Remove_OtherPlayerObj(int id);

	void Create_Bullet(int id, XMFLOAT3 pos, XMFLOAT3 xmf3look);
	void SetPosition_Bullet(int id, XMFLOAT3 pos, XMFLOAT3 xmf3look);

	void SetPosition_NPC(int id, XMFLOAT3 pos);
	void SetVectors_NPC(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);

	//==================================================

};

