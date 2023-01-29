//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

ID3D12DescriptorHeap* CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.8f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 1500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 5.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 1600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 5.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	m_pLights[4].m_bEnable = true;
	m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights[4].m_fRange = 200.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 3*6+3*13+1*7); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(4.0f, 1.0f, 4.0f);
	XMFLOAT3 xmf3Normal(0.0f, 0.2, 0.0);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Stage2.raw"), 257, 257, xmf3Scale, xmf3Normal);

	m_nHierarchicalGameObjects = 63;
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];

	CLoadedModelInfo* pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[0] = new CAngrybotObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pAngrybotModel, 1);
	m_ppHierarchicalGameObjects[0]->SetPosition(410.0f, m_pTerrain->GetHeight(410.0f, 735.0f), 735.0f);
	if (pAngrybotModel) delete pAngrybotModel;

	CLoadedModelInfo* pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[1] = new CEthanObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, 1);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[1]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 670.0f), 670.0f);
	m_ppHierarchicalGameObjects[1]->SetScale(3.0f, 3.0f, 3.0f);
	if (pEthanModel) delete pEthanModel;

	CLoadedModelInfo* pGSModels1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[2] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGSModels1, 1);
	m_ppHierarchicalGameObjects[2]->SetPosition(450.0f, m_pTerrain->GetHeight(450.0f, 450.0f), 450.0f);
	m_ppHierarchicalGameObjects[2]->SetScale(3.0f, 3.0f, 3.0f);
	if (pGSModels1) delete pGSModels1;

	CLoadedModelInfo* pGSModels2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[3] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGSModels2, 1);
	m_ppHierarchicalGameObjects[3]->SetPosition(450.0f, m_pTerrain->GetHeight(450.0f, 550.0f), 550.0f);
	m_ppHierarchicalGameObjects[3]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[3]->SetScale(3.0f, 3.0f, 3.0f);
	if (pGSModels2) delete pGSModels2;

	CLoadedModelInfo* pGSModels3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[4] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGSModels3, 1);
	m_ppHierarchicalGameObjects[4]->SetPosition(550.0f, m_pTerrain->GetHeight(550.0f, 550.0f), 550.0f);
	m_ppHierarchicalGameObjects[4]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[4]->SetScale(3.0f, 3.0f, 3.0f);
	if (pGSModels3) delete pGSModels3;

	CLoadedModelInfo* pGSModels4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[5] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGSModels4, 1);
	m_ppHierarchicalGameObjects[5]->SetPosition(880.0f, m_pTerrain->GetHeight(880.0f, 880.0f), 880.0f);
	m_ppHierarchicalGameObjects[5]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppHierarchicalGameObjects[5]->SetScale(3.0f, 3.0f, 3.0f);
	if (pGSModels4) delete pGSModels4;


	CLoadedModelInfo* pCSModels1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels7 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels8 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels9 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels10 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels11 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels12 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pCSModels13 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);

	m_ppHierarchicalGameObjects[6] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels1, 1);
	m_ppHierarchicalGameObjects[6]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 90.0f);
	m_ppHierarchicalGameObjects[6]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[6]->SetScale(6.0f, 2.0f, 2.0f);
	if (pCSModels1) delete pCSModels1;

	m_ppHierarchicalGameObjects[7] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels2, 1);
	m_ppHierarchicalGameObjects[7]->SetPosition(300.0f, m_pTerrain->GetHeight(300.0f, 50.0f), 60.0f);
	m_ppHierarchicalGameObjects[7]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[7]->SetScale(6.0f, 2.0f, 3.0f);
	if (pCSModels2) delete pCSModels2;

	m_ppHierarchicalGameObjects[8] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels3, 1);
	m_ppHierarchicalGameObjects[8]->SetPosition(450.0f, m_pTerrain->GetHeight(450.0f, 40.0f), 150.0f);
	m_ppHierarchicalGameObjects[8]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppHierarchicalGameObjects[8]->SetScale(20.0f, 2.0f, 3.0f);
	if (pCSModels3) delete pCSModels3;

	m_ppHierarchicalGameObjects[9] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels4, 1);
	m_ppHierarchicalGameObjects[9]->SetPosition(300.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 200.0f);
	m_ppHierarchicalGameObjects[9]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[9]->SetScale(15.0f, 2.0f, 3.0f);
	if (pCSModels4) delete pCSModels4;
	m_ppHierarchicalGameObjects[10] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels5, 1);
	m_ppHierarchicalGameObjects[10]->SetPosition(550.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 275.0f);
	m_ppHierarchicalGameObjects[10]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[10]->SetScale(15.0f, 2.0f, 3.0f);
	if (pCSModels5) delete pCSModels5;

	m_ppHierarchicalGameObjects[11] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels6, 1);
	m_ppHierarchicalGameObjects[11]->SetPosition(150.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 200.0f);
	m_ppHierarchicalGameObjects[11]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[11]->SetScale(12.0f, 2.0f, 3.0f);
	if (pCSModels6) delete pCSModels6;

	m_ppHierarchicalGameObjects[12] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels7, 1);
	m_ppHierarchicalGameObjects[12]->SetPosition(150.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 100.0f);
	m_ppHierarchicalGameObjects[12]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[12]->SetScale(15.0f, 1.5f, 3.0f);
	if (pCSModels7) delete pCSModels7;

	m_ppHierarchicalGameObjects[13] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels8, 1);
	m_ppHierarchicalGameObjects[13]->SetPosition(110.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 240.0f);
	m_ppHierarchicalGameObjects[13]->Rotate(0.0f, 300.0f, 0.0f);
	m_ppHierarchicalGameObjects[13]->SetScale(8.0f, 2.0f, 3.0f);
	if (pCSModels8) delete pCSModels8;

	m_ppHierarchicalGameObjects[14] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels9, 1);
	m_ppHierarchicalGameObjects[14]->SetPosition(250.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 350.0f);
	m_ppHierarchicalGameObjects[14]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[14]->SetScale(10.0f, 2.0f, 3.0f);
	if (pCSModels9) delete pCSModels9;

	m_ppHierarchicalGameObjects[15] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels10, 1);
	m_ppHierarchicalGameObjects[15]->SetPosition(350.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 320.0f);
	m_ppHierarchicalGameObjects[15]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[15]->SetScale(15.0f, 2.0f, 3.0f);
	if (pCSModels10) delete pCSModels10;

	m_ppHierarchicalGameObjects[16] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels11, 1);
	m_ppHierarchicalGameObjects[16]->SetPosition(760.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 215.0f);
	m_ppHierarchicalGameObjects[16]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[16]->SetScale(10.0f, 2.0f, 3.0f);
	if (pCSModels11) delete pCSModels11;

	m_ppHierarchicalGameObjects[17] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels12, 1);
	m_ppHierarchicalGameObjects[17]->SetPosition(710.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 130.0f);
	m_ppHierarchicalGameObjects[17]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppHierarchicalGameObjects[17]->SetScale(8.0f, 2.0f, 3.0f);
	if (pCSModels12) delete pCSModels12;

	m_ppHierarchicalGameObjects[18] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCSModels13, 1);
	m_ppHierarchicalGameObjects[18]->SetPosition(640.0f, m_pTerrain->GetHeight(300.0f, 620.0f), 335.0f);
	m_ppHierarchicalGameObjects[18]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[18]->SetScale(6.0f, 2.0f, 3.0f);
	if (pCSModels13) delete pCSModels13;

	CLoadedModelInfo* pBasicBuilding1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	CLoadedModelInfo* pBasicBuilding2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	CLoadedModelInfo* pBasicBuilding3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	CLoadedModelInfo* pBasicBuilding4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	CLoadedModelInfo* pBasicBuilding5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	CLoadedModelInfo* pBasicBuilding6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	CLoadedModelInfo* pBasicBuilding7 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Building.bin", NULL);
	m_ppHierarchicalGameObjects[19] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding1, 1);
	m_ppHierarchicalGameObjects[19]->SetPosition(420.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 400.0f);
	m_ppHierarchicalGameObjects[19]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[19]->SetScale(30.0f, 10.0f, 8.0f);
	if (pBasicBuilding1) delete pBasicBuilding1;

	m_ppHierarchicalGameObjects[20] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding2, 1);
	m_ppHierarchicalGameObjects[20]->SetPosition(650.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 200.0f);
	m_ppHierarchicalGameObjects[20]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[20]->SetScale(40.0f, 10.0f, 8.0f);
	if (pBasicBuilding2) delete pBasicBuilding2;

	m_ppHierarchicalGameObjects[21] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding3, 1);
	m_ppHierarchicalGameObjects[21]->SetPosition(700.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 400.0f);
	m_ppHierarchicalGameObjects[21]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[21]->SetScale(15.0f, 6.0f, 8.0f);
	if (pBasicBuilding3) delete pBasicBuilding3;

	m_ppHierarchicalGameObjects[22] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding4, 1);
	m_ppHierarchicalGameObjects[22]->SetPosition(450.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 350.0f);
	m_ppHierarchicalGameObjects[22]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[22]->SetScale(30.0f, 5.0f, 8.0f);
	if (pBasicBuilding4) delete pBasicBuilding4;

	m_ppHierarchicalGameObjects[23] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding5, 1);
	m_ppHierarchicalGameObjects[23]->SetPosition(200.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 500.0f);
	m_ppHierarchicalGameObjects[23]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[23]->SetScale(30.0f, 10.0f, 8.0f);
	if (pBasicBuilding5) delete pBasicBuilding5;

	m_ppHierarchicalGameObjects[24] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding6, 1);
	m_ppHierarchicalGameObjects[24]->SetPosition(550.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 400.0f);
	m_ppHierarchicalGameObjects[24]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[24]->SetScale(30.0f, 10.0f, 8.0f);
	if (pBasicBuilding6) delete pBasicBuilding6;

	m_ppHierarchicalGameObjects[25] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pBasicBuilding7, 1);
	m_ppHierarchicalGameObjects[25]->SetPosition(800.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 300.0f);
	m_ppHierarchicalGameObjects[25]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[25]->SetScale(30.0f, 10.0f, 8.0f);
	if (pBasicBuilding7) delete pBasicBuilding7;

	CLoadedModelInfo* pWallModels1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels7 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels8 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels9 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	CLoadedModelInfo* pWallModels10 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/untitled.blend131321.bin", NULL);
	m_ppHierarchicalGameObjects[26] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels1, 1);
	m_ppHierarchicalGameObjects[26]->SetPosition(850.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 250.0f);
	m_ppHierarchicalGameObjects[26]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[26]->SetScale(6.0f, 2.0f, 4.0f);
	if (pWallModels1) delete pWallModels1;

	m_ppHierarchicalGameObjects[27] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels2, 1);
	m_ppHierarchicalGameObjects[27]->SetPosition(875.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 150.0f);
	m_ppHierarchicalGameObjects[27]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[27]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels2) delete pWallModels2;

	m_ppHierarchicalGameObjects[28] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels3, 1);
	m_ppHierarchicalGameObjects[28]->SetPosition(120.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 325.0f);
	m_ppHierarchicalGameObjects[28]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[28]->SetScale(8.0f, 2.0f, 8.0f);
	if (pWallModels3) delete pWallModels3;

	m_ppHierarchicalGameObjects[29] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels4, 1);
	m_ppHierarchicalGameObjects[29]->SetPosition(900.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 400.0f);
	m_ppHierarchicalGameObjects[29]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[29]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels4) delete pWallModels4;

	m_ppHierarchicalGameObjects[30] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels5, 1);
	m_ppHierarchicalGameObjects[30]->SetPosition(160.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 605.0f);
	m_ppHierarchicalGameObjects[30]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[30]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels5) delete pWallModels5;

	m_ppHierarchicalGameObjects[31] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels6, 1);
	m_ppHierarchicalGameObjects[31]->SetPosition(600.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 500.0f);
	m_ppHierarchicalGameObjects[31]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[31]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels6) delete pWallModels6;

	m_ppHierarchicalGameObjects[32] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels7, 1);
	m_ppHierarchicalGameObjects[32]->SetPosition(525.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 650.0f);
	m_ppHierarchicalGameObjects[32]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[32]->SetScale(8.0f, 2.0f, 8.0f);
	if (pWallModels7) delete pWallModels7;

	m_ppHierarchicalGameObjects[33] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels8, 1);
	m_ppHierarchicalGameObjects[33]->SetPosition(750.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 800.0f);
	m_ppHierarchicalGameObjects[33]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[33]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels8) delete pWallModels8;

	m_ppHierarchicalGameObjects[34] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels9, 1);
	m_ppHierarchicalGameObjects[34]->SetPosition(400.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 500.0f);
	m_ppHierarchicalGameObjects[34]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[34]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels9) delete pWallModels9;

	m_ppHierarchicalGameObjects[35] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pWallModels10, 1);
	m_ppHierarchicalGameObjects[35]->SetPosition(250.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 795.0f);
	m_ppHierarchicalGameObjects[35]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[35]->SetScale(10.0f, 2.0f, 8.0f);
	if (pWallModels10) delete pWallModels10;

	CLoadedModelInfo* pEnemyBuildingModels1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels7 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels8 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels9 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels10 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	CLoadedModelInfo* pEnemyBuildingModels11 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/CementShack2.bin", NULL);
	m_ppHierarchicalGameObjects[36] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels1, 1);
	m_ppHierarchicalGameObjects[36]->SetPosition(500.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 850.0f);
	m_ppHierarchicalGameObjects[36]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[36]->SetScale(20.0f, 2.0f, 4.0f);
	if (pEnemyBuildingModels1) delete pEnemyBuildingModels1;

	m_ppHierarchicalGameObjects[37] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels2, 1);
	m_ppHierarchicalGameObjects[37]->SetPosition(380.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 940.0f);
	m_ppHierarchicalGameObjects[37]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[37]->SetScale(6.0f, 2.0f, 3.0f);
	if (pEnemyBuildingModels2) delete pEnemyBuildingModels2;

	m_ppHierarchicalGameObjects[38] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels3, 1);
	m_ppHierarchicalGameObjects[38]->SetPosition(600.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 940.0f);
	m_ppHierarchicalGameObjects[38]->Rotate(0.0f, 270.0f, 0.0f);
	m_ppHierarchicalGameObjects[38]->SetScale(6.0f, 2.0f, 3.0f);
	if (pEnemyBuildingModels3) delete pEnemyBuildingModels3;

	m_ppHierarchicalGameObjects[39] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels4, 1);
	m_ppHierarchicalGameObjects[39]->SetPosition(500.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 775.0f);
	m_ppHierarchicalGameObjects[39]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[39]->SetScale(15.0f, 2.0f, 4.0f);
	if (pEnemyBuildingModels4) delete pEnemyBuildingModels4;

	m_ppHierarchicalGameObjects[40] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels5, 1);
	m_ppHierarchicalGameObjects[40]->SetPosition(150.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 850.0f);
	m_ppHierarchicalGameObjects[40]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[40]->SetScale(15.0f, 2.0f, 2.0f);
	if (pEnemyBuildingModels5) delete pEnemyBuildingModels5;

	m_ppHierarchicalGameObjects[41] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels6, 1);
	m_ppHierarchicalGameObjects[41]->SetPosition(685.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 875.0f);
	m_ppHierarchicalGameObjects[41]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[41]->SetScale(8.0f, 2.0f, 3.0f);
	if (pEnemyBuildingModels6) delete pEnemyBuildingModels6;

	m_ppHierarchicalGameObjects[42] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels7, 1);
	m_ppHierarchicalGameObjects[42]->SetPosition(850.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 900.0f);
	m_ppHierarchicalGameObjects[42]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[42]->SetScale(15.0f, 2.0f, 2.0f);
	if (pEnemyBuildingModels7) delete pEnemyBuildingModels7;

	m_ppHierarchicalGameObjects[43] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels8, 1);
	m_ppHierarchicalGameObjects[43]->SetPosition(850.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 750.0f);
	m_ppHierarchicalGameObjects[43]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[43]->SetScale(12.0f, 2.0f, 3.0f);
	if (pEnemyBuildingModels8) delete pEnemyBuildingModels8;

	m_ppHierarchicalGameObjects[44] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels9, 1);
	m_ppHierarchicalGameObjects[44]->SetPosition(750.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 335.0f);
	m_ppHierarchicalGameObjects[44]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[44]->SetScale(12.0f, 2.0f, 2.0f);
	if (pEnemyBuildingModels9) delete pEnemyBuildingModels9;

	m_ppHierarchicalGameObjects[45] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels10, 1);
	m_ppHierarchicalGameObjects[45]->SetPosition(700.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 515.0f);
	m_ppHierarchicalGameObjects[45]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[45]->SetScale(12.0f, 2.0f, 2.0f);
	if (pEnemyBuildingModels10) delete pEnemyBuildingModels10;

	m_ppHierarchicalGameObjects[46] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEnemyBuildingModels11, 1);
	m_ppHierarchicalGameObjects[46]->SetPosition(750.0f, m_pTerrain->GetHeight(350.0f, 80.0f), 650.0f);
	m_ppHierarchicalGameObjects[46]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[46]->SetScale(14.0f, 2.0f, 2.0f);
	if (pEnemyBuildingModels11) delete pEnemyBuildingModels11;


	CLoadedModelInfo* pGreatModels1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels7 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels8 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pGreatModels9 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[47] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels1, 1);
	m_ppHierarchicalGameObjects[47]->SetPosition(650.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 450.0f);
	m_ppHierarchicalGameObjects[47]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[47]->SetScale(3.0f, 3.0f, 3.0f);
	if (pGreatModels1) delete pGreatModels1;

	m_ppHierarchicalGameObjects[48] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels2, 1);
	m_ppHierarchicalGameObjects[48]->SetPosition(920.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 125.0f);
	m_ppHierarchicalGameObjects[48]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[48]->SetScale(3.0f, 2.0f, 3.0f);
	if (pGreatModels2) delete pGreatModels2;

	m_ppHierarchicalGameObjects[49] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels3, 1);
	m_ppHierarchicalGameObjects[49]->SetPosition(250.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 800.0f);
	m_ppHierarchicalGameObjects[49]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[49]->SetScale(3.0f, 2.0f, 3.0f);
	if (pGreatModels3) delete pGreatModels3;

	m_ppHierarchicalGameObjects[50] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels4, 1);
	m_ppHierarchicalGameObjects[50]->SetPosition(850.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 550.0f);
	m_ppHierarchicalGameObjects[50]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[50]->SetScale(6.0f, 2.0f, 6.0f);
	if (pGreatModels4) delete pGreatModels4;

	m_ppHierarchicalGameObjects[51] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels5, 1);
	m_ppHierarchicalGameObjects[51]->SetPosition(100.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 800.0f);
	m_ppHierarchicalGameObjects[51]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[51]->SetScale(2.0f, 2.0f, 2.0f);
	if (pGreatModels5) delete pGreatModels5;

	m_ppHierarchicalGameObjects[52] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels6, 1);
	m_ppHierarchicalGameObjects[52]->SetPosition(250.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 675.0f);
	m_ppHierarchicalGameObjects[52]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[52]->SetScale(2.0f, 2.0f, 2.0f);
	if (pGreatModels6) delete pGreatModels6;

	m_ppHierarchicalGameObjects[53] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels7, 1);
	m_ppHierarchicalGameObjects[53]->SetPosition(100.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 675.0f);
	m_ppHierarchicalGameObjects[53]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[53]->SetScale(2.0f, 2.0f, 2.0f);
	if (pGreatModels7) delete pGreatModels7;

	m_ppHierarchicalGameObjects[54] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels8, 1);
	m_ppHierarchicalGameObjects[54]->SetPosition(125.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 400.0f);
	m_ppHierarchicalGameObjects[54]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[54]->SetScale(4.0f, 2.0f, 4.0f);
	if (pGreatModels8) delete pGreatModels8;

	m_ppHierarchicalGameObjects[55] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pGreatModels9, 1);
	m_ppHierarchicalGameObjects[55]->SetPosition(325.0f, m_pTerrain->GetHeight(230.0f, 640.0f), 550.0f);
	m_ppHierarchicalGameObjects[55]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[55]->SetScale(4.0f, 2.0f, 4.0f);
	if (pGreatModels9) delete pGreatModels9;

	CLoadedModelInfo* pArtBuilding1 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pArtBuilding2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pArtBuilding3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pArtBuilding4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pArtBuilding5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	CLoadedModelInfo* pArtBuilding6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/GeneratorShed.bin", NULL);
	m_ppHierarchicalGameObjects[56] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pArtBuilding1, 1);
	m_ppHierarchicalGameObjects[56]->SetPosition(150.0f, m_pTerrain->GetHeight(300.0f, 300.0f), 465.0f);
	m_ppHierarchicalGameObjects[56]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[56]->SetScale(20.0f, 5.0f, 8.0f);
	if (pArtBuilding1) delete pArtBuilding1;

	m_ppHierarchicalGameObjects[57] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pArtBuilding2, 1);
	m_ppHierarchicalGameObjects[57]->SetPosition(450.0f, m_pTerrain->GetHeight(450.0f, 700.0f), 700.0f);
	m_ppHierarchicalGameObjects[57]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[57]->SetScale(20.0f, 4.0f, 8.0f);
	if (pArtBuilding2) delete pArtBuilding2;

	m_ppHierarchicalGameObjects[58] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pArtBuilding3, 1);
	m_ppHierarchicalGameObjects[58]->SetPosition(75.0f, m_pTerrain->GetHeight(75.0f, 750.0f), 750.0f);
	m_ppHierarchicalGameObjects[58]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[58]->SetScale(20.0f, 4.0f, 8.0f);
	if (pArtBuilding3) delete pArtBuilding3;

	m_ppHierarchicalGameObjects[59] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pArtBuilding4, 1);
	m_ppHierarchicalGameObjects[59]->SetPosition(125.0f, m_pTerrain->GetHeight(125.0f, 500.0f), 500.0f);
	m_ppHierarchicalGameObjects[59]->Rotate(0.0f, 90.0f, 0.0f);
	m_ppHierarchicalGameObjects[59]->SetScale(15.0f, 8.0f, 8.0f);
	if (pArtBuilding4) delete pArtBuilding4;

	m_ppHierarchicalGameObjects[60] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pArtBuilding5, 1);
	m_ppHierarchicalGameObjects[60]->SetPosition(70.0, m_pTerrain->GetHeight(745.0f, 440.0f), 240.0f);
	m_ppHierarchicalGameObjects[60]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[60]->SetScale(20.0f, 10.0f, 8.0f);
	if (pArtBuilding5) delete pArtBuilding5;

	m_ppHierarchicalGameObjects[61] = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pArtBuilding6, 1);
	m_ppHierarchicalGameObjects[61]->SetPosition(0.0f, m_pTerrain->GetHeight(350.0f, 600.0f), 0.0f);
	m_ppHierarchicalGameObjects[61]->Rotate(0.0f, 0.0f, 0.0f);
	m_ppHierarchicalGameObjects[61]->SetScale(40.0f, 4.0f, 8.0f);
	if (pArtBuilding6) delete pArtBuilding6;

	CLoadedModelInfo* pEagleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Eagle.bin", NULL);
	m_ppHierarchicalGameObjects[62] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
	m_ppHierarchicalGameObjects[62]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[62]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
	if (pEagleModel) delete pEagleModel;

	//m_nShaders = 1;
	//m_ppShaders = new CShader * [m_nShaders];

	//CEthanObjectsShader* pEthanObjectsShader = new CEthanObjectsShader();
	//pEthanObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, m_pTerrain);
	//m_ppShaders[0] = pEthanObjectsShader;
	//if (pEthanModel) delete pEthanModel;


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	/*CSceneShader* pSceneShader = new CSceneShader();
	pSceneShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pSceneShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Models/Scene.bin", m_pTerrain);
	m_ppShaders[1] = pSceneShader;*/

}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Release();
		delete[] m_ppHierarchicalGameObjects;
	}

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[15];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:

	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}
float theta;
void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Animate(fTimeElapsed);

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

	static float fAngle = 0.0f;
	fAngle -= 1.50f;
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::RotationYawPitchRoll(0.0, -fAngle, 0.0f);
	XMFLOAT3 xmfPosition = Vector3::TransformCoord(XMFLOAT3(50.0, 0.0, 0.0), xmf4x4Rotate);

	theta += fTimeElapsed / 10.0f;
	for (int i = 47; i < 56; i++)
		m_ppHierarchicalGameObjects[i]->Rotate(0.0,0.0,0.0);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < 63; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			/*m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);*/
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}