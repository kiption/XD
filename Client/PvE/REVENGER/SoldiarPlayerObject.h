#pragma once
#include "Player.h"
#include "MissileObject.h"
class SoldiarPlayerObject : public CPlayer
{
public:
	SoldiarPlayerObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* playermodel);
	virtual ~SoldiarPlayerObject();
	CGameObject* m_pHeadFindFrame{ NULL };
public:
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void Move(ULONG nDirection, float fDistance, bool bVelocity, XMFLOAT3 slideVec);
	void ReloadState();
	virtual void ShotState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent = NULL);
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
	float m_fShotDelay = 0.0f;
};

