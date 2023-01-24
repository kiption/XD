

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

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 1000.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(30.0f, 30.0f, 30.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 1200.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.8f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(0.0f, 10.0f, 2.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.5f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_bEnable = true;

	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_fRange = 3000.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);


	m_pMaterials = new MATERIALS;
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

}


void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);


	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(100.0f, 20.5f, 100.0f);
	XMFLOAT3 xmf3Normal(0.0f, 0.5f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/Mountain.raw"), 257, 257, 257, 257, xmf3Scale, xmf3Normal);
	m_pTerrain->SetPosition(0.0, 0.0, 0.0);


	XMFLOAT3 xmf4ScaleW(80.0f, 2.5f, 80.0);
	m_pUseWaterMove = new CUseWaterMoveTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/waterterrain8bit.raw"), 257, 257, 8, 8, xmf4ScaleW, xmf3Normal);
	m_pUseWaterMove->SetPosition(0.0, 60.0f, 0.0);


	m_nShaders = 4;

	m_ppShaders = new CObjectsShader * [m_nShaders];

	CObjectsShader* pObjectsShader = new CObjectsShader();
	pObjectsShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[0] = pObjectsShader;

	CBillboardObjectShader* pBillboardObjectShader = new CBillboardObjectShader();
	pBillboardObjectShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pBillboardObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders[1] = pBillboardObjectShader;

	CSnowObjectShader* pCSnowObjectShader = new CSnowObjectShader();
	pCSnowObjectShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pCSnowObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppShaders[2] = pCSnowObjectShader;

	CCrossHairShader* pCrossHeadShader = new CCrossHairShader();
	pCrossHeadShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pCrossHeadShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pPlayer);
	m_ppShaders[3] = pCrossHeadShader;




	int m_nNPCShaders = 3;
	m_ppNPCShaders = new CNPCShader * [m_nNPCShaders];
	CNPCShader* pCNPCShader = new CNPCShader();
	pCNPCShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pCNPCShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppNPCShaders[0] = pCNPCShader;

	CBulletMotionShader* pCBulletMotionShader = new CBulletMotionShader();
	pCBulletMotionShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pCBulletMotionShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppNPCShaders[1] = pCBulletMotionShader;

	CExplosionShader* pCExplosionShader = new CExplosionShader();
	pCExplosionShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pCExplosionShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pCExplosionShader->SetActive(false);
	m_ppNPCShaders[2] = pCExplosionShader;
	/// ///
	m_ppOtherPlayers = new COtherPlayerShader * [1];
	COtherPlayerShader* pCOtherPlayerShader = new COtherPlayerShader();
	pCOtherPlayerShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	pCOtherPlayerShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppOtherPlayers[0] = pCOtherPlayerShader;

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

	m_nEnvironmentMappingShaders = 1;
	m_ppEnvironmentMappingShaders = new CDynamicCubeMappingShader * [m_nEnvironmentMappingShaders];

	m_ppEnvironmentMappingShaders[0] = new CDynamicCubeMappingShader(256);
	m_ppEnvironmentMappingShaders[0]->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	m_ppEnvironmentMappingShaders[0]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);

	BuildDefaultLightsAndMaterials();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pOutlineShader = new COutlineShader();
	m_pOutlineShader->CreateGraphicsPipelineState(pd3dDevice, m_pd3dGraphicsRootSignature, 0);
	m_pOutlineShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void CScene::ReleaseObjects()
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

	if (m_ppOtherPlayers)
	{
		for (int i = 0; i < 1; i++)
		{
			m_ppOtherPlayers[i]->ReleaseShaderVariables();
			m_ppOtherPlayers[i]->ReleaseObjects();
			m_ppOtherPlayers[i]->Release();
		}
		delete[] m_ppOtherPlayers;
	}

	if (m_pOutlineShader)
	{
		m_pOutlineShader->ReleaseShaderVariables();
		delete m_pOutlineShader;
	}

	if (m_ppEnvironmentMappingShaders)
	{
		for (int i = 0; i < m_nEnvironmentMappingShaders; i++)
		{
			m_ppEnvironmentMappingShaders[i]->ReleaseShaderVariables();
			m_ppEnvironmentMappingShaders[i]->ReleaseObjects();
			m_ppEnvironmentMappingShaders[i]->Release();
		}
		delete[] m_ppEnvironmentMappingShaders;
	}

	if (m_ppParticleObjects)
	{
		for (int i = 0; i < m_nParticleObjects; i++) delete m_ppParticleObjects[i];
		delete[] m_ppParticleObjects;
	}

	ReleaseShaderVariables();

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;
	if (m_pUseWaterMove) delete m_pUseWaterMove;
	if (m_pLights) delete[] m_pLights;
	if (m_pMaterials) delete m_pMaterials;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[18];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 7;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //t0: gtxtStandardTextures[7] //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
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
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtDetailAlbedoTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtDetailNormalTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 5;
	pd3dDescriptorRanges[8].BaseShaderRegister = 14; //t14~18: gtxtTerrainTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 19; //t19: gtxtTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 3;
	pd3dDescriptorRanges[10].BaseShaderRegister = 20; //t20~22: gtxtWaterTexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 23; //t23: gtxtParticleTexture
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 24; //t24: gtxtRandomTexture
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[14].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[14].NumDescriptors = 1;
	pd3dDescriptorRanges[14].BaseShaderRegister = 25; //t25: gtxtRandomOnSphereTexture
	pd3dDescriptorRanges[14].RegisterSpace = 0;
	pd3dDescriptorRanges[14].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[15].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[15].NumDescriptors = 1;
	pd3dDescriptorRanges[15].BaseShaderRegister = 6; // GameObject b6
	pd3dDescriptorRanges[15].RegisterSpace = 0;
	pd3dDescriptorRanges[15].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[16].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[16].NumDescriptors = 1;
	pd3dDescriptorRanges[16].BaseShaderRegister = 26; //t26: gtxtTextureCube : Dynamic Texture
	pd3dDescriptorRanges[16].RegisterSpace = 0;
	pd3dDescriptorRanges[16].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[17].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[17].NumDescriptors = 1;
	pd3dDescriptorRanges[17].BaseShaderRegister = 8; // DynamicObject b8
	pd3dDescriptorRanges[17].RegisterSpace = 0;
	pd3dDescriptorRanges[17].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[22];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; // GameObject b2
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

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[12].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[14].Descriptor.ShaderRegister = 5; //FrameWorkInfo
	pd3dRootParameters[14].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[12]; //t23: gtxtParticleTexture
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[13]; //t24: gtxtRandomTexture
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[17].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[14]; //t25: gtxtRandomOnSphereTexture
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[18].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[18].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[15];
	pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[19].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[19].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[19].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[16]; // t26: gtxtTextureCube : Dynamic Texture
	pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[20].Descriptor.ShaderRegister = 7; //Material
	pd3dRootParameters[20].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[21].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[21].DescriptorTable.NumDescriptorRanges = 1; // Dynamic
	pd3dRootParameters[21].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[17];
	pd3dRootParameters[21].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

