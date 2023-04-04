#include "stdafx.h"
#include "HumanPlayer.h"


CHumanPlayer::CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext )
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pShader = new CPlayerShader();
	m_pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
	m_pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 4 + 1 + 1);

	pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Soldier_demo.bin", m_pShader);
	SetChild(pAngrybotModel->m_pModelRootObject,true);
	pAngrybotModel->m_pModelRootObject->SetScale(14.0, 14.0, 14.0);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 3, pAngrybotModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);

	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
	m_pSkinnedAnimationController->SetCallbackKeys(2, 1);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.8f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 1.2f, _T("Sound/Footstep02.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0, 0.8f, _T("Sound/Footstep02.wav"));
#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CLoadedModelInfo* pBulletMesh = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", m_pShader);
	for (int i = 0; i < BULLETS; i++)
	{
		pBulletObject = new CBulletObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh->m_pModelRootObject, true);
		pBulletObject->SetMovingSpeed(7000.0f);
		pBulletObject->SetActive(false);
		pBulletObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 0, pBulletMesh);
		m_ppBullets[i] = pBulletObject;
	}

	CLoadedModelInfo* pBulletMesh2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", m_pShader);
	for (int i = 0; i < BULLETS2; i++)
	{
		pBulletObject2 = new CBulletObject(m_fBulletEffectiveRange);
		pBulletObject2->SetChild(pBulletMesh2->m_pModelRootObject, true);
		pBulletObject2->SetMovingSpeed(7000.0f);
		pBulletObject2->SetActive(false);
		pBulletObject2->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 0, pBulletMesh2);
		m_ppBullets2[i] = pBulletObject2;
	}

	PrepareAnimate();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);


	if (pBulletMesh) delete pBulletMesh;
	if (pBulletMesh2) delete pBulletMesh2;
	if (pAngrybotModel) delete pAngrybotModel;
}

CHumanPlayer::~CHumanPlayer()
{
}

void CHumanPlayer::PrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("military_helicopter_blades");
	m_pTailRotorFrame = FindFrame("TailRotor");
}

void CHumanPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{

	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {


			m_ppBullets[i]->Animate(fTimeElapsed);
		}

		if (m_ppBullets2[i]->m_bActive) {


			m_ppBullets2[i]->Animate(fTimeElapsed);
		}
	}
	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
}

void CHumanPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
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



void CHumanPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

void CHumanPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer::Render(pd3dCommandList, pCamera);
	for (int i = 0; i < BULLETS; i++) {
		if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->Render(pd3dCommandList, pCamera); }
		if (m_ppBullets2[i]->m_bActive) { m_ppBullets2[i]->Render(pd3dCommandList, pCamera); }
	}
}
void CHumanPlayer::Move(ULONG nDirection, float fDistance, bool bVelocity)
{
	if (nDirection)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
	}

	CPlayer::Move(nDirection, fDistance, bVelocity);
}
void CHumanPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (::IsZero(fLength))
		{
			m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_pSkinnedAnimationController->SetTrackEnable(1, false);
			m_pSkinnedAnimationController->SetTrackEnable(2, false);
			m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
		}
	}
}
void CHumanPlayer::FireBullet(CGameObject* pLockedObject)
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
		xmf3Position.x -= 3.0f;
		XMFLOAT3 xmf3Direction = PlayerLook;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 20.0f, false));

		pBulletObject->m_xmf4x4Transform = m_xmf4x4World;
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y + .0, xmf3FirePosition.z));
		pBulletObject->SetScale(25.0, 25.0, 40.5);
		pBulletObject->Rotate(90.0, 0.0, 0.0);
		pBulletObject->SetActive(true);
	}

	if (pBulletObject2)
	{

		XMFLOAT3 xmf3Position = this->GetPosition();
		xmf3Position.x += 3.0f;
		XMFLOAT3 xmf3Direction = PlayerLook;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 20.0f, false));

		pBulletObject2->m_xmf4x4Transform = m_xmf4x4World;
		pBulletObject2->SetMovingDirection(xmf3Direction);
		pBulletObject2->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y + .0, xmf3FirePosition.z));
		pBulletObject2->SetScale(125.0, 125.0, 140.5);
		pBulletObject2->Rotate(90.0, 0.0, 0.0);
		pBulletObject2->SetActive(true);
	}
}

CCamera* CHumanPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
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
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.2f);
		m_pCamera->SetOffset(XMFLOAT3(5.0f, 8.0f, -40.0f));
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