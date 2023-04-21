#include "stdafx.h"
#include "HelicopterPlayer.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
HeliPlayer::HeliPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CGameObject* pGameObject = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
	SetChild(pGameObject, false);
	pGameObject->SetScale(1.0, 1.0, 1.0);
	pGameObject->SetCurScene(SCENE1STAGE);
	CLoadedModelInfo* pBulletMesh = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", NULL);
	for (int i = 0; i < BULLETS; i++)
	{
		pBulletObject = new CBulletObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh->m_pModelRootObject, true);
		pBulletObject->SetMovingSpeed(4000.0f);
		pBulletObject->SetActive(false);
		pBulletObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 0, pBulletMesh);
		m_ppBullets[i] = pBulletObject;
		m_ppBullets[i]->SetCurScene(SCENE1STAGE);
	}

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);
	OnPrepareAnimate();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	SetPosition(XMFLOAT3(310.0f, 590.0f, 590.0f));
	SetScale(XMFLOAT3(1.2, 1.2, 1.2));

	m_xoobb = BoundingOrientedBox(XMFLOAT3(this->GetPosition()), XMFLOAT3(15.0, 13.0, 20.0), XMFLOAT4(0, 0, 0, 1));

	if (pBulletMesh) delete pBulletMesh;
}

HeliPlayer::~HeliPlayer()
{
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) delete m_ppBullets[i];
}

void HeliPlayer::Firevalkan(CGameObject* pLockedObject)
{
	CBulletObject* pBulletObject = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			pBulletObject->Reset();
			break;
		}
	}
	XMFLOAT3 PlayerLook = this->GetLookVector();
	XMFLOAT3 CameraLook = m_pCamera->GetLookVector();
	XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerLook, CameraLook));
	if (pBulletObject)
	{

		XMFLOAT3 xmf3Position = this->GetPosition();
		XMFLOAT3 xmf3Direction = PlayerLook;
		xmf3Direction.y -= 0.1f;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 0.0f, false));

		pBulletObject->m_xmf4x4ToParent = m_xmf4x4World;
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y, xmf3FirePosition.z));
		pBulletObject->Rotate(130.0, 0.0, 0.0);
		pBulletObject->SetScale(15.0, 15.0, 25.0);
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
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f,2.6f, 0.5f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 1.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, -27.0f));
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
	m_pTailRotorFrame = FindFrame("rescue_2");
	m_pMainRotorFrame = FindFrame("rescue_1");

	CPlayer::OnPrepareAnimate();
}
void HeliPlayer::Animate(float fTimeElapsed)
{
	/*for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {

			m_ppBullets[i]->Animate(fTimeElapsed);
		}
	}*/
	CPlayer::Animate(fTimeElapsed);
}

void HeliPlayer::Animate(float fTimeElapse, XMFLOAT4X4* pxmf4x4Parent)
{
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
	CPlayer::Animate(fTimeElapse, pxmf4x4Parent);
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
	if(m_nCurScene!=OPENINGSCENE) CPlayer::Render(pd3dCommandList, pCamera);

}

void HeliPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}


void HeliPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

}
