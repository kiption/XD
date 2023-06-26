#pragma once
#pragma once


#include "Timer.h"
#include "Player.h"
#include "HumanPlayer.h"
#include "HelicopterPlayer.h"
#include "Scene.h"
#include "Stage1.h"
#include "Stage2.h"
#include "GameSound.h"
#include "PostProcessShader.h"
// Server
#include <queue>
enum SEND_MOUSE_BUTTON { SEND_NONCLICK, SEND_BUTTON_L, SEND_BUTTON_R };
enum SEND_KEY_TYPE {
	SEND_KEY_NUM1, SEND_KEY_NUM2,
	SEND_KEY_W, SEND_KEY_A, SEND_KEY_S, SEND_KEY_D,
	SEND_KEY_UP, SEND_KEY_LEFT, SEND_KEY_DOWN, SEND_KEY_RIGHT,
	SEND_KEY_SPACEBAR,
	SEND_KEYUP_MOVEKEY
};

struct MouseInputVal {
	char button;
	float delX, delY;

	MouseInputVal() { button = SEND_NONCLICK; delX = delY = 0.f; }
	MouseInputVal(char btn, float dx, float dy) { button = btn; delX = dx; delY = dy; }
};

struct BulletPos {
	float x, y, z;
};

struct LoginSceneInfo {
	float sx, sy, lx, ly;
};



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

	void CreateShaderVariables();
	void ReleaseShaderVariables();
	void UpdateShaderVariables();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ChangeScene(DWORD nMode);
	DWORD						m_nMode = OPENINGSCENE;

	GameSound gamesound;
	int m_NumOfUI = 50;
	bool UI_Switch = false;
	bool m_bRollState = false;
	bool m_Login[4]{ false };
	bool m_GameState[3]{ false };
	LoginSceneInfo loginpos[4];
	LoginSceneInfo gamepos[3];
	LoginSceneInfo lobbypos[3];

	vector<WCHAR>IDProcess;
	vector<WCHAR>PWProcess;
	vector<WCHAR>IPProcess;
	int m_LoginScene = 0;
public:
	PostProcessShader* m_pPostProcessingShader = NULL;
#ifdef _WITH_DIRECT2D
	void CreateDirect2DDevice();
#endif
	static const UINT			m_nSwapChainBuffers = 2;

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
	ID2D1Effect* m_pd2dfxBitmapSource[50];
	ID2D1Effect* m_pd2dfxGaussianBlur[50];
	ID2D1Effect* m_pd2dfxEdgeDetection[50];
	ID2D1DrawingStateBlock1* m_pd2dsbDrawingState = NULL;
	IWICFormatConverter* m_pwicFormatConverter = NULL;
	int							m_nDrawEffectImage = 0;
#endif
#endif
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

	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;
	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;
	D3D12_VIEWPORT				m_d3dViewport;
	D3D12_RECT					m_d3dScissorRect;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

public:
	XMFLOAT3	PrevPosition;
	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;
	CGameObject* m_pGameObject = NULL;
	POINT						m_ptOldCursorPos;
	CMaterial* m_pMaterial = NULL;
	_TCHAR						m_pszFrameRate[70];
	float ShootCnt = 0.0;

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
	WCHAR m_remainNPCPrint[20];
	
	int m_mainmissionnum = 0;
	int m_submissionnum = 0;

	int m_currHp;
	int m_currbullet;
	
	int m_10MinOfTime;
	int m_1MinOfTime;
	int m_10SecOftime;
	int m_1SecOfTime;

	int m_remainNPC;

	int m_killArmy;
	int m_AttackFly;
	int m_survive = 12;

	int m_CurrentPlayerNum;

	int m_occupationnum = 0;
	queue<BulletPos> m_shoot_info;

	wchar_t killNPCprint[100];
	wchar_t occupationPrint[100];
	wchar_t SurviveSecPrint[20];
	wchar_t FlyAtkPrint[20];
	wchar_t KillArmyPrint[20];


	bool W_KEY, A_KEY, S_KEY, D_KEY, SPACE_KEY,SHOOT_KEY = false;
//==================================================

//==================================================
//			  서버 통신에 필요한 함수들
//==================================================
public:
	// 새로운 입력이 있는지 확인하는 함수입니다.
	bool checkNewInput_Keyboard();
	bool checkNewInput_Mouse();

	// 키입력 큐에서 값을 하나 꺼내옵니다.
	short popInputVal_Keyboard();
	MouseInputVal popInputVal_Mouse();

	// 객체 값을 얻는 함수입니다.
	XMFLOAT3 getMyPosition();
	XMFLOAT3 getMyRightVec();
	XMFLOAT3 getMyLookVec();
	XMFLOAT3 getMyUpVec();

	// 객체 좌표,벡터 값 최신화 함수입니다.
	void setPosition_Self(XMFLOAT3 pos);
	void setVectors_Self(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void setPosition_OtherPlayer(int id, XMFLOAT3 pos);
	void setVectors_OtherPlayer(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void remove_OtherPlayer(int id);

	void setPosition_Npc(int id, XMFLOAT3 pos);
	void setVectors_Npc(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void remove_Npcs(int id);

	// 클라이언트 객체의 상태 최신화 함수
	int a_id;
	bool active = false;
	void otherPlayerReturnToIdle(int p_id);
	void otherPlayerForwardMotion(int p_id);
	void otherPlayerBackwardMotion(int p_id);
	void otherPlayerSfrateMotion(int p_id);
	void otherPlayerShootingMotion(int p_id);
	void otherPlayerDyingMotion(int p_id);
	bool m_bDamageOn = false;
	float m_pPlayerRotate_z = 0.0f;
	float m_pPlayerRotate_x = 0.0f;
//==================================================
	// 서버에서 받은 Bound 값과의 충돌설정 함수입니다.
	//player - map
	bool CollisionMap_by_PLAYER(XMFLOAT3 mappos, XMFLOAT3 mapextents, CGameObject* pTargetGameObject);
	//bullet - map
	void CollisionMap_by_BULLET(XMFLOAT3 mappos, XMFLOAT3 mapextents);
	//npc - (player/map/bullet)
	void CollisionNPC_by_PLAYER(XMFLOAT3 npcpos, XMFLOAT3 npcextents);
	void CollisionNPC_by_MAP(XMFLOAT3 npcpos, XMFLOAT3 npcextents, XMFLOAT3 mapcenter, XMFLOAT3 mapextents);
	void CollisionNPC_by_BULLET(XMFLOAT3 npcpos, XMFLOAT3 npcextents);
	void CollisionEndWorldObject(XMFLOAT3 pos, XMFLOAT3 extents);

	bool HumanCollsiion = false;
	bool HeliCollsiion = false;
	void CollisionDummiesObjects(int id);

//=================================================
	// 충돌 모션과 이펙트 처리 함수입니다.

	//void Motion_BulletbyPlayer(int id, XMFLOAT3 mappos, XMFLOAT3 mapextents);
	BoundingOrientedBox m_mapxmoobb;
	BoundingOrientedBox m_mapStorexmoobb;
	BoundingOrientedBox m_npcoobb;
	BoundingOrientedBox m_worldmoobb;
	bool m_bCollisionCheck = false;

	float m_fResponCount = 0.0;
//=================================================
	//NPC Attack
	void HeliNpcUnderAttack(int id, XMFLOAT3 ToLook);
};



//npc - map
//npc - bullet