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
inline float RandFm(float fMin, float fMax)
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}
XMVECTOR RandomHittingVectorOnSphereBillboard()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandFm(-1.0f, 1.0f), RandFm(-3.0f, 3.0f), RandFm(-1.0f, 1.0f), 0.5f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}
XMVECTOR RandomUnitVectorOnSphereBillboard()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandFm(-1.0f, 1.0f), RandFm(-0.1f, 0.1f), RandFm(-0.5f, 0.5f), 0.5f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
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
D3D12_INPUT_LAYOUT_DESC BloodMarkShader::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC BloodMarkShader::CreateBlendState(int nPipelineState)
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



void BloodMarkShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);
}

D3D12_SHADER_BYTECODE BloodMarkShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE BloodMarkShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

void BloodMarkShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/blood.dds", RESOURCE_TEXTURE2D, 0);


	CMaterial* pSpriteMaterial = new CMaterial(1);


	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);



	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.5, 2.5, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);


	m_ppObjects = new CGameObject * [m_nObjects];

	CBillboardObject** ppParticleObject = new CBillboardObject * [m_nObjects];


	for (int j = 0; j < 1; j++)
	{
		ppParticleObject[j] = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		ppParticleObject[j]->SetMesh(pSpriteMesh);
		ppParticleObject[j]->SetMaterial(0, pSpriteMaterial);
		m_ppObjects[j] = ppParticleObject[j];
	}

}

void BloodMarkShader::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void BloodMarkShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bActiveMark)
	{

		xmf3CameraPosition = pCamera->GetPosition();
		m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0, 1, 0));
		m_ppObjects[0]->SetPosition(81.f, 12.0, 800.f);
		BillboardShader::Render(pd3dCommandList, pCamera, nPipelineState);
	}
}

void BloodMarkShader::AnimateObjects(float fTimeElapsed)
{

	BillboardShader::AnimateObjects(fTimeElapsed);
}

void BloodMarkShader::ReleaseUploadBuffers()
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

D3D12_DEPTH_STENCIL_DESC BillboardParticleShader::CreateDepthStencilState(int nPipelinestates)
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

void BillboardParticleShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);

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
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/40Smoke2.dds", RESOURCE_TEXTURE2D, 0);
	CTexture* ppSpriteTextures2 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures2->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/40Smoke2.dds", RESOURCE_TEXTURE2D, 0);
	CTexture* ppSpriteTextures3 = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures3->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/40Smoke2.dds", RESOURCE_TEXTURE2D, 0);

	pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial2 = new CMaterial(1);
	pSpriteMaterial3 = new CMaterial(1);

	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);
	pSpriteMaterial2->SetTexture(ppSpriteTextures2, 0);
	pSpriteMaterial3->SetTexture(ppSpriteTextures3, 0);

	m_fWidth = 10.0f;
	m_fHeight = 10.0f;
	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, m_fWidth, m_fHeight, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 20;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures2, 0, 15);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures3, 0, 15);

	m_ppObjects = new CGameObject * [m_nObjects];

	CBillboardParticleObject** ppParticleObject = new CBillboardParticleObject * [m_nObjects];


	for (int j = 0; j < m_nObjects; j++)
	{
		ppParticleObject[j] = new CBillboardParticleObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		ppParticleObject[j]->SetMesh(pSpriteMesh);
		ppParticleObject[j]->SetMaterial(0, pSpriteMaterial);
		ppParticleObject[j]->SetPosition(0.0, 0.0, 0.0);
		m_ppObjects[j] = ppParticleObject[j];
	}
	NextPosition = XMFLOAT3(0.0, 0.0, 0.0);


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

		if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3Position, XMFLOAT3(0.0f, 1.0, 0.0f));

	}
	BillboardShader::Render(pd3dCommandList, pCamera, 0);
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
		XMFLOAT3 xf_GravityAccel = XMFLOAT3(0.0, 5.8f, 0.0);
		float f_EmmitTime = 2.0;
		float a_LifeTime = 1.0;
		float f_ResetTime = {};
		float NewY{};
		fTimeElapsed += 0.0345;

		float Time = fTimeElapsed - f_EmmitTime;
		if (Time < 0.0)
		{

		}
		else
		{

			randomLiftHeight = uidha(dre);
			randomLiftHeighiest = uidhs(dre);
			Time = a_LifeTime * XMScalarModAngle(Time / a_LifeTime);
			newPosition.x = m_ppObjects[j]->m_xmf4x4ToParent._41 + xf_Velocity.x * Time + 0.5 * xf_GravityAccel.x * Time * Time + randomX;
			newPosition.y = m_ppObjects[j]->m_xmf4x4ToParent._42 + xf_Velocity.y * Time + 0.5 * xf_GravityAccel.y * Time * Time + randomY;
			newPosition.z = m_ppObjects[j]->m_xmf4x4ToParent._43 + xf_Velocity.z * Time + 0.5 * xf_GravityAccel.z * Time * Time + randomZ;
			if (newPosition.y > 800.0)
			{
				newPosition.y = NextPosition.y;
				newPosition.x = NextPosition.x;
				newPosition.z = NextPosition.z;
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


D3D12_BLEND_DESC MuzzleFlameBillboard::CreateBlendState(int nPipelineState)
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

void MuzzleFlameBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);

}


