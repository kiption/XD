#pragma once
#include "Player.h"
#include "MissileObject.h"
#include "ShadowShader.h"
#include "ObjcetsShaderList.h"
#include "GameSound.h"


class HeliPlayer : public PlayerMgr
{
public:
	HeliPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,GameObjectMgr* model, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~HeliPlayer();

public:
	GameObjectMgr* m_FrameTopRotor = NULL;
	GameObjectMgr* m_FrameTailRotor = NULL;
	GameObjectMgr* m_FrameHeliglass = NULL;
	GameObjectMgr* m_FrameCleanse = NULL;
	GameObjectMgr* m_FrameLefttyre = NULL;
	GameObjectMgr* m_FrameCleanser_2 = NULL;
	GameObjectMgr* m_FrameHeliBody = NULL;
	GameObjectMgr* m_FrameRightDoor = NULL;
	GameObjectMgr* m_FrameBackDoor = NULL;
	GameObjectMgr* m_FrameLeftDoor = NULL;
	GameObjectMgr* m_FrameRighttyre = NULL;
	GameObjectMgr* m_FrameBacktyre = NULL;
	GameObjectMgr* m_pFrameFragObj11 = NULL;
	GameObjectMgr* m_pChairPoint = NULL;

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
	GameObjectMgr* pGameObject = NULL;

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
	BulletEffectShader* pBCBulletEffectShader = NULL;
	CtridgeObject* pBulletObject = NULL;
	CtridgeObject* m_ppBullets[BULLETS];
	void Firevalkan(GameObjectMgr* pLockedObject);
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
	bool m_FallSwitch = false;
private:
	GameSound gamesound;
};