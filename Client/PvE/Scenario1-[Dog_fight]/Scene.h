//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "BillboardShader.h"
#include "ParticleShader.h"
#include "OutlineShader.h"
#include "ObjectShader.h"
#include "DynamicMappingShader.h"

#include "ParticleObejct.h"
#include "SkyboxObject.h"
#include "TerrainObject.h"
#include "BillboardObject.h"
#include "Player.h"
#include "MainPlayer.h"
#include "GameSound.h"


#define MAX_LIGHTS			16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
	int						m_nLights;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};


class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;


	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();


	bool ProcessInput(UCHAR *pKeysBuffer);
    virtual void AnimateObjects(CCamera* pCamera,float fTimeElapsed);
	virtual void AnimateObjects(float fTimeElapsed) {}
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);
	void OnPreRender(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, HANDLE hFenceEvent);
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void ReleaseUploadBuffers();

	void RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void RenderSprite(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void OnPostRenderParticle();
	
public:
	

	int									m_nGameObjects = 0;
	CGameObject							**m_ppGameObjects = NULL;

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

	CPlayerShader* m_pPlayerShader= NULL;
	CParticleObject** m_ppParticleObjects = NULL;
	int	m_nParticleObjects;

	GameSound gamesound;

	CDynamicCubeMappingShader** m_ppEnvironmentMappingShaders = NULL;
	int							m_nEnvironmentMappingShaders = 0;
	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;
	COutlineShader* m_pOutlineShader = NULL;
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights = 0;
	LIGHT								*m_pLights = NULL;
	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	ID3D12Resource* m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;
	MATERIALS* m_pMaterials = NULL;

	CUseWaterMoveTerrain				*m_pUseWaterMove = NULL;
	CMultiSpriteObject *m_pCMultiSpriteObject = NULL;

	CPlayer								*m_pPlayer = NULL;

	void WarMode();
	bool m_bWarMode = false;
	int CollisionCheck;
	int CollisionEND = 0;
	bool m_bOutlineMode = false;
};
