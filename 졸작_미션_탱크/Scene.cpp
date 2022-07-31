//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 3;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.5f, 0.2f, 0.4f, 1.0f);

	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 500.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 5000.0f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.01f);
	m_pLights[0].m_bEnable = true;

	m_pLights[1].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[1].m_fRange = 500.0;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.8f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.6f, 0.5f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 0.5f, 0.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_pLights[1].m_fFalloff = 7.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(120.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[1].m_bEnable = true;

	m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights[2].m_fRange = 200.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 2000.0f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(0.0001f, 0.0001f, 0.0001f);
	m_pLights[2].m_fFalloff = 6.0f;
	m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(120.0f));
	m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[2].m_bEnable = true;


}


uniform_real_distribution<> uidx(80.0, 1500.0); uniform_real_distribution<> uidz(80.0, 1500.0);
uniform_real_distribution<> uidy(20.0, 25.0); uniform_real_distribution<> uidR(0.0, 20.0);
void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	BuildDefaultLightsAndMaterials();

	m_ppShaderObjcet = new CShader * [3];

	m_ppShaderObjcet[0] = new CIlluminatedShader();
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, &m_ppShaderObjcet[0]);
	m_ppShaderObjcet[0]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);



	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	CMaterialColors* pMaterialColors = new CMaterialColors();
	pMaterialColors->m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pMaterialColors->m_xmf4Diffuse = XMFLOAT4(0.63f, 0.8f, 0.1f, 1.0f);
	pMaterialColors->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 6.0f); //(r,g,b,a=power)
	pMaterialColors->m_xmf4Emissive = XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f);

	CMaterial* pMaterial = new CMaterial();
	pMaterial->SetShader(pTerrainShader);
	pMaterial->SetMaterialColors(pMaterialColors);

	XMFLOAT3 xmf3Scale(16.0f, 1.5f, 16.0f);
	XMFLOAT3 xmf3Pos(0.0, 0.0f, 0.0);
	XMFLOAT4 xmf4Color(0.0f, 0.2f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Assets/Image/Terrain/terrain-8bit.raw"), 512, 512, 16, 16, xmf3Scale, xmf4Color);

	m_pTerrain->SetPosition(xmf3Pos);
	m_pTerrain->Rotate(0.0, 0.0, 0.0);
	m_pTerrain->SetMaterial(pMaterial);


	m_nGameObjects = 15;
	m_ppGameObjects = new CTankObject * [m_nGameObjects];

	CTankObject* pGameModel = CTankObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/tree.bin");

	CobstacleObject* pTree1 = NULL;
	pTree1 = new CobstacleObject();
	pTree1->SetChild(pGameModel, true);
	pTree1->OnInitialize();
	pTree1->SetScale(5.0, 10.0, 5.0);
	pTree1->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[0] = pTree1;

	CobstacleObject* pTree2 = NULL;
	pTree2 = new CobstacleObject();
	pTree2->SetChild(pGameModel, true);
	pTree2->OnInitialize();
	pTree2->SetScale(5.0, 10.0, 5.0);
	pTree2->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[1] = pTree2;

	CobstacleObject* pTree3 = NULL;
	pTree3 = new CobstacleObject();
	pTree3->SetChild(pGameModel, true);
	pTree3->OnInitialize();
	pTree3->SetScale(5.0, 10.0, 5.0);
	pTree3->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[2] = pTree3;

	CobstacleObject* pTree4 = NULL;
	pTree4 = new CobstacleObject();
	pTree4->SetChild(pGameModel, true);
	pTree4->OnInitialize();
	pTree4->SetScale(5.0, 10.0, 5.0);
	pTree4->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[3] = pTree4;

	CobstacleObject* pTree5 = NULL;
	pTree5 = new CobstacleObject();
	pTree5->SetChild(pGameModel, true);
	pTree5->OnInitialize();
	pTree5->SetScale(5.0, 10.0, 5.0);
	pTree5->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[4] = pTree5;
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CobstacleObject* pTree6 = NULL;
	pTree6 = new CobstacleObject();
	pTree6->SetChild(pGameModel, true);
	pTree6->OnInitialize();
	pTree6->SetScale(5.0, 10.0, 5.0);
	pTree6->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[5] = pTree6;


	CobstacleObject* pTree7 = NULL;
	pTree7 = new CobstacleObject();
	pTree7->SetChild(pGameModel, true);
	pTree7->OnInitialize();
	pTree7->SetScale(5.0, 10.0, 5.0);
	pTree7->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[6] = pTree7;


	CobstacleObject* pTree8 = NULL;
	pTree8 = new CobstacleObject();
	pTree8->SetChild(pGameModel, true);
	pTree8->OnInitialize();
	pTree8->SetScale(5.0, 10.0, 5.0);
	pTree8->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[7] = pTree8;


	CobstacleObject* pTree9 = NULL;
	pTree9 = new CobstacleObject();
	pTree9->SetChild(pGameModel, true);
	pTree9->OnInitialize();
	pTree9->SetScale(5.0, 10.0, 5.0);
	pTree9->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[8] = pTree9;


	CobstacleObject* pTree10 = NULL;
	pTree10 = new CobstacleObject();
	pTree10->SetChild(pGameModel, true);
	pTree10->OnInitialize();
	pTree10->SetScale(5.0, 10.0, 5.0);
	pTree10->SetPosition(uidx(dre), uidy(dre), uidz(dre));
	m_ppGameObjects[9] = pTree10;

	CTankObject* pGameModel2 = CTankObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/stone_largeB.bin");

	CobstacleObject* pRock1 = NULL;
	pRock1 = new CobstacleObject();
	pRock1->SetChild(pGameModel2, true);
	pRock1->OnInitialize();
	pRock1->SetScale(2.0, 2.0, 2.0);
	pRock1->SetPosition(uidx(dre), uidR(dre), uidz(dre));
	m_ppGameObjects[10] = pRock1;

	CobstacleObject* pRock2 = NULL;
	pRock2 = new CobstacleObject();
	pRock2->SetChild(pGameModel2, true);
	pRock2->OnInitialize();
	pRock2->SetScale(2.0, 2.0, 2.0);
	pRock2->SetPosition(uidx(dre), uidR(dre), uidz(dre));
	m_ppGameObjects[11] = pRock2;

	CobstacleObject* pRock3 = NULL;
	pRock3 = new CobstacleObject();
	pRock3->SetChild(pGameModel2, true);
	pRock3->OnInitialize();
	pRock3->SetScale(2.0, 2.0, 2.0);
	pRock3->SetPosition(uidx(dre), uidR(dre), uidz(dre));
	m_ppGameObjects[12] = pRock3;

	CobstacleObject* pRock4 = NULL;
	pRock4 = new CobstacleObject();
	pRock4->SetChild(pGameModel2, true);
	pRock4->OnInitialize();
	pRock4->SetScale(2.0, 2.0, 2.0);
	pRock4->SetPosition(uidx(dre), uidR(dre), uidz(dre));
	m_ppGameObjects[13] = pRock4;

	CobstacleObject* pRock5 = NULL;
	pRock5 = new CobstacleObject();
	pRock5->SetChild(pGameModel2, true);
	pRock5->OnInitialize();
	pRock5->SetScale(2.0, 2.0, 2.0);
	pRock5->SetPosition(uidx(dre), uidR(dre), uidz(dre));
	m_ppGameObjects[14] = pRock5;


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pLights) delete[] m_pLights;


	for (int i = 0; i < 2; i++)
	{
		m_ppShaderObjcet[i]->Release();
	}
	if (m_ppShaderObjcet) delete[] m_ppShaderObjcet;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];

	//Camera
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//GameObject
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2;
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//terrain
	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 16;
	pd3dRootParameters[2].Constants.ShaderRegister = 3;
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//Lights
	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 4;
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
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
	if (m_ppShaderObjcet)
	{
		for (int i = 0; i < 3; i++)
			m_ppShaderObjcet[i]->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{

	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

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
		switch (wParam)
		{
		case VK_DELETE:
			((CTankPlayer*)m_pPlayer)->FireBullet(NULL);
			break;

		case VK_F4:
			FightMode = true;
			turn = m_nGameObjects;
			pickingSight = true;
			break;

		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::PickingToPlayer()
{


}
void CScene::CheckObjectByBulletCollisions()
{

}

void CScene::CheckObjectByPlayerCollisions()
{
	for (int i = 10; i < 15; i++)
	{
		if (m_ppGameObjects[i]->oobb.Intersects(m_pPlayer->oobb)) {

			if (m_ppGameObjects[i]->GetPosition().x< m_pPlayer->GetPosition().x || m_ppGameObjects[i]->GetPosition().z > m_pPlayer->GetPosition().z) {

				m_pPlayer->m_xmf3Position.z -= 1.0f;
				m_pPlayer->m_xmf3Position.x += 1.0f;
			}
			if (m_ppGameObjects[i]->GetPosition().x > m_pPlayer->GetPosition().x || m_ppGameObjects[i]->GetPosition().z < m_pPlayer->GetPosition().z) {

				m_pPlayer->m_xmf3Position.z += 1.0f;
				m_pPlayer->m_xmf3Position.x -= 1.0f;
			}
		}
	}
}


void CScene::AnimateObjects(float fTimeElapsed)
{

	m_fElapsedTime = fTimeElapsed;
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);

	if (m_pLights)
	{
		XMFLOAT3 offset = XMFLOAT3(0, 20, 50);
		m_pLights[2].m_xmf3Position.z = m_pPlayer->GetPosition().z - 2.0F;
		m_pLights[2].m_xmf3Position.y = m_pPlayer->GetPosition().y + 32.0;
		m_pLights[2].m_xmf3Position.x = m_pPlayer->GetPosition().x + 5.0;
		m_pLights[2].m_xmf3Direction = m_pPlayer->GetLookVector();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_pPlayer->GetPosition()), XMLoadFloat3(&offset)));
		m_pLights[0].m_xmf3Position = m_ppGameObjects[6]->GetPosition();
		m_pLights[0].m_xmf3Direction = m_ppGameObjects[6]->GetLook();
	}

	CheckObjectByPlayerCollisions();
	CheckObjectByBulletCollisions();
	PickingToPlayer();
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++)
	{
		if (m_ppGameObjects[i])
		{
			m_ppGameObjects[i]->Animate(m_fElapsedTime, NULL);
			m_ppGameObjects[i]->UpdateTransform(NULL);
			m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

float CScene::distanceEnermyToPlayer(CTankObject* e)
{

	return 0;
}

