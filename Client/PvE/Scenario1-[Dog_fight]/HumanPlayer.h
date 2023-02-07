#pragma once

#include "Player.h"
#include "MissileObject.h"
#include "HelicopterObejct.h"
#include "TerrainObject.h"

#define BULLETS					150
#define BULLETS2				150
class CHumanPlayer : public CPlayer
{
public:
	CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CHumanPlayer();

	float						m_fBulletEffectiveRange = 2000.0f;
	CBulletObject* pBulletObject = NULL;
	CBulletObject* pBulletObject2 = NULL;
	CMi24Object* pPlayerObject = NULL;
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;
	CGameObject* pHumanModel = NULL;
	
private:
	virtual void PrepareAnimate();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);

public:

	CBulletObject* m_ppBullets[BULLETS];
	CBulletObject* m_ppBullets2[BULLETS2];
	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	virtual void Update(float fTimeElapsed);

	
	void FireBullet(CGameObject* pLockedObject);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