D3D12_SHADER_BYTECODE MuzzleFlameBillboard::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSmokeBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE MuzzleFlameBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

void MuzzleFlameBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/MuzzleFlame.dds", RESOURCE_TEXTURE2D, 0);


	CMaterial* pSpriteMaterial = new CMaterial(1);

	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);

	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);

	m_ppObjects = new CGameObject * [m_nObjects];
	CResponeObject** pResponObject = new CResponeObject * [m_nObjects];
	for (int j = 0; j < m_nObjects; j++)
	{
		pResponObject[j] = new CResponeObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pResponObject[j]->SetMesh(pSpriteMesh);
		pResponObject[j]->SetMaterial(0, pSpriteMaterial);
		m_ppObjects[j] = pResponObject[j];
	}


}

void MuzzleFlameBillboard::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void MuzzleFlameBillboard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState, XMFLOAT3 Look, XMFLOAT3 Position)
{
	if (m_bShotActive == true)
	{

		CPlayer* pPlayer = pCamera->GetPlayer();
		XMFLOAT3 CameraPosition = pCamera->GetPosition();
		XMFLOAT3 CameraLook = pCamera->GetLookVector();
		XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
		XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
		XMFLOAT3 xmf3Position = Vector3::Add(Position, Vector3::ScalarProduct(CameraLook, 4.0f, false));
		xmf3Position.y += 0.4f;
		xmf3Position.x -= 0.02f;
		for (int j = 0; j < m_nObjects; j++)
		{

			if (m_ppObjects[j])m_ppObjects[j]->SetPosition(xmf3Position);
			if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(CameraPosition, XMFLOAT3(0.0f, 1.0, 0.0f));

			BillboardShader::Render(pd3dCommandList, pCamera, 0);
		}
	}
}

void MuzzleFlameBillboard::AnimateObjects(float fTimeElapsed)
{
	BillboardShader::AnimateObjects(fTimeElapsed);
}

void MuzzleFlameBillboard::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}



D3D12_BLEND_DESC SparkBillboard::CreateBlendState(int nPipelineState)
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

void SparkBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);

}


D3D12_SHADER_BYTECODE SparkBillboard::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSmokeBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE SparkBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

void SparkBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/Overheat.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);
	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.15f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_nObjects = EXPLOSION_SPARK;
	m_ppObjects = new CGameObject * [m_nObjects];
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);

	CBillboardObject** pSparkObject = new CBillboardObject * [m_nObjects];
	for (int j = 0; j < m_nObjects; j++)
	{
		pSparkObject[j] = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pSparkObject[j]->SetMesh(pSpriteMesh);
		pSparkObject[j]->SetMaterial(0, pSpriteMaterial);
		pSparkObject[j]->SetPosition(XMFLOAT3(330.0, 40.0, -230.0));
		m_ppObjects[j] = pSparkObject[j];

	}

	for (int i = 0; i < EXPLOSION_SPARK; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomUnitVectorOnSphereBillboard());
}

void SparkBillboard::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void SparkBillboard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 0.0f, false));

	for (int j = 0; j < m_nObjects; j++)
	{
		//ParticlePosition = XMFLOAT3(58.0f,12.0f,800.0f);
		if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0, 1.0f));

	}
	BillboardShader::Render(pd3dCommandList, pCamera, 0);
}

