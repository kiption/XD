#pragma once

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Stage2.h"
#include "GameSound.h"
#include "MainPlayer.h"
#include "HumanPlayer.h"
#include <queue> //S

enum MButton { L_BUTTON, R_BUTTON };

class UILayer;

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


struct CB_FRAMEWORK_INFO
{
	float					m_fCurrentTime=0.0f;
	float					m_fElapsedTime=0.0f;
	float					m_fSecondsPerFirework = 0.1f;
	int						m_nFlareParticlesToEmit = 30;
	XMFLOAT3				m_xmf3Gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);
	int						m_nMaxFlareType2Particles = 15;
};

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateCommandQueueAndList();
	void CreateDirect3DDevice();

//==================================================
//			  서버 통신을 위한 것들...
//==================================================
public:
	SceneManager* m_pScene = NULL;
	// 서버로 보낼 키보드 입력값
	queue<short> q_keyboardInput;

	// 서버로 보낼 마우스 입력값
	queue<MouseInputVal> q_mouseInput;

	// 서버에서 받은 총알 개수
	WCHAR m_myBullet[20];
	WCHAR m_myhp[20];

	int m_currHp;
	int m_time;
//==================================================

//==================================================
//			  서버 통신에 필요한 함수들
//==================================================
public:
	// 새로운 입력이 있는지 확인하는 함수입니다.
	bool CheckNewInputExist_Keyboard();
	bool CheckNewInputExist_Mouse();

	// 키입력 큐에서 값을 하나 꺼내옵니다.
	short PopInputVal_Keyboard();
	MouseInputVal PopInputVal_Mouse();

	// 객체 값 최신화 함수입니다.
	void SetPosition_PlayerObj(XMFLOAT3 pos);
	void SetVectors_PlayerObj(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);

	void SetPosition_OtherPlayerObj(int id, XMFLOAT3 pos);
	void SetVectors_OtherPlayerObj(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void Remove_OtherPlayerObj(int id);

	void Create_Bullet(int id, XMFLOAT3 pos, XMFLOAT3 xmf3look);


	void SetPosition_NPC(int id, XMFLOAT3 pos);
	void SetVectors_NPC(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);

	void SetPosition_Bullet(int id, XMFLOAT3 pos, XMFLOAT3 xmf3right, XMFLOAT3 xmf3up, XMFLOAT3 xmf3look);
	
	void UpdateUI();
//==================================================

#ifdef _WITH_DIRECT2D
	void CreateDirect2DDevice();
#endif

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

	void CreateShaderVariables();
	void ReleaseShaderVariables();
	void UpdateShaderVariables();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void ChangeScene(DWORD nMode);
	int m_NumOfUI = 8;
	DWORD						m_nMode = SCENE1STAGE;
	CPlayer* m_pPlayer = NULL;
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

	static const UINT			m_nSwapChainBuffers = 3;
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
	GameSound m_GameSound;
	///
	UINT64						m_pnFenceValues[m_nSwapChainBuffers];
	ID3D12CommandAllocator* m_ppd3dCommandAllocators[m_nSwapChainBuffers];
	ID3D12Device* m_pd3d12Device = NULL;
	D3D12_VIEWPORT				m_d3dViewport;
	D3D12_RECT					m_d3dScissorRect;
#ifdef _WITH_DIRECT2D
	ID3D11On12Device* m_pd3d11On12Device = NULL;
	ID3D11DeviceContext* m_pd3d11DeviceContext = NULL;
	ID2D1Factory3* m_pd2dFactory = NULL;
	IDWriteFactory* m_pdWriteFactory = NULL;
	ID2D1Device2* m_pd2dDevice = NULL;
	ID2D1DeviceContext2* m_pd2dDeviceContext = NULL;

	ID3D11Resource* m_ppd3d11WrappedBackBuffers[m_nSwapChainBuffers];
	ID2D1Bitmap1* m_ppd2dRenderTargets[m_nSwapChainBuffers];

	ID2D1SolidColorBrush* m_pd2dbrBackground = NULL;
	ID2D1SolidColorBrush* m_pd2dbrBorder = NULL;
	IDWriteTextFormat* m_pdwFont = NULL;
	IDWriteTextLayout* m_pdwTextLayout = NULL;
	ID2D1SolidColorBrush* m_pd2dbrText = NULL;

#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	IWICImagingFactory* m_pwicImagingFactory = NULL;
	ID2D1Effect* m_pd2dfxBitmapSource[8];
	ID2D1Effect* m_pd2dfxGaussianBlur[8];
	ID2D1Effect* m_pd2dfxEdgeDetection[8];
	ID2D1DrawingStateBlock1* m_pd2dsbDrawingState = NULL;
	IWICFormatConverter* m_pwicFormatConverter = NULL;
	int							m_nDrawEffectImage = 0;
#endif
#endif

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;



	CCamera						*m_pCamera = NULL;
	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	_TCHAR						m_pszCaption[100];

	UILayer						*m_pUILayer = NULL;

protected:
	ID3D12Resource* m_pd3dcbFrameworkInfo = NULL;
	CB_FRAMEWORK_INFO* m_pcbMappedFrameworkInfo = NULL;

};

