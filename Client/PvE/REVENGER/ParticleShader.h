#pragma once
#include "StandardShader.h"

class ParticleShader : public BillboardShader
{
public:
	ParticleShader();
	virtual ~ParticleShader();
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(int nPipelineState);
	virtual UINT GetNumRenderTargets(int nPipelineState);
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget);
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOuputState(int nPipelineState);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState);
};