void SparkBillboard::AnimateObjects(float fTimeElapsed)
{

	if (m_bActive == true)
	{
		XMFLOAT3 gravity = XMFLOAT3(0, -9.8f, +9.5);
		m_fElapsedTimes += fTimeElapsed * 3.0f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < EXPLOSION_SPARK; i++)
			{
				gravity = XMFLOAT3(0, -RandomBillboard(2.0f, 3.0f), 0);
				m_fExplosionSpeed = RandomBillboard(2.0f, 10.0f);

				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.y * m_fElapsedTimes * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.z * m_fElapsedTimes * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->m_xmf4x4ToParent._41 = m_pxmf4x4Transforms[i]._41;
				m_ppObjects[i]->m_xmf4x4ToParent._42 = m_pxmf4x4Transforms[i]._42;
				m_ppObjects[i]->m_xmf4x4ToParent._43 = m_pxmf4x4Transforms[i]._43;

			}
		}
		else
		{
			m_fElapsedTimes = 0.0f;
		}
	}


	BillboardShader::AnimateObjects(fTimeElapsed);
}

void SparkBillboard::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

D3D12_BLEND_DESC BloodHittingBillboard::CreateBlendState(int nPipelineState)
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

void BloodHittingBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);

}


D3D12_SHADER_BYTECODE BloodHittingBillboard::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSmokeBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE BloodHittingBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}
void BloodHittingBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/blood.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);
	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 5.5f, 5.5f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_nObjects = 10;
	m_ppObjects = new CGameObject * [m_nObjects];
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);

	CBillboardObject** pSparkObject = new CBillboardObject * [m_nObjects];
	for (int j = 0; j < m_nObjects; j++)
	{
		pSparkObject[j] = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pSparkObject[j]->SetMesh(pSpriteMesh);
		pSparkObject[j]->SetMaterial(0, pSpriteMaterial);
		m_ppObjects[j] = pSparkObject[j];

	}

	for (int i = 0; i < m_nObjects; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomHittingVectorOnSphereBillboard());
}

void BloodHittingBillboard::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void BloodHittingBillboard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 0.0f, false));

	for (int j = 0; j < m_nObjects; j++)
	{
		//ParticlePosition = XMFLOAT3(58.0f, 12.0f, 900.0f);
		if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0, 0.0f));

	}
	BillboardShader::Render(pd3dCommandList, pCamera, 0);
}

void BloodHittingBillboard::AnimateObjects(float fTimeElapsed)
{
	if (m_bActive == true)
	{
		XMFLOAT3 gravity = XMFLOAT3(-8.8f, 9.8f, 0);
		m_fElapsedTimes += fTimeElapsed * 7.0f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < m_nObjects; i++)
			{
				gravity = XMFLOAT3(-9.8f, 9.8f, 0);
				m_fExplosionSpeed = +RandomBillboard(3.0f, 8.1f);

				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.x * m_fElapsedTimes * m_fElapsedTimes;;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.y * m_fElapsedTimes * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->m_xmf4x4ToParent._41 = m_pxmf4x4Transforms[i]._41;
				m_ppObjects[i]->m_xmf4x4ToParent._42 = m_pxmf4x4Transforms[i]._42;
				m_ppObjects[i]->m_xmf4x4ToParent._43 = m_pxmf4x4Transforms[i]._43;

			}
		}
		else
		{
			m_fElapsedTimes = 0.0f;
			m_bActive = false;
		}
	}





}

void BloodHittingBillboard::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////

D3D12_INPUT_LAYOUT_DESC BulletMarkBillboard::CreateInputLayout(int nPipelineState)
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

D3D12_BLEND_DESC BulletMarkBillboard::CreateBlendState(int nPipelineState)
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

void BulletMarkBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);

}

D3D12_SHADER_BYTECODE BulletMarkBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE BulletMarkBillboard::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

void BulletMarkBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/BulletMark.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);
	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 2.5, 2.5, 0.0f, 0.0f, 0.0f, 0.0f);
	m_nObjects = 1;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);
	m_ppObjects = new CGameObject * [m_nObjects];
	CBillboardObject** ppParticleObject = new CBillboardObject * [m_nObjects];
	for (int j = 0; j < 1; j++)
	{
		ppParticleObject[j] = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		ppParticleObject[j]->SetMesh(pSpriteMesh);
		ppParticleObject[j]->SetMaterial(0, pSpriteMaterial);
		m_ppObjects[j] = ppParticleObject[j];
	}
}

