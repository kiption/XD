//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"

CShader::CShader()
{
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char *pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char *)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

#define _WITH_WFOPEN
//#define _WITH_STD_STREAM

#ifdef _WITH_STD_STREAM
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#endif

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob)
{
	UINT nReadBytes = 0;
#ifdef _WITH_WFOPEN
	FILE *pFile = NULL;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize = ::ftell(pFile);
	BYTE *pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	nReadBytes = (UINT)::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
#endif
#ifdef _WITH_STD_STREAM
	std::ifstream ifsFile;
	ifsFile.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	nReadBytes = (int)ifsFile.tellg();
	BYTE *pByteCode = new BYTE[*pnReadBytes];
	ifsFile.seekg(0);
	ifsFile.read((char *)pByteCode, nReadBytes);
	ifsFile.close();
#endif

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	if (ppd3dShaderBlob)
	{
		*ppd3dShaderBlob = NULL;
		HRESULT hResult = D3DCreateBlob(nReadBytes, ppd3dShaderBlob);
		memcpy((*ppd3dShaderBlob)->GetBufferPointer(), pByteCode, nReadBytes);
		d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
		d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
	}
	else
	{
		d3dShaderByteCode.BytecodeLength = nReadBytes;
		d3dShaderByteCode.pShaderBytecode = pByteCode;
	}

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader();
	m_d3dPipelineStateDesc.PS = CreatePixelShader();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_d3dPipelineStateDesc.NumRenderTargets = 1;
	m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState)
{
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
}

void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	OnPrepareRender(pd3dCommandList);
}

void CShader::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrain", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrain", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkyBox", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardShader::CStandardShader()
{
}

CStandardShader::~CStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader()
{
}

CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardObjectsShader::CStandardObjectsShader()
{
}

CStandardObjectsShader::~CStandardObjectsShader()
{
}

void CStandardObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
}

void CStandardObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CStandardObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CStandardObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CStandardObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CStandardShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHellicopterObjectsShader::CHellicopterObjectsShader()
{
}

CHellicopterObjectsShader::~CHellicopterObjectsShader()
{
}

float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float Random()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
    float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
    xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
    xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
    xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}

void CHellicopterObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	m_nObjects = 40;
	m_ppObjects = new CGameObject*[m_nObjects];

	CLoadedModelInfo *pSuperCobraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/SuperCobra.bin", this);
	CLoadedModelInfo *pGunshipModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Gunship.bin", this);

	int nColumnSpace = 5, nColumnSize = 30;           
    int nFirstPassColumnSize = (m_nObjects % nColumnSize) > 0 ? (nColumnSize - 1) : nColumnSize;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	int nObjects = 0;
    for (int h = 0; h < nFirstPassColumnSize; h++)
    {
        for (int i = 0; i < floor(float(m_nObjects) / float(nColumnSize)); i++)
        {
			if (nObjects % 2)
			{
				m_ppObjects[nObjects] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pSuperCobraModel->m_pModelRootObject, true);
			}
			else
			{
				m_ppObjects[nObjects] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pGunshipModel->m_pModelRootObject, true);
			}
			float fHeight = pTerrain->GetHeight(390.0f, 670.0f);
			XMFLOAT3 xmf3Position = RandomPositionInSphere(XMFLOAT3(390.0f, fHeight + 35.0f, 670.0f), Random(20.0f, 100.0f), h - int(floor(nColumnSize / 2.0f)), nColumnSpace);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z) + Random(0.0f, 25.0f);
			m_ppObjects[nObjects]->SetPosition(xmf3Position);
			m_ppObjects[nObjects]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppObjects[nObjects++]->OnPrepareAnimate();
		}
    }

    if (nFirstPassColumnSize != nColumnSize)
    {
        for (int i = 0; i < m_nObjects - int(floor(float(m_nObjects) / float(nColumnSize)) * nFirstPassColumnSize); i++)
        {
			if (nObjects % 2)
			{
				m_ppObjects[nObjects] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pSuperCobraModel->m_pModelRootObject, true);
			}
			else
			{
				m_ppObjects[nObjects] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pGunshipModel->m_pModelRootObject, true);
			}
			m_ppObjects[nObjects]->SetPosition(RandomPositionInSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), Random(20.0f, 100.0f), nColumnSize - int(floor(nColumnSize / 2.0f)), nColumnSpace));
			m_ppObjects[nObjects]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppObjects[nObjects++]->OnPrepareAnimate();
        }
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pSuperCobraModel) delete pSuperCobraModel;
	if (pGunshipModel) delete pGunshipModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationObjectsShader::CSkinnedAnimationObjectsShader()
{
}

