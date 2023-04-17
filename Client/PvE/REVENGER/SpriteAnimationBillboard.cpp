#include "stdafx.h"
#include "SpriteAnimationBillboard.h"
#include "Scene.h"
D3D12_INPUT_LAYOUT_DESC CSpriteTexturedShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

void CSpriteTexturedShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
}

D3D12_SHADER_BYTECODE CSpriteTexturedShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSpriteTexturedShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSpriteTextured", "ps_5_1", ppd3dShaderBlob));
}

void CSpriteObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CSpriteObjectsShader::AnimateObjects(float fTimeElapsed)
{
}

void CSpriteObjectsShader::ReleaseObjects()
{
}

void CSpriteObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CSpriteObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	
}

void CSpriteObjectsShader::ReleaseShaderVariables()
{
	CSpriteTexturedShader::ReleaseShaderVariables();
}

void CSpriteObjectsShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void CSpriteObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CSpriteTexturedShader::Render(pd3dCommandList, pCamera, nPipelineState);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

D3D12_RASTERIZER_DESC SpriteAnimationBillboard::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	//	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

D3D12_BLEND_DESC SpriteAnimationBillboard::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void SpriteAnimationBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);

}

D3D12_SHADER_BYTECODE SpriteAnimationBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteAnimation", "vs_5_1", ppd3dShaderBlob));
}

void SpriteAnimationBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pTerrainMaterial = new CMaterial(1);
	pTerrainMaterial->SetTexture(ppSpriteTextures);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 80.0, 80.0, 0.0f, 0.0f, 0.0f, 0.0f);


	m_nObjects = 1;
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager* pScene = NULL;
	pScene->CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);//+
	pScene->CreateConstantBufferView(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	for (int i = 0; i < m_nObjects; i++)
	{

		m_ppObjects = new CGameObject * [m_nObjects];
		CMultiSpriteObject* pThirdObject = NULL;
		pThirdObject = new CMultiSpriteObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pThirdObject->SetMesh(pSpriteMesh);
		pThirdObject->SetMaterial(0, pTerrainMaterial);
		//pScene->SetCbvGPUDescriptorHandlePtr(pScene->m_d3dCbvGPUDescriptorNextHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));

		pThirdObject->m_fSpeed = 3.0f / (ppSpriteTextures->m_nRows * ppSpriteTextures->m_nCols);
		m_ppObjects[i] = pThirdObject;
	}
}

void SpriteAnimationBillboard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bActive)
	{
		XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
		CPlayer* pPlayer = pCamera->GetPlayer();
		XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
		XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
		xmf3PlayerPosition.y += 5.0f;
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 50.0f, false));
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->SetPosition(xmf3Position);
				m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			}
		}

		BillboardShader::Render(pd3dCommandList, pCamera, 0);
	}
}

void SpriteAnimationBillboard::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

void SpriteAnimationBillboard::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void SpriteAnimationBillboard::AnimateObjects(float fTimeElapsed)
{
}
