#include "stdafx.h"
#include "MainPlayer.h"


CMainPlayer::CMainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pShader = new CPlayerShader();
	m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
	m_pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 4 + 1 + 1);

	CGameObject* pGameObject = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GO.bin", m_pShader);
	SetChild(pGameObject);
	pGameObject->SetScale(1.0, 1.0, 1.0);

	CGameObject* pBulletMesh = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mi24.bin", m_pShader);
	for (int i = 0; i < BULLETS; i++)
	{
		pBulletObject = new CBulletObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh, true);
		pBulletObject->SetMovingSpeed(8000.0f);
		pBulletObject->SetActive(false);
		m_ppBullets[i] = pBulletObject;
	}

	CGameObject* pBulletMesh2 = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mi24.bin", m_pShader);
	for (int i = 0; i < BULLETS; i++)
	{
		pBulletObject2 = new CBulletObject(m_fBulletEffectiveRange);
		pBulletObject2->SetChild(pBulletMesh2, true);
		pBulletObject2->SetMovingSpeed(8000.0f);
		pBulletObject2->SetActive(false);
		m_ppBullets2[i] = pBulletObject2;
	}

	PrepareAnimate();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CMainPlayer::~CMainPlayer()
{
}

void CMainPlayer::PrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("military_helicopter_blades");
	m_pTailRotorFrame = FindFrame("TailRotor");
}

void CMainPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{

	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {

			m_ppBullets[i]->Rotate(0.0, 0.0, 50.0f);
			m_ppBullets[i]->Animate(fTimeElapsed);
		}

		if (m_ppBullets2[i]->m_bActive) {

			m_ppBullets2[i]->Rotate(0.0, 0.0, 50.0f);
			m_ppBullets2[i]->Animate(fTimeElapsed);
		}
	}
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}

	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}
	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
}

void CMainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;

	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 8.0f;
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}

}



void CMainPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

void CMainPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer::Render(pd3dCommandList, pCamera);
	for (int i = 0; i < BULLETS; i++) {
		if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->Render(pd3dCommandList, pCamera); }
		if (m_ppBullets2[i]->m_bActive) { m_ppBullets2[i]->Render(pd3dCommandList, pCamera); }
	}
}
void CMainPlayer::FireBullet(CGameObject* pLockedObject)
{

	if (pLockedObject)
	{
		SetLookAt(pLockedObject->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
		UpdateTransform();
	}


	CBulletObject* pBulletObject = NULL;
	CBulletObject* pBulletObject2 = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			pBulletObject->Reset();
			break;
		}

	}

	for (int i = 0; i < BULLETS2; i++)
	{
		if (!m_ppBullets2[i]->m_bActive)
		{
			pBulletObject2 = m_ppBullets2[i];
			pBulletObject2->Reset();
			break;
		}
	}
	XMFLOAT3 PlayerLook = this->GetLookVector();
	XMFLOAT3 CameraLook = m_pCamera->GetLookVector();
	XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerLook, CameraLook));

	if (pBulletObject)
	{

		XMFLOAT3 xmf3Position = this->GetPosition();
		xmf3Position.x -= 10.0f;
		XMFLOAT3 xmf3Direction = TotalLookVector;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 100.0f, false));

		pBulletObject->m_xmf4x4Transform = m_xmf4x4World;
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y + .0, xmf3FirePosition.z));
		pBulletObject->SetScale(1.0, 1.0, 1.5);
		pBulletObject->SetActive(true);
	}

	if (pBulletObject2)
	{

		XMFLOAT3 xmf3Position = this->GetPosition();
		xmf3Position.x += 10.0f;
		XMFLOAT3 xmf3Direction = TotalLookVector;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 100.0f, false));

		pBulletObject2->m_xmf4x4Transform = m_xmf4x4World;
		pBulletObject2->SetMovingDirection(xmf3Direction);
		pBulletObject2->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y + .0, xmf3FirePosition.z));
		pBulletObject2->SetScale(1.0, 1.0, 1.5);
		pBulletObject2->SetActive(true);
	}
}

CCamera* CMainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(2.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(2.5f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.1f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(0.25f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(25.5f);
		SetMaxVelocityY(20.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 15.4f, 15.5f));
		m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 70.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(0.5f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(25.5f);
		SetMaxVelocityY(20.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 8.0f, -40.0f));
		m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 8000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}