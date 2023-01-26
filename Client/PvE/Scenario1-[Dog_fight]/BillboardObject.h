#pragma once
#include "Object.h"

class CBillboardObject : public CGameObject
{
public:
	CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBillboardObject();

	virtual void Animate(float fTimeElapsed);

	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;
};