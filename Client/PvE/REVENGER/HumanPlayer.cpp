#include "stdafx.h"
#include "HumanPlayer.h"
#include "ShadowShader.h"
CHumanPlayer::CHumanPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* playermodel, void* pContext)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	SetChild(playermodel->m_pModelRootObject, true);
	playermodel->m_pModelRootObject->SetCurScene(SCENE1STAGE);
	SetScale(XMFLOAT3(7, 7, 7));
	m_pBulletFindFrame = playermodel->m_pModelRootObject->FindFrame("Rifle__1_");
	m_pHeadFindFrame = playermodel->m_pModelRootObject->FindFrame("head");

	//7°³
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 9, playermodel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
	m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
	m_pSkinnedAnimationController->SetTrackAnimationSet(7, 7);
	m_pSkinnedAnimationController->SetTrackAnimationSet(8, 8);


	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);

	m_pSkinnedAnimationController->SetTrackSpeed(6, 0.5f);

	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
	m_pSkinnedAnimationController->SetCallbackKeys(2, 2);
	m_pSkinnedAnimationController->SetCallbackKeys(3, 2);
	m_pSkinnedAnimationController->SetCallbackKeys(4, 2);


#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(3, 0, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.0f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.4f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0, 0.0f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 1, 0.4f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(3, 0, 0.0f, _T("Sound/Footstep03.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(3, 1, 0.4f, _T("Sound/Footstep03.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(4, 0, 0.0f, _T("Sound/Footstep03.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(4, 1, 0.4f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);



	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);





}

CHumanPlayer::~CHumanPlayer()
{
	
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
		SetMaxVelocityXZ(30.0f);
		SetMaxVelocityY(0.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 7.0f, 0.0f));
		m_pCamera->SetPosition(Vector3::Add(
			XMFLOAT3(m_pHeadFindFrame->GetPosition().x, m_pHeadFindFrame->GetPosition().y, m_pHeadFindFrame->GetPosition().z),
			m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 6000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case CLOSEUP_PERSON_CAMERA:
		SetFriction(600);
		SetGravity(XMFLOAT3(0.0f, -3.f, 0.0f));
		SetMaxVelocityXZ(32.0);
		SetMaxVelocityY(0.0f);
		m_pCamera = OnChangeCamera(CLOSEUP_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(-0.6f, 0.16f, 0.24f));
		m_pCamera->SetPosition(Vector3::Add(
			XMFLOAT3(m_pHeadFindFrame->GetPosition().x, m_pHeadFindFrame->GetPosition().y, m_pHeadFindFrame->GetPosition().z)
			, m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 4500.0f, ASPECT_RATIO, 70.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(600);
		SetGravity(XMFLOAT3(0.0f, -3.f, 0.0f));
		SetMaxVelocityXZ(40.0);
		SetMaxVelocityY(0.0);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 8.0f, -12.0f));
		//m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z), m_pCamera->GetOffset()));
		m_pCamera->SetPosition(m_xmf3Position);
		m_pCamera->GenerateProjectionMatrix(1.01f, 4500.0f, ASPECT_RATIO, 70.0f);
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
	float fHeight = 6.15f + 2.0f;
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
		if (m_pCamera->GetMode() == CLOSEUP_PERSON_CAMERA)
		{
			CSpaceShipCamera* p2rdPersonCamera = (CSpaceShipCamera*)m_pCamera;
			p2rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

void CHumanPlayer::DieState()
{
	if (m_bDieState == true)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
		m_pSkinnedAnimationController->SetTrackEnable(7, false);
		m_pSkinnedAnimationController->SetTrackEnable(8, true);
		m_pSkinnedAnimationController->SetTrackAnimationSet(0, 8);
		m_pSkinnedAnimationController->SetTrackPosition(8, 0.0f);
	}

}

void CHumanPlayer::ReloadState()
{
	if (m_bReloadState == true)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, true);
		m_pSkinnedAnimationController->SetTrackAnimationSet(0, 5);
		m_pSkinnedAnimationController->SetTrackSpeed(5,1.8f);
		m_pSkinnedAnimationController->SetTrackPosition(5, 0.0f);
	}
}

void CHumanPlayer::ShotState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, true);;
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 6);

	CPlayer::Animate(EleapsedTime, pxmf4x4Parent);
}

void CHumanPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, XMFLOAT3 slideVec)
{
	m_bReloadState = false;
	m_bDieState = false;
	m_bMoveUpdate = true;
	m_pSkinnedAnimationController->m_pAnimationTracks->m_nType = ANIMATION_TYPE_LOOP;
	if (dwDirection & DIR_FORWARD)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
		m_pSkinnedAnimationController->SetTrackEnable(7, false);
		m_pSkinnedAnimationController->SetTrackEnable(8, false);
	}
	if (dwDirection & DIR_BACKWARD)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, true);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
		m_pSkinnedAnimationController->SetTrackEnable(7, false);
		m_pSkinnedAnimationController->SetTrackEnable(8, false);
	}
	if (dwDirection & DIR_LEFT)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, true);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
		m_pSkinnedAnimationController->SetTrackEnable(7, false);
		m_pSkinnedAnimationController->SetTrackEnable(8, false);
	}
	if (dwDirection & DIR_RIGHT)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, true);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
		m_pSkinnedAnimationController->SetTrackEnable(7, false);

	}
	if (dwDirection & DIR_DOWN)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);
		m_pSkinnedAnimationController->SetTrackEnable(6, false);
		m_pSkinnedAnimationController->SetTrackEnable(7, false);
		m_pSkinnedAnimationController->SetTrackEnable(8, false);
	}
	if (DIR_UP)
	{

	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity, slideVec);
}



void CHumanPlayer::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{

	if (m_bReloadState == true)
		m_pSkinnedAnimationController->m_pAnimationTracks->m_nType = ANIMATION_TYPE_ONCE;
	if (m_bReloadState == false && m_bDieState!=true)
	{
		m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
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
			m_bMoveUpdate = false;
			m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_pSkinnedAnimationController->SetTrackEnable(1, false);
			m_pSkinnedAnimationController->SetTrackEnable(2, false);
			m_pSkinnedAnimationController->SetTrackEnable(3, false);
			m_pSkinnedAnimationController->SetTrackEnable(4, false);
			m_pSkinnedAnimationController->SetTrackEnable(5, false);
			m_pSkinnedAnimationController->SetTrackEnable(6, false);
			m_pSkinnedAnimationController->SetTrackEnable(7, false);
			m_pSkinnedAnimationController->SetTrackEnable(8, false);
			m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
		}
	}

}

void CHumanPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CShader* pShader, CCamera* pCamera)
{
	if (m_bZoomMode == false)
	{
		CPlayer::Render(pd3dCommandList, pShader, pCamera);

		if (pBCBulletEffectShader) pBCBulletEffectShader->Render(pd3dCommandList, pCamera, false);
		for (int i = 0; i < BULLETS; i++)
			if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->ShadowRender(pd3dCommandList, pCamera, true, pShader); }
	}
}

void CHumanPlayer::FireBullet(CGameObject* pLockedObject)
{
}

