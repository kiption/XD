#pragma once
#include "Shader.h"

class SceneMgr;
class PostProcessShader: public ShaderMgr
{
public:
	PostProcessShader();
	virtual ~PostProcessShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineStates);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineStates);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineStates);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineStates);

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState);

	virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	virtual void CreateResourcesAndRtvsSrvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, UINT nShaderResources);

	virtual void OnPrepareRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle);
	virtual void OnPostRenderTarget(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext, int nPipelineStates);

protected:
	Texture* m_pTexture = NULL;
	Texture** m_ppTextures = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE* m_pd3dRtvCPUDescriptorHandles = NULL;

public:
	Texture* GetTexture() { return(m_pTexture); }
	ID3D12Resource* GetTextureResource(UINT nIndex) { return(m_pTexture->GetResource(nIndex)); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCPUDescriptorHandle(UINT nIndex) { return(m_pd3dRtvCPUDescriptorHandles[nIndex]); }

};


struct PS_CB_DRAW_OPTIONS
{
	XMINT4							m_xmn4DrawOptions;
};

class CTextureToFullScreenShader : public PostProcessShader
{
public:
	CTextureToFullScreenShader();
	virtual ~CTextureToFullScreenShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineStates);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineStates);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void ReleaseShaderVariables();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext, int nPipelineStates);

protected:
	ID3D12Resource* m_pd3dcbDrawOptions = NULL;
	PS_CB_DRAW_OPTIONS* m_pcbMappedDrawOptions = NULL;
};

///<summary> 화면 우상단에 그림자 뎁스 맵 디버그 표시 (전용 루트시그/힙 사용) </summary>
class CShadowMapDebugShader : public ShaderMgr
{
public:
	void CreateDebugResources(ID3D12Device* pd3dDevice);
	void CopyDepthSliceToHeap(ID3D12Device* pd3dDevice, D3D12_CPU_DESCRIPTOR_HANDLE srcCpuHandle);
	ID3D12RootSignature* GetDebugRootSignature() { return m_pd3dDebugRootSignature; }
	ID3D12DescriptorHeap* GetDebugDescriptorHeap() { return m_pd3dDebugSrvHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetDebugSrvGpuHandle() { return m_d3dDebugSrvGpuHandle; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, bool bPrerender);
	virtual void ReleaseObjects();

	ID3D12RootSignature* m_pd3dDebugRootSignature = NULL;
	ID3D12DescriptorHeap* m_pd3dDebugSrvHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dDebugSrvCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dDebugSrvGpuHandle;
};
