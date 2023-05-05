#pragma once
#include "Player.h"
#include "MissileObject.h"
#include "ShadowShader.h"
#include "ObjcetsShaderList.h"
#include "GameSound.h"


class HeliPlayer : public CPlayer
{
public:
	HeliPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~HeliPlayer();
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTail2RotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;
	CGameObject* pGameObject = NULL;
public:
	float m_fBulletEffectiveRange = 2000.0f;
	CBulletEffectShader* pBCBulletEffectShader = NULL;
	CValkanObject* pBulletObject = NULL;
	CValkanObject* m_ppBullets[BULLETS];
	void Firevalkan(CGameObject* pLockedObject);
public:
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPrepareAnimate();
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	//virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapse, XMFLOAT4X4* pxmf4x4Parent);
	virtual void Update(float fTimeElapsed);

	BoundingOrientedBox m_xoobb = BoundingOrientedBox(XMFLOAT3(),XMFLOAT3(),XMFLOAT4());

private:
	GameSound gamesound;
};