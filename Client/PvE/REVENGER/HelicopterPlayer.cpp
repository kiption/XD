#include "stdafx.h"
#include "HelicopterPlayer.h"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
HeliPlayer::HeliPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	pGameObject = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);

	SetChild(pGameObject, false);
	pGameObject->SetScale(1.0, 1.0, 1.2);
	pGameObject->SetCurScene(SCENE1STAGE);


	pBCBulletEffectShader = new CBulletEffectShader();
	pBCBulletEffectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pBCBulletEffectShader->SetCurScene(SCENE1STAGE);

	for (int i = 0; i < BULLETS; i++)
	{

		CGameObject* pBulletMesh = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", pBCBulletEffectShader);
		pBulletObject = new CValkanObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh, false);
		pBulletObject->SetMovingSpeed(1000.0f);
		pBulletObject->SetActive(false);
		pBulletObject->SetCurScene(SCENE1STAGE);
		m_ppBullets[i] = pBulletObject;
		pBulletMesh->AddRef();
	}

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);
	OnPrepareAnimate();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	SetPosition(XMFLOAT3(140.f, 60.f, -270.f));
	m_xoobb = BoundingOrientedBox(XMFLOAT3(this->GetPosition()), XMFLOAT3(15.0, 13.0, 20.0), XMFLOAT4(0, 0, 0, 1));
}

HeliPlayer::~HeliPlayer()
{
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) delete m_ppBullets[i];
}

void HeliPlayer::Firevalkan(CGameObject* pLockedObject)
{
	gamesound.shootingSound();
	CValkanObject* pBulletObject = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			pBulletObject->Reset();
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 PlayerLook = GetLookVector();
		XMFLOAT3 CameraLook = m_pCamera->GetLookVector();
		XMFLOAT3 CaemraPosition = m_pCamera->GetPosition();
		XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerLook, CameraLook));
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Direction = TotalLookVector;

		pBulletObject->m_xmf4x4ToParent = m_xmf4x4World;
		if (m_pCamera->m_nMode == SPACESHIP_CAMERA)
		{
			xmf3Direction.y += 0.1f;
			XMFLOAT3 xmf3FirePosition = Vector3::Add(CaemraPosition, Vector3::ScalarProduct(xmf3Direction, 60.0f, false));
			pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
			pBulletObject->Rotate(80.0, 0.0, 0.0);
		}
		else
		{
			XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 0.0f, false));
			pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
			pBulletObject->Rotate(90.0, 0.0, 0.0);
		}
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(4.0, 10.0, 4.0);
		pBulletObject->SetActive(true);

	}
}

CCamera* HeliPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(5.0f);
		SetGravity(XMFLOAT3(0.0f, -0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 1.5, 2.6f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 4000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(5.0f, 5.0f, -20.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 8000.0f, ASPECT_RATIO, 80.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z), m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void HeliPlayer::OnPrepareAnimate()
{
	CPlayer::OnPrepareAnimate();
	m_pTailRotorFrame = FindFrame("rescue_2");
	m_pMainRotorFrame = FindFrame("rescue_1");
	m_pTail2RotorFrame = FindFrame("NULL");

}


void HeliPlayer::Animate(float fTimeElapse, XMFLOAT4X4* pxmf4x4Parent)
{
	CPlayer::Animate(fTimeElapse, pxmf4x4Parent);
	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive)
		{
			m_ppBullets[i]->Animate(fTimeElapse);

		}
	}
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 10.0f) * fTimeElapse);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 10.0f) * fTimeElapse);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}
	//if (m_pTail2RotorFrame)
	//{
	//	XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 15.0f) * fTimeElapse);
	//	m_pTail2RotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTail2RotorFrame->m_xmf4x4ToParent);
	//}
	if (m_xmf3Position.y > 200.0f)
	{
		//SetGravity(XMFLOAT3(0.0f, -20.0f, 0.0f));
	}
	if (m_xmf3Position.y < 10.0)
	{
		//SetGravity(XMFLOAT3(0.0f, -0.0f, 0.0f));
		m_bCollisionTerrain = true;
	}
	else
	{
		m_bCollisionTerrain = false;
	}

}

void HeliPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

void HeliPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA || m_pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			CThirdPersonCamera* p3rdPersonCamera = (CThirdPersonCamera*)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());

			CSpaceShipCamera* p2rdPersonCamera = (CSpaceShipCamera*)m_pCamera;
			p2rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}


void HeliPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer::Render(pd3dCommandList, pCamera);

	if (pBCBulletEffectShader) pBCBulletEffectShader->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < BULLETS; i++)if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->Render(pd3dCommandList, pCamera); }


}

void HeliPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}


void HeliPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

}
