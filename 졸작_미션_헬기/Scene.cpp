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

	m_xmf4GlobalAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.8f);

	m_pLights[0].m_nType = SPOT_LIGHT;
	m_pLights[0].m_fRange = 500.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.8f, 0.1f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.1f, 0.6f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.9f, 0.3f, 0.3f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.01f);
	m_pLights[0].m_fFalloff = 8.0f;
	m_pLights[0].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[0].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[0].m_bEnable = true;

	
	///////////////// BackGround LIGHT ////////////////////////////
	
	m_pLights[1].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[1].m_fRange = 500.0;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.5f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.1f, 0.3f, 0.1f, 0.5f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_pLights[1].m_fFalloff = 7.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(120.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[1].m_bEnable = true;

	
	/////////////////// Player LIGHT///////////////////////////////
	
	m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights[2].m_fRange = 500.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[2].m_fFalloff = 8.0f;
	m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(100.0f));
	m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(50.0f));
	m_pLights[2].m_bEnable = true;


}


uniform_real_distribution<> uidx(0.0, 10.0); uniform_real_distribution<> uidz(0.0, 10.0);
uniform_real_distribution<> uidy(0.0, 10.0); uniform_real_distribution<> uidR(0.0, 10.0);
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
	pMaterialColors->m_xmf4Ambient = XMFLOAT4(0.15f, 0.35f, 0.1f, 0.5f);
	pMaterialColors->m_xmf4Diffuse = XMFLOAT4(0.33f, 0.5f, 0.1f, 0.8f);
	pMaterialColors->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 6.0f); //(r,g,b,a=power)
	pMaterialColors->m_xmf4Emissive = XMFLOAT4(0.01f, 0.01, 0.01f, 1.0f);

	CMaterial* pMaterial = new CMaterial();
	pMaterial->SetShader(pTerrainShader);
	pMaterial->SetMaterialColors(pMaterialColors);

	XMFLOAT3 xmf3Scale(16.0f, 10.0f, 16.0f);
	XMFLOAT3 xmf3Pos(0.0, 0.0f, 0.0);
	XMFLOAT4 xmf4Color(0.0f, 2.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Assets/Image/Terrain/terrain_heli_8bit_512x512.raw"), 512, 512, 16, 16, xmf3Scale, xmf4Color);

	m_pTerrain->SetPosition(xmf3Pos);
	m_pTerrain->Rotate(0.0, 0.0, 0.0);
	m_pTerrain->SetMaterial(pMaterial);



	m_nGameObjects = 11;
	m_ppGameObjects = new CHelicopterObject * [m_nGameObjects];

	CHelicopterObject* pGameModel = CHelicopterObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/tree.bin");
	CHelicopterObject* pGameModel2 = CHelicopterObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/stone_largeB.bin");
	CobstacleObject* pTree1 = NULL;

	CobstacleObject* pRockParticle = NULL;
	pRockParticle = new CobstacleObject();
	pRockParticle->SetChild(pGameModel2, true);
	pRockParticle->OnInitialize();
	pRockParticle->SetScale(2.0, 2.0, 2.0);
	pRockParticle->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[0] = pRockParticle;


	CobstacleObject* pRockParticle2 = NULL;
	pRockParticle2 = new CobstacleObject();
	pRockParticle2->SetChild(pGameModel2, true);
	pRockParticle2->OnInitialize();
	pRockParticle2->SetScale(0.2, 0.2, 0.2);
	pRockParticle2->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[1] = pRockParticle2;


	CobstacleObject* pRockParticle3 = NULL;
	pRockParticle3 = new CobstacleObject();
	pRockParticle3->SetChild(pGameModel2, true);
	pRockParticle3->OnInitialize();
	pRockParticle3->SetScale(0.2, 0.2, 0.2);
	pRockParticle3->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[2] = pRockParticle3;


	CobstacleObject* pRockParticle4 = NULL;
	pRockParticle4 = new CobstacleObject();
	pRockParticle4->SetChild(pGameModel2, true);
	pRockParticle4->OnInitialize();
	pRockParticle4->SetScale(0.2, 0.2, 0.2);
	pRockParticle4->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[3] = pRockParticle4;


	CobstacleObject* pRockParticle5 = NULL;
	pRockParticle5 = new CobstacleObject();
	pRockParticle5->SetChild(pGameModel2, true);
	pRockParticle5->OnInitialize();
	pRockParticle5->SetScale(0.2, 0.2, 0.2);
	pRockParticle5->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[4] = pRockParticle5;

	

	CobstacleObject* pRockParticle6 = NULL;
	pRockParticle6 = new CobstacleObject();
	pRockParticle6->SetChild(pGameModel2, true);
	pRockParticle6->OnInitialize();
	pRockParticle6->SetScale(0.2, 0.2, 0.2);
	pRockParticle6->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[5] = pRockParticle6;

	CobstacleObject* pRockParticle7 = NULL;
	pRockParticle7 = new CobstacleObject();
	pRockParticle7->SetChild(pGameModel2, true);
	pRockParticle7->OnInitialize();
	pRockParticle7->SetScale(0.2, 0.2, 0.2);
	pRockParticle7->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[6] = pRockParticle7;

	CobstacleObject* pRockParticle8 = NULL;
	pRockParticle8 = new CobstacleObject();
	pRockParticle8->SetChild(pGameModel2, true);
	pRockParticle8->OnInitialize();
	pRockParticle8->SetScale(0.2, 0.2, 0.2);
	pRockParticle8->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[7] = pRockParticle8;

	CobstacleObject* pRockParticle9 = NULL;
	pRockParticle9 = new CobstacleObject();
	pRockParticle9->SetChild(pGameModel2, true);
	pRockParticle9->OnInitialize();
	pRockParticle9->SetScale(0.2, 0.2, 0.2);
	pRockParticle9->SetPosition(300.0, 10.0, 300.0);
	pRockParticle9->SetPosition(uidx(dre), uidR(dre), uidz(dre));
	m_ppGameObjects[8] = pRockParticle9;

	CobstacleObject* pRockParticle10 = NULL;
	pRockParticle10 = new CobstacleObject();
	pRockParticle10->SetChild(pGameModel2, true);
	pRockParticle10->OnInitialize();
	pRockParticle10->SetScale(0.2, 0.2, 0.2);
	pRockParticle10->SetPosition(300.0, 10.0, 300.0);
	m_ppGameObjects[9] = pRockParticle10;

	CHelicopterObject* StopZone = CHelicopterObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Hellicopter/HELI1.bin");

	CobstacleObject* pStopZone = NULL;
	pStopZone = new CobstacleObject();
	pStopZone->SetChild(StopZone, true);
	pStopZone->OnInitialize();
	pStopZone->SetScale(5000.0, 500.0, 5000.0);
	pStopZone->SetPosition(2650.0, 5.0, 1950.0);
	m_ppGameObjects[10] = pStopZone;

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
			((CHelicopterPlayer*)m_pPlayer)->FireBullet(NULL);
			break;
		case VK_END:
			((CHelicopterPlayer*)m_pPlayer)->HellFire();
			((CHelicopterPlayer*)m_pPlayer)->m_MissileCount++;
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

///////////////////////////////////////////Missile Collision///////////////////////////////////////////
void CScene::CheckObjectByBulletCollisions()
{
	// Check Control Point => Missile 9 : m_pRocketFrame9 
	m_pPlayer->m_pRocketFrame9->oobb = BoundingOrientedBox(m_pPlayer->m_pRocketFrame9->GetPosition(),
		XMFLOAT3(5.0, 5.0, 5.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));

	
	if (m_ppGameObjects[0]->oobb.Intersects(m_pPlayer->m_pRocketFrame9->oobb)) {
		
		for (int i = 1; i < 10; i++) {
			m_ppGameObjects[i]->m_xmf4x4Transform._41 += RandF(-20, 20);
			m_ppGameObjects[i]->m_xmf4x4Transform._42 += RandF(0, 0);
			m_ppGameObjects[i]->m_xmf4x4Transform._43 += RandF(-20, 20);
		}
	
	}
	if (m_pPlayer->m_MissileActive == false) {
		
			for (int i = 1; i < 10;i++) {
			m_ppGameObjects[i]->m_xmf4x4Transform._41 = m_ppGameObjects[0]->m_xmf4x4Transform._41;
			m_ppGameObjects[i]->m_xmf4x4Transform._42 = m_ppGameObjects[0]->m_xmf4x4Transform._42;
			m_ppGameObjects[i]->m_xmf4x4Transform._43 = m_ppGameObjects[0]->m_xmf4x4Transform._43;
		}
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////

void CScene::CheckObjectByPlayerCollisions()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_ppGameObjects[i]->oobb.Intersects(m_pPlayer->oobb)) {

			if (m_ppGameObjects[i]->GetPosition().x + 12.0 < m_pPlayer->GetPosition().x || m_ppGameObjects[i]->GetPosition().z + 10.0 > m_pPlayer->GetPosition().z) {

				m_pPlayer->m_xmf3Position.z -= 1.0f;
				m_pPlayer->m_xmf3Position.x += 1.0f;
			}
			if (m_ppGameObjects[i]->GetPosition().x - 10.0 > m_pPlayer->GetPosition().x || m_ppGameObjects[i]->GetPosition().z - 12.0 < m_pPlayer->GetPosition().z) {

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
	pBulletObject = new CBulletObject(500.0);
	if (m_pLights)
	{
		
		XMFLOAT3 offset = XMFLOAT3(0, 0,0);
		m_pLights[2].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[2].m_xmf3Direction = m_pPlayer->GetLookVector();
		XMStoreFloat3(&offset, XMVectorAdd(XMLoadFloat3(&m_pPlayer->GetPosition()), XMLoadFloat3(&offset)));
		
		
		m_pLights[0].m_xmf3Position = pBulletObject->GetPosition();
		m_pLights[0].m_xmf3Direction = m_pPlayer->GetLookVector();
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


