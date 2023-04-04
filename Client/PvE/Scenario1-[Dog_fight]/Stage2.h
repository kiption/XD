#pragma once
#include "Scene.h"

class CStage2 :public SceneManager
{
public:
	CStage2();
	~CStage2();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;


	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();


	bool ProcessInput(UCHAR* pKeysBuffer);
	virtual void AnimateObjects(CCamera* pCamera, float fTimeElapsed);
	virtual void AnimateObjects(float fTimeElapsed) {}
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void OnPreRender(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, HANDLE hFenceEvent);
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void ReleaseUploadBuffers();

	void RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void RenderSprite(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void OnPostRenderParticle();

public:


	int									m_nGameObjects = 0;
	CGameObject** m_ppGameObjects = NULL;

	int									m_nShaders = 0;
	int									m_nCShaders = 0;
	int									m_nOtherPlayers = 0;

	int									m_nSpriteShaders = 0;
	CSpriteObjectShader** m_ppSpriteShaders = NULL;
	CObjectsShader** m_ppShaders = NULL;
	CNPCShader** m_ppNPCShaders = NULL;
	CExplosionShader** m_ppExplosion = NULL;

	CMultiSpriteObject** m_ppSpriteObjects = NULL;
	int	m_nSpriteObjects;

	CPlayerShader* m_pPlayerShader = NULL;
	CParticleObject** m_ppParticleObjects = NULL;
	int	m_nParticleObjects;

	GameSound gamesound;

	CDynamicCubeMappingShader** m_ppEnvironmentMappingShaders = NULL;
	int							m_nEnvironmentMappingShaders = 0;
	CSkyBox* m_pSkyBox = NULL;
	CHeightMapTerrain* m_pTerrain = NULL;
	COutlineShader* m_pOutlineShader = NULL;
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights = 0;
	LIGHT* m_pLights = NULL;
	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	ID3D12Resource* m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;
	MATERIALS* m_pMaterials = NULL;

	CUseWaterMoveTerrain* m_pUseWaterMove = NULL;
	CMultiSpriteObject* m_pCMultiSpriteObject = NULL;

	CPlayer* m_pPlayer = NULL;


	bool m_bWarMode = false;
	bool m_bOutlineMode = false;
};