#include "stdafx.h"
#include "BillboardShader.h"
#include "BillboardObject.h"
#include "TerrainObject.h"
#include "Player.h"

uniform_int_distribution<int>randomwidth(0, 4500);
uniform_int_distribution<int>randomHeight(1100, 1550);
uniform_real_distribution<float>Start_xz(100.0, 2500.0);
uniform_real_distribution<float>Start_y(800.0, 850.0);

CBillboardShader::CBillboardShader()
{
}

CBillboardShader::~CBillboardShader()
{
}

D3D12_SHADER_BYTECODE CBillboardShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CBillboardShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoard", "ps_5_1", ppd3dShaderBlob));
}

D3D12_RASTERIZER_DESC CBillboardShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

D3D12_BLEND_DESC CBillboardShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}


void CBillboardObjectShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}


D3D12_BLEND_DESC CBillboardObjectShader::CreateBlendState(int nPipelineState)
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

D3D12_SHADER_BYTECODE CBillboardObjectShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CBillboardObjectShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CBillboardObjectShader::CreateInputLayout(int nPipelineState)
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

uniform_real_distribution<float>TreeHeight(150.0, 200.0);
void CBillboardObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppTreeTextures[3];
	ppTreeTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTreeTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/ReafTree.dds", RESOURCE_TEXTURE2D, 0);
	ppTreeTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTreeTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/ReafTree.dds", RESOURCE_TEXTURE2D, 0);
	ppTreeTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppTreeTextures[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/ReafTree.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppTreeMaterials[3];
	ppTreeMaterials[0] = new CMaterial();
	ppTreeMaterials[0]->SetTexture(ppTreeTextures[0]);
	ppTreeMaterials[1] = new CMaterial();
	ppTreeMaterials[1]->SetTexture(ppTreeTextures[1]);
	ppTreeMaterials[2] = new CMaterial();
	ppTreeMaterials[2]->SetTexture(ppTreeTextures[2]);

	CTexturedRectMesh* pTreeMesh00 = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, TreeHeight(dre), 0.0f, 0.0f, 0.0f, 0.0f);
	CTexturedRectMesh* pTreeMesh01 = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 100.0f, TreeHeight(dre), 0.0f, 0.0f, 0.0f, 0.0f);
	CTexturedRectMesh* pTreeMesh02 = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 60.0f, 200.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	CRawFormatImage* pRawFormatImage = new CRawFormatImage(L"Image/terrain.raw", 257, 257, true);

	int nGrassObjects = 0, nFlowerObjects = 0, nBlacks = 0, nOthers = 0, nTreeObjects[3] = { 0, 0, 0 };
	for (int z = 2; z <= 254; z++)
	{
		for (int x = 2; x <= 254; x++)
		{
			BYTE nPixel = pRawFormatImage->GetRawImagePixel(x, z);
			switch (nPixel)
			{
			case 160: nTreeObjects[1]++; break;
			case 190: nTreeObjects[1]++; break;
			case 230: nTreeObjects[2]++; break;
			case 252: nTreeObjects[0]++; break;
			case 0: nBlacks++; break;
			default: nOthers++; break;
			}
		}
	}
	m_nObjects = nTreeObjects[0] + nTreeObjects[1] + nTreeObjects[2];


	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 3);
	CreateShaderResourceViews(pd3dDevice, ppTreeTextures[0], 0, 12);
	CreateShaderResourceViews(pd3dDevice, ppTreeTextures[1], 0, 12);
	CreateShaderResourceViews(pd3dDevice, ppTreeTextures[2], 0, 12);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	int nTerrainWidth = int(pTerrain->GetWidth());
	int nTerrainLength = int(pTerrain->GetLength());
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();

	m_ppObjects = new CGameObject * [m_nObjects];

	CBillboardObject* pBillboardObject = NULL;

	for (int nObjects = 0, z = 2; z <= 254; z++)
	{
		for (int x = 2; x <= 254; x++)
		{
			BYTE nPixel = pRawFormatImage->GetRawImagePixel(x, z);
			float fyOffset = 0.0f;
			CMaterial* pMaterial = NULL;
			CMesh* pMesh = NULL;

			switch (nPixel)
			{
			case 160:
				pMesh = pTreeMesh01;
				pMaterial = ppTreeMaterials[1];
				fyOffset = 23.0f * 0.5f;
				break;
			case 190:
				pMesh = pTreeMesh01;
				pMaterial = ppTreeMaterials[1];
				fyOffset = 23.0f * 0.5f;
				break;
			case 230:
				pMesh = pTreeMesh00;
				pMaterial = ppTreeMaterials[2];
				fyOffset = 13.0f * 0.5f;
				break;
			case 252:
				pMesh = pTreeMesh02;
				pMaterial = ppTreeMaterials[0];
				fyOffset = 25.0f * 0.5f;
				break;
			default:
				break;
			}

			if (pMesh && pMaterial)
			{
				pBillboardObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

				pBillboardObject->SetMesh(0, pMesh);
				pBillboardObject->SetMaterial(0, pMaterial);
				float xPosition = x * xmf3Scale.x;
				float zPosition = z * xmf3Scale.z;
				float fHeight = pTerrain->GetHeight(xPosition, zPosition);
				pBillboardObject->SetPosition(xPosition, (fHeight + (fyOffset)), zPosition);
				m_ppObjects[nObjects++] = pBillboardObject;
			}
		}
	}
}
void CBillboardObjectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState);
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 0.5f, 0.0f));
	}
}

void CBillboardObjectShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CBillboardObjectShader::AnimateObjects(float fTimeElapsed)
{

}

void CBillboardObjectShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}


void CCrossHairShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

D3D12_INPUT_LAYOUT_DESC CCrossHairShader::CreateInputLayout(int nPipelineState)
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


D3D12_BLEND_DESC CCrossHairShader::CreateBlendState(int nPipelineState)
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

D3D12_SHADER_BYTECODE CCrossHairShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CCrossHairShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}
void CCrossHairShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures[1];
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/SpaceCross.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppSpriteMaterials[1];
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppSpriteTextures[0]);

	CTexturedRectMesh* pSpriteMesh[1];
	pSpriteMesh[0] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, m_fWidth, m_fHeight, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1;
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < m_nObjects; i++)
	{
		CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[i], 0, 12);

		m_ppObjects = new CGameObject * [m_nObjects];
		CBillboardObject* pThirdObject = NULL;
		pThirdObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pThirdObject->SetMesh(0, pSpriteMesh[i]);
		pThirdObject->SetMaterial(0, ppSpriteMaterials[i]);
		m_ppObjects[i] = pThirdObject;
	}
}


void CCrossHairShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}

void CCrossHairShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CCrossHairShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (pCamera->GetMode() == THIRD_PERSON_CAMERA)
	{
		XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
		CPlayer* pPlayer = pCamera->GetPlayer();
		XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
		XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 80.0f, false));
		xmf3Position.y += 6.0f;
		m_fWidth = 70.0; m_fHeight = 70.0f;
		m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppObjects[0]->SetPosition(xmf3Position);
		CObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState);

	}
	else if (pCamera->GetMode() == SPACESHIP_CAMERA)
	{
		XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
		XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
		CPlayer* pPlayer = pCamera->GetPlayer();
		XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
		XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 30.0f, false));
		xmf3Position.y += 0.0f;
		m_fWidth = 60.0f; m_fHeight = 60.0f;
		m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppObjects[0]->SetPosition(xmf3Position);
		CObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState);
	}
}

void CCrossHairShader::AnimateObjects(float fTimeElapsed)
{

}

void CSnowObjectShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}


D3D12_BLEND_DESC CSnowObjectShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_DEST_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_SUBTRACT;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = 15;

	return(d3dBlendDesc);
}

D3D12_SHADER_BYTECODE CSnowObjectShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CSnowObjectShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_INPUT_LAYOUT_DESC CSnowObjectShader::CreateInputLayout(int nPipelineState)
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


void CSnowObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures[1];
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Rain.dds", RESOURCE_TEXTURE2D, 0);


	CMaterial* ppSpriteMaterials[1];
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppSpriteTextures[0]);

	CTexturedRectMesh* pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.5f, 12.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1500;
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[0], 0, 12);
	m_ppObjects = new CGameObject * [m_nObjects];

	CBillboardObject* pCRainObject = NULL;
	for (int j = 0; j < m_nObjects; j++)
	{
		pCRainObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pCRainObject->SetMesh(0, pSpriteMesh);
		pCRainObject->SetMaterial(0, ppSpriteMaterials[0]);
		pCRainObject->SetPosition(XMFLOAT3(randomwidth(dre), randomHeight(dre), randomwidth(dre)));
		m_ppObjects[j] = pCRainObject;
	}
}

void CSnowObjectShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CSnowObjectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 80.0f, false));
	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->SetLookAt(xmf3Position, XMFLOAT3(0.0f, 0.1, 0.0f));
	}
	CObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState);

}

void CSnowObjectShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->m_xmf4x4Transform._42 -= 10.0f;

		if (m_ppObjects[j]->m_xmf4x4Transform._42 < 550.0)
		{
			m_ppObjects[j]->SetPosition(XMFLOAT3(randomwidth(dre), randomHeight(dre), randomwidth(dre)));
		}
	}
}

void CSnowObjectShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}

void CBulletMotionShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

D3D12_INPUT_LAYOUT_DESC CBulletMotionShader::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC CBulletMotionShader::CreateBlendState(int nPipelineState)
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

