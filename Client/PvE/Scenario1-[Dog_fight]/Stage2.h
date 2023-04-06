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
#include "Scene.h"


#define MAX_LIGHTS			16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3


class CStage2 : public SceneManager
{
public:
	CStage2();
	~CStage2();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, ID3D12Resource* pd3dDepthStencilBuffer);
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
	float m_fBulletEffectiveRange = 2000.0f;
	CBulletObject* pBulletObject = NULL;
	CBulletObject* m_ppBullets[BULLETS];
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
	CPlayerShader* m_pShader = NULL;

	CMultiSpriteObject** m_ppSpriteObjects = NULL;
	int	m_nSpriteObjects;


	CParticleObject** m_ppParticleObjects = NULL;
	int	m_nParticleObjects;

	GameSound gamesound;
	int GetCurScene() { return m_nCurScene; }
	void SetCurScene(int nCurScene) { m_nCurScene = nCurScene; }

	bool m_bWarMode = false;
	bool m_bOutlineMode = false;
};

