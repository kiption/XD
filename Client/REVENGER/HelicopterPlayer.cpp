#include "stdafx.h"
#include "HelicopterPlayer.h"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
HeliPlayer::HeliPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, GameObjectMgr* model, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	pGameObject = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);

	SetChild(pGameObject, false);
	pGameObject->SetScale(1.1, 1.0, 1.2);
	pGameObject->SetCurScene(INGAME_SCENE);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);
	OnPrepareAnimate();

	m_pMainRotorFramePos = m_FrameTopRotor->m_xmf4x4ToParent;
	m_pTailRotorFramePos = m_FrameTailRotor->m_xmf4x4ToParent;
	m_pFrameFragObj1Pos = m_FrameHeliglass->m_xmf4x4ToParent;
	m_pFrameFragObj2Pos = m_FrameCleanse->m_xmf4x4ToParent;
	m_pFrameFragObj3Pos = m_FrameLefttyre->m_xmf4x4ToParent;
	m_pFrameFragObj4Pos = m_FrameCleanser_2->m_xmf4x4ToParent;
	m_pFrameFragObj5Pos = m_FrameHeliBody->m_xmf4x4ToParent;
	m_pFrameFragObj6Pos = m_FrameRightDoor->m_xmf4x4ToParent;
	m_pFrameFragObj7Pos = m_FrameBackDoor->m_xmf4x4ToParent;
	m_pFrameFragObj8Pos = m_FrameLeftDoor->m_xmf4x4ToParent;
	m_pFrameFragObj9Pos = m_FrameRighttyre->m_xmf4x4ToParent;
	m_pFrameFragObj10Pos = m_FrameBacktyre->m_xmf4x4ToParent;


	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

HeliPlayer::~HeliPlayer()
{

}

void HeliPlayer::Resetpartition()
{
	m_FrameTopRotor->m_xmf4x4ToParent = m_pMainRotorFramePos;
	m_FrameTailRotor->m_xmf4x4ToParent = m_pTailRotorFramePos;
	m_FrameHeliglass->m_xmf4x4ToParent = m_pFrameFragObj1Pos;
	m_FrameCleanse->m_xmf4x4ToParent = m_pFrameFragObj2Pos;
	m_FrameLefttyre->m_xmf4x4ToParent = m_pFrameFragObj3Pos;
	m_FrameCleanser_2->m_xmf4x4ToParent = m_pFrameFragObj4Pos;
	m_FrameHeliBody->m_xmf4x4ToParent = m_pFrameFragObj5Pos;
	m_FrameRightDoor->m_xmf4x4ToParent = m_pFrameFragObj6Pos;
	m_FrameBackDoor->m_xmf4x4ToParent = m_pFrameFragObj7Pos;
	m_FrameLeftDoor->m_xmf4x4ToParent = m_pFrameFragObj8Pos;
	m_FrameRighttyre->m_xmf4x4ToParent = m_pFrameFragObj9Pos;
	m_FrameBacktyre->m_xmf4x4ToParent = m_pFrameFragObj10Pos;
	//m_pCamera->m_xmf4x4View = m_pResetCameraPos;

}

void HeliPlayer::Firevalkan(GameObjectMgr* pLockedObject)
{
}

CCamera* HeliPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(300.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(30.0f);
		SetMaxVelocityY(5.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, -80.0f));
		m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z), m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case CLOSEUP_PERSON_CAMERA:
		SetFriction(600.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(80.0f);
		SetMaxVelocityY(15.0f);
		m_pCamera = OnChangeCamera(CLOSEUP_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetOffset(XMFLOAT3(-1.3f, 6.9, 11.0f));
		m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z), m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 80.0f);
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
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.5f, -25.0f));
		m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y - 15.0f, m_xmf3Position.z), m_pCamera->GetOffset()));
		m_pCamera->GenerateProjectionMatrix(1.01f, 8000.0f, ASPECT_RATIO, 80.0f);
		m_pCamera->SetViewport(10, 10, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(4, 4, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	//m_pCamera->SetPosition(Vector3::Add(XMFLOAT3(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z), m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void HeliPlayer::OnPrepareAnimate()
{
	PlayerMgr::OnPrepareAnimate();
	m_FrameTailRotor = FindFrame("rescue_2");
	m_FrameTopRotor = FindFrame("rescue_1");
	m_FrameHeliglass = FindFrame("glass");
	m_FrameCleanse = FindFrame("cleanser");
	m_FrameLefttyre = FindFrame("left_tyre");
	m_FrameCleanser_2 = FindFrame("cleanser_1");
	m_FrameHeliBody = FindFrame("helicopter");
	m_FrameRightDoor = FindFrame("right_door");
	m_FrameBackDoor = FindFrame("back_door");
	m_FrameLeftDoor = FindFrame("left_door");
	m_FrameRighttyre = FindFrame("right_tyre");
	m_FrameBacktyre = FindFrame("back_tyre");
	m_pChairPoint = FindFrame("ChairPoint");
}


void HeliPlayer::Animate(float fTimeElapse, XMFLOAT4X4* pxmf4x4Parent)
{
	m_xoobb = BoundingOrientedBox(m_xmf3Position, { 4, 6, 8 }, { 0, 0, 0, 1 });
	PlayerMgr::Animate(fTimeElapse, pxmf4x4Parent);

	if (m_FrameTopRotor)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 8.0f) * fTimeElapse);
		m_FrameTopRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTopRotor->m_xmf4x4ToParent);
	}
	if (m_FrameTailRotor)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 8.0f) * fTimeElapse);
		m_FrameTailRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTailRotor->m_xmf4x4ToParent);
	}

	if (m_bDieState == true)
	{
		FallDown(fTimeElapse);	
	}

	if (m_bDieState == false)
	{
		SetGravity(XMFLOAT3(0.0, 0.0, 0.0));
	}


	LimitAltitude();

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
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA || m_pCamera->GetMode() == CLOSEUP_PERSON_CAMERA)
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
	PlayerMgr::Render(pd3dCommandList, NULL, pCamera);
}

void HeliPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, XMFLOAT3 slideVec)
{
	PlayerMgr::Move(dwDirection, fDistance, bUpdateVelocity, slideVec);
}


void HeliPlayer::Update(float fTimeElapsed)
{
	PlayerMgr::Update(fTimeElapsed);

}

