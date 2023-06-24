#pragma once
#include "Shader.h"
class BoundingWireShader : public CShader
{
public:
	BoundingWireShader();
	virtual ~BoundingWireShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelinestates);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelinestates);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelinestates);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		int nPipelineState);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera,int nPipelinestates);
};

