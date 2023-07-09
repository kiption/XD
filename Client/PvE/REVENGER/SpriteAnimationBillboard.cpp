#include "stdafx.h"
#include "SpriteAnimationBillboard.h"
#include "Scene.h"
#include "Stage1.h"
D3D12_INPUT_LAYOUT_DESC CSpriteTexturedShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

void CSpriteTexturedShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);
}

D3D12_SHADER_BYTECODE CSpriteTexturedShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpritTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSpriteTexturedShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSpritTextured", "ps_5_1", ppd3dShaderBlob));
}

void CSpriteObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CSpriteObjectsShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CSpriteObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}


}

void CSpriteObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CSpriteObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		CB_GAMEOBJECT_INFO* pbMappedcbGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
		for (int i = 0; i < m_ppObjects[j]->m_nMaterials; i++)
		{
			for (int k = 0; k < m_ppObjects[j]->m_ppMaterials[i]->m_nTextures; k++)
			{

				if (m_ppObjects[j]->m_ppMaterials[i] && m_ppObjects[j]->m_ppMaterials[i]->m_ppTextures[k])
				{
					XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4Texture, 
					XMMatrixTranspose(XMLoadFloat4x4(&(m_ppObjects[j]->m_ppMaterials[i]->m_ppTextures[k]->m_xmf4x4Texture))));
				}
			}

		}
	}
}

void CSpriteObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}


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
	CSpriteTexturedShader::Render(pd3dCommandList, pCamera, nPipelineState, NULL);

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

void SpriteAnimationBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);
}

D3D12_SHADER_BYTECODE SpriteAnimationBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteAnimation", "vs_5_1", ppd3dShaderBlob));
}

void SpriteAnimationBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures[2];
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8);
	ppSpriteTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);
	ppSpriteTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8);
	ppSpriteTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pTerrainMaterial[2];
	pTerrainMaterial[0] = new CMaterial(1);
	pTerrainMaterial[1] = new CMaterial(1);

	pTerrainMaterial[0]->SetTexture(ppSpriteTextures[0], 0);
	pTerrainMaterial[1]->SetTexture(ppSpriteTextures[1], 0);

	
	CTexturedRectMesh* pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5.0, 5.0, 0.0f, 0.0f, 0.0f, 0.0f);



	SceneManager* pScene = NULL;
	m_nObjects = 2;
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pScene->CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[0], 0, 15);//+
	pScene->CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[1], 0, 15);//+
	pScene->CreateConstantBufferView(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	m_ppObjects = new CGameObject * [m_nObjects];
	CMultiSpriteObject** pThirdObject = new CMultiSpriteObject * [m_nObjects];

	for (int i = 0; i < m_nObjects; i++)
	{

		pThirdObject[i] = new CMultiSpriteObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pThirdObject[i]->SetMesh(pSpriteMesh);
		pThirdObject[i]->SetMaterial(0, pTerrainMaterial[i]);
		pScene->SetCbvGPUDescriptorHandlePtr(pScene->m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		pThirdObject[i]->m_fSpeed = 6.0f / (ppSpriteTextures[i]->m_nRows * ppSpriteTextures[i]->m_nCols);
		m_ppObjects[i] = pThirdObject[i];
		m_ppObjects[i]->AddRef();
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

		XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 50.0f, false));
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{

				m_ppObjects[j]->SetPosition(-15.0 + j * 20,10.0 + j * 20,800.0+j*20);
			
				m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			}
		}

		CSpriteObjectsShader::Render(pd3dCommandList, pCamera, 0);
	}
}

void SpriteAnimationBillboard::ReleaseUploadBuffers()
{
	CSpriteObjectsShader::ReleaseUploadBuffers();
}

void SpriteAnimationBillboard::ReleaseObjects()
{
	CSpriteObjectsShader::ReleaseObjects();
}

void SpriteAnimationBillboard::AnimateObjects(float fTimeElapsed)
{
	/*for (int j = 0; j < m_nObjects; j++)
	{
		ExplosionPosition.x = m_ppObjects[j]->m_xmf4x4ToParent._41;
		ExplosionPosition.y = m_ppObjects[j]->m_xmf4x4ToParent._42;
		ExplosionPosition.z = m_ppObjects[j]->m_xmf4x4ToParent._43;
	}*/
	CSpriteObjectsShader::AnimateObjects(fTimeElapsed);
}
