#pragma once
#include "Object.h"

class CBulletObject : public GameObjectMgr
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	virtual void Animate(float fElapsedTime);
	virtual void SetChild(GameObjectMgr* pChild, bool bReferenceUpdate = false);
	float						m_fBulletEffectiveRange = 1500.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 5.0f;
	GameObjectMgr* m_pLockedObject = NULL;
	CCamera* m_pCamera = NULL;
	PlayerMgr* m_pPlayer = NULL;
	void SetCatridgePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};

class CtridgeObject : public GameObjectMgr
{
public:
	CtridgeObject(float fEffectiveRange);
	virtual ~CtridgeObject();

public:
	virtual void Animate(float fElapsedTime);
	virtual void SetChild(GameObjectMgr* pChild, bool bReferenceUpdate = false);
	float						m_fBulletEffectiveRange = 1500.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 5.0f;
	GameObjectMgr* m_pLockedObject = NULL;
	CCamera* m_pCamera = NULL;
	PlayerMgr* m_pPlayer = NULL;
	float m_fShotDelay = 0.f;
	void SetCatridgePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};

class CNPCbulletObject : public GameObjectMgr
{
public:
	CNPCbulletObject(float fEffectiveRange);
	virtual ~CNPCbulletObject();

public:
	virtual void Animate(float fElapsedTime);
	virtual void SetChild(GameObjectMgr* pChild, bool bReferenceUpdate = false);
	float						m_fBulletEffectiveRange = 1500.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 5.0f;
	GameObjectMgr* m_pLockedObject = NULL;
	CCamera* m_pCamera = NULL;
	PlayerMgr* m_pPlayer = NULL;
	float m_fShotDelay = 0.f;
	void SetCatridgePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};