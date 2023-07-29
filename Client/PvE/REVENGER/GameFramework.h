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
#include <array>

enum SEND_MOUSE_BUTTON { SEND_NONCLICK, SEND_BUTTON_L, SEND_BUTTON_R };
enum SEND_KEY_TYPE {
	SEND_KEY_W, SEND_KEY_A, SEND_KEY_S, SEND_KEY_D, SEND_KEY_R,
	SEND_KEY_Q, SEND_KEY_E,
	SEND_KEY_SPACEBAR,
	SEND_KEYUP_MOVEKEY,
	SEND_KEY_INSERT, SEND_KEY_DELETE, SEND_KEY_END, SEND_KEY_PGUP
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
	WCHAR name[20];
	int currnum_of_people; // current number of people
	int ready_state;
};

struct MYRoomUser {
	int ready_state;
	WCHAR User_name[12];
	bool armyCheck = false;
	bool HeliCheck = false;
	int id;
	MYRoomUser() {
		ready_state = 0;
		ZeroMemory(User_name, 24);
		id = -1;
	}
	void clear() {
		ready_state = 0;
		ZeroMemory(User_name, 24);
		id = -1;
		armyCheck = false;
		HeliCheck = false;
	}
};

struct Roomname {
	WCHAR* str1 = L"R E V E N G E R";
	WCHAR* str2 = L"���� ���� ���";
	WCHAR* str3 = L"��ſ� ���� �ؿ�";
	WCHAR* str4 = L"�츮�� �ְ��� ��";
	WCHAR* str5 = L"���ǵ� ����";
	WCHAR* str6 = L"���� ����";
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
	int id;

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
	void ShotDelay();
	DWORD m_nMode = OPENINGSCENE;
	bool m_bDieMotion = false;
	bool m_bHeliDyingState = false;
	GameSound gamesound;

	float deltax = 0.0;

	// ���콺 �ΰ���
	float MouseResponsivenessX = 350.0f;
	float MouseResponsivenessY = 550.0f;
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
	ID2D1Effect* m_pd2dfxBitmapSource[UICOUNTERS];
	ID2D1Effect* m_pd2dfxGaussianBlur[UICOUNTERS];
	ID2D1Effect* m_pd2dfxEdgeDetection[UICOUNTERS];

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
	XMFLOAT3 PrevPosition;
	CPlayer* m_pPlayer = NULL;
	CCamera* m_pCamera = NULL;
	CGameObject* m_pGameObject = NULL;
	POINT m_ptOldCursorPos;
	CMaterial* m_pMaterial = NULL;
	_TCHAR	m_pszFrameRate[70];
	float ShootCnt = 0.0;

	//==================================================
	//			  ���� ����� ���� �͵�...
	//==================================================
public:
	enum INGAME_ROLE { R_NONE, R_RIFLE, R_HELI };
	enum LOGINSCENE { LS_OPENING, LS_LOBBY, LS_ROOM, LS_CREATE_ROOM };
	SceneManager* m_pScene = NULL;

	// �⺻ ����
	int m_NumOfUI = UICOUNTERS;
	int m_myID = -1;
	int m_MAX_USER;
	int m_Max_NPCs;
	int m_CurrentPlayerNum;
	queue<short> q_keyboardInput;			// ������ ���� Ű���� �Է°�

	queue<MouseInputVal> q_mouseInput;		// ������ ���� ���콺 �Է°�

	// �κ� ���� UI
	int m_roominMyId;
	int m_ingame_role = R_NONE;				// ���� (���Ѽ�, ���)
	int m_LobbyPage = 0;

	// �κ� ����
	int m_myRoomNum = 99;
	int m_LoginScene = 0;

	Roomname RoomnameList;
	WCHAR* createRoomName;
	WCHAR* currRoomName;
	vector <LobbyRoom> m_LobbyRoom_Info;
	array <MYRoomUser, 3> m_MyRoom_Info;

	// �κ� UI ���� ����
	bool m_ingame = false;
	bool m_infoReady = false;
	bool m_infoChoose = false;
	bool m_CreateRoomOkButton = false;
	bool m_RoomBackButton = false;
	bool m_bRollState = false;
	bool role_change_a2h_click = false;		// army -> heli
	bool role_change_h2a_click = false;		// heli -> human

	float m_infoReadyTime = 0.0f;
	float m_infoChooseTime = 0.0f;

	float m_StartKey = 0;
	float m_ReadyKey = 0;

	// ������ �� �� Ŭ�� ��ǥ
	bool m_GameClick[3]{ false };
	bool m_LobbyClick[3]{ false };
	bool m_LobbyRoomClick[8]{ false };		// �κ� ȭ�鿡 ���̴� �� 7ĭ
	bool m_RoomClick[3]{ false };

	LoginSceneInfo gamepos[2];
	LoginSceneInfo lobbypos[5];
	LoginSceneInfo lobbyroompos[8];			// �κ� ȭ�鿡 ���̴� �� 7ĭ
	LoginSceneInfo roompos[3];
	LoginSceneInfo createpos[2];
	LoginSceneInfo choicejob[6];

	// �ΰ���
	// �� ���� �ڽ� ����
	int m_currHp;
	int m_currbullet;


	// �� ���� �ð� ����
	int m_10MinOfTime;
	int m_1MinOfTime;
	int m_10SecOftime;
	int m_1SecOfTime;

	// �� ���� �̹��� UI
	bool m_SniperOn = false;
	bool m_BloodSplatterOn = false;
	bool m_bHeliHittingMotion = false;
	bool m_HeliPlayerWarnningSwitch = false;
	bool m_HeliPlayerWarnningUISwitch = false;
	bool UI_Switch = false;

