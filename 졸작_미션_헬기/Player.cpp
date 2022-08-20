//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::setTerrain(LPVOID pPlayerUpdatedContext)
{
	m_pPlayerUpdatedContext = pPlayerUpdatedContext;
}



void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0.0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance * 0.15f);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance * 0.15f);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance * 0.15f);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance * 0.15f);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance * 0.15f);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance * 0.055f);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{

		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{

		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
	{
	
	
		
		if (STOPZONE == false) {

			if (x != 0.0f)
			{
				XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
				m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
				m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			}
		
		}
		if (STOPZONE == true)
		{

			m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		}
		
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		
			
		}
		m_pCamera->Rotate(x, y, z);
		
	}
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
}

void CHelicopterPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	/*지형에서 플레이어의 현재 위치 (x, z)의 지형 높이(y)를 구한다. 그리고 플레이어 메쉬의 높이가 12이고 플레이어의
	중심이 직육면체의 가운데이므로 y 값에 메쉬의 높이의 절반을 더하면 플레이어의 위치가 된다.*/

	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 10.0f;
	/*플레이어의 위치 벡터의 y-값이 음수이면(예를 들어, 중력이 적용되는 경우) 플레이어의 위치 벡터의 y-값이 점점
	작아지게 된다. 이때 플레이어의 현재 위치 벡터의 y 값이 지형의 높이(실제로 지형의 높이 + 6)보다 작으면 플레이어
	의 일부가 지형 아래에 있게 된다. 이러한 경우를 방지하려면 플레이어의 속도 벡터의 y 값을 0으로 만들고 플레이어
	의 위치 벡터의 y-값을 지형의 높이(실제로 지형의 높이 + 6)로 설정한다. 그러면 플레이어는 항상 지형 위에 있게 된다.*/
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}

}

CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 1.0f, 0.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}	
	else if ((nNewCameraMode == THIRD_PERSON_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	UpdateTransform(NULL);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CHelicopterObject::Render(pd3dCommandList, pCamera);
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CHelicopterPlayer::CHelicopterPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{

	m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);
	
	
	CHelicopterObject* pBulletMesh = CHelicopterObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Hellicopter/Bullet.bin");

	for (int i = 0; i < BULLETS; i++)
	{
		
		pBulletObject = new CBulletObject(m_fBulletEffectiveRange);
		
		pBulletObject->SetChild(pBulletMesh, true);
		pBulletObject->SetRotationSpeed(90.0f);
		pBulletObject->SetMovingSpeed(500.0f);
		pBulletObject->SetScale(1.3, 1.3, 1.3);
		pBulletObject->SetActive(false);
		m_ppBullets[i] = pBulletObject;

	}
	
	CHelicopterObject* pGameObject = CHelicopterObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Hellicopter/Apache.bin");
	SetChild(pGameObject, true);
	pGameObject->Rotate(0.0f, -5.0f, 0.0f);
	pGameObject->SetScale(1.0f, 1.0, 1.0);
	pGameObject->SetPosition(0.0, 0, 0.0);

	OnInitialize();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CHelicopterPlayer::~CHelicopterPlayer()
{
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) delete m_ppBullets[i];
}

void CHelicopterPlayer::OnInitialize()
{
	m_pMainRotorFrame = FindFrame("rotor");
	m_pMainTailRotorFrame = FindFrame("black_m_7");
	m_pSubTailRotorFrame = FindFrame("black_m_6");
	m_pRocketFrame1 = FindFrame("rocket");
	m_pRocketFrame2 = FindFrame("rocket_1");
	m_pRocketFrame3 = FindFrame("rocket_2");
	m_pRocketFrame4 = FindFrame("rocket_3");
	m_pRocketFrame5 = FindFrame("rocket_4");
	m_pRocketFrame6 = FindFrame("rocket_5");
	m_pRocketFrame7 = FindFrame("rocket_6");
	m_pRocketFrame8 = FindFrame("rocket_7");
	m_pRocketFrame9 = FindFrame("rocket_8");

		m_pRocketFrame1->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame2->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame3->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame4->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame5->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame6->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame7->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame8->SetScale(1.0, 1.5, 1.0);
		m_pRocketFrame9->SetScale(1.0, 1.5, 1.0);
}

void CHelicopterPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{

	
	if(m_MissileActive==true)
	{

		m_pRocketFrame1->m_xmf4x4Transform._43 += 5.f;
		if (m_pRocketFrame1->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame1->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame2->m_xmf4x4Transform._43 += 8.f;
		if (m_pRocketFrame2->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame2->m_xmf4x4Transform._43 = 2.f; 

			m_pRocketFrame3->m_xmf4x4Transform._43 += 9.f;
		if (m_pRocketFrame3->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame3->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame4->m_xmf4x4Transform._43 += 7.f;
		if (m_pRocketFrame4->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame4->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame5->m_xmf4x4Transform._43 += 8.f;
		if (m_pRocketFrame5->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame5->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame6->m_xmf4x4Transform._43 += 7.f;
		if (m_pRocketFrame6->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame6->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame7->m_xmf4x4Transform._43 += 6.f;
		if (m_pRocketFrame7->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame7->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame8->m_xmf4x4Transform._43 += 5.f;
		if (m_pRocketFrame8->m_xmf4x4Transform._43 >= m_MissileRange)
			m_pRocketFrame8->m_xmf4x4Transform._43 = 2.f;

		m_pRocketFrame9->m_xmf4x4Transform._43 += 8.f;
		if (m_pRocketFrame9->m_xmf4x4Transform._43 >= m_MissileRange) {
			
			m_MissileActive = false;
			m_pRocketFrame9->m_xmf4x4Transform._43 = 2.f;
		}

	}

	if (m_MissileActive == false) 
	{
		m_pRocketFrame1->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame2->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame3->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame4->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame5->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame6->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame7->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame8->m_xmf4x4Transform._43 = -12.f;
		m_pRocketFrame9->m_xmf4x4Transform._43 = -12.f;
	}

	
	//////////////////////////////////////////////////////////////착륙 모션////////////////////////////////////////////////
	if (this->GetPosition().x > 2500.0 && this->GetPosition().x < 2800.0 &&
		this->GetPosition().z > 1800.0 && this->GetPosition().z < 2100.0 && 
		this->GetPosition().y<200.0) {

	

		STOPZONE = true;
		if (this->GetPosition().y < 300.0) {

			delrot -= 0.003f;
			if (delrot <= 0.0)
				delrot = 0.0f;

			
		}
		
		if (m_pMainRotorFrame)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * delrot) * fTimeElapsed);
			m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
		}
		if (m_pMainTailRotorFrame)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * delrot) * fTimeElapsed);
			m_pMainTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainTailRotorFrame->m_xmf4x4Transform);
		}
		if (m_pSubTailRotorFrame)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationZ(XMConvertToRadians(360.0f * delrot) * fTimeElapsed);
			m_pSubTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pSubTailRotorFrame->m_xmf4x4Transform);
		}

		SetGravity(XMFLOAT3(0.0f, -2.0f, 0.0f));
	}
	else
	{
		STOPZONE = false;
		if (this->GetPosition().y > 10.0) {
			delrot += 0.002f;
			if (delrot >= 2.0)
				delrot = 2.0f;
		}
		if (m_pMainRotorFrame)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
			m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
		}
		if (m_pMainTailRotorFrame)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
			m_pMainTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainTailRotorFrame->m_xmf4x4Transform);
		}
		if (m_pSubTailRotorFrame)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationZ(XMConvertToRadians(360.0f *4.0) * fTimeElapsed);
			m_pSubTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pSubTailRotorFrame->m_xmf4x4Transform);
		}
			SetGravity(XMFLOAT3(0.0f, 2.0f, 0.0f));
		if (this->GetPosition().y > 600.0) {
			SetGravity(XMFLOAT3(0.0f, -2.0f, 0.0f));
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i< BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			
			m_ppBullets[i]->Animate(fTimeElapsed);

		}
	}
	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
	oobb = BoundingOrientedBox(GetPosition(), XMFLOAT3(15.0, 10.0, 30.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
}

void CHelicopterPlayer::OnPrepareRender()
{ 
	CPlayer::OnPrepareRender();
}

void CHelicopterPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CHelicopterObject::Render(pd3dCommandList, pCamera);
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]->m_bActive) {
	
		m_ppBullets[i]->Render(pd3dCommandList, pCamera);
	}
}

	
void CHelicopterPlayer::FireBullet(CHelicopterObject* pLockedObject)
{
	
	
	CBulletObject* pBulletObject = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			m_MissileActive = true;
			pBulletObject = m_ppBullets[i];
			break;
			
		}
	}
	XMFLOAT3 PlayerHelicpoterPosition = GetLook();
	XMFLOAT3 AnermyHelicopterPosition = m_pCamera->GetLookVector();
	XMFLOAT3 AnermyTOPlayerLookVector = Vector3::Normalize(Vector3::Add(PlayerHelicpoterPosition, AnermyHelicopterPosition));
	
	if (pBulletObject)
	{
		
		XMFLOAT3 xmf3Position  = GetPosition();
		
		XMFLOAT3 xmf3Direction = AnermyTOPlayerLookVector;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, true));
		pBulletObject->m_xmf4x4World = m_xmf4x4Transform;

		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetActive(true);
	}
}

CCamera* CHelicopterPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:

		SetFriction(2.0f);
		SetGravity(XMFLOAT3(0.0f, -1.0f, 0.0f));
		SetMaxVelocityXZ(2.5f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(-5.0f,  0.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(100.5f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(40.0f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(100.5f);
		SetGravity(XMFLOAT3(0.0f, 2.0f, 0.0f));
		SetMaxVelocityXZ(25.5f);
		SetMaxVelocityY(20.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.05f);
		m_pCamera->SetOffset(XMFLOAT3(-5.0f, 15.0f, -120.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 6000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

