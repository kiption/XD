#include "stdafx.h"
#include "ShadowShader.h"


CIlluminatedShader::CIlluminatedShader()
{
}

CIlluminatedShader::~CIlluminatedShader()
{
}

D3D12_INPUT_LAYOUT_DESC CIlluminatedShader::CreateInputLayout(int nPipelineState)
{
	return D3D12_INPUT_LAYOUT_DESC();
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return D3D12_SHADER_BYTECODE();
}

void CIlluminatedShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
}

CShadowInObjectsShader::CShadowInObjectsShader()
{
}

CShadowInObjectsShader::~CShadowInObjectsShader()
{
}

void CShadowInObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CShadowInObjectsShader::AnimateObjects(float fTimeElapsed)
{
}

void CShadowInObjectsShader::ReleaseObjects()
{
}

void CShadowInObjectsShader::ReleaseUploadBuffers()
{
}

void CShadowInObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

void CShadowInObjectsShader::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

BoundingBox CShadowInObjectsShader::CalculateBoundingBox()
{
	return BoundingBox();
}



CDepthRenderShader::~CDepthRenderShader()
{
}

D3D12_DEPTH_STENCIL_DESC CDepthRenderShader::CreateDepthStencilState(int nPipelineState)
{
	return D3D12_DEPTH_STENCIL_DESC();
}

D3D12_RASTERIZER_DESC CDepthRenderShader::CreateRasterizerState(int nPipelineState)
{
	return D3D12_RASTERIZER_DESC();
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return D3D12_SHADER_BYTECODE();
}

void CDepthRenderShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CDepthRenderShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CDepthRenderShader::ReleaseShaderVariables()
{
}

void CDepthRenderShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CDepthRenderShader::ReleaseObjects()
{
}

void CDepthRenderShader::PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CDepthRenderShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void CDepthRenderShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
}

CShadowMapShader::CShadowMapShader(CShadowInObjectsShader* pObjectsShader)
{
}

CShadowMapShader::~CShadowMapShader()
{
}

D3D12_DEPTH_STENCIL_DESC CShadowMapShader::CreateDepthStencilState(int nPipelineState)
{
	return D3D12_DEPTH_STENCIL_DESC();
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return D3D12_SHADER_BYTECODE();
}

void CShadowMapShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShadowMapShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShadowMapShader::ReleaseShaderVariables()
{
}

void CShadowMapShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CShadowMapShader::ReleaseObjects()
{
}

void CShadowMapShader::ReleaseUploadBuffers()
{
}

void CShadowMapShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
}

void CShadowMapShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
}
