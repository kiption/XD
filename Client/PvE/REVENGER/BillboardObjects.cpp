#include "stdafx.h"
#include "BillboardObjects.h"
#include "Shader.h"

CBillboardObject::CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed)
{
	if (m_fRotationAngle <= -1.5f) m_fRotationDelta = 1.0f;
	if (m_fRotationAngle >= +1.5f) m_fRotationDelta = -1.0f;
	m_fRotationAngle += m_fRotationDelta * fTimeElapsed;

	Rotate(0.0f, 0.0f, m_fRotationAngle);

	CGameObject::Animate(fTimeElapsed);
}

CRainObject::CRainObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CRainObject::~CRainObject()
{
}

void CRainObject::Animate(float fTimeElapsed)
{
	m_xmf4x4ToParent._42 -= 40.0f;
	CGameObject::Animate(fTimeElapsed);
}
