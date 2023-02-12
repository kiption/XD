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
	m_nObjects = 10;

	m_ppObjects = new CGameObject * [m_nObjects];

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	CLoadedModelInfo* pBuildingModel = pModel;
	if (!pBuildingModel) pBuildingModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pBuildingModel, 1);

	}

	m_ppObjects[0]->SetPosition(850.0f, pTerrain->GetHeight(850.0f, 1250.0f), 1250.0f);
	m_ppObjects[0]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[0]->SetScale(26.0f, 2.0f, 24.0f);

	m_ppObjects[1]->SetPosition(875.0f, pTerrain->GetHeight(875.0f, 1150.0f), 1150.0f);
	m_ppObjects[1]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[1]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[2]->SetPosition(1120.0f, pTerrain->GetHeight(1120.0f, 1325.0f), 1325.0f);
	m_ppObjects[2]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[2]->SetScale(8.0f, 2.0f, 8.0f);

	m_ppObjects[3]->SetPosition(1900.0f, pTerrain->GetHeight(1900.0f, 1400.0f), 1400.0f);
	m_ppObjects[3]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[3]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[4]->SetPosition(1160.0f, pTerrain->GetHeight(1160.0f, 1605.0f), 1605.0f);
	m_ppObjects[4]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppObjects[4]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[5]->SetPosition(1600.0f, pTerrain->GetHeight(1600.0f, 2500.0f), 2500.0f);
	m_ppObjects[5]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[5]->SetScale(20.0f, 2.0f, 28.0f);

	m_ppObjects[6]->SetPosition(1525.0f, pTerrain->GetHeight(1525.0f, 1650.0f), 1650.0f);
	m_ppObjects[6]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[6]->SetScale(8.0f, 2.0f, 8.0f);

	m_ppObjects[7]->SetPosition(1750.0f, pTerrain->GetHeight(1750.0f, 2800.0f), 2800.0f);
	m_ppObjects[7]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppObjects[7]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[8]->SetPosition(1400.0f, pTerrain->GetHeight(1400.0f, 1500.0f), 1500.0f);
	m_ppObjects[8]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[8]->SetScale(10.0f, 2.0f, 8.0f);

	m_ppObjects[9]->SetPosition(1250.0f, pTerrain->GetHeight(1250.0f, 1795.0f), 1795.0f);
	m_ppObjects[9]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppObjects[9]->SetScale(10.0f, 2.0f, 8.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
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
	}

	m_ppObjects[0]->SetPosition(420.0f, pTerrain->GetHeight(420.0f, 400.0f)-50.0f + 0.0f, 400.0f);
	m_ppObjects[0]->Rotate(0.0, 0.0f, 0.0f);
	m_ppObjects[0]->SetScale(30.0f, 10.0f, 30.0f);

	m_ppObjects[1]->SetPosition(650.0f, pTerrain->GetHeight(650.0f, 200.0f) - 50.0f + 0.0f, 200.0f);
	m_ppObjects[1]->Rotate(0.0, 180.0f, 0.0f);
	m_ppObjects[1]->SetScale(40.0f, 10.0f, 40.0f);

	m_ppObjects[2]->SetPosition(700.0f, pTerrain->GetHeight(700.0f, 400.0f) - 50.0f + 0.0f, 400.0f);
	m_ppObjects[2]->Rotate(0.0, 90.0f, 0.0f);
	m_ppObjects[2]->SetScale(15.0f, 6.0f, 15.0f);



	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pBuildingModel) delete pBuildingModel;
}
