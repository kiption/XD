#pragma once

#include "Shader.h"
#include "Skybox.h"
#include "ObjcetsShaderList.h"
#include "BillboardObjectsShader.h"
#include "SkyboxShader.h"
#include "TerrainShader.h"
#include "ShadowShader.h"
#include "PostProcessShader.h"
#include "BoundingWire.h"
#include "Player.h"
#include "HumanPlayer.h"
#include "HelicopterPlayer.h"
#include "Object.h"
#include "MissileObject.h"
#include "ParticleObject.h"
#include "GameSound.h"

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT4							m_xmf4Emissive;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};
class SceneMgr
{
public:
	SceneMgr();
	~SceneMgr();
public:
	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void BuildDefaultLightsAndMaterials();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, ID3D12Resource* pd3dDepthStencilBuffer);

	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	//virtual void AnimateObjects(CCamera* pCamera, float fTimeElapsed);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	bool ProcessInput(UCHAR* pKeysBuffer);
	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {};
	void LightMotion();
	void SetCurScene(int nCurScene) { m_nCurScene = nCurScene; }
	int GetCurScene() { return m_nCurScene; }

	virtual void ReleaseUploadBuffers();
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);


	BoundingWireShader* m_pBoundingBoxShader = NULL;

	float lightCounter = 0.0f;
	XMFLOAT3 SmokePosition;
	XMFLOAT3 ExplosingPosition;
	GameSound gamesound;
	GameObjectMgr** m_ppGameObject = NULL;
	int m_nObjects = 0;
	ID3D12Resource* m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;
	MATERIALS* m_pMaterials = NULL;
public:
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;

	int	m_nCurScene = OPENING_SCENE;

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
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dShadowGPUDescriptorHandle;//�׸��� �ؽ��ĸ� �ѱ������ ��ũ������ �ּ�

public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	static void CreateDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	static void CreateCBV(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateCBVs(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static void CreateSRVs(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	static void CreateSRVs(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats) {};
	static void CreateSRVs(ID3D12Device* pd3dDevice, Texture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex, ID3D12Resource* pShadowMap);

	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorStartHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	float m_fBulletEffectiveRange = 2000.0f;
	CBulletObject* pBulletObject = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	GameObjectMgr* m_pPlayer = NULL;

public:
	CShadowMapShader* m_pShadowShader = NULL;

	CDepthRenderShader* m_pDepthRenderShader = NULL;
	TreeBlendingShader* m_pTreeBlendShadowShader = NULL;

	int count = 0;
	BoundingBox						m_xmBoundingBox;
public:
	float								m_fElapsedTime = 0.0f;

	int									m_nPlayerObjects = 0;
	GameObjectMgr** m_ppPlayerObjects = NULL;

	int									m_nHierarchicalGameObjects = 0;
	GameObjectMgr** m_ppHierarchicalGameObjects = NULL;

	XMFLOAT3							m_xmf3RotatePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	int									m_nShaders = 0;
	ObjectStore** m_ppShaders = NULL;

	int									m_nHumanShaders = 0;
	ShaderMgr** m_ppHumanShaders = NULL;

	int									m_nShadowShaders = 0;
	ObjectStore** m_ppShadowShaders = NULL;


	ShaderMgr* m_pShader = NULL;
	BulletEffectShader* m_pBulletEffect = NULL;

	int									m_nMapShaders = 0;
	int									m_nStageMapShaders = 0;
	int									m_nFragShaders = 0;

	CFragmentsShader** m_ppFragShaders = NULL;

	COpeningBackScene* m_pSkyBox = NULL;
	CHeightMapTerrain* m_pTerrain = NULL;
	int									m_nBillboardShaders = 0;
	BillboardShader** m_pBillboardShader = NULL;
	int									m_nLights = 0;
	//LIGHT* m_pLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
	float							m_fLightRotationAngle = 0.0f;

	LIGHTS* m_pLights = NULL;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;

};
