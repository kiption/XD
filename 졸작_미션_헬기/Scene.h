//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"
#include <random>
#include <array>
#include <utility>

#define MAX_LIGHTS			16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

static std::random_device rd;
static std::default_random_engine dre(rd());

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
	void ReleaseUploadBuffers();
	void CheckObjectByBulletCollisions();
	void CheckObjectByPlayerCollisions();
	void PickingToPlayer();
	CPlayer						*m_pPlayer = NULL;
	CBulletObject* m_pBullet = NULL;
	CHelicopterPlayer* m_HeliPlayer = NULL;
public:
	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

	CHelicopterObject**				m_ppGameObjects = NULL;
	int							m_nGameObjects = 6;

	LIGHT						*m_pLights = NULL;
	int							m_nLights = 0;
	bool						pickingSight = false;
	bool						FightMode = false;
	XMFLOAT4					m_xmf4GlobalAmbient;

	ID3D12Resource				*m_pd3dcbLights = NULL;
	LIGHTS						*m_pcbMappedLights = NULL;
	CCamera* m_pCamera=NULL;
	float						m_fElapsedTime = 0.0f;
	bool isRotate = false;
	bool isAcc = false;
	float sumBoostDis = 0;
	float rotateAngle = 0.0;
	int turn = m_nGameObjects;
	CHeightMapTerrain* m_pTerrain = NULL;
	CHelicopterPlayer* m_pHelicopterPlayer=NULL;
	CBulletObject* pBulletObject = NULL;
	CShader* m_pShader = NULL;

	static XMFLOAT3				m_pxmf3SphereVectors[20];

	inline float RandF(float fMin, float fMax)
	{
		return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
	}
	float dx=0.0;
	float dy=0.0;
	float dz=0.0;
	
	int m_turn=0;
protected:
	CTerrainShader* terrainShader;
	CShader** m_ppShaderObjcet;

};
