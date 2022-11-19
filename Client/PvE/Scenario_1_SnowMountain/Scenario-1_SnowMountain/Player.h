#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object.h"
#include "Camera.h"
#include "Shader.h"
class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;
	XMFLOAT3					m_xmf3Look;



	XMFLOAT3					m_xmf3Velocity;
	XMFLOAT3     				m_xmf3Gravity;
	float           			m_fMaxVelocityXZ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

	LPVOID						m_pPlayerUpdatedContext;
	LPVOID						m_pPlayerUpdatedContext2;
	LPVOID						m_pCameraUpdatedContext;

	CObjectsShader*m_pShader = NULL;
	CPlayerShader*m_pPShader = NULL;
	CObjectsShader**m_ppShaders = NULL;

public:
	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;
	CCamera						*m_pCamera = NULL;
	CPlayer();
	virtual ~CPlayer();
	bool test = false;
	XMFLOAT3				m_xmf3Position;
	XMFLOAT3 GetPosition() { return(m_xmf3Position);}
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetVectorsByServer(const XMFLOAT3& xmf3right, const XMFLOAT3& xmf3up, const XMFLOAT3& xmf3look) {m_xmf3Right = xmf3right;m_xmf3Up = xmf3up;m_xmf3Look = xmf3look;}

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

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	virtual void WaterOnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	void SetWPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext2 = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	void SetTerrain(LPVOID pPlayerUpdatedContext);
	void SetWaterSurface(LPVOID pPlayerUpdatedContext);
public:
	float m_MissileRange = 1500.0f;
	// Missile ON/OFF
	bool m_MissileActive;
	bool m_ZoomInActive = false;
	bool m_bStopZone = false;
};

#define BULLETS					50
class CMainPlayer : public CPlayer
{
public:
	CMainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CMainPlayer();

	float						m_fBulletEffectiveRange = 1800.0f;
	CMi24Object					*pPlayerObject = NULL;
	CBulletObject				*pBulletObject = NULL;
	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;
	CGameObject					*m_pBulletFrame = NULL;
	CUseWaterMoveTerrain* m_pWaterTerrain = NULL;

private:
	virtual void PrepareAnimate();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
	void StopZoneAnimate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void WaterOnPlayerUpdateCallback(float fTimeElapsed);
public:

	CBulletObject* m_ppBullets[BULLETS];
	void FireBullet(CGameObject* pLockedObject);
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);


	float m_fdelrot = 1.0;
	BoundingOrientedBox xoobb = BoundingOrientedBox(GetPosition(), XMFLOAT3(15.0, 10.0, 30.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
};


