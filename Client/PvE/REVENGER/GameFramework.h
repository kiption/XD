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
	SEND_KEY_W, SEND_KEY_A, SEND_KEY_S, SEND_KEY_D, SEND_KEY_R,
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

struct ChatInfo {
	WCHAR chatData[80];
};

struct SendChat {
	char chatData[60];
};

struct LobbyRoom {
	int num;
	WCHAR* name;
	int currnum_of_people; // current number of people
	int ready_state;
};

struct MYRoomUser {
	int ready_state;
	WCHAR* User_name;
};

struct Roomname {
	WCHAR* str1 = L"즐겁지 않은 졸작";
	WCHAR* str2 = L"빠른 게임 고고";
	WCHAR* str3 = L"즐거운 게임 해요";
};

struct CollideMapInfo
{
	XMFLOAT3 m_pos;
	XMFLOAT3 m_scale;
	XMFLOAT3 m_local_forward;
	XMFLOAT3 m_local_right;
	XMFLOAT3 m_local_rotate;
	float m_angle_aob;
	float m_angle_boc;

	BoundingOrientedBox m_xoobb;

	CollideMapInfo() {
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_scale = { 0.0f, 0.0f, 0.0f };;
		m_xoobb = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void InfoClear() {
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_scale = { 0.0f, 0.0f, 0.0f };
		m_xoobb = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void setBB() {
		XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(m_local_rotate.x, m_local_rotate.y, m_local_rotate.z);

		XMFLOAT4 oriented;
		XMStoreFloat4(&oriented, rotation);

		m_xoobb = BoundingOrientedBox(XMFLOAT3(m_pos.x, m_pos.y, m_pos.z), XMFLOAT3(m_scale.x, m_scale.y, m_scale.z), oriented);
	}
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
	int m_NumOfUI = 54;
	bool UI_Switch = false;
	bool m_bRollState = false;
	bool m_LoginClick[4]{ false };
	bool m_GameClick[3]{ false };
	bool m_LobbyClick[3]{ false };
	bool m_RoomClick[3]{ false };
	bool m_SniperOn = false;
	LoginSceneInfo loginpos[4];
	LoginSceneInfo gamepos[3];
	LoginSceneInfo lobbypos[3];
	LoginSceneInfo roompos[3];
	LoginSceneInfo createpos[2];

	bool m_bLoginInfoSend = false;
	int m_LoginScene = 0;
	float m_StartKey = 0;
	float m_ReadyKey = 0;
	bool m_CameraShaking = false;
	float deltax = 0.0;
	float MouseResponsiveness = 700.0f;
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
	IDWriteTextFormat* m_pdwFont[6];
	IDWriteTextLayout* m_pdwTextLayout[6];
	ID2D1SolidColorBrush* m_pd2dbrText[6];

#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	IWICImagingFactory* m_pwicImagingFactory = NULL;
	ID2D1Effect* m_pd2dfxBitmapSource[54];
	ID2D1Effect* m_pd2dfxGaussianBlur[54];
	ID2D1Effect* m_pd2dfxEdgeDetection[54];
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

	IDXGIFactory4* m_pdxgiFactory = NULL;
	IDXGISwapChain3* m_pdxgiSwapChain = NULL;
	ID3D12Device* m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;
	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator* m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue* m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;

	ID3D12Fence* m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;
	D3D12_VIEWPORT				m_d3dViewport;
	D3D12_RECT					m_d3dScissorRect;

#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

public:
	XMFLOAT3	PrevPosition;
	CPlayer* m_pPlayer = NULL;
	CCamera* m_pCamera = NULL;
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
	Roomname RoomnameList;
	WCHAR m_LoginID[20];
	WCHAR m_LoginPW[20];
	WCHAR m_LoginIP[20];
	WCHAR m_InsertChat[60];
	WCHAR* createRoomName;
	int m_myRoomNum = 13;

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
	int m_LobbyPage = 0;
	vector<CollideMapInfo> mapcol_info;
	vector<LobbyRoom>m_LobbyRoom_Info;
	vector<MYRoomUser> m_MyRoom_Info;
	queue<BulletPos> m_shoot_info;

	queue<ChatInfo> m_chat_info;
	queue<SendChat> m_mychat_log;

	wchar_t killNPCprint[100];
	wchar_t occupationPrint[100];
	wchar_t SurviveSecPrint[20];
	wchar_t FlyAtkPrint[20];
	wchar_t KillArmyPrint[20];


	bool W_KEY, A_KEY, S_KEY, D_KEY, SPACE_KEY, SHOOT_KEY = false;

	bool player_dead = false;
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
	void DyingMotionNPC(int id);
	void MoveMotionNPC(int id);

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