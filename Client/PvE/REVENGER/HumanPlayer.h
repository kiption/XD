#pragma once
#include "Player.h"
#include "MissileObject.h"

class CHumanPlayer : public CPlayer
{
public:
	CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CHumanPlayer();
	CLoadedModelInfo* pAngrybotModel = NULL;
	CAngrybotObject* pSoldiarObject = NULL;
	CGameObject* m_pBulletFindFrame{ NULL };
public:
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

	virtual void Update(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
	float m_fBulletEffectiveRange = 2000.0f;
	CValkanObject* pBulletObject = NULL;
	CValkanObject* m_ppBullets[BULLETS];
	void FireBullet(CGameObject* pLockedObject);
};