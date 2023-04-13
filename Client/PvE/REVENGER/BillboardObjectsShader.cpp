#include "stdafx.h"
#include "BillboardObjectsShader.h"
#include "Stage1.h"
#include "Scene.h"
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
D3D12_INPUT_LAYOUT_DESC CrossHairShader::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC CrossHairShader::CreateBlendState(int nPipelineState)
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

void CrossHairShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
}

D3D12_SHADER_BYTECODE CrossHairShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CrossHairShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

void CrossHairShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/SpaceCross.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pTerrainMaterial = new CMaterial(1);
	pTerrainMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 40.0, 40.0, 0.0f, 0.0f, 0.0f, 0.0f);


	m_nObjects = 1;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < m_nObjects; i++)
	{
		SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);//+
		m_ppObjects = new CGameObject * [m_nObjects];
		CBillboardObject* pThirdObject = NULL;
		pThirdObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pThirdObject->SetMesh(pSpriteMesh);
		pThirdObject->SetMaterial(0, pTerrainMaterial);
		m_ppObjects[i] = pThirdObject;
	}
}

void CrossHairShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void CrossHairShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
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
	BillboardShader::Render(pd3dCommandList, pCamera,0);
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

D3D12_INPUT_LAYOUT_DESC BillboardParticleShader::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC BillboardParticleShader::CreateBlendState(int nPipelineState)
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

void BillboardParticleShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);

}

D3D12_SHADER_BYTECODE BillboardParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE BillboardParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSmokeBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

void BillboardParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/smoke2.dds", RESOURCE_TEXTURE2D, 0);
	CTexture* ppSpriteTextures2 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures2->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/smoke2.dds", RESOURCE_TEXTURE2D, 0);
	CTexture* ppSpriteTextures3 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures3->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/smoke2.dds", RESOURCE_TEXTURE2D, 0);

	pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial2 = new CMaterial(1);
	pSpriteMaterial3 = new CMaterial(1);

	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);
	pSpriteMaterial2->SetTexture(ppSpriteTextures2, 0);
	pSpriteMaterial3->SetTexture(ppSpriteTextures3, 0);

	m_fWidth = 20.0f;
	m_fHeight = 20.0f;
	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, m_fWidth, m_fHeight, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 200;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures2, 0, 15);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures3, 0, 15);

	m_ppObjects = new CGameObject * [m_nObjects];

	CBillboardParticleObject** ppParticleObject = new CBillboardParticleObject * [m_nObjects];

	
		for (int j = 0; j < m_nObjects-200; j++)
		{
			m_ppObjects[j] = new CBillboardParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			m_ppObjects[j]->SetMesh(pSpriteMesh);
			m_ppObjects[j]->SetMaterial(0, pSpriteMaterial);
			m_ppObjects[j]->SetPosition(0.0,0.0,0.0);

		}
		for (int k = m_nObjects-200; k < m_nObjects-100; k++)
		{
			m_ppObjects[k] = new CBillboardParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			m_ppObjects[k]->SetMesh(pSpriteMesh);
			m_ppObjects[k]->SetMaterial(0, pSpriteMaterial2);
			m_ppObjects[k]->SetPosition(0.0,0.0,0.0);
		
		}
		for (int k = m_nObjects-100; k < m_nObjects; k++)
		{
			m_ppObjects[k] = new CBillboardParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			m_ppObjects[k]->SetMesh(pSpriteMesh);
			m_ppObjects[k]->SetMaterial(0, pSpriteMaterial3);
			m_ppObjects[k]->SetPosition(0.0, 0.0, 0.0);

		}
		NextPosition = XMFLOAT3(0.0,0.0,0.0);

	
}

void BillboardParticleShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void BillboardParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 0.0f, false));

	for (int j = 0; j < m_nObjects; j++)
	{
		/*if (m_ppObjects[j]->m_xmf4x4ToParent._42 > randomLiftHeight)
		{
			m_ppObjects[j]->SetMaterial(0, pSpriteMaterial2);
		}
		if (m_ppObjects[j]->m_xmf4x4ToParent._42 > randomLiftHeighiest)
		{
			m_ppObjects[j]->SetMaterial(0, pSpriteMaterial2);
		}*/
		if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3Position, XMFLOAT3(0.0f, 0.1, 0.0f));
		
	}
	BillboardShader::Render(pd3dCommandList, pCamera,0);
}

