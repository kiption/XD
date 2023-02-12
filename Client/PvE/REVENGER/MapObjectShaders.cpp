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
	m_nObjects = 10;

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
	m_ppObjects[0]->SetScale(6.0f, 2.0f, 4.0f);

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
	m_ppObjects[5]->SetScale(10.0f, 2.0f, 8.0f);

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
