#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object.h"
#include "Camera.h"

class CPlayer : public CHelicopterObject
{
protected:
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;

	XMFLOAT3					m_xmf3Look = XMFLOAT3(0,0,1);
	XMFLOAT3					m_xmf3Velocity;
	XMFLOAT3     				m_xmf3Gravity;
	float           			m_fMaxVelocityXZ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

	LPVOID						m_pCameraUpdatedContext;
	CHelicopterObject* m_pHO = NULL;
public:

	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;
	CCamera						*m_pCamera = NULL;
	LPVOID						m_pPlayerUpdatedContext;
	XMFLOAT3					m_xmf3Position;

	
	CPlayer();
	virtual ~CPlayer();
	virtual XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	virtual XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	virtual XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	virtual XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	

	void setTerrain(LPVOID pPlayerUpdatedContext);
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }
	
	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void Update(float fTimeElapsed);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) {  }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	bool							STOPZONE = false;
	bool							m_gunbarrelControl = false;
public:
	CHelicopterObject* m_pMainRotorFrame = NULL;
	CHelicopterObject* m_pRocketFrame1 = NULL;
	CHelicopterObject* m_pRocketFrame2 = NULL;
	CHelicopterObject* m_pRocketFrame3 = NULL;
	CHelicopterObject* m_pRocketFrame4 = NULL;
	CHelicopterObject* m_pRocketFrame5 = NULL;
	CHelicopterObject* m_pRocketFrame6 = NULL;
	CHelicopterObject* m_pRocketFrame7 = NULL;
	CHelicopterObject* m_pRocketFrame8 = NULL;
	CHelicopterObject* m_pRocketFrame9 = NULL;
	CHelicopterObject* m_pRocketFrame10 = NULL;
	CHelicopterObject* m_pMainTailRotorFrame = NULL;
	CHelicopterObject* m_pSubTailRotorFrame = NULL;

	CBulletObject* pBulletObject = NULL;

	float m_MissileRange = 1500.0f;
	// Missile ON/OFF
	bool m_MissileActive;
};

#define BULLETS					50

class CHelicopterPlayer : public CPlayer
{
public:
	CHelicopterPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CHelicopterPlayer();

public:
	float							m_fBulletEffectiveRange = 500.0f;
	CHelicopterObject*				m_pPlayerObejct = NULL;
	CBulletObject* pBulletObject = NULL;
	float delrot = 2.0; 
	int m_MissileCount=0;
	

private:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();

public:
	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(CHelicopterObject* pLockedObject);
	void HellFire();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};


