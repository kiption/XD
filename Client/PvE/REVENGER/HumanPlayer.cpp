#include "stdafx.h"
#include "HumanPlayer.h"

CHumanPlayer::CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	pSoldiarModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/MODEL.bin", NULL);
	SetChild(pSoldiarModel->m_pModelRootObject, true);
	Rotate(0.0, 90.0, 0.0);
	pSoldiarModel->m_pModelRootObject->SetCurScene(SCENE1STAGE);
	//Weapon_R
	m_pBulletFindFrame = pSoldiarModel->m_pModelRootObject->FindFrame("Bip001_L_Finger21");
	m_pHeadFindFrame = pSoldiarModel->m_pModelRootObject->FindFrame("SK_Soldier_Head_LOD1");

	//7°³
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 7, pSoldiarModel); 

	m_pSkinnedAnimationController->SetTrackAnimationSet(0,0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1,1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2,2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3,3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(4,4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(5,5);
	m_pSkinnedAnimationController->SetTrackAnimationSet(6,6);

	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
	m_pSkinnedAnimationController->SetCallbackKeys(2, 1);
	m_pSkinnedAnimationController->SetCallbackKeys(1, 5);
	m_pSkinnedAnimationController->SetCallbackKeys(5, 1);
	m_pSkinnedAnimationController->SetCallbackKeys(1, 6);
	m_pSkinnedAnimationController->SetCallbackKeys(6, 1);

#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.1f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.2f, _T("Sound/Footstep02.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0, 0.8f, _T("Sound/Shooting.wav"));
#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	

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

	//SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);


	m_xoobb = BoundingOrientedBox(XMFLOAT3(this->GetPosition()), XMFLOAT3(15.0, 18.0, 13.0), XMFLOAT4(0, 0, 0, 1));

	if (pSoldiarModel) delete pSoldiarModel;

}

CHumanPlayer::~CHumanPlayer()
{
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) delete m_ppBullets[i];
}

CCamera* CHumanPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{

	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(400.0);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(20.0f);
		SetMaxVelocityY(0.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.5f, -2.2f));
		m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_pHeadFindFrame->GetPosition().x, m_pHeadFindFrame->GetPosition().y+1.0f, m_pHeadFindFrame->GetPosition().z), m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 6000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(0.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(15.0f);
		SetMaxVelocityY(5.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 1.0f, 3.0f));
		m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_pHeadFindFrame->GetPosition()), m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 6000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(500);
		SetGravity(XMFLOAT3(0.0f, 0.0, 0.0f));
		SetMaxVelocityXZ(15.0f);
		SetMaxVelocityY(5.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 18.0f, -25.0f));
		m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z), m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 6000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CHumanPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
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

void CHumanPlayer::OnCameraUpdateCallback(float fTimeElapsed)
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
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera* p3rdPersonCamera = (CThirdPersonCamera*)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
		if (m_pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			CSpaceShipCamera* p2rdPersonCamera = (CSpaceShipCamera*)m_pCamera;
			p2rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

void CHumanPlayer::JumpState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent)
{
	
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, true);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 6);
	
	CPlayer::Animate(EleapsedTime, pxmf4x4Parent);
}

void CHumanPlayer::RollState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent)
{
	
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, true);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 5);

	
	CPlayer::Animate(EleapsedTime, pxmf4x4Parent);
}

void CHumanPlayer::ShootState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent)
{
	
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, true);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	CPlayer::Animate(EleapsedTime, pxmf4x4Parent);
}

void CHumanPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection& DIR_FORWARD || dwDirection & DIR_BACKWARD)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
	}
	if (dwDirection & DIR_LEFT || dwDirection & DIR_RIGHT)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, true);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
	}
	
	if (dwDirection & DIR_UP)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, true);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
	}
	if (dwDirection & DIR_DOWN)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, true);
	}
	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}



void CHumanPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	for (int i = 0; i < BULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive) {
			
			m_ppBullets[i]->Animate(fTimeElapsed);
		
		}
	}
	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
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
			m_pSkinnedAnimationController->SetTrackEnable(3, false);
			m_pSkinnedAnimationController->SetTrackEnable(4, false);
			m_pSkinnedAnimationController->SetTrackEnable(5, false);
			m_pSkinnedAnimationController->SetTrackEnable(6, false);
			m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
		}
	}
	
}

void CHumanPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer::Render(pd3dCommandList, pCamera);
	if (pBCBulletEffectShader) pBCBulletEffectShader->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < BULLETS; i++) 
		if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->Render(pd3dCommandList, pCamera); }
}

void CHumanPlayer::FireBullet(CGameObject* pLockedObject)
{

	gamesound.shootingSound();
	CValkanObject* pBulletObject = NULL;
	for (int i = 0; i < BULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			gamesound.~GameSound();
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

		XMFLOAT3 PlayerLook = GetLookVector();
		XMFLOAT3 CameraLook = m_pCamera->GetLookVector();
		XMFLOAT3 CaemraPosition = m_pCamera->GetPosition();
		XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerLook, CameraLook));
		XMFLOAT3 xmf3Position = m_pBulletFindFrame->GetPosition();
		XMFLOAT3 xmf3Direction = CameraLook;

		pBulletObject->m_xmf4x4ToParent = m_xmf4x4World;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 10.0f, false));
		
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->Rotate(90.0, 0.0, 0.0);
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y, xmf3FirePosition.z));
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(0.2, 0.2, 1.0);
		pBulletObject->SetActive(true);
	
	}

}

