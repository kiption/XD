#include "stdafx.h"
#include "BillboardObjectsShader.h"
#include "Stage1.h"

default_random_engine dre;
uniform_int_distribution<int>randomwidth(0, 3000);
uniform_int_distribution<int>randomHeight(10, 1550);
float RandomBillboard(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float RandomBillboard()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomBillboardPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
	float fAngle = RandomBillboard() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
	xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
	xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + RandomBillboard();
	xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}
D3D12_INPUT_LAYOUT_DESC CrossHairShader::CreateInputLayout()
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

D3D12_BLEND_DESC CrossHairShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE CrossHairShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CrossHairShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CrossHairShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/SpaceCross.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pTerrainMaterial = new CMaterial(1);
	pTerrainMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList,40.0, 40.0, 0.0f, 0.0f, 0.0f, 0.0f);


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

void CrossHairShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void CrossHairShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 120.0f, false));
	xmf3Position.y += 5.0f;
	xmf3Position.z += 30.0f;
	m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[0]->SetPosition(xmf3Position);
	BillboardShader::Render(pd3dCommandList, pCamera);
}

void CrossHairShader::AnimateObjects(float fTimeElapsed)
{
	BillboardShader::AnimateObjects(fTimeElapsed);
}

void CrossHairShader::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D3D12_INPUT_LAYOUT_DESC BillboardParticleShader::CreateInputLayout()
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

D3D12_BLEND_DESC BillboardParticleShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE BillboardParticleShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE BillboardParticleShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void BillboardParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Smoke.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 60.0f,50.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1000;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);
	m_ppObjects = new CGameObject * [m_nObjects];
	CBillboardParticleObject* pParticleObject = NULL;

	for (int i = 0; i < m_nObjects; i++)
	{
		pParticleObject = new CBillboardParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pParticleObject->SetMesh(pSpriteMesh);
		pParticleObject->SetMaterial(0, pSpriteMaterial);
		pParticleObject->SetPosition(100.0,100.0,100.0);
		m_ppObjects[i] = pParticleObject;
	}
}

void BillboardParticleShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void BillboardParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 20.0f, false));

	for (int j = 0; j < m_nObjects; j++)
	{
		//if (m_ppObjects[j]->m_xmf4x4ToParent._42 < 5.0)
		//{
		//	m_ppObjects[j]->SetPosition(0.0,400.0,0.0);
		//}
		if(m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3Position, XMFLOAT3(0.0f, 1.0, 0.0f));
	}
	BillboardShader::Render(pd3dCommandList, pCamera);
}

void BillboardParticleShader::AnimateObjects(float fTimeElapsed)
{
	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float>uid(-0.5,0.5);
	float randomX{};
	float randomY{};
	float randomZ{};

	float scale = 2.0f;
	for (int j = 0; j < m_nObjects; j++)
	{
		randomX =uid(dre);
		randomY =uid(dre);
		randomZ =uid(dre);
		XMFLOAT3 xf_Velocity = XMFLOAT3(0.001, 0.0, 0.0f);
		XMFLOAT3 xf_GravityAccel = XMFLOAT3(0.0, 1.8f, 0.0);
		float f_EmmitTime = {};
		float a_LifeTime = {};
		float f_ResetTime = {};
		float NewY{};
		fTimeElapsed += 0.003f;
		XMFLOAT3 newPosition = XMFLOAT3(0, 0, 0);
		float Time = fTimeElapsed- f_EmmitTime;
		if (Time < 0.0)
		{

		}
		else
		{
			//NewY = a_LifeTime * XMScalarModAngle(Time / a_LifeTime);
			m_ppObjects[j]->m_xmf4x4ToParent._41 = m_ppObjects[j]->m_xmf4x4ToParent._41 + xf_Velocity.x * Time + 0.5 * xf_GravityAccel.x * Time * Time+ randomX;
			m_ppObjects[j]->m_xmf4x4ToParent._42 = m_ppObjects[j]->m_xmf4x4ToParent._42 + xf_Velocity.y * Time + 0.5 * xf_GravityAccel.y * Time * Time+ randomY;
			m_ppObjects[j]->m_xmf4x4ToParent._43 = m_ppObjects[j]->m_xmf4x4ToParent._43 + xf_Velocity.z * Time + 0.5 * xf_GravityAccel.z * Time * Time+ randomZ;
		}

	}
	BillboardShader::AnimateObjects(fTimeElapsed);
}

void BillboardParticleShader::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D3D12_INPUT_LAYOUT_DESC ValkanEffectShader::CreateInputLayout()
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

D3D12_BLEND_DESC ValkanEffectShader::CreateBlendState()
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

D3D12_SHADER_BYTECODE ValkanEffectShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE ValkanEffectShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void ValkanEffectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	SceneManager* m_pScene = NULL;
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Overheat.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.0f, 14.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	m_pScene->CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);
	m_ppObjects = new CGameObject * [m_nObjects];
	CBillboardParticleObject* pRainObject = NULL;

	for (int i = 0; i < m_nObjects; i++)
	{
		pRainObject = new CBillboardParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pRainObject->SetMesh(pSpriteMesh);
		pRainObject->SetMaterial(0, pSpriteMaterial);
		m_ppObjects[i] = pRainObject;
	}
}

void ValkanEffectShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void ValkanEffectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	//xmf3PlayerPosition = pPlayer->GetPosition();

	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 80.0f, false));

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3Position, XMFLOAT3(0.0f, 0.1, 0.0f));
	}
	BillboardShader::Render(pd3dCommandList, pCamera);
}

void ValkanEffectShader::AnimateObjects(float fTimeElapsed)
{
	//BillboardShader::AnimateObjects(fTimeElapsed);
}

void ValkanEffectShader::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}
