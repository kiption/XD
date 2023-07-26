#pragma once
#include "Player.h"
#include "MissileObject.h"
#include "ShadowShader.h"
#include "ObjcetsShaderList.h"
#include "GameSound.h"


class HeliPlayer : public CPlayer
{
public:
	HeliPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,CGameObject* model, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~HeliPlayer();

public:
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;
	CGameObject* m_pFrameFragObj1 = NULL;
	CGameObject* m_pFrameFragObj2 = NULL;
	CGameObject* m_pFrameFragObj3 = NULL;
	CGameObject* m_pFrameFragObj4 = NULL;
	CGameObject* m_pFrameFragObj5 = NULL;
	CGameObject* m_pFrameFragObj6 = NULL;
	CGameObject* m_pFrameFragObj7 = NULL;
	CGameObject* m_pFrameFragObj8 = NULL;
	CGameObject* m_pFrameFragObj9 = NULL;
	CGameObject* m_pFrameFragObj10 = NULL;
	CGameObject* m_pFrameFragObj11 = NULL;

	BoundingOrientedBox ParticleFrame1;
	BoundingOrientedBox ParticleFrame2;
	BoundingOrientedBox ParticleFrame3;
	BoundingOrientedBox ParticleFrame4;
	BoundingOrientedBox ParticleFrame5;
	BoundingOrientedBox ParticleFrame6;
	BoundingOrientedBox ParticleFrame7;
	BoundingOrientedBox ParticleFrame8;
	BoundingOrientedBox ParticleFrame9;
	BoundingOrientedBox ParticleFrame10;
	BoundingOrientedBox ParticleFrame11;
	BoundingOrientedBox ParticleFrame12;


	XMFLOAT3 ParticlePosition{};
	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_HELICOPTER];
	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 25.0f;
	float						m_fExplosionSpeed = 1.0f;
	float						m_fExplosionRotation = 5.0f;
	XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_HELICOPTER];
	CGameObject* pGameObject = NULL;

public:
	XMFLOAT4X4 m_pMainRotorFramePos ;
	XMFLOAT4X4 m_pTailRotorFramePos;
	XMFLOAT4X4 m_pFrameFragObj1Pos;
	XMFLOAT4X4 m_pFrameFragObj2Pos;
	XMFLOAT4X4 m_pFrameFragObj3Pos;
	XMFLOAT4X4 m_pFrameFragObj4Pos;
	XMFLOAT4X4 m_pFrameFragObj5Pos;
	XMFLOAT4X4 m_pFrameFragObj6Pos;
	XMFLOAT4X4 m_pFrameFragObj7Pos;
	XMFLOAT4X4 m_pFrameFragObj8Pos;
	XMFLOAT4X4 m_pFrameFragObj9Pos;
	XMFLOAT4X4 m_pFrameFragObj10Pos;
	XMFLOAT4X4 m_pFrameFragObj11Pos;
	XMFLOAT4X4 m_pResetCameraPos;
	void Resetpartition();
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
	void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, XMFLOAT3 slideVec);
	//virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapse, XMFLOAT4X4* pxmf4x4Parent);
	virtual void Update(float fTimeElapsed);
	void FallDown(float fTimeElapsed);
	float m_fShotDelay = 0.f;
	BoundingOrientedBox m_xoobb = BoundingOrientedBox(XMFLOAT3(),XMFLOAT3(),XMFLOAT4());
	void LimitAltitude();
	bool m_bDieState = false;
private:
	GameSound gamesound;
};