void BulletMarkBillboard::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void BulletMarkBillboard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bActive == true)
	{
		xmf3CameraPosition = pCamera->GetPosition();
		m_ppObjects[0]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0, 1, 0));
		BillboardShader::Render(pd3dCommandList, pCamera, nPipelineState);
	}

}

void BulletMarkBillboard::AnimateObjects(float fTimeElapsed)
{
	if (m_bActive == true)
	{
		XMFLOAT3 gravity = XMFLOAT3(-9.8f, 9.8f, 0);
		m_fElapsedTimes += fTimeElapsed * 5.0f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < m_nObjects; i++)
			{
				gravity = XMFLOAT3(0, 0, 0);
				m_fExplosionSpeed = 2.0;

				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], 1.0 * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->m_xmf4x4ToParent._41 = m_pxmf4x4Transforms[i]._41;
				m_ppObjects[i]->m_xmf4x4ToParent._42 = m_pxmf4x4Transforms[i]._42;
				m_ppObjects[i]->m_xmf4x4ToParent._43 = m_pxmf4x4Transforms[i]._43;

			}
		}
		else
		{
			m_fElapsedTimes = 0.0f;
		}
	}

	BillboardShader::AnimateObjects(fTimeElapsed);
}

void BulletMarkBillboard::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}



//
D3D12_BLEND_DESC HeliHittingMarkBillboard::CreateBlendState(int nPipelineState)
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

void HeliHittingMarkBillboard::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);

}


D3D12_SHADER_BYTECODE HeliHittingMarkBillboard::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSmokeBillBoardTextured", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE HeliHittingMarkBillboard::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", ppd3dShaderBlob));
}
void HeliHittingMarkBillboard::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CTexture* ppSpriteTextures = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	ppSpriteTextures->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/40Smoke.dds", RESOURCE_TEXTURE2D, 0);
	CMaterial* pSpriteMaterial = new CMaterial(1);
	pSpriteMaterial->SetTexture(ppSpriteTextures, 0);
	CTexturedRectMesh* pSpriteMesh;
	pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_nObjects = HITTINGMARKS;
	m_ppObjects = new CGameObject * [m_nObjects];
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SceneManager::CreateShaderResourceViews(pd3dDevice, ppSpriteTextures, 0, 15);

	CBillboardObject** pSparkObject = new CBillboardObject * [m_nObjects];
	for (int j = 0; j < m_nObjects; j++)
	{
		pSparkObject[j] = new CBillboardObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		pSparkObject[j]->SetMesh(pSpriteMesh);
		pSparkObject[j]->SetMaterial(0, pSpriteMaterial);
		m_ppObjects[j] = pSparkObject[j];

	}

	for (int i = 0; i < m_nObjects; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomHittingVectorOnSphereBillboard());
}

void HeliHittingMarkBillboard::ReleaseObjects()
{
	BillboardShader::ReleaseObjects();
}

void HeliHittingMarkBillboard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CPlayer* pPlayer = pCamera->GetPlayer();
	XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
	XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
	XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
	XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 0.0f, false));

	for (int j = 0; j < m_nObjects; j++)
	{
		//ParticlePosition = XMFLOAT3(58.0f, 12.0f, 900.0f);
		if (m_ppObjects[j])m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0, 0.0f));

	}
	BillboardShader::Render(pd3dCommandList, pCamera, 0);
}

void HeliHittingMarkBillboard::AnimateObjects(float fTimeElapsed)
{
	if (m_bActive == true)
	{
		XMFLOAT3 gravity = XMFLOAT3(0.0f, 9.8f, 0);
		m_fElapsedTimes += fTimeElapsed * 2.0f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < m_nObjects; i++)
			{
				gravity = XMFLOAT3(0.0f, 9.8f, 0);
				m_fExplosionSpeed = +RandomBillboard(3.0f, 5.1f);

				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.y * m_fElapsedTimes * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->m_xmf4x4ToParent._41 = m_pxmf4x4Transforms[i]._41;
				m_ppObjects[i]->m_xmf4x4ToParent._42 = m_pxmf4x4Transforms[i]._42;
				m_ppObjects[i]->m_xmf4x4ToParent._43 = m_pxmf4x4Transforms[i]._43;

			}
		}
		else
		{
			m_fElapsedTimes = 0.0f;
			m_bActive = false;
		}
	}





}

void HeliHittingMarkBillboard::ReleaseUploadBuffers()
{
	BillboardShader::ReleaseUploadBuffers();
}

