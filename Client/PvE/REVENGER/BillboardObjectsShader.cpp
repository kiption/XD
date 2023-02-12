#include "stdafx.h"
#include "BillboardObjectsShader.h"
#include "Stage1.h"

D3D12_INPUT_LAYOUT_DESC BillboardObjectsShader::CreateInputLayout()
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

D3D12_BLEND_DESC BillboardObjectsShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE BillboardObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE BillboardObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void BillboardObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/SpaceCross.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pTerrainMaterial = new CMaterial(1);
	pTerrainMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList,60.0, 60.0, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1;
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < m_nObjects; i++)
	{
		SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);//+
		m_ppObjects = new CGameObject * [m_nObjects];
		CBillboardObject* pThirdObject = NULL;
		pThirdObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pThirdObject->SetMesh(pSpriteMesh);
		pThirdObject->SetMaterial(0,pTerrainMaterial);
		m_ppObjects[i] = pThirdObject;
	}
}

void BillboardObjectsShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void BillboardObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 180.0f, false));
	xmf3Position.y += 40.0f;
	m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[0]->SetPosition(xmf3Position);
	BillboardShader::Render(pd3dCommandList, pCamera);
}

void BillboardObjectsShader::AnimateObjects(float fTimeElapsed)
{
	BillboardShader::AnimateObjects(fTimeElapsed);
}

void BillboardObjectsShader::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}
