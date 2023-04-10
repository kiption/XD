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
		Move(XMFLOAT3(0.0f, m_fRotationAngle, 0.0));
	

	CGameObject::Animate(fTimeElapsed);
}

CBillboardParticleObject::CBillboardParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CBillboardParticleObject::~CBillboardParticleObject()
{
}

void CBillboardParticleObject::Animate(float fTimeElapsed)
{
	/*random_device rd;
	uniform_int_distribution<int>dre(1, 5);
	XMFLOAT3 xf_Position;
	xf_Position.x = m_xmf4x4ToParent._41;
	xf_Position.y = m_xmf4x4ToParent._42;
	xf_Position.z= m_xmf4x4ToParent._43;
	XMFLOAT3 xf_Velocity= XMFLOAT3(0.01,0.0,1.0);
	XMFLOAT3 xf_GravityAccel =XMFLOAT3(0.0,9.8,0.0);
	float f_EmmitTime={};
	float a_LifeTime={};
	float f_ResetTime = {};
	float NewY{};

	XMFLOAT4 newPosition = XMFLOAT4(0, 0, 0, 1);
	float Time = fTimeElapsed - f_EmmitTime;
	if (Time < 0.0)
	{

	}
	else
	{
		NewY = a_LifeTime * XMScalarModAngle(Time / a_LifeTime);
		newPosition.x = xf_Position.x + xf_Velocity.x * Time + 0.5 * xf_GravityAccel.x * Time * Time;
		newPosition.y = xf_Position.y + xf_Velocity.y * Time + 0.5 * xf_GravityAccel.y * Time * Time;
		newPosition.z = xf_Position.z + xf_Velocity.z * Time + 0.5 * xf_GravityAccel.z * Time * Time;

		m_xmf4x4ToParent._41 = newPosition.x;
		m_xmf4x4ToParent._42 = newPosition.y;
		m_xmf4x4ToParent._43 = newPosition.z;
	}*/
	CGameObject::Animate(fTimeElapsed);
}