#else
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 7;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtStandardTextures[7] //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

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
#endif

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[4];

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
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[3].MipLODBias = 0;
	pd3dSamplerDescs[3].MaxAnisotropy = 1;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].ShaderRegister = 3;
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
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
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHTS) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	if (m_pcbMappedMaterials) ::memcpy(m_pcbMappedMaterials, m_pMaterials, sizeof(MATERIALS));
}

void CScene::ReleaseShaderVariables()
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

void CScene::ReleaseUploadBuffers()
{

	if (m_pUseWaterMove) m_pUseWaterMove->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();

	for (int i = 0; i < m_nEnvironmentMappingShaders; i++) m_ppEnvironmentMappingShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 3; i++) m_ppNPCShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 1; i++) m_ppOtherPlayers[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
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

		case 'F':
			m_ppShaders[2]->SetActive(!m_ppShaders[2]->GetActive());
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

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(CCamera* pCamera, float fTimeElapsed)
{
	m_pUseWaterMove->Animate(fTimeElapsed);
	for (int i = 0; i < m_nEnvironmentMappingShaders; i++) m_ppEnvironmentMappingShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->AnimateObject(pCamera, fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < 3; i++) if (m_ppNPCShaders[i]) m_ppNPCShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < 1; i++) if (m_ppOtherPlayers[i]) m_ppOtherPlayers[i]->AnimateObjects(fTimeElapsed);

	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < 10; i++) m_ppNPCShaders[0]->m_ppObjects[i]->xoobb = BoundingOrientedBox(XMFLOAT3(m_ppNPCShaders[0]->m_ppObjects[i]->GetPosition()), XMFLOAT3(18.0, 20.0, 25.0), XMFLOAT4(0, 0, 0, 1));
	m_pPlayer->xoobb = BoundingOrientedBox(XMFLOAT3(m_pPlayer->GetPosition()), XMFLOAT3(15.0, 15.0, 18.0), XMFLOAT4(0, 0, 0, 1));
	CBulletObject** ppBullets = ((CMainPlayer*)m_pPlayer)->m_ppBullets;
	CBulletObject** ppBulletsR = ((CMainPlayer*)m_pPlayer)->m_ppBullets2;
	for (int j = 0; j < BULLETS; j++) { ppBullets[j]->xoobb = BoundingOrientedBox(XMFLOAT3(ppBullets[j]->GetPosition()), XMFLOAT3(8.0, 8.0, 12.0), XMFLOAT4(0, 0, 0, 1)); }
	for (int j = 0; j < BULLETS2; j++) { ppBulletsR[j]->xoobb = BoundingOrientedBox(XMFLOAT3(ppBulletsR[j]->GetPosition()), XMFLOAT3(8.0, 8.0, 12.0), XMFLOAT4(0, 0, 0, 1)); }

	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < 10; i++)
	{
		if (m_ppNPCShaders[0]->m_ppObjects[i]->xoobb.Intersects(m_pPlayer->xoobb))
		{
			m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._43 -= 2.0f;
		}
	}

	for (int j = 0; j < BULLETS; j++)
	{
		for (int i = 0; i < 10; i++)
		{
			for (int i = 0; i < 10; i++)
			{
				if (ppBullets[j]->m_bActive && m_ppNPCShaders[0]->m_ppObjects[i]->xoobb.Intersects(ppBullets[j]->xoobb))
				{
					gamesound.collisionSound();
					m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._42 -= 2.0f;
					m_ppNPCShaders[2]->m_bActive = true;
					CollisionCheck = i;
					ppBullets[j]->Reset();
				}
			}

			if (ppBullets[j]->m_bActive)
			{
				m_ppNPCShaders[1]->m_bBulletActive = true;
				m_ppNPCShaders[1]->BulletPosition = ppBullets[j]->GetPosition();
			}
		}
	}

	for (int j = 0; j < BULLETS2; j++)
	{
		for (int i = 0; i < 10; i++)
		{
			for (int i = 0; i < 10; i++)
			{
				if (ppBulletsR[j]->m_bActive && m_ppNPCShaders[0]->m_ppObjects[i]->xoobb.Intersects(ppBulletsR[j]->xoobb))
				{
					gamesound.collisionSound();
					m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._42 -= 2.0f;
					m_ppNPCShaders[2]->m_bActive = true;
					CollisionCheck = i;
					ppBulletsR[j]->Reset();
				}
			}

			if (ppBulletsR[j]->m_bActive)
			{
				m_ppNPCShaders[1]->m_bBulletActive = true;
				m_ppNPCShaders[1]->BulletPositionR = ppBulletsR[j]->GetPosition();
			}
		}
	}

	if (m_ppNPCShaders[2]->m_bActive == true)
	{

		m_ppNPCShaders[2]->m_fTime += 0.1;
		m_ppNPCShaders[2]->TargetPosition = m_ppNPCShaders[0]->m_ppObjects[CollisionCheck]->GetPosition();

		if (m_ppNPCShaders[2]->m_fTime > 0.0)
		{
			m_ppNPCShaders[2]->m_fSpeed = 1;
		}
		if (m_ppNPCShaders[2]->m_fTime > 1.0)
		{
			m_ppNPCShaders[2]->m_fSpeed = 2;
		}
		if (m_ppNPCShaders[2]->m_fTime > 2.0)
		{
			m_ppNPCShaders[2]->m_fSpeed = 3;
		}
		if (m_ppNPCShaders[2]->m_fTime > 3.0)
		{
			m_ppNPCShaders[1]->m_bBulletActive = false;
			m_ppNPCShaders[2]->m_bActive = false;
			m_ppNPCShaders[2]->m_fTime = 0.0;
			CollisionCheck = 0;
		}

	}

	if (m_bWarMode == true) WarMode();
	for (int i = 0; i < 10; i++)
	{
		float fHeight = m_pTerrain->GetHeight(m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._41, m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._43) + 4.0f;
		if (m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._42 < fHeight)
		{
			m_ppNPCShaders[0]->m_ppObjects[i]->m_xmf4x4Transform._42 = fHeight + 2.0f;
		}
	}

	if (m_pLights)
	{
		XMFLOAT3 PlayerLook = m_pPlayer->GetLookVector();
		XMFLOAT3 PlayerPos = m_pPlayer->GetPosition();
		CCamera* pCameraLook = m_pPlayer->GetCamera();
		XMFLOAT3 pCameraLookV = pCameraLook->GetLookVector();

		XMFLOAT3 TotalLookVector = Vector3::Normalize(Vector3::Add(PlayerLook, pCameraLookV));
		PlayerPos.z += 15.0f;
		m_pLights[1].m_xmf3Position = PlayerPos;
		m_pLights[1].m_xmf3Direction = TotalLookVector;
	}
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
	//for (int i = 0; i < m_nEnvironmentMappingShaders; i++)m_ppEnvironmentMappingShaders[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i])m_ppShaders[i]->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < 3; i++) if (m_ppNPCShaders[i])m_ppNPCShaders[i]->Render(pd3dCommandList, pCamera, 0);
	//for (int i = 0; i < 1; i++) if (m_ppOtherPlayers[i])m_ppOtherPlayers[i]->Render(pd3dCommandList, pCamera, 0);

	//if (m_pUseWaterMove) m_pUseWaterMove->Render(pd3dCommandList, pCamera);

	if (m_bOutlineMode)
	{
		for (int i = 0; i < m_ppNPCShaders[0]->m_nObjects; i++)
		{
			m_pOutlineShader->UpdateShaderVariables(pd3dCommandList, m_ppNPCShaders[0]->m_ppObjects[i]);
			m_pOutlineShader->Render(pd3dCommandList, pCamera, 0);
			m_ppNPCShaders[0]->m_ppObjects[i]->Render(pd3dCommandList, pCamera);
			m_pOutlineShader->Render(pd3dCommandList, pCamera, 1);
			m_ppNPCShaders[0]->m_ppObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}
void CScene::OnPreRender(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, HANDLE hFenceEvent)
{
	for (int i = 0; i < m_nEnvironmentMappingShaders; i++)
	{
		m_ppEnvironmentMappingShaders[i]->OnPreRender(pd3dDevice, pd3dCommandQueue, pd3dFence, hFenceEvent, this);
	}
}
void CScene::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

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

void CScene::RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->Render(pd3dCommandList, pCamera);
}

