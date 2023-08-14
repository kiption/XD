//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"

struct TOOBJECTSPACEINFO
{
	XMFLOAT4X4						m_xmf4x4ToTexture;

	XMFLOAT4						m_xmf4Position;
};

struct TOLIGHTSPACES
{
	TOOBJECTSPACEINFO				m_pToLightSpaces[MAX_LIGHTS];
};
struct LIGHT;

class ShaderMgr
{
public:
	ShaderMgr();
	virtual ~ShaderMgr();

private:
	int									m_nReferences = 0;
public:
	bool							m_bActive = true;
public:
	void SetActive(bool bActive) { m_bActive = bActive; }
	bool GetActive() { return(m_bActive); }
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	bool m_ShadowActive = true;

	// Use PipelineStates
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState);

	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState);
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOuputState(int nPipelineState);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateHullShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateDomainShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState);
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(int nPipelineState) { return(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); }
	virtual UINT GetNumRenderTargets(int nPipelineState) { return(1); }
	virtual DXGI_FORMAT GetRTVFormat(int nPipelineState, int nRenderTarget) { return(DXGI_FORMAT_R8G8B8A8_UNORM); }
	virtual DXGI_FORMAT GetDSVFormat(int nPipelineState) { return(DXGI_FORMAT_D24_UNORM_S8_UINT); }
	virtual void SetPipelineState(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob = NULL);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {}
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World, CMaterial* pMaterial);
	virtual void ReleaseShaderVariables();
	//virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState);
	//virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, bool bPrerender);

	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }
	void SetCurScene(int nCurScene) { m_nCurScene = nCurScene; }

public:
	ID3D12Resource* m_pd3dcbFrameworkInfo = NULL;
	CB_FRAMEWORK_INFO* m_pcbMappedFrameworkInfo = NULL;


	int									m_nPipelineStates = 0;
	ID3D12PipelineState** m_ppd3dPipelineStates = NULL;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	float								m_fElapsedTime = 0.0f;

	int									m_nCurScene;
public:
	GameObjectMgr** m_ppObjects = 0;
	CLoadedModelInfo* m_pSkinObjects = 0;
	CAnimationController* m_pAnimationController = 0;
	int									m_nObjects = 0;
};