void BillboardParticleShader::AnimateObjects(float fTimeElapsed)
{
	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float>uidx(-0.5, 0.5);
	uniform_real_distribution<float>uidy(-0.0, 0.3);
	uniform_real_distribution<float>uidz(-0.5, 0.5);
	uniform_real_distribution<float>uidha(400.0, 500.0);
	uniform_real_distribution<float>uidhs(600.5, 800.0);

	float randomX{};
	float randomY{};
	float randomZ{};
	uniform_real_distribution<float>uidw(15.0, 30.0);
	uniform_real_distribution<float>uidh(15.0, 30.0);



	for (int j = 0; j < m_nObjects; j++)
	{
		

		randomX = uidx(dre);
		randomY = uidy(dre);
		randomZ = uidz(dre);
		XMFLOAT3 xf_Velocity = XMFLOAT3(0.001, 0.0, 0.0f);
		XMFLOAT3 xf_GravityAccel = XMFLOAT3(0.0, 1.8f, 0.0);
		float f_EmmitTime = 2.0;
		float a_LifeTime = 1.0;
		float f_ResetTime = {};
		float NewY{};
		fTimeElapsed += 0.0165;
	
		float Time = fTimeElapsed - f_EmmitTime;
		if (Time < 0.0)
		{

		}
		else
		{
			m_fWidth = 1.0f;
			m_fHeight = 1.0f;
		
			randomLiftHeight = uidha(dre);
			randomLiftHeighiest = uidhs(dre);
			Time = a_LifeTime * XMScalarModAngle(Time / a_LifeTime);
			newPosition.x = m_ppObjects[j]->m_xmf4x4ToParent._41 + xf_Velocity.x * Time + 0.5 * xf_GravityAccel.x * Time * Time + randomX;
			newPosition.y = m_ppObjects[j]->m_xmf4x4ToParent._42 + xf_Velocity.y * Time + 0.5 * xf_GravityAccel.y * Time * Time + randomY;
			newPosition.z = m_ppObjects[j]->m_xmf4x4ToParent._43 + xf_Velocity.z * Time + 0.5 * xf_GravityAccel.z * Time * Time + randomZ;
			if (newPosition.y > 800.0)
			{
				newPosition.y= NextPosition.y;
				newPosition.x= NextPosition.x;
				newPosition.z= NextPosition.z;
			}
			m_ppObjects[j]->m_xmf4x4ToParent._43 = newPosition.z;
			m_ppObjects[j]->m_xmf4x4ToParent._42 = newPosition.y;
			m_ppObjects[j]->m_xmf4x4ToParent._41 = newPosition.x;
		}

	}
	BillboardShader::AnimateObjects(fTimeElapsed);
}

void BillboardParticleShader::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D12_INPUT_LAYOUT_DESC OpeningBillboardBanner::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC OpeningBillboardBanner::CreateBlendState(int nPipelineState)
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

void OpeningBillboardBanner::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);

}

D3D12_SHADER_BYTECODE OpeningBillboardBanner::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE OpeningBillboardBanner::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

void OpeningBillboardBanner::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Opening.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pTerrainMaterial = new CMaterial(1);
	pTerrainMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 120.0, 80.0, 0.0f, 0.0f, 0.0f, 0.0f);


	m_nObjects = 1;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	for (int i = 0; i < m_nObjects; i++)
	{
		SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);//+
		m_ppObjects = new CGameObject * [m_nObjects];
		CBillboardObject* pThirdObject = NULL;
		pThirdObject = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pThirdObject->SetMesh(pSpriteMesh);
		pThirdObject->SetMaterial(0, pTerrainMaterial);
		m_ppObjects[i] = pThirdObject;
	}
}

void OpeningBillboardBanner::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void OpeningBillboardBanner::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = pCamera->GetLookVector();
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3CameraPosition, Vector3::ScalarProduct(xmf3CameraLook, 120.0f, false));
	xmf3Position.y += 5.0f;
	xmf3Position.x -= 10.0f;
	xmf3Position.z += 80.0f;
	m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[0]->SetPosition(xmf3Position);
	BillboardShader::Render(pd3dCommandList, pCamera, 0);
}

void OpeningBillboardBanner::AnimateObjects(float fTimeElapsed)
{
	BillboardShader::AnimateObjects(fTimeElapsed);
}

void OpeningBillboardBanner::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}