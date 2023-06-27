#pragma once
#include "Player.h"
#include "MissileObject.h"
#include "ObjcetsShaderList.h"
class SceneManager;
class CShadowMapShader;
class CHumanPlayer : public CPlayer
{
public:
	CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,CLoadedModelInfo* playermodel, void* pContext = NULL);
	virtual ~CHumanPlayer();
	CLoadedModelInfo* pSoldiarModel = NULL;
	CGameObject* pSoldiarObject = NULL;
	CGameObject* m_pBulletFindFrame{ NULL };
	CGameObject* m_pHeadFindFrame{ NULL };
public:
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	CBulletEffectShader* pBCBulletEffectShader = NULL;
	
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void JumpState();
	void ReloadState();
	virtual void ShootState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	virtual void Update(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	
	bool m_bReloadState = false;
	bool m_bJumeState = false;

public:
	float m_fBulletEffectiveRange = 1600.0f;
	CBulletObject* pBulletObject = NULL;
	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(CGameObject* pLockedObject);
	float m_fShootDelay = 0.0f;
};