CSkinnedAnimationObjectsShader::~CSkinnedAnimationObjectsShader()
{
}

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
}

void CSkinnedAnimationObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CSkinnedAnimationObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CSkinnedAnimationObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAngrybotObjectsShader::CAngrybotObjectsShader()
{
}

CAngrybotObjectsShader::~CAngrybotObjectsShader()
{
}

void CAngrybotObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject*[m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	CLoadedModelInfo *pAngrybotModel = pModel;
	if (!pAngrybotModel) pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Angrybot.bin", NULL);

	int nObjects = 0;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int z = -zObjects; z <= zObjects; z++)
		{
			m_ppObjects[nObjects] = new CAngrybotObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pAngrybotModel, 1);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, (nObjects % 2));
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackSpeed(0, (nObjects % 2) ? 0.25f : 1.0f);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackPosition(0, (nObjects % 3) ? 0.85f : 0.0f);
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch*x + 390.0f, 0.0f, 730.0f + fzPitch * z);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[nObjects]->SetPosition(xmf3Position);
			m_ppObjects[nObjects++]->SetScale(2.0f, 2.0f, 2.0f);
		}
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pAngrybotModel) delete pAngrybotModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CEthanObjectsShader::CEthanObjectsShader()
{
}

CEthanObjectsShader::~CEthanObjectsShader()
{
}

void CEthanObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	m_nObjects = 1;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Jerrycan.bin", NULL);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);
		m_ppObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	}

	m_ppObjects[0]->SetPosition(420.0f, pTerrain->GetHeight(420.0f, 400.0f) + 16.0f, 400.0f);
	m_ppObjects[0]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[0]->SetScale(2.0f, 2.0f, 2.0f);

	/*m_ppObjects[1]->SetPosition(650.0f, pTerrain->GetHeight(650.0f, 200.0f) + 16.0f, 200.0f);
	m_ppObjects[1]->Rotate(270.0f, 180.0f, 0.0f);
	m_ppObjects[1]->SetScale(40.0f, 10.0f, 8.0f);

	m_ppObjects[2]->SetPosition(700.0f, pTerrain->GetHeight(700.0f, 400.0f) + 16.0f, 400.0f);
	m_ppObjects[2]->Rotate(270.0f, 90.0f, 0.0f);
	m_ppObjects[2]->SetScale(15.0f, 6.0f, 8.0f);

	m_ppObjects[3]->SetPosition(450.0f, pTerrain->GetHeight(450.0f, 350.0f) + 16.0f, 350.0f);
	m_ppObjects[3]->Rotate(270.0f, 180.0f, 0.0f);
	m_ppObjects[3]->SetScale(30.0f, 5.0f, 8.0f);

	m_ppObjects[4]->SetPosition(200.0f, pTerrain->GetHeight(200.0f, 500.0f) + 16.0f, 500.0f);
	m_ppObjects[4]->Rotate(270.0f, 90.0f, 0.0f);
	m_ppObjects[4]->SetScale(30.0f, 10.0f, 8.0f);

	m_ppObjects[5]->SetPosition(550.0f, pTerrain->GetHeight(550.0f, 400.0f) + 16.0f, 400.0f);
	m_ppObjects[5]->Rotate(270.0f, 0.0f, 0.0f);
	m_ppObjects[5]->SetScale(30.0f, 10.0f, 8.0f);

	m_ppObjects[6]->SetPosition(800.0f, pTerrain->GetHeight(800.0f, 300.0f) + 16.0f, 300.0f);
	m_ppObjects[6]->Rotate(270.0f, 180.0f, 0.0f);
	m_ppObjects[6]->SetScale(30.0f, 10.0f, 8.0f);

	m_ppObjects[7]->SetPosition(150.0f, pTerrain->GetHeight(150.0f, 465.0f) + 16.0f, 465.0f);
	m_ppObjects[7]->Rotate(270.0f, 180.0f, 0.0f);
	m_ppObjects[7]->SetScale(20.0f, 5.0f, 8.0f);

	m_ppObjects[8]->SetPosition(450.0f, pTerrain->GetHeight(450.0f, 700.0f) + 16.0f, 700.0f);
	m_ppObjects[8]->Rotate(270.0f, 0.0f, 0.0f);
	m_ppObjects[8]->SetScale(20.0f, 4.0f, 8.0f);

	m_ppObjects[9]->SetPosition(75.0f, pTerrain->GetHeight(75.0f, 750.0f) + 16.0f, 750.0f);
	m_ppObjects[9]->Rotate(270.0f, 180.0f, 0.0f);
	m_ppObjects[9]->SetScale(20.0f, 4.0f, 8.0f);

	m_ppObjects[10]->SetPosition(125.0f, pTerrain->GetHeight(125.0f, 500.0f) + 16.0f, 500.0f);
	m_ppObjects[10]->Rotate(270.0f, 90.0f, 0.0f);
	m_ppObjects[10]->SetScale(15.0f, 8.0f, 8.0f);

	m_ppObjects[11]->SetPosition(745.0f, pTerrain->GetHeight(745.0f, 440.0f) + 16.0f, 440.0f);
	m_ppObjects[11]->Rotate(270.0f, 180.0f, 0.0f);
	m_ppObjects[11]->SetScale(20.0f, 10.0f, 8.0f);

	m_ppObjects[12]->SetPosition(350.0f, pTerrain->GetHeight(350.0f, 600.0f) + 16.0f, 600.0f);
	m_ppObjects[12]->Rotate(270.0f, 0.0f, 0.0f);
	m_ppObjects[12]->SetScale(40.0f, 4.0f, 8.0f);*/

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}

