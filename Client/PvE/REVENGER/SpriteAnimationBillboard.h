#pragma once
#include "Shader.h"
#include "BillboardObjectsShader.h"
class SceneManager;
class CSpriteTexturedShader : public CShader
{
public:
	CSpriteTexturedShader() {};
	virtual ~CSpriteTexturedShader() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
};

class CSpriteObjectsShader : public CSpriteTexturedShader
{
public:
	CSpriteObjectsShader() {};
	virtual ~CSpriteObjectsShader() {};

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);

public:

	XMFLOAT3 ExplosionPosition = XMFLOAT3(0.0, 0.0, 0.0);
	ID3D12Resource* m_pd3dcbGameObjects = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObjects = NULL;
};
class SpriteAnimationBillboard : public CSpriteObjectsShader
{
public:
	SpriteAnimationBillboard() {};
	virtual ~SpriteAnimationBillboard() {};
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed);
};

