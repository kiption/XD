
#include "stdafx.h"
#include "Scene.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::BuildDefaultLightsAndMaterials()
{
	
	m_nLights = 5;
	m_pLights = new LIGHTS[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHTS) * m_nLights);
	
	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 1000.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(100.0f, 650.0f,100.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 1200.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.8f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.3f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 10.0f, 2.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 5.5f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_bEnable = true;

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_fRange = 3000.0f;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights->m_pLights[2].m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 3000.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.43f, 0.43f, 0.43f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.23f, 0.23f, 0.23f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(+550, 660.0f, 500.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(-0.1f, -1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.05f);
	m_pLights->m_pLights[3].m_fFalloff = 14.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pLights->m_pLights[4].m_bEnable = false;
	m_pLights->m_pLights[4].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[4].m_fRange = 3000.0f;
	m_pLights->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.43f, 0.43f, 0.43f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Specular = XMFLOAT4(0.43f, 0.43f, 0.43f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Position = XMFLOAT3(+250, 650.0f, -500.0f);
	m_pLights->m_pLights[4].m_xmf3Direction = XMFLOAT3(-0.6f, -1.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.01f);

	m_pMaterials = new MATERIALS;
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(0.128f, 0.128f, 0.128f, 1.0f), XMFLOAT4(0.8f, 0.18f, 0.18f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(0.28f, 0.28f, 0.28f, 1.0f), XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

}


void SceneManager::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, ID3D12Resource* pd3dDepthStencilBuffer)
{
	m_pd3dGraphicsRootSignature = ::GraphicsRootSignatureSt1(pd3dDevice);

	//CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(35.0f, 3.0f, 35.0f);
	XMFLOAT3 xmf3Normal(0.0f, 0.5f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/terrain033.raw"), 513, 513, 513, 513, xmf3Scale, xmf3Normal);
	m_pTerrain->SetPosition(0.0, 0.0, 0.0);

	XMFLOAT3 xmf4ScaleW(25.0f, 2.0f, 25.0);
	m_pUseWaterMove = new CUseWaterMoveTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/waterterrain8bit.raw"), 257, 257, 8, 8, xmf4ScaleW, xmf3Normal);
	m_pUseWaterMove->SetPosition(2500.0, 65.0f, 2500.0);

	m_nShaders = 4;
	m_ppShaders = new CObjectsShader * [m_nShaders];
	DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	CPlayerShader* pPlayerShader = new CPlayerShader();
	pPlayerShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pPlayerShader->SetCurScene(SCENE1STAGE);
	pPlayerShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);

	CGameObject* pGameObject = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", pPlayerShader);
	for (int i = 0; i < 100; i++)
	{
		pBulletObject = new CBulletObject(NULL);
		pBulletObject->SetCurScene(SCENE1STAGE);
		pBulletObject->SetChild(pGameObject, false);
		m_ppBullets[i] = pBulletObject;
	
	}


	CObjectsShader* pObjectsShader = new CObjectsShader();
	pObjectsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pObjectsShader->SetCurScene(SCENE1STAGE);
	pObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[0] = pObjectsShader;

	CBillboardObjectShader* pBillboardObjectShader = new CBillboardObjectShader();
	pBillboardObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pBillboardObjectShader->SetCurScene(SCENE1STAGE);
	pBillboardObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders[1] = pBillboardObjectShader;

	CSnowObjectShader* pCSnowObjectShader = new CSnowObjectShader();
	pCSnowObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pCSnowObjectShader->SetCurScene(SCENE1STAGE);
	pCSnowObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[2] = pCSnowObjectShader;

	CCrossHairShader* pCrossHeadShader = new CCrossHairShader();
	pCrossHeadShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pCrossHeadShader->SetCurScene(SCENE1STAGE);
	pCrossHeadShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pPlayer);
	m_ppShaders[3] = pCrossHeadShader;

	int m_nNPCShaders = 3;
	m_ppNPCShaders = new CNPCShader * [m_nNPCShaders];
	CNPCShader* pCNPCShader = new CNPCShader();
	pCNPCShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pCNPCShader->SetCurScene(SCENE1STAGE);
	pCNPCShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppNPCShaders[0] = pCNPCShader;

	CBulletMotionShader* pCBulletMotionShader = new CBulletMotionShader();
	pCBulletMotionShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pCBulletMotionShader->SetCurScene(SCENE1STAGE);
	pCBulletMotionShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppNPCShaders[1] = pCBulletMotionShader;

	CBulletMotionShader* pCExplosionShader1 = new CBulletMotionShader();
	pCExplosionShader1->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pCExplosionShader1->SetCurScene(SCENE1STAGE);
	pCExplosionShader1->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pCExplosionShader1->SetActive(false);
	m_ppNPCShaders[2] = pCExplosionShader1;


	m_nParticleObjects = 4;
	m_ppParticleObjects = new CParticleObject * [m_nParticleObjects];
	m_ppParticleObjects[0] = new CParticleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		XMFLOAT3(m_ppNPCShaders[0]->m_ppObjects[4]->GetPosition()), XMFLOAT3(0.0f, 65.0f, 0.0f), 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(10.0f, 10.0f), MAX_PARTICLES);

	m_ppParticleObjects[1] = new CParticleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		XMFLOAT3(m_ppNPCShaders[0]->m_ppObjects[3]->GetPosition()), XMFLOAT3(0.0f, 75.0f, 0.0f), 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(12.0f, 12.0f), MAX_PARTICLES);

	m_ppParticleObjects[2] = new CParticleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		XMFLOAT3(m_ppNPCShaders[0]->m_ppObjects[2]->GetPosition()), XMFLOAT3(0.0f, 65.0f, 0.0f), 0.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(15.0f, 15.0f), MAX_PARTICLES);

	m_ppParticleObjects[3] = new CParticleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		XMFLOAT3(m_ppNPCShaders[0]->m_ppObjects[1]->GetPosition()), XMFLOAT3(0.0f, 85.0f, 0.0f), 0.0f, XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(8.0f, 8.0f), MAX_PARTICLES);

	//m_nEnvironmentMappingShaders = 1;
	//m_ppEnvironmentMappingShaders = new CDynamicCubeMappingShader * [m_nEnvironmentMappingShaders];
	//m_ppEnvironmentMappingShaders[0] = new CDynamicCubeMappingShader(256);
	//m_ppEnvironmentMappingShaders[0]->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	//m_ppEnvironmentMappingShaders[0]->SetCurScene(SCENE1STAGE);
	//m_ppEnvironmentMappingShaders[0]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);

	m_pOutlineShader = new COutlineShader();
	m_pOutlineShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	m_pOutlineShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_nShadowShaders = 1;
	m_ppShadowShaders = new CShader * [m_nShaders];
	CShadowInObjectsShader* pObjectShader = new CShadowInObjectsShader();
	pObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pObjectShader->SetCurScene(SCENE1STAGE);
	pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	
	m_xmBoundingBox = pObjectShader->CalculateBoundingBox();

	m_ppShadowShaders[0] = pObjectShader;

	m_pDepthRenderShader = new CDepthRenderShader(pObjectShader, m_pcbMappedLights->m_pLights);
	DXGI_FORMAT pdxgiRtvFormats[1] = { DXGI_FORMAT_R32_FLOAT };
	m_pDepthRenderShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT, 0);
	m_pDepthRenderShader->SetCurScene(SCENE1STAGE);
	m_pDepthRenderShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);

	m_pShadowShader = new CShadowMapShader(pObjectShader);
	m_pShadowShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	m_pShadowShader->SetCurScene(SCENE1STAGE);
	m_pShadowShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pDepthRenderShader->GetDepthTexture());


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void SceneManager::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

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
	if (m_ppShadowShaders)
	{
		for (int i = 0; i < m_nShadowShaders; i++)
		{
			m_ppShadowShaders[i]->ReleaseShaderVariables();
			m_ppShadowShaders[i]->ReleaseObjects();
			m_ppShadowShaders[i]->Release();
		}
		delete[] m_ppShadowShaders;
	}
	if (m_ppNPCShaders)
	{
		for (int i = 0; i < 3; i++)
		{
			m_ppNPCShaders[i]->ReleaseShaderVariables();
			m_ppNPCShaders[i]->ReleaseObjects();
			m_ppNPCShaders[i]->Release();
		}
		delete[] m_ppNPCShaders;
	}
	if (m_pOutlineShader)
	{
		m_pOutlineShader->ReleaseShaderVariables();
		delete m_pOutlineShader;
	}

	//if (m_ppEnvironmentMappingShaders)
	//{
	//	for (int i = 0; i < m_nEnvironmentMappingShaders; i++)
	//	{
	//		m_ppEnvironmentMappingShaders[i]->ReleaseShaderVariables();
	//		m_ppEnvironmentMappingShaders[i]->ReleaseObjects();
	//		m_ppEnvironmentMappingShaders[i]->Release();
	//	}
	//	delete[] m_ppEnvironmentMappingShaders;
	//}

	if (m_ppParticleObjects)
	{
		for (int i = 0; i < m_nParticleObjects; i++) delete m_ppParticleObjects[i];
		delete[] m_ppParticleObjects;
	}
	if (m_pDepthRenderShader)
	{
		m_pDepthRenderShader->ReleaseShaderVariables();
		m_pDepthRenderShader->ReleaseObjects();
		m_pDepthRenderShader->Release();
	}
	if (m_pShadowShader)delete m_pShadowShader;

	ReleaseShaderVariables();

	if (m_pTerrain) delete m_pTerrain;
	if (m_pShader) delete m_pShader;
	if (m_pSkyBox) delete m_pSkyBox;
	if (m_pUseWaterMove) delete m_pUseWaterMove;
	if (m_pLights)  delete[] m_pLights;
	if (m_pMaterials) delete m_pMaterials;
}
void SceneManager::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbLightElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbLightElementBytes, 
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);
}
void SceneManager::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHTS) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));

	if (m_pcbMappedMaterials) ::memcpy(m_pcbMappedMaterials, m_pMaterials, sizeof(MATERIALS));
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
	if (m_pUseWaterMove) m_pUseWaterMove->ReleaseShaderVariables();
	if (m_pTerrain) m_pTerrain->ReleaseShaderVariables();
	if (m_pSkyBox) m_pSkyBox->ReleaseShaderVariables();
}
void SceneManager::ReleaseUploadBuffers()
{
	if (m_pUseWaterMove) m_pUseWaterMove->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();

	if (m_pShadowShader) m_pShadowShader->ReleaseUploadBuffers();
	if (m_pDepthRenderShader) m_pDepthRenderShader->ReleaseUploadBuffers();

	//for (int i = 0; i < m_nEnvironmentMappingShaders; i++) m_ppEnvironmentMappingShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nShadowShaders; i++) m_ppShadowShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 3; i++) m_ppNPCShaders[i]->ReleaseUploadBuffers();
	//for (int i = 0; i < 1; i++) m_ppSpriteShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
}
bool SceneManager::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}
bool SceneManager::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{

		case 'F':
			m_ppSpriteShaders[0]->m_bSpriteActive = !m_ppSpriteShaders[0]->m_bSpriteActive;
			break;
		case 'V':
			m_bWarMode = !m_bWarMode;
			break;
		case 'T':
			m_bOutlineMode = !m_bOutlineMode;
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
bool SceneManager::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}
void SceneManager::AnimateObjects(CCamera* pCamera, float fTimeElapsed)
{
	m_pUseWaterMove->Animate(fTimeElapsed);
	//for (int i = 0; i < m_nEnvironmentMappingShaders; i++) m_ppEnvironmentMappingShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->AnimateObject(pCamera, fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < 3; i++) if (m_ppNPCShaders[i]) m_ppNPCShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
	

		m_fLightRotationAngle += fTimeElapsed;
		XMMATRIX xmmtxRotation = XMMatrixRotationY(fTimeElapsed * 1.0f);
			//XMStoreFloat3(&m_pLights->m_pLights[0].m_xmf3Direction, XMVector3TransformNormal(XMLoadFloat3(&m_pLights->m_pLights[0].m_xmf3Direction), xmmtxRotation));
			XMStoreFloat3(&m_pLights->m_pLights[3].m_xmf3Direction, XMVector3TransformNormal(XMLoadFloat3(&m_pLights->m_pLights[3].m_xmf3Direction), xmmtxRotation));
			XMStoreFloat3(&m_pLights->m_pLights[4].m_xmf3Direction, XMVector3TransformNormal(XMLoadFloat3(&m_pLights->m_pLights[4].m_xmf3Direction), xmmtxRotation));
			//XMStoreFloat3(&m_pLights->m_pLights[1].m_xmf3Direction, XMVector3TransformNormal(XMLoadFloat3(&m_pLights->m_pLights[1].m_xmf3Direction), xmmtxRotation));

	}
	if (m_pLights)
	{
		XMFLOAT3 PlayerLook = m_pPlayer->GetLookVector();
		XMFLOAT3 PlayerPos = m_pPlayer->GetPosition();
		CCamera* pCameraLook = m_pPlayer->GetCamera();
		XMFLOAT3 pCameraLookV = pCameraLook->GetLookVector();

		XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerLook, pCameraLookV));
		
		m_pLights->m_pLights[1].m_xmf3Position = PlayerPos;
		m_pLights->m_pLights[1].m_xmf3Direction = TotalLookVector;
	}
}
void SceneManager::OnShadowPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pDepthRenderShader->PrepareShadowMap(pd3dCommandList);
	for (int j = 0; j < MAX_LIGHTS; j++)
	{
		if (m_pLights->m_pLights[j].m_bEnable)
		{
			XMFLOAT3 xmf3Position = m_pLights->m_pLights[j].m_xmf3Position;
			XMFLOAT3 xmf3Look = m_pLights->m_pLights[j].m_xmf3Direction;
			XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

			XMMATRIX xmmtxView = XMMatrixLookToLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Up));

			float fNearPlaneDistance = 20.0f, fFarPlaneDistance = m_pLights->m_pLights[j].m_fRange;

			XMMATRIX xmmtxProjection{};
			if (m_pLights->m_pLights[j].m_nType == DIRECTIONAL_LIGHT)
			{
				float fWidth = _PLANE_WIDTH, fHeight = _PLANE_HEIGHT;
				xmmtxProjection = XMMatrixOrthographicLH(fWidth, fHeight, fNearPlaneDistance, fFarPlaneDistance);
			}
			else if (m_pLights->m_pLights[j].m_nType == SPOT_LIGHT)
			{
				float fFovAngle = 60.0f; // m_pLights->m_pLights[j].m_fPhi = cos(60.0f);
				float fAspectRatio = float(_DEPTH_BUFFER_WIDTH) / float(_DEPTH_BUFFER_HEIGHT);
				xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
			}
			else if (m_pLights->m_pLights[j].m_nType == POINT_LIGHT)
			{
	//			//ShadowMap[6]
			}

			m_pDepthRenderShader->m_ppDepthRenderCameras[j]->SetPosition(xmf3Position);
			XMStoreFloat4x4(&m_pDepthRenderShader->m_ppDepthRenderCameras[j]->m_xmf4x4View, xmmtxView);
			XMStoreFloat4x4(&m_pDepthRenderShader->m_ppDepthRenderCameras[j]->m_xmf4x4Projection, xmmtxProjection);

			XMMATRIX xmmtxToTexture = XMMatrixTranspose(xmmtxView * xmmtxProjection * m_pDepthRenderShader->m_xmProjectionToTexture);
			XMStoreFloat4x4(&m_pDepthRenderShader->m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4x4ToTexture, xmmtxToTexture);

			m_pDepthRenderShader->m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position = XMFLOAT4(xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);

			::SynchronizeResourceTransition(pd3dCommandList, m_pDepthRenderShader->m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

			FLOAT pfClearColor[4] = { 1.0f,0.6f,0.6f, 1.0 };
			pd3dCommandList->ClearRenderTargetView(m_pDepthRenderShader->m_pd3dRtvCPUDescriptorHandles[j], pfClearColor, 0, NULL);

			pd3dCommandList->ClearDepthStencilView(m_pDepthRenderShader->m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);

			pd3dCommandList->OMSetRenderTargets(1, &m_pDepthRenderShader->m_pd3dRtvCPUDescriptorHandles[j], TRUE, &m_pDepthRenderShader->m_d3dDsvDescriptorCPUHandle);

			m_pDepthRenderShader->Render(pd3dCommandList, m_pDepthRenderShader->m_ppDepthRenderCameras[j],0);

			::SynchronizeResourceTransition(pd3dCommandList, m_pDepthRenderShader->m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		}
		else
		{
			m_pDepthRenderShader->m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position.w = 0.0f;
		}
	}
}
void SceneManager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	UpdateShaderVariables(pd3dCommandList);
	m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
	////for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i])m_ppShaders[i]->Render(pd3dCommandList, pCamera, 0);
	//////for (int i = 0; i < 3; i++) if (m_ppNPCShaders[i])m_ppNPCShaders[i]->Render(pd3dCommandList, pCamera, 0);
	//if (m_pUseWaterMove) m_pUseWaterMove->Render(pd3dCommandList, pCamera);
	//if (m_bOutlineMode)
	//{
	//	for (int i = 0; i < m_ppNPCShaders[0]->m_nObjects; i++)
	//	{
	//		m_pOutlineShader->UpdateShaderVariables(pd3dCommandList, m_ppNPCShaders[0]->m_ppObjects[i]);
	//		m_pOutlineShader->Render(pd3dCommandList, pCamera, 0);
	//		m_ppNPCShaders[0]->m_ppObjects[i]->Render(pd3dCommandList, pCamera);
	//		m_pOutlineShader->Render(pd3dCommandList, pCamera, 1);
	//		m_ppNPCShaders[0]->m_ppObjects[i]->Render(pd3dCommandList, pCamera);
	//	}
	//}

	if (m_pShadowShader) m_pShadowShader->Render(pd3dCommandList, pCamera, 0);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);
}
void SceneManager::OnPreRender(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, HANDLE hFenceEvent)
{
	/*for (int i = 0; i < m_nEnvironmentMappingShaders; i++)
	{
		m_ppEnvironmentMappingShaders[i]->OnPreRender(pd3dDevice, pd3dCommandQueue, pd3dFence, hFenceEvent, this);
	}*/
}
void SceneManager::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	UpdateShaderVariables(pd3dCommandList);

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
void SceneManager::RenderSprite(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//for (int i = 0; i < m_nSpriteObjects; i++) m_ppSpriteObjects[i]->Render(pd3dCommandList, pCamera);
}
void SceneManager::OnPostRenderParticle()
{
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->OnPostRender();
}





