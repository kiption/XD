#pragma once
#include "SkinAnimationShader.h"
#include "Terrain.h"
#include "ParticleObject.h"

class CHellicopterObjectsShader : public CStandardObjectsShader
{
public:
	CHellicopterObjectsShader();
	virtual ~CHellicopterObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

class CHumanObjectsShader : public CSkinnedAnimationObjectsShader
{
public:
	CHumanObjectsShader();
	virtual ~CHumanObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

class CAngrybotObjectsShader : public CSkinnedAnimationObjectsShader
{
public:
	CAngrybotObjectsShader();
	virtual ~CAngrybotObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};


class CBulletEffectShader : public CStandardObjectsShader
{
public:
	CBulletEffectShader();
	virtual ~CBulletEffectShader();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
};

class CFragmentsShader : public CStandardShader
{
public:
	CFragmentsShader() {};
	virtual ~CFragmentsShader() {};

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();
	XMFLOAT3 ParticlePosition{};
	bool m_bActive = false;

	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_DEBRISES];

	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 20.0f;
	float						m_fExplosionSpeed = 10.0f;
	float						m_fExplosionRotation = 720.0f;
	XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
};

class CHelicopterBulletMarkParticleShader : public CFragmentsShader
{
public:
	CHelicopterBulletMarkParticleShader() {};
	virtual ~CHelicopterBulletMarkParticleShader() {};

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();
	XMFLOAT3 ParticlePosition{};
	bool m_bActive = false;

	XMFLOAT4X4					m_pxmf4x4Transforms[BLOODEXPLOSION_DEBRISES];

	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 0.6f;
	float						m_fExplosionSpeed = 3.0f;
	float						m_fExplosionRotation = 720.0f;
	XMFLOAT3 m_pxmf3SphereVectors[BLOODEXPLOSION_DEBRISES];
};