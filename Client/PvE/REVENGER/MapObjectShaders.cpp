#include "stdafx.h"
#include "MapObjectShaders.h"

WallObjectShaders::WallObjectShaders()
{
}

WallObjectShaders::~WallObjectShaders()
{
}

void WallObjectShaders::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	m_nObjects = 13;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);

	}

	m_ppObjects[0]->SetPosition(850.0f, pTerrain->GetHeight(850.0f, 1250.0f), 1250.0f);
	m_ppObjects[0]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[0]->SetScale(16.0f, 6.0f, 14.0f);

	m_ppObjects[1]->SetPosition(875.0f, pTerrain->GetHeight(875.0f, 1150.0f), 1150.0f);
	m_ppObjects[1]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[1]->SetScale(10.0f, 6.0f, 18.0f);

	m_ppObjects[2]->SetPosition(1120.0f, pTerrain->GetHeight(1120.0f, 1325.0f), 1325.0f);
	m_ppObjects[2]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[2]->SetScale(18.0f, 6.0f, 18.0f);

	m_ppObjects[3]->SetPosition(1900.0f, pTerrain->GetHeight(1900.0f, 1400.0f), 1400.0f);
	m_ppObjects[3]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[3]->SetScale(10.0f, 7.0f, 18.0f);

	m_ppObjects[4]->SetPosition(1160.0f, pTerrain->GetHeight(1160.0f, 1605.0f), 1605.0f);
	m_ppObjects[4]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[4]->SetScale(10.0f, 5.0f, 8.0f);

	m_ppObjects[5]->SetPosition(1600.0f, pTerrain->GetHeight(1600.0f, 2500.0f), 2500.0f);
	m_ppObjects[5]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[5]->SetScale(10.0f, 5.0f, 8.0f);

	m_ppObjects[6]->SetPosition(1525.0f, pTerrain->GetHeight(1525.0f, 1650.0f), 1650.0f);
	m_ppObjects[6]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[6]->SetScale(8.0f, 2.0f, 18.0f);

	m_ppObjects[7]->SetPosition(1750.0f, pTerrain->GetHeight(1750.0f, 2800.0f), 2800.0f);
	m_ppObjects[7]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[7]->SetScale(10.0f, 6.0f, 18.0f);

	m_ppObjects[8]->SetPosition(1400.0f, pTerrain->GetHeight(1400.0f, 1500.0f), 1500.0f);
	m_ppObjects[8]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[8]->SetScale(10.0f, 6.0f, 8.0f);

	m_ppObjects[9]->SetPosition(1250.0f, pTerrain->GetHeight(1250.0f, 1795.0f), 1795.0f);
	m_ppObjects[9]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[9]->SetScale(10.0f, 2.0f, 18.0f);

	m_ppObjects[10]->SetPosition(250.0f, pTerrain->GetHeight(250.0f, 795.0f), 795.0f);
	m_ppObjects[10]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[10]->SetScale(10.0f, 2.0f, 18.0f);

	m_ppObjects[11]->SetPosition(1250.0f, pTerrain->GetHeight(1250.0f, 2225.0f), 2225.0f);
	m_ppObjects[11]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[11]->SetScale(10.0f, 2.0f, 18.0f);

	m_ppObjects[12]->SetPosition(1050.0f, pTerrain->GetHeight(1050.0f, 1595.0f), 1595.0f);
	m_ppObjects[12]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[12]->SetScale(10.0f, 2.0f, 18.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}

void BuildingObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	m_nObjects = 5;
	m_ppObjects = new CGameObject * [m_nObjects];

	CGameObject* pSuperCobraModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GameObject.bin", this);
	pSuperCobraModel->SetScale(2.0, 2.0, 2.0);


	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pSuperCobraModel,false);
		m_ppObjects[i]->SetPosition(1000.0+(i*50), 500.0, 1000.0);
		m_ppObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppObjects[i]->SetScale(5.0f, 5.0f, 5.0f);
		pSuperCobraModel->AddRef();
	}


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void BunkerObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	m_nObjects = 3;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/bunker_1_fbx.bin", NULL);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);
		m_ppObjects[i]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppObjects[i]->SetScale(40.0, 35.0f, 40.0f);
	}

	m_ppObjects[0]->SetPosition(420.0f, pTerrain->GetHeight(420.0f, 1100.0f)-15.0f, 1100.0f);
	m_ppObjects[0]->Rotate(55.0, 0.0f, 0.0f);

	m_ppObjects[1]->SetPosition(650.0f, pTerrain->GetHeight(650.0f, 1800.0f) - 15.0f, 1800.0f);
	m_ppObjects[1]->Rotate(55.0, 0.0f, 0.0f);

	m_ppObjects[2]->SetPosition(1700.0f, pTerrain->GetHeight(1700.0f, 1400.0f) - 15.0f, 1400.0f);
	m_ppObjects[2]->Rotate(55.0, 0.0f, 0.0f);




	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}

/////////////////////////////////////////////
void CMapObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	int GeneratorModels = 4;
	int WallModels = 6;
	m_nObjects = GeneratorModels + WallModels;
	m_ppObjects = new CGameObject * [m_nObjects];
	
	CGameObject* pGeneratorModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Generator_Room.bin", NULL);
	for (int i = 0; i < GeneratorModels; i++)
	{
		m_ppObjects[i] = new CMi24Object(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pGeneratorModel,false);

		m_ppObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppObjects[i]->SetScale(50.0, 50.0, 50.0);
		pGeneratorModel->AddRef();
	}
	m_ppObjects[0]->SetPosition(XMFLOAT3(3000.0f , pTerrain->GetHeight(3000.0,2000.0f) + 700.0f, 2000.0f));
	m_ppObjects[1]->SetPosition(XMFLOAT3(1200.0f , pTerrain->GetHeight(1200.0f, 2700.0f), 2700.0f));
	m_ppObjects[2]->SetPosition(XMFLOAT3(2000.0f , pTerrain->GetHeight(2000.0f, 1500.0f), 1500.0f));
	m_ppObjects[3]->SetPosition(XMFLOAT3(750.0f , pTerrain->GetHeight(750.0f, 2400.0f), 2400.0f));

	CGameObject* pWallModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	for (int i = GeneratorModels; i < WallModels+ GeneratorModels; i++)
	{
		m_ppObjects[i] = new CMi24Object(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pWallModel, false);
		m_ppObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppObjects[i]->SetScale(10.0, 10.0, 10.0);
		pWallModel->AddRef();
	}
	m_ppObjects[4]->SetPosition(XMFLOAT3(3500.0f, pTerrain->GetHeight(3500.0f, 2600.0f), 2600.0f));
	m_ppObjects[5]->SetPosition(XMFLOAT3(1700.0f, pTerrain->GetHeight(1700.0f, 2200.0f), 2200.0f));
	m_ppObjects[6]->SetPosition(XMFLOAT3(2700.0f, pTerrain->GetHeight(2700.0f, 500.0f), 500.0f));
	m_ppObjects[7]->SetPosition(XMFLOAT3(1750.0f, pTerrain->GetHeight(1750.0f, 2900.0f), 2900.0f));
	m_ppObjects[8]->SetPosition(XMFLOAT3(2500.0f, pTerrain->GetHeight(2500.0f, 1300.0f), 1300.0f));
	m_ppObjects[9]->SetPosition(XMFLOAT3(1900.0f, pTerrain->GetHeight(1900.0f, 2700.0f), 2700.0f));


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

D3D12_BLEND_DESC CMapObjectShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;

	return (d3dBlendDesc);
}



void CMapObjectShader::AnimateObjects(float fTimeElapsed)
{

}
void CMapObjectShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CMapObjectShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CMapObjectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
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