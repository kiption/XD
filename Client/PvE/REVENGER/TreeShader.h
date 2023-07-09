#pragma once/*
#include "Shader.h"
#include "Scene.h"
class TreeObjectsShader : public CShader
{
public:
	TreeObjectsShader() {};
	virtual ~TreeObjectsShader() {};
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		int nPipelineState);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
public:
	CGameObject** m_ppTreeObjectList = NULL;
	int m_nTreeObjectList = NULL;
};
class TreeDepthRenderShader : public CShader
{
public:
	TreeDepthRenderShader(TreeObjectsShader* pObjectsShader, LIGHT* pLights);
	virtual ~TreeDepthRenderShader();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget) { return(DXGI_FORMAT_R32_FLOAT); }
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState) { return(DXGI_FORMAT_D32_FLOAT); }
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	void PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		int nPipelineState);

public:
	CTexture* m_pDepthTexture = NULL;

	CCamera* m_ppDepthRenderCameras[MAX_DEPTH_TEXTURES];

	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRtvCPUDescriptorHandles[MAX_DEPTH_TEXTURES];

	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
	ID3D12Resource* m_pd3dDepthBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;
	XMMATRIX						m_xmProjectionToTexture;

public:
	CTexture* GetDepthTexture() { return(m_pDepthTexture); }
	ID3D12Resource* GetDepthTextureResource(UINT nIndex) { return(m_pDepthTexture->GetResource(nIndex)); }

public:
	TreeObjectsShader* m_pObjectsShader = NULL;


public:
	SceneManager* m_pScene = NULL;
	LIGHT* m_pLights = NULL;
	TOLIGHTSPACES* m_pToLightSpaces;

	ID3D12Resource* m_pd3dcbToLightSpaces = NULL;
	TOLIGHTSPACES* m_pcbMappedToLightSpaces = NULL;
};
class TreeShadowMapShader : public CShader
{
public:
	TreeShadowMapShader(TreeObjectsShader* pObjectsShader);
	virtual ~TreeShadowMapShader();
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState);

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		int nPipelineState);

public:
	TreeObjectsShader* m_pObjectsShader = NULL;
	CTexture* m_pDepthTexture = NULL;
};
*/
