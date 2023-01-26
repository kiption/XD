#pragma once
#include "Object.h"

class CBulletObject : public CGameObject
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	virtual void Animate(float fElapsedTime);
	virtual void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);
	float						m_fBulletEffectiveRange = 1500.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 5.0f;
	CGameObject* m_pLockedObject = NULL;
	CCamera* m_pCamera = NULL;
	CPlayer* m_pPlayer = NULL;
	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();

};

