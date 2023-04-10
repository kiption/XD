#pragma once
#include "ObjcetsShaderList.h"
#include "BillboardObjects.h"
#include "MissileObject.h"

class SceneManager;

class CrossHairShader : public BillboardShader
{
public:
	CrossHairShader() {};
	virtual ~CrossHairShader() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();

#ifdef _WITH_BATCH_MATERIAL
	CMaterial* m_ppGrassMaterials[2] = { NULL, NULL };
	CMaterial* m_ppFlowerMaterials[2] = { NULL, NULL };
#endif
};

class BillboardParticleShader : public BillboardShader
{
public:
	BillboardParticleShader() {};
	virtual ~BillboardParticleShader() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();

	float m_fWidth = 0.0f;
	float m_fHeight = 0.0f;
};



class OpeningBillboardBanner : public BillboardShader
{
public:
	OpeningBillboardBanner() {};
	virtual ~OpeningBillboardBanner() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();

	float m_fWidth = 0.0f;
	float m_fHeight = 0.0f;
};