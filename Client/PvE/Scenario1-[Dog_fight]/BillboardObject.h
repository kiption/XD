#pragma once
#include "Object.h"

class CBillboardObject : public CGameObject
{
public:
	CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBillboardObject();

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent=NULL);

	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;
	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;
};

class CMultiSpriteObject : public CGameObject
{
public:
	CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CMultiSpriteObject();
	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;
	float m_fSpeed = 0.1f;
	float m_fTime = 0.1f;
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};