#pragma once
#include "SceneMgr.h"
#include "SpriteAnimationBillboard.h"
#include "BoundingWire.h"
class Player;
class CHumanPlayer;

class MainGameScene : public SceneMgr
{
public:
	MainGameScene();
	~MainGameScene();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void BuildDefaultLightsAndMaterials();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, ID3D12Resource* pd3dDepthStencilBuffer);
	virtual void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	bool ProcessInput(UCHAR* pKeysBuffer);
	virtual void AnimateObjects(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void OtherHeliPlayerTransformStore();
	void OtherHeliPlayerTransfromReset();
	void BillBoardRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position);
	void MuzzleFlameRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position);
	void NPCMuzzleFlamedRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position);
	void HealPackZoneInfo();
	void ReleaseUploadBuffers();
	void PlayerByPlayerCollision();
	void SetPositionPilotHuman();
	void NpcByPlayerCollsiion();
	void PlayersMoveReflect();
	bool m_bSendCollsion = false;
public:
	static void CreateDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	static void CreateSRVs(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	static void CreateSRVs(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats) {};
	static void CreateSRVs(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex, ID3D12Resource* pShadowMap);
	static void CreateCBV(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateCBVs(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
public:
	static ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dShadowGPUDescriptorHandle;//그림자 텍스쳐를 넘기기위한 디스크립터의 주소
public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }
	float m_fElapsedTime = 0.0f;
	bool m_bPartitionEnd = false;
	int	m_nBillboardShaders = 0;
	int m_nSpriteBillboards = 0;

	BillboardShader** m_pBillboardShader = NULL;
	CSpriteObjectsShader** m_ppSpriteBillboard = NULL;
	CSkyBox* m_pSkyBox = NULL;
	CHeightMapTerrain* m_pTerrain = NULL;

	int	m_nLights = 0;
	XMFLOAT4 m_xmf4GlobalAmbient;
	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	void Firevalkan(GameObjectMgr* Objects, XMFLOAT3 ToPlayerLook);
	void ParticleCollisionResult();
	void OtherPlayerFirevalkan(GameObjectMgr* Objects, XMFLOAT3 ToPlayerLook);
	void PlayerFirevalkan(CCamera* pCamera, XMFLOAT3 Look);
	void Reflectcartridgecase(GameObjectMgr* Objects);
public:
	bool m_bHeliParticleCollisionCheck = false;
	bool m_bHumanParticleCollisionCheck = false;
	float m_fBulletEffectiveRange = 2000.0f;
	BulletEffectShader* pBCBulletEffectShader = NULL;
	CValkanObject* pBulletObject = NULL;
	CValkanObject* m_ppBullets[HELIBULLETS];
	CValkanObject* pCartridge = NULL;
	CValkanObject* m_ppCartridge[CARTRIDGES];
	CValkanObject* pValkan = NULL;
	CValkanObject* m_ppValkan[HELICOPTERVALKANS];
public:
	XMFLOAT4X4 m_pMainRotorFrameP{};
	XMFLOAT4X4 m_pTailRotorFrameP{};
	XMFLOAT4X4 m_pFrameFragObj1P{};
	XMFLOAT4X4 m_pFrameFragObj2P{};
	XMFLOAT4X4 m_pFrameFragObj3P{};
	XMFLOAT4X4 m_pFrameFragObj4P{};
	XMFLOAT4X4 m_pFrameFragObj5P{};
	XMFLOAT4X4 m_pFrameFragObj6P{};
	XMFLOAT4X4 m_pFrameFragObj7P{};
	XMFLOAT4X4 m_pFrameFragObj8P{};
	XMFLOAT4X4 m_pFrameFragObj9P{};
	XMFLOAT4X4 m_pFrameFragObj10P{};
	XMFLOAT4X4 m_pFrameFragObj11P{};
};