CObjectsShader::CObjectsShader()
{
}

CObjectsShader::~CObjectsShader()
{
}

void CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
}

void CObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(0.16f);
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbGElementBytes = ((sizeof(CB_SCENEMODEL_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbSceneObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbGElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbSceneObject->Map(0, NULL, (void**)&m_pcbMappedSceneGameObject);

}

void CObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_SCENEMODEL_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		CB_SCENEMODEL_INFO* pcbSceneGameObject = (CB_SCENEMODEL_INFO*)((UINT8*)m_pcbMappedSceneGameObject + (j * ncbElementBytes));
		XMStoreFloat4x4(&pcbSceneGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
		if (m_ppObjects[j]->m_ppMaterials)
		{
			if (m_ppObjects[j]->m_ppMaterials[0] && m_ppObjects[j]->m_ppMaterials[0]->m_pTexture)//오류
			{
				XMStoreFloat4x4(&pcbSceneGameObject->m_xmf4x4Texture, XMMatrixTranspose(XMLoadFloat4x4(&(m_ppObjects[j]->m_ppMaterials[0]->m_pTexture->m_xmf4x4Texture))));
			}
		}
	}

}

void CObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbSceneObject)
	{
		m_pd3dcbSceneObject->Unmap(0, NULL);
		m_pd3dcbSceneObject->Release();
	}

	CShader::ReleaseShaderVariables();
}

D3D12_INPUT_LAYOUT_DESC CSceneShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXTURECOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSceneShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSScene", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CSceneShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSScene", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CSceneShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, void* pContext)
{
	int nSceneTextures = 0;
	m_ppObjects = ::LoadGameObjectsFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrFileName, &m_nObjects);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CSceneShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CSceneShader::AnimateObjects(float fTimeElapsed)
{
	
	CObjectsShader::AnimateObjects(fTimeElapsed);
}

void CSceneShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);


		}
	}
}

void CSceneShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}

void CWallObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	m_nObjects = 10;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);
		m_ppObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	}

	m_ppObjects[0]->SetPosition(850.0f, pTerrain->GetHeight(850.0f, 250.0f), 250.0f);
	m_ppObjects[0]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[0]->SetScale(6.0f, 2.0f, 4.0f);

	m_ppObjects[1]->SetPosition(875.0f, pTerrain->GetHeight(875.0f, 150.0f), 150.0f);
	m_ppObjects[1]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[1]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[2]->SetPosition(120.0f, pTerrain->GetHeight(120.0f, 325.0f), 325.0f);
	m_ppObjects[2]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[2]->SetScale(8.0f, 2.0f, 8.0f);

	m_ppObjects[3]->SetPosition(900.0f, pTerrain->GetHeight(900.0f, 400.0f), 400.0f);
	m_ppObjects[3]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[3]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[4]->SetPosition(160.0f, pTerrain->GetHeight(160.0f, 605.0f), 605.0f);
	m_ppObjects[4]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[4]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[5]->SetPosition(600.0f, pTerrain->GetHeight(600.0f, 500.0f), 500.0f);
	m_ppObjects[5]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[5]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[6]->SetPosition(525.0f, pTerrain->GetHeight(525.0f, 650.0f), 650.0f);
	m_ppObjects[6]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[6]->SetScale(8.0f, 2.0f, 8.0f);

	m_ppObjects[7]->SetPosition(750.0f, pTerrain->GetHeight(750.0f, 800.0f), 800.0f);
	m_ppObjects[7]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[7]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[8]->SetPosition(400.0f, pTerrain->GetHeight(400.0f, 500.0f), 500.0f);
	m_ppObjects[8]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[8]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[9]->SetPosition(250.0f, pTerrain->GetHeight(250.0f, 795.0f), 795.0f);
	m_ppObjects[9]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[9]->SetScale(10.0f, 2.0f, 8.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}

void CBuildingObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{

	m_nObjects = 24;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);
		m_ppObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	}
	m_ppObjects[0]->SetPosition(350.0f, pTerrain->GetHeight(350.0f, 90.0f), 90.0f);
	m_ppObjects[0]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[0]->SetScale(6.0f, 2.0f, 2.0f);

	m_ppObjects[1]->SetPosition(300.0f, pTerrain->GetHeight(300.0f, 60.0f), 60.0f);
	m_ppObjects[1]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[1]->SetScale(6.0f, 2.0f, 3.0f);

	m_ppObjects[2]->SetPosition(450.0f, pTerrain->GetHeight(450.0f, 150.0f), 150.0f);
	m_ppObjects[2]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppObjects[2]->SetScale(20.0f, 2.0f, 3.0f);

	m_ppObjects[3]->SetPosition(300.0f, pTerrain->GetHeight(300.0f, 200.0f), 200.0f);
	m_ppObjects[3]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[3]->SetScale(15.0f, 2.0f, 3.0f);
	
	m_ppObjects[4]->SetPosition(550.0f, pTerrain->GetHeight(550.0f, 275.0f), 275.0f);
	m_ppObjects[4]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[4]->SetScale(15.0f, 2.0f, 3.0f);

	m_ppObjects[5]->SetPosition(150.0f, pTerrain->GetHeight(150.0f, 200.0f), 200.0f);
	m_ppObjects[5]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[5]->SetScale(12.0f, 2.0f, 3.0f);

	m_ppObjects[6]->SetPosition(150.0f, pTerrain->GetHeight(150.0f, 100.0f), 100.0f);
	m_ppObjects[6]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[6]->SetScale(15.0f, 1.5f, 3.0f);

	m_ppObjects[7]->SetPosition(110.0f, pTerrain->GetHeight(110.0f, 240.0f), 240.0f);
	m_ppObjects[7]->Rotate(0.0f, 300.0f, 0.0f);
	m_ppObjects[7]->SetScale(8.0f, 2.0f, 3.0f);

	m_ppObjects[8]->SetPosition(250.0f, pTerrain->GetHeight(250.0f, 350.0f), 350.0f);
	m_ppObjects[8]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[8]->SetScale(10.0f, 2.0f, 3.0f);

	m_ppObjects[9]->SetPosition(350.0f, pTerrain->GetHeight(350.0f, 320.0f), 320.0f);
	m_ppObjects[9]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[9]->SetScale(15.0f, 2.0f, 3.0f);

	m_ppObjects[10]->SetPosition(760.0f, pTerrain->GetHeight(760.0f, 215.0f), 215.0f);
	m_ppObjects[10]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[10]->SetScale(10.0f, 2.0f, 3.0f);

	m_ppObjects[11]->SetPosition(710.0f, pTerrain->GetHeight(710.0f, 130.0f), 130.0f);
	m_ppObjects[11]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppObjects[11]->SetScale(8.0f, 2.0f, 3.0f);

	m_ppObjects[12]->SetPosition(640.0f, pTerrain->GetHeight(640.0f, 335.0f), 335.0f);
	m_ppObjects[12]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[12]->SetScale(6.0f, 2.0f, 3.0f);

	m_ppObjects[13]->SetPosition(500.0f, pTerrain->GetHeight(500.0f, 850.0f), 850.0f);
	m_ppObjects[13]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[13]->SetScale(20.0f, 2.0f, 4.0f);

	m_ppObjects[14]->SetPosition(380.0f, pTerrain->GetHeight(380.0f, 940.0f), 940.0f);
	m_ppObjects[14]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[14]->SetScale(6.0f, 2.0f, 3.0f);

	m_ppObjects[15]->SetPosition(600.0f, pTerrain->GetHeight(600.0f, 940.0f), 940.0f);
	m_ppObjects[15]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppObjects[15]->SetScale(6.0f, 2.0f, 3.0f);
	
	m_ppObjects[16]->SetPosition(500.0f, pTerrain->GetHeight(500.0f, 775.0f), 775.0f);
	m_ppObjects[16]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[16]->SetScale(15.0f, 2.0f, 4.0f);

	m_ppObjects[17]->SetPosition(150.0f, pTerrain->GetHeight(150.0f, 850.0f), 850.0f);
	m_ppObjects[17]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[17]->SetScale(15.0f, 2.0f, 2.0f);

	m_ppObjects[18]->SetPosition(685.0f, pTerrain->GetHeight(685.0f, 875.0f), 875.0f);
	m_ppObjects[18]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[18]->SetScale(8.0f, 2.0f, 3.0f);

	m_ppObjects[19]->SetPosition(850.0f, pTerrain->GetHeight(850.0f, 900.0f), 900.0f);
	m_ppObjects[19]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[19]->SetScale(15.0f, 2.0f, 2.0f);

	m_ppObjects[20]->SetPosition(850.0f, pTerrain->GetHeight(850.0f, 750.0f), 750.0f);
	m_ppObjects[20]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[20]->SetScale(12.0f, 2.0f, 3.0f);

	m_ppObjects[21]->SetPosition(750.0f, pTerrain->GetHeight(750.0f, 335.0f), 335.0f);
	m_ppObjects[21]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[21]->SetScale(12.0f, 2.0f, 2.0f);

	m_ppObjects[22]->SetPosition(700.0f, pTerrain->GetHeight(700.0f, 515.0f), 515.0f);
	m_ppObjects[22]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[22]->SetScale(12.0f, 2.0f, 2.0f);

	m_ppObjects[23]->SetPosition(750.0f, pTerrain->GetHeight(750.0f, 650.0f), 650.0f);
	m_ppObjects[23]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[23]->SetScale(14.0f, 2.0f, 2.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}

void CGeneratorShedShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	m_nObjects = 1;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);
		m_ppObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	}
	m_ppObjects[0]->SetPosition(450.0f, pTerrain->GetHeight(450.0f, 450.0f), 450.0f);
	m_ppObjects[0]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[0]->SetScale(3.0f, 3.0f, 3.0f);

	/*m_ppObjects[1]->SetPosition(450.0f, pTerrain->GetHeight(450.0f, 550.0f), 550.0f);
	m_ppObjects[1]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[1]->SetScale(3.0f, 3.0f, 3.0f);

	m_ppObjects[2]->SetPosition(550.0f, pTerrain->GetHeight(550.0f, 550.0f), 550.0f);
	m_ppObjects[2]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[2]->SetScale(3.0f, 3.0f, 3.0f);

	m_ppObjects[3]->SetPosition(550.0f, pTerrain->GetHeight(550.0f, 450.0f), 450.0f);
	m_ppObjects[3]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppObjects[3]->SetScale(3.0f, 3.0f, 3.0f);

	m_ppObjects[4]->SetPosition(650.0f, pTerrain->GetHeight(650.0f, 450.0f), 450.0f);
	m_ppObjects[4]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[4]->SetScale(3.0f, 3.0f, 3.0f);

	m_ppObjects[5]->SetPosition(920.0f, pTerrain->GetHeight(920.0f, 125.0f), 125.0f);
	m_ppObjects[5]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[5]->SetScale(3.0f, 2.0f, 3.0f);

	m_ppObjects[6]->SetPosition(250.0f, pTerrain->GetHeight(250.0f, 800.0f), 800.0f);
	m_ppObjects[6]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[6]->SetScale(3.0f, 2.0f, 3.0f);

	m_ppObjects[7]->SetPosition(850.0f, pTerrain->GetHeight(850.0f, 550.0f), 550.0f);
	m_ppObjects[7]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[7]->SetScale(6.0f, 2.0f, 6.0f);

	m_ppObjects[8]->SetPosition(100.0f, pTerrain->GetHeight(100.0f, 800.0f), 800.0f);
	m_ppObjects[8]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[8]->SetScale(2.0f, 2.0f, 2.0f);

	m_ppObjects[9]->SetPosition(250.0f, pTerrain->GetHeight(250.0f, 675.0f), 675.0f);
	m_ppObjects[9]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[9]->SetScale(2.0f, 2.0f, 2.0f);

	m_ppObjects[10]->SetPosition(100.0f, pTerrain->GetHeight(100.0f, 675.0f), 675.0f);
	m_ppObjects[10]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[10]->SetScale(2.0f, 2.0f, 2.0f);

	m_ppObjects[11]->SetPosition(125.0f, pTerrain->GetHeight(125.0f, 400.0f), 400.0f);
	m_ppObjects[11]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[11]->SetScale(4.0f, 2.0f, 4.0f);

	m_ppObjects[12]->SetPosition(325.0f, pTerrain->GetHeight(325.0f, 550.0f), 550.0f);
	m_ppObjects[12]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[12]->SetScale(4.0f, 2.0f, 4.0f);*/

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}
