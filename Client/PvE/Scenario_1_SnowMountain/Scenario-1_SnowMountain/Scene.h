//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"


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

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseUploadBuffers();
public:
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);
	
public:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;

	int									m_nGameObjects = 0;
	CGameObject							**m_ppGameObjects = NULL;

	int									m_nShaders = 0;
	int									m_nCShaders = 0;
	int									m_nOtherPlayers = 0;
	CObjectsShader** m_ppShaders = NULL;
	CObjectsShader* m_pOShader = NULL;
	CShader** m_ppCShaders = NULL;
	CShader* m_pShader = NULL;
	COtherPlayerShader* m_pOtherPlayerShader = NULL;


	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;
	
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights = 0;
	LIGHT								*m_pLights = NULL;
	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;


	CUseWaterMoveTerrain				*m_pUseWaterMove = NULL;
	CSnowObject							*m_pSnowObject = NULL;
	CExplosionObject					*m_pBillboard = NULL;

	XMFLOAT4X4							m_xmf4x4WaterAnimation;
	int CollisionCount;
	bool m_bEndAttackMotion = false;
	bool m_bInWaterProof = false;

	void WarMode();
	bool m_bWarMode=false;
	CPlayer								*m_pPlayer = NULL;
};