	// �ΰ��� ���� UI Print
	int m_otherHP[2];
	wchar_t killNPCprint[100];
	wchar_t occupationPrint[100];
	WCHAR m_OtherName[2][20];

	WCHAR m_CompleteChat[60];
	WCHAR m_InsertChat[60];

	queue<ChatInfo> m_chat_info;
	queue<SendChat> m_mychat_log;

	// �̼� UI ���� ����
	bool m_missionFailed = false;
	bool m_missionClear = false;
	bool m_spendYourlife = false;

	int m_remainNPC;
	int m_mainmissionnum = 0;
	int m_occupationnum = 0;
	int m_printTime = 0;
	float m_missionFailedUI = 0.0f;
	float m_missionClearUI = 0.0f;

	// �� ���� ������Ʈ ����
	bool b_heli_mapcollide = false;
	bool m_CameraShaking = false;
	bool W_KEY, A_KEY, S_KEY, D_KEY, SPACE_KEY, SHOOT_KEY = false;
	bool isComplete = false;
	bool b_imdeadplayer = false;
	bool b_players_dead[3] = { false, };

	int b_heli_mapcollide_cooldown = 0;

	vector<CollideMapInfo> mapcol_info;
	queue<BulletPos> m_shoot_info;

	//==================================================

	//==================================================
	//			  ���� ��ſ� �ʿ��� �Լ���
	//==================================================
public:
	// ���ο� �Է��� �ִ��� Ȯ���ϴ� �Լ��Դϴ�.
	bool checkNewInput_Keyboard();
	bool checkNewInput_Mouse();

	// Ű�Է� ť���� ���� �ϳ� �����ɴϴ�.
	short popInputVal_Keyboard();
	MouseInputVal popInputVal_Mouse();

	// ��ü ���� ��� �Լ��Դϴ�.
	XMFLOAT3 getMyPosition();
	XMFLOAT3 getMyRightVec();
	XMFLOAT3 getMyLookVec();
	XMFLOAT3 getMyUpVec();
	XMFLOAT3 getMyCameraLookVec();

	// ��ü ��ǥ,���� �� �ֽ�ȭ �Լ��Դϴ�.

	// �� �ڽ� �ֽ�ȭ
	void setPosition_Self(XMFLOAT3 pos);
	void setVectors_Self(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);

	// �ٸ� �÷��̾� �ֽ�ȭ
	void setPosition_SoldiarOtherPlayer(int id, XMFLOAT3 pos);
	void setPosition_HeliOtherPlayer(XMFLOAT3 pos);
	void setVectors_SoldiarOtherPlayer(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void setVectors_HeliOtherPlayer(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void remove_OtherPlayer(int id);

	// �÷��̾� ���º�ȭ
	void HeliPlayerUnderAttack(XMFLOAT3 ToLook);
	void MyPlayerDieMotion();
	void MyPlayerRespawnMotion();
	void OtherPlayerResponeMotion(int id);

	// ���� �÷��̾� ���ۻ���
	void otherPlayerReturnToIdle(int id);
	void otherPlayerForwardMotion(int id);
	void otherPlayerBackwardMotion(int id);
	void otherPlayerSfrateMotion(int id);
	void otherPlayerShootingMotion(int id);
	void otherPlayerDyingMotion(int id);
	void otherHeliPlayerDyingMotion();

	// NPC �ֽ�ȭ
	void setPosition_Npc(int id, XMFLOAT3 pos);
	void setVectors_Npc(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec);
	void remove_Npcs(int id);

	// NPC ���� ����
	void NpcHittingMotion(int p_id);
	void DyingMotionNPC(int id);
	void AttackMotionNPC(int id);
	void MoveMotionNPC(int id);

	// ���� �ֽ�ȭ
	void updateHealpack(int h_id, bool isEffectOn);

	int a_id;
	bool active = false;
	bool m_bDamageOn = false;
	bool HumanCollsiion = false;
	bool HeliCollsiion = false;
	float m_pPlayerRotate_z = 0.0f;
	float m_pPlayerRotate_x = 0.0f;
	bool m_bFreeViewCamera = false;
	// MYRoomUser
	void setRoomUserInfo(int index, wchar_t* user_name, int user_state) {
		m_MyRoom_Info[index].ready_state = user_state;
		memcpy(m_MyRoom_Info[index].User_name, user_name, 24);
	}

	bool CollisionMap_by_PLAYER(XMFLOAT3 mappos, XMFLOAT3 mapextents, CGameObject* pTargetGameObject);
	void CollisionMap_by_BULLET(XMFLOAT3 mappos);
	void CollisionNPC_by_PLAYER(XMFLOAT3 npcpos, XMFLOAT3 npcextents);
	void CollisionNPC_by_MAP(XMFLOAT3 npcpos, XMFLOAT3 npcextents, XMFLOAT3 mapcenter, XMFLOAT3 mapextents);
	void CollisionNPC_by_BULLET(XMFLOAT3 npcpos, XMFLOAT3 npcextents);
	void CollisionEndWorldObject(XMFLOAT3 pos, XMFLOAT3 extents);

	//=================================================
	// �浹 ��ǰ� ����Ʈ ó�� �Լ��Դϴ�.
	BoundingOrientedBox m_mapxmoobb;
	BoundingOrientedBox m_mapStorexmoobb;
	BoundingOrientedBox m_npcoobb;
	BoundingOrientedBox m_worldmoobb;
	bool m_bCollisionCheck = false;

	float m_fResponCount = 0.0;
	//=================================================
	//NPC Attack
	void NpcUnderAttack(XMFLOAT3 ToLook, int npc_id);
	void NpcNoneUnderAttack();
};