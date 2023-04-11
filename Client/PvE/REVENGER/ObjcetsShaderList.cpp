#include "stdafx.h"
#include "ObjcetsShaderList.h"
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
CHellicopterObjectsShader::CHellicopterObjectsShader()
{
}

CHellicopterObjectsShader::~CHellicopterObjectsShader()
{
}

void CHellicopterObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	m_nObjects = 20;
	m_ppObjects = new CGameObject * [m_nObjects];

	CLoadedModelInfo* pGunshipModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", this);

	int nColumnSpace = 5, nColumnSize = 30;
	int nFirstPassColumnSize = (m_nObjects % nColumnSize) > 0 ? (nColumnSize - 1) : nColumnSize;

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;


	for (int i = 0; i < m_nObjects; i++)
	{

		m_ppObjects[i] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pGunshipModel->m_pModelRootObject, true);

		m_ppObjects[i]->SetPosition(RandomPositionInSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), Random(20.0f, 100.0f), nColumnSize - int(floor(nColumnSize / 2.0f)), nColumnSpace));
		m_ppObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppObjects[i]->OnPrepareAnimate();

	}


	CreateShaderVariables(pd3dDevice, pd3dCommandList);

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

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
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

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera, nPipelineState);

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

void CAngrybotObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject * [m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pAngrybotModel = pModel;
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
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch * x + 390.0f, 0.0f, 730.0f + fzPitch * z);
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

void CEthanObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject * [m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pEthanModel = pModel;
	if (!pEthanModel) pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Ethan.bin", NULL);

	int nObjects = 0;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int z = -zObjects; z <= zObjects; z++)
		{
			m_ppObjects[nObjects] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pEthanModel, 1);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, (nObjects % 4));
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.25f);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackPosition(0, (nObjects % 10) * 0.35f);
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch * x + 290.0f, 0.0f, 750.0f + fzPitch * z);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[nObjects++]->SetPosition(xmf3Position);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pEthanModel) delete pEthanModel;
}

CHumanObjectsShader::CHumanObjectsShader()
{
}

CHumanObjectsShader::~CHumanObjectsShader()
{
}

void CHumanObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{

	m_nObjects = 10;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pEthanModel = pModel;
	if (!pEthanModel) pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Soldier_demo.bin", NULL);

	for (int x = 0; x <= m_nObjects; x++)
	{

		m_ppObjects[x] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pEthanModel, 1);
		m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.25f);
		m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackPosition(0, 0);
		XMFLOAT3 xmf3Position = XMFLOAT3(1610.0, 146.0f, 2250.0f);
		xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
		m_ppObjects[x]->SetScale(7.0, 7.0, 7.0);
		m_ppObjects[x]->SetPosition(xmf3Position);

	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pEthanModel) delete pEthanModel;
}

CBulletEffectShader::CBulletEffectShader()
{
}

CBulletEffectShader::~CBulletEffectShader()
{
}


D3D12_INPUT_LAYOUT_DESC CBulletEffectShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

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

D3D12_SHADER_BYTECODE CBulletEffectShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CBulletEffectShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBulletStandard", "ps_5_1", ppd3dShaderBlob));
}

void CBulletEffectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CStandardObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState);
}

D3D12_SHADER_BYTECODE CFragmentsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSParticleStandard", "ps_5_1", ppd3dShaderBlob));
}

void CFragmentsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;


	m_nObjects = 80;
	m_ppObjects = new CGameObject * [m_nObjects];
	CGameObject* pFragmentModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Sphere.bin", this);
	m_ppObjects[0] = new CMi24Object(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_ppObjects[0]->SetScale(300.0, 300.0, 300.0);
	m_ppObjects[0]->SetChild(pFragmentModel, false);
	m_ppObjects[0]->SetPosition(18.0, 55.0, -45.0);
	pFragmentModel->AddRef();
	for (int i = 1; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CMi24Object(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pFragmentModel, false);
		m_ppObjects[i]->SetScale(50.0, 50.0, 50.0);
		pFragmentModel->AddRef();
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CFragmentsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CFragmentsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera, 0);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}

}

D3D12_INPUT_LAYOUT_DESC CFragmentsShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

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

void CFragmentsShader::AnimateObjects(float fTimeElapsed)
{
	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float>uidr(50.0, 60.0);
	uniform_real_distribution<float>uidy(80.0, 90.0);
	uniform_real_distribution<float>uidz(-0.5, 0.5);

	float gravity = 9.8f;
	float time = fTimeElapsed;
	float velocity = time * uidy(dre);
	float radian = uidr(dre);
	float dirtheta = XMConvertToRadians(radian);

	//float randomX{};
	//float randomY{};
	//float randomZ{};

	//theta->Random°ª (-180 ~ 180) 
	//vx ->Random°ª t * RANDOM;


	if (m_bActive == true)
	{

		for (int j = 1; j < m_nObjects; j++)
		{
			radian = uidr(dre);
			dirtheta = XMConvertToRadians(radian);
			time += 0.01f;
			m_ppObjects[j]->m_xmf4x4ToParent._41 += (velocity * cos(dirtheta)) * time;
			m_ppObjects[j]->m_xmf4x4ToParent._42 += -0.5 * gravity * time * time + (velocity * sin(dirtheta) * time);
			m_ppObjects[j]->m_xmf4x4ToParent._43 += (velocity * cos(dirtheta)) * time;

			if (m_ppObjects[j]->m_xmf4x4ToParent._42 < 0.0f)
			{
				m_ppObjects[j]->SetPosition(65.0, 155.0, 18.0);
			}
		}
	}
}

void CFragmentsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}
