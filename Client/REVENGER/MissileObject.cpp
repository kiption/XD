#include "stdafx.h"
#include "MissileObject.h"

CBulletObject::CBulletObject(float fEffectiveRange) : GameObjectMgr()
{
	m_fBulletEffectiveRange = fEffectiveRange;
}
CBulletObject::~CBulletObject()
{
}
void CBulletObject::SetCatridgePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}
void CBulletObject::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}
void CBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, m_fRotationSpeed * fElapsedTime, 0.0f);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
	m_fMovingDistance += fDistance;
	GameObjectMgr::Animate(fElapsedTime);
	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();
}

void CBulletObject::SetChild(GameObjectMgr* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

///////////////////////////////////////////////////////////////////////////////////


CtridgeObject::CtridgeObject(float fEffectiveRange) : GameObjectMgr(10)
{
	m_fBulletEffectiveRange = fEffectiveRange;
}
CtridgeObject::~CtridgeObject()
{
}
void CtridgeObject::SetCatridgePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}
void CtridgeObject::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}
void CtridgeObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;



		float fDistance = m_fMovingSpeed * fElapsedTime;
		XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, m_fRotationSpeed * fElapsedTime, 0.0f);
		m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
		XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
		XMFLOAT3 xmf3Position = GetPosition();
		xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
		SetPosition(xmf3Position);
		m_fMovingDistance += fDistance;
		GameObjectMgr::Animate(fElapsedTime);
	

	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime))
	{
		Reset(); 
	}
}

void CtridgeObject::SetChild(GameObjectMgr* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}


///////////////////////////////////////////////////////////////////////////////////


CNPCbulletObject::CNPCbulletObject(float fEffectiveRange) : GameObjectMgr()
{
	m_fBulletEffectiveRange = fEffectiveRange;
}
CNPCbulletObject::~CNPCbulletObject()
{
}
void CNPCbulletObject::SetCatridgePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}
void CNPCbulletObject::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}
void CNPCbulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;



	float fDistance = m_fMovingSpeed * fElapsedTime;
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, m_fRotationSpeed * fElapsedTime, 0.0f);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
	m_fMovingDistance += fDistance;
	GameObjectMgr::Animate(fElapsedTime);


	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime))
	{
		Reset();
	}
}

void CNPCbulletObject::SetChild(GameObjectMgr* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}