//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

ID3D12DescriptorHeap* SceneManager::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	SceneManager::m_d3dSrvGPUDescriptorNextHandle;


SceneManager::SceneManager()
{

}

SceneManager::~SceneManager()
{
	
}

bool SceneManager::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool SceneManager::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void SceneManager::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);


}

void SceneManager::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
	
	
	//if (m_pcbMappedMaterials) ::memcpy(m_pcbMappedMaterials, m_pMaterials, sizeof(MATERIALS));


}

void SceneManager::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
	if (m_pd3dcbMaterials)
	{
		m_pd3dcbMaterials->Unmap(0, NULL);
		m_pd3dcbMaterials->Release();
	}
}

void SceneManager::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0, 0.0, 0.0);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 600.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 5.5f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(50.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = POINT_LIGHT;
	m_pLights[3].m_fRange = 300.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.9f, 0.4f, 0.2f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.3f, 0.1, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.9f, 0.5f, 0.1f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(0.0, 0.0, 0.0);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[4].m_nType = SPOT_LIGHT;
	m_pLights[4].m_fRange = 500.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.9f, 0.4f, 0.1f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.4f, 0.4f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.9f, 0.4f, 0.3f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[4].m_fFalloff = 6.0f;
	m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(22.0f));
}

void SceneManager::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 1 + 2 + 1 + 1 + 4 * 5 + 2 + 4 + 270);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_pSkyBox->SetCurScene(SCENE1STAGE);

	XMFLOAT3 xmf3Scale(10.0f, 4.0f, 10.0f);
	XMFLOAT3 xmf3Normal(1.0f, 1.0f, 1.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/terrain033.raw"), 513, 513, xmf3Scale, xmf3Normal);
	m_pTerrain->SetCurScene(SCENE1STAGE);


	m_nHierarchicalGameObjects = 0;
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];


	m_nBillboardShaders = 2;
	m_pBillboardShader = new BillboardShader * [m_nBillboardShaders];

	CrossHairShader* pCrossHairShader = new CrossHairShader();
	pCrossHairShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pCrossHairShader->SetCurScene(SCENE1STAGE);
	pCrossHairShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[0] = pCrossHairShader;

	//RainShader* pRainShader = new RainShader();
	//pRainShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	//pRainShader->SetCurScene(SCENE1STAGE);
	//pRainShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	//m_pBillboardShader[1] = pRainShader;

	ValkanEffectShader* pValkanEffectShader = new ValkanEffectShader();
	pValkanEffectShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pValkanEffectShader->SetCurScene(SCENE1STAGE);
	pValkanEffectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[1] = pValkanEffectShader;

	/*m_nShaders = 1;
	m_ppShaders = new CStandardObjectsShader * [m_nShaders];

	CHellicopterObjectsShader* pOtherPlayerShader = new CHellicopterObjectsShader();
	pOtherPlayerShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pOtherPlayerShader->SetCurScene(SCENE1STAGE);
	pOtherPlayerShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL,m_pTerrain);
	m_ppShaders[0] = pOtherPlayerShader;*/

	m_nHierarchicalGameObjects = 5;
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{

		m_ppHierarchicalGameObjects[i] = new CMi24Object(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		CGameObject* pOtherPlayerModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
		m_ppHierarchicalGameObjects[i]->SetChild(pOtherPlayerModel, false);
		m_ppHierarchicalGameObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppHierarchicalGameObjects[i]->SetScale(10.0, 10.0, 10.0);
		m_ppHierarchicalGameObjects[i]->OnPrepareAnimate();
		pOtherPlayerModel->AddRef();
	}

	m_nMapShaders = 1;
	m_ppMapShaders = new CMapObjectShader * [m_nMapShaders];

	CMapObjectShader* pMapObjectShader = new CMapObjectShader();
	pMapObjectShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pMapObjectShader->SetCurScene(SCENE1STAGE);
	pMapObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppMapShaders[0] = pMapObjectShader;


	CBulletEffectShader* pBCBulletEffectShader = new CBulletEffectShader();
	pBCBulletEffectShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pBCBulletEffectShader->SetCurScene(SCENE1STAGE);
	m_pBulletEffect = pBCBulletEffectShader;

	CLoadedModelInfo* pBulletMesh = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", m_pBulletEffect);
	for (int i = 0; i < 100; i++)
	{
		pBulletObject = new CBulletObject(NULL);
		pBulletObject->SetChild(pBulletMesh->m_pModelRootObject, true);
		pBulletObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 0, pBulletMesh);
		m_ppBullets[i] = pBulletObject;
	}

	m_nParticleObjects = 1;
	m_ppParticleObjects = new CParticleObject * [m_nParticleObjects];
	m_ppParticleObjects[0] = new CParticleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
	XMFLOAT3(1955.0, 620.0, 1933.0), XMFLOAT3(0.0f, 65.0f, 0.0f), 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(10.0f, 10.0f), MAX_PARTICLES);


	gamesound.SpeakMusic();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void SceneManager::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();



	if (m_pBillboardShader)
	{
		for (int i = 0; i < m_nBillboardShaders; i++)
		{
			m_pBillboardShader[i]->ReleaseShaderVariables();
			m_pBillboardShader[i]->ReleaseObjects();
			m_pBillboardShader[i]->Release();
		}
		delete[] m_pBillboardShader;
	}

	if (m_ppMapShaders)
	{
		for (int i = 0; i < m_nMapShaders; i++)
		{
			m_ppMapShaders[i]->ReleaseShaderVariables();
			m_ppMapShaders[i]->ReleaseObjects();
			m_ppMapShaders[i]->Release();
		}
		delete[] m_ppMapShaders;
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
	if (m_ppParticleObjects)
	{
		for (int i = 0; i < m_nParticleObjects; i++) delete m_ppParticleObjects[i];
		delete[] m_ppParticleObjects;
	}
	ReleaseShaderVariables();
	if (m_pLights) delete[] m_pLights;

}

ID3D12RootSignature* SceneManager::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[15];

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

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 1;
	pd3dDescriptorRanges[10].BaseShaderRegister = 14; //t14: gtxtBillboardTexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 1;
	pd3dDescriptorRanges[11].BaseShaderRegister = 15; //t15: gtxtParticleTexture
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 16; //t16: gtxtRandomTexture
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 17; //t17: gtxtRandomOnSphereTexture
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[14].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[14].NumDescriptors = 1;
	pd3dDescriptorRanges[14].BaseShaderRegister = 9; // GameObject b9
	pd3dDescriptorRanges[14].RegisterSpace = 0;
	pd3dDescriptorRanges[14].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[22];

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

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[11]; 
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[17].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[12]; 
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[18].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[18].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[13];
	pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[19].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[19].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[19].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[14];
	pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[20].Descriptor.ShaderRegister = 10; //Material
	pd3dRootParameters[20].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[21].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[21].Descriptor.ShaderRegister = 11; //FrameWorkInfo
	pd3dRootParameters[21].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[21].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
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

bool SceneManager::ProcessInput(UCHAR* pKeysBuffer)
{
	return false;
}

void SceneManager::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->AnimateObject(m_pPlayer->GetCamera(), fTimeElapsed);

	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nBillboardShaders; i++) if (m_pBillboardShader[i]) m_pBillboardShader[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nMapShaders; i++) if (m_ppMapShaders[i]) m_ppMapShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Animate(fTimeElapsed);

	/*for (int i = 0; i < m_ppMapShaders[0]->m_ppObjects[0]->m_nMeshes; i++)
	{
		m_ppMapShaders[0]->m_ppObjects[0]->m_xoobb = BoundingOrientedBox(
			XMFLOAT3(m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBCenter),
			XMFLOAT3(m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBExtents),
			XMFLOAT4(0, 0, 0, 1));
	}
	for (int j = 0; j < ((HeliPlayer*)m_pPlayer)->m_nMeshes; j++)
	{
		((HeliPlayer*)m_pPlayer)->m_xoobb = BoundingOrientedBox(
			XMFLOAT3(((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBCenter),
			XMFLOAT3(((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBExtents),
			XMFLOAT4(0, 0, 0, 1));
	}
	for (int j = 0; j < ((HeliPlayer*)m_pPlayer)->m_nMeshes; j++)for (int i = 0; i < m_ppMapShaders[0]->m_ppObjects[0]->m_nMeshes; i++)
	if (((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBExtents.x >= m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBExtents.x||
		((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBExtents.y >= m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBExtents.y ||
		((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBExtents.z >= m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBExtents.z ||
		((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBCenter.x >= m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBCenter.x||
		((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBCenter.y >= m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBCenter.y||
		((HeliPlayer*)m_pPlayer)->m_ppMeshes[j]->m_xmf3AABBCenter.z >= m_ppMapShaders[0]->m_ppObjects[0]->m_ppMeshes[i]->m_xmf3AABBCenter.z)
	{
		exit(0);
	}*/

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

}

void SceneManager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	//if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	//D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	//pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights
	//D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	//pd3dCommandList->SetGraphicsRootConstantBufferView(20, d3dcbMaterialsGpuVirtualAddress); //Materials

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nMapShaders; i++) if (m_ppMapShaders[i]) m_ppMapShaders[i]->Render(pd3dCommandList, pCamera);


	//if (m_pBulletEffect) m_pBulletEffect->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nBillboardShaders; i++) if (m_pBillboardShader[i]) m_pBillboardShader[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) m_ppBullets[i]->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			m_ppHierarchicalGameObjects[i]->AnimateObject(pCamera,m_fElapsedTime);
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void SceneManager::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	//for (int i = 0; i < BULLETS; i++) if (m_ppBullets[i]) m_ppBullets[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nBillboardShaders; i++) m_pBillboardShader[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nMapShaders; i++) m_ppMapShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->ReleaseUploadBuffers();

}

void SceneManager::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights
	}
	if (m_pd3dcbMaterials)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(20, d3dcbMaterialsGpuVirtualAddress); //Materials
	}
}

void SceneManager::RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->Render(pd3dCommandList, pCamera);
}

void SceneManager::OnPostRenderParticle()
{
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->OnPostRender();
}

void SceneManager::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
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
	//
	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE SceneManager::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
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

void SceneManager::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}


