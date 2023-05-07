#pragma once
#include "Object.h"
class SceneManager;
class CBillboardObject : public CGameObject
{
public:
	CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBillboardObject();

	virtual void Animate(float fTimeElapsed);

	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;
	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;
};
class CResponeObject : public CGameObject
{
public:
	CResponeObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CResponeObject();
	XMFLOAT3 VEC{};
	virtual void Animate(float fTimeElapsed);
	bool m_bResponeAnimation = false;
	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;
	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;


};
class CBillboardParticleObject : public CGameObject
{
public:
	CBillboardParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBillboardParticleObject();

	virtual void Animate(float fTimeElapsed);

	float m_fRotationAngle = 0.0f;
	float m_fRotationDelta = 1.0f;
	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;
};
class CMultiSpriteObject : public CGameObject
{
public:
	CMultiSpriteObject(/*ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature*/);
	virtual ~CMultiSpriteObject();
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	float m_fSpeed = 0.1f;
	float m_fTime = 0.0f;
};