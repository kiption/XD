#include "stdafx.h"
#include "SoldiarPlayerObject.h"

SoldiarPlayerObject::SoldiarPlayerObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* playermodel)
{
}

SoldiarPlayerObject::~SoldiarPlayerObject()
{
}

CCamera* SoldiarPlayerObject::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	return nullptr;
}

void SoldiarPlayerObject::OnPlayerUpdateCallback(float fTimeElapsed)
{
}

void SoldiarPlayerObject::OnCameraUpdateCallback(float fTimeElapsed)
{
}

void SoldiarPlayerObject::Move(ULONG nDirection, float fDistance, bool bVelocity)
{
}

void SoldiarPlayerObject::ReloadState()
{
}

void SoldiarPlayerObject::ShootState(float EleapsedTime, XMFLOAT4X4* pxmf4x4Parent)
{
}

void SoldiarPlayerObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
}

void SoldiarPlayerObject::Update(float fTimeElapsed)
{
}

void SoldiarPlayerObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void SoldiarPlayerObject::FireBullet(CGameObject* pLockedObject)
{
}