D3D12_SHADER_BYTECODE CBulletMotionShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CBulletMotionShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}
void CBulletMotionShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures[3];
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/MainExplosion.dds", RESOURCE_TEXTURE2D, 0);
	ppSpriteTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/MainExplosion.dds", RESOURCE_TEXTURE2D, 0);
	ppSpriteTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/MainExplosion.dds", RESOURCE_TEXTURE2D, 0);


	CMaterial* ppSpriteMaterials[3];
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppSpriteTextures[0]);
	ppSpriteMaterials[1] = new CMaterial();
	ppSpriteMaterials[1]->SetTexture(ppSpriteTextures[1]);
	ppSpriteMaterials[2] = new CMaterial();
	ppSpriteMaterials[2]->SetTexture(ppSpriteTextures[2]);


	CTexturedRectMesh* pSpriteMesh[3];
	pSpriteMesh[0] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	pSpriteMesh[1] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	pSpriteMesh[2] = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 3;
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 3);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	for (int i = 0; i < m_nObjects; i++)
	{
		CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[i], 0, 12);
	}

	m_ppObjects = new CGameObject * [m_nObjects];
	CBillboardObject* pBulletObject = NULL;

	for (int j = 0; j < m_nObjects; j++)
	{
		pBulletObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pBulletObject->SetMesh(0, pSpriteMesh[j]);
		pBulletObject->SetMaterial(0, ppSpriteMaterials[j]);
		m_ppObjects[j] = pBulletObject;
	}
}

void CBulletMotionShader::ReleaseUploadBuffers()
{
	CNPCShader::ReleaseUploadBuffers();
}

void CBulletMotionShader::ReleaseObjects()
{
	CNPCShader::ReleaseObjects();
}

void CBulletMotionShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 20.0f, false));
	if (pCamera->GetMode() == THIRD_PERSON_CAMERA)
	{
		BulletPosition.y;
	}
	else if (pCamera->GetMode() == SPACESHIP_CAMERA)
	{
		BulletPosition.y;
	}
	if (m_bBulletActive == true)
	{
		CNPCShader::Render(pd3dCommandList, pCamera, nPipelineState);

		for (int i = 0; i < 3; i++)
		{
			m_ppObjects[0]->SetPosition(xmf3Position);
			m_ppObjects[i]->SetLookAt(xmf3Position, XMFLOAT3(0, 1, 0));
		}
	}
}
void CBulletMotionShader::AnimateObjects(float fTimeElapsed)
{
	////CPlayer* pPlayer = NULL;
	////m_ppObjects[0]->SetPosition(pPlayer->GetPosition());

	m_ppObjects[1]->SetPosition(BulletPosition);
	m_ppObjects[2]->SetPosition(BulletPositionR);
}


void CExplosionShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

D3D12_INPUT_LAYOUT_DESC CExplosionShader::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC CExplosionShader::CreateBlendState(int nPipelineState)
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

D3D12_RASTERIZER_DESC CExplosionShader::CreateRasterizerState()
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

D3D12_SHADER_BYTECODE CExplosionShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteAnimation", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CExplosionShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSpriteAnimation", "ps_5_1", ppd3dShaderBlob));
}
void CExplosionShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures[2];
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8);
	ppSpriteTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);
	ppSpriteTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 6, 6);
	ppSpriteTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Explosion_6x6.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppSpriteMaterials[2];
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppSpriteTextures[0]);
	ppSpriteMaterials[1] = new CMaterial();
	ppSpriteMaterials[1]->SetTexture(ppSpriteTextures[1]);

	CSpriteTexturedRectMesh* pSpriteMesh = new CSpriteTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	UINT ncbElementBytes = ((sizeof(CB_SPRITEBILLBOARD_INFO) + 255) & ~255);
	m_nObjects = 2;
	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[0], 0, 12);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[1], 0, 12);

	m_ppObjects = new CGameObject * [m_nObjects];
	XMFLOAT3 xmf3Position = XMFLOAT3(1030.0f, 180.0f, 1410.0f);
	CMultiSpriteObject* pExplodeObject = NULL;

	for (int j = 0; j < m_nObjects; j++)
	{
		pExplodeObject = new CMultiSpriteObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pExplodeObject->SetMesh(0, pSpriteMesh);
		pExplodeObject->SetMaterial(0, ppSpriteMaterials[j]);
		pExplodeObject->SetPosition(XMFLOAT3(xmf3Position.x, xmf3Position.y, xmf3Position.z));
		pExplodeObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j));
		pExplodeObject->m_fSpeed = 3.0f / (ppSpriteTextures[j]->m_nRows * ppSpriteTextures[j]->m_nCols);
		m_ppObjects[j] = pExplodeObject;
	}


}

void CExplosionShader::ReleaseUploadBuffers()
{
	CSpriteObjectShader::ReleaseUploadBuffers();

}

void CExplosionShader::ReleaseObjects()
{
	CSpriteObjectShader::ReleaseObjects();
}

void CExplosionShader::AnimateObjects(float fTimeElapsed)
{
}

void CExplosionShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bSpriteActive)
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
				m_ppObjects[j]->SetPosition(xmf3Position);
				m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
				m_ppObjects[j]->Render(pd3dCommandList, pCamera);
			}
		}

		CSpriteObjectShader::Render(pd3dCommandList, pCamera,0);

	}
}