void CScene::OnPostRenderParticle()
{
	for (int i = 0; i < m_nParticleObjects; i++) m_ppParticleObjects[i]->OnPostRender();
}

void CScene::WarMode()
{
	XMFLOAT3 xmf3CameraPosition = m_pPlayer->GetCamera()->GetPosition();

	for (int j = 0; j < 5; j++)
	{
		m_ppNPCShaders[0]->m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0, 1, 0));
		m_ppNPCShaders[0]->m_ppObjects[j]->m_xmf4x4Transform._41 += (((-m_ppNPCShaders[0]->m_ppObjects[j]->m_xmf4x4Transform._41 + m_pPlayer->GetPosition().x) / 10.0f) - (j)+5.0) / 50;
		m_ppNPCShaders[0]->m_ppObjects[j]->m_xmf4x4Transform._42 += (((-m_ppNPCShaders[0]->m_ppObjects[j]->m_xmf4x4Transform._42 + m_pPlayer->GetPosition().y)) / 10 + 3.5) / 100;
		m_ppNPCShaders[0]->m_ppObjects[j]->m_xmf4x4Transform._43 += (((-m_ppNPCShaders[0]->m_ppObjects[j]->m_xmf4x4Transform._43 + m_pPlayer->GetPosition().z) / 10.0f) - (j)+3.0) / 50;


	}
	for (int k = 5; k < 10; k++)
	{
		m_ppNPCShaders[0]->m_ppObjects[k]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0, 1, 0));
		m_ppNPCShaders[0]->m_ppObjects[k]->m_xmf4x4Transform._41 += (((-m_ppNPCShaders[0]->m_ppObjects[k]->m_xmf4x4Transform._41 + m_pPlayer->GetPosition().x) / 10.0f) - k + 3.0) / 50;
		m_ppNPCShaders[0]->m_ppObjects[k]->m_xmf4x4Transform._42 += (((-m_ppNPCShaders[0]->m_ppObjects[k]->m_xmf4x4Transform._42 + m_pPlayer->GetPosition().y)) / 10 + 3.5) / 100;
		m_ppNPCShaders[0]->m_ppObjects[k]->m_xmf4x4Transform._43 += (((-m_ppNPCShaders[0]->m_ppObjects[k]->m_xmf4x4Transform._43 + m_pPlayer->GetPosition().z) / 10.0f) - k + 5.0) / 50;


	}
}