void HeliPlayer::FallDown(float fTimeElapsed)
{
	m_FallSwitch = true;
	XMFLOAT3 gravity = XMFLOAT3(0.0, -1.5, 0);
	float FallingMaxHeight = -18.5f;
	float staticValue = 5.3f;
	float staticValueZ = 6.3f;
	XMVECTOR staticDir1 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.25, -0.5, 0.25, 0.0)));
	XMVECTOR staticDir2 = XMVector3Normalize(XMVECTOR(XMVectorSet(-0.25, -0.5, 0.35, 0.0)));
	XMVECTOR staticDir3 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.25, -0.5, 0.15, 0.0)));
	XMVECTOR staticDir4 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.15, -0.5, -0.5, 0.0)));
	XMVECTOR staticDir5 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.25, -0.5, -0.35, 0.0)));
	XMVECTOR staticDir6 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.25, -0.5, 0.25, 0.0)));
	XMVECTOR staticDir7 = XMVector3Normalize(XMVECTOR(XMVectorSet(-0.1, -0.5, -0.25, 0.0)));
	for (int i = 0; i < 2; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir1);
	for (int i = 2; i < 4; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir2);
	for (int i = 4; i < 5; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir3);
	for (int i = 5; i < 6; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir4);
	for (int i = 6; i < 7; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir5);
	for (int i = 7; i < 9; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir6);
	for (int i = 9; i < EXPLOSION_HELICOPTER; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir7);
	XMMATRIX xmmtxRotateRow = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
		XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
		XMConvertToRadians(360.0f * 0.4) * fTimeElapsed);

	XMMATRIX xmmtxRotateFast = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(360.0f * 2.8) * fTimeElapsed,
		XMConvertToRadians(360.0f * 2.8) * fTimeElapsed,
		XMConvertToRadians(360.0f * 2.8) * fTimeElapsed);
	if (m_FrameTailRotor->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(360.0f * 1.8) * fTimeElapsed,
			XMConvertToRadians(360.0f * 1.8) * fTimeElapsed,
			XMConvertToRadians(360.0f * 1.8) * fTimeElapsed);
		m_FrameTailRotor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[0].x * staticValue * fTimeElapsed;
		m_FrameTailRotor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[0].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameTailRotor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[0].z * staticValueZ * fTimeElapsed;
		m_FrameTailRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTailRotor->m_xmf4x4ToParent);
	}
	if (m_FrameTopRotor->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed);
		m_FrameTopRotor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[1].x * staticValue * fTimeElapsed;
		m_FrameTopRotor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[1].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameTopRotor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[1].z * staticValueZ * fTimeElapsed;
		m_FrameTopRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTopRotor->m_xmf4x4ToParent);
	}


	m_FrameHeliglass->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[2].x * staticValue * fTimeElapsed;
	m_FrameHeliglass->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[2].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameHeliglass->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[2].z * staticValueZ * fTimeElapsed;
	m_FrameHeliglass->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateRow, m_FrameHeliglass->m_xmf4x4ToParent);


	m_FrameCleanse->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[3].x * staticValue * fTimeElapsed;
	m_FrameCleanse->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[3].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameCleanse->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[3].z * staticValueZ * fTimeElapsed;
	m_FrameCleanse->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateFast, m_FrameCleanse->m_xmf4x4ToParent);


	m_FrameLefttyre->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[4].x * staticValue * fTimeElapsed;
	m_FrameLefttyre->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[4].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameLefttyre->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[4].z * staticValueZ * fTimeElapsed;
	m_FrameLefttyre->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateFast, m_FrameLefttyre->m_xmf4x4ToParent);

	if (m_FrameCleanser_2->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		XMMATRIX xmmtxRotateMid = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(360.0f * 1.8) * fTimeElapsed,
			XMConvertToRadians(360.0f * 1.8) * fTimeElapsed,
			XMConvertToRadians(360.0f * 1.8) * fTimeElapsed);
		m_FrameCleanser_2->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[5].x * staticValue * fTimeElapsed;
		m_FrameCleanser_2->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[5].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameCleanser_2->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[5].z * staticValueZ * fTimeElapsed;
		m_FrameCleanser_2->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateMid, m_FrameCleanser_2->m_xmf4x4ToParent);
	}
	if (m_FrameHeliBody->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed);
		m_FrameHeliBody->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[6].x * staticValue * fTimeElapsed;
		m_FrameHeliBody->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[6].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;		//
		m_FrameHeliBody->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[6].z * staticValueZ * fTimeElapsed;
		m_FrameHeliBody->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameHeliBody->m_xmf4x4ToParent);
	}
	if (m_FrameRightDoor->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(360.0f * 2.8) * fTimeElapsed,
			XMConvertToRadians(360.0f * 2.8) * fTimeElapsed,
			XMConvertToRadians(360.0f * 2.8) * fTimeElapsed);
		m_FrameRightDoor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[7].x * staticValue * fTimeElapsed;
		m_FrameRightDoor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[7].z * staticValueZ * fTimeElapsed;
		m_FrameRightDoor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[7].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameRightDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameRightDoor->m_xmf4x4ToParent);
	}


	m_FrameBackDoor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[8].x * staticValue * fTimeElapsed;
	m_FrameBackDoor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[8].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameBackDoor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[8].z * staticValueZ * fTimeElapsed;
	m_FrameBackDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateRow, m_FrameBackDoor->m_xmf4x4ToParent);

	if (m_FrameLeftDoor->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed,
			XMConvertToRadians(360.0f * 0.4) * fTimeElapsed);
		m_FrameLeftDoor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[9].x * staticValue * fTimeElapsed;
		m_FrameLeftDoor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[9].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameLeftDoor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[9].z * staticValueZ * fTimeElapsed;
		m_FrameLeftDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameLeftDoor->m_xmf4x4ToParent);
	}
	if (m_FrameRighttyre->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		m_FrameRighttyre->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[10].x * staticValue * fTimeElapsed;
		m_FrameRighttyre->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[10].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameRighttyre->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[10].z * staticValueZ * fTimeElapsed;
	}
	if (m_FrameBacktyre->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		m_FrameBacktyre->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[11].x * staticValue * fTimeElapsed;
		m_FrameBacktyre->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[11].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameBacktyre->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[11].z * staticValueZ * fTimeElapsed;
	}
}

void HeliPlayer::LimitAltitude()
{
	if (this->m_xmf4x4ToParent._42>90.0)
	{
		SetPosition(XMFLOAT3(this->GetPosition().x,89.99f, this->GetPosition().z));
	}
}
