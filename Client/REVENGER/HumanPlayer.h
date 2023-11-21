#pragma once
#include "Player.h"
#include "MissileObject.h"
#include "ObjcetsShaderList.h"
class SceneMgr;
class CShadowMapShader;
class CHumanPlayer : public PlayerMgr
{
public:
	CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,CLoadedModelInfo* playermodel, void* pContext = NULL);
	virtual ~CHumanPlayer();
	CLoadedModelInfo* pSoldiarModel = NULL;
	GameObjectMgr* pSoldiarObject = NULL;
	GameObjectMgr* m_pBulletFindFrame{ NULL };
	GameObjectMgr* m_pHeadFindFrame{ NULL };
	XMFLOAT4X4 m_pResetCameraPos;
	bool m_bDieState = false;
public:
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	BulletEffectShader* pBCBulletEffectShader = NULL;
	
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, XMFLOAT3 slideVec);
	void DyingMotion();
	void ReloadState();
	virtual void ShotState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	virtual void Update(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList,ShaderMgr* pShader, CCamera* pCamera);
	void ResetCamera();
	bool m_bMoveUpdate = false;
	bool m_bReloadState = false;
	bool m_bZoomMode = false;
public:
	float m_fBulletEffectiveRange = 1600.0f;
	CBulletObject* pBulletObject = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(GameObjectMgr* pLockedObject);
	float m_fShotDelay = 0.0f;
};
