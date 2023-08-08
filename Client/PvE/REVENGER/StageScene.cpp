#include "stdafx.h"
#include "StageScene.h"
ID3D12DescriptorHeap* MainGameScene::m_pd3dCbvSrvDescriptorHeap = NULL;
D3D12_CPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dSrvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	MainGameScene::m_d3dSrvGPUDescriptorNextHandle;
float RandomDir(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}
float RandomDir()
{
	return(rand() / float(RAND_MAX));
}
inline float RandD(float fMin, float fMax)
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}
DirectX::XMFLOAT3 RandomDirection(float fMin, float fMax)
{
	float x = RandD(-2.0f, 2.0f);
	float y = RandD(0.1f, 0.3f);
	float z = RandD(0.1f, 0.2f);

	DirectX::XMVECTOR direction = DirectX::XMVectorSet(x, y, z, 0.0f);
	direction = DirectX::XMVector3Normalize(direction);

	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, direction);

	return result;
}
DirectX::XMFLOAT3 RandomPosition(float fMin, float fMax)
{
	float x = RandD(-25.0f, -45.0f);
	float y = 8.0f;
	float z = RandD(740.0f, 760.0f);

	DirectX::XMVECTOR direction = DirectX::XMVectorSet(x, y, z, 0.0f);
	direction = DirectX::XMVector3Normalize(direction);

	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, direction);

	return result;
}

MainGameScene::MainGameScene() : SceneMgr()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

MainGameScene::~MainGameScene()
{
}
void MainGameScene::BuildDefaultLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));
	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 40000.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2, 0.2f, 0.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.6, 0.6, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(-400, 500.0f, 1000.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.3f, -1.0f, -1.0f);

	m_pLights->m_pLights[1].m_bEnable = true;
	m_pLights->m_pLights[1].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 40000.0;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2, 0.2f, 0.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7, 0.7, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.2, 0.2f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-650, 700.0f, 1200.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.4f, -1.0f, -1.0f);

	m_pLights->m_pLights[2].m_bEnable = false;
	m_pLights->m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[2].m_fRange = 400.0;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.9f, 0.5f, 0.1f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.5f, 0.1f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.8f, 0.5f, 0.1f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Emissive = XMFLOAT4(0.9f, 0.5f, 0.1f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Position = XMFLOAT3(20.0, 15, 50);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, -0.0f);
	m_pLights->m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.001f);
	m_pLights->m_pLights[2].m_fFalloff = 80.0f;
	m_pLights->m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(359.0f));
	m_pLights->m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(10.0));

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 400.0;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -0.2f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 14.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

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

void MainGameScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, ID3D12Resource* pd3dDepthStencilBuffer)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	CreateDescriptorHeaps(pd3dDevice, 0, 350);
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_pSkyBox->SetCurScene(INGAME_SCENE);

	m_nBillboardShaders = 17;
	m_pBillboardShader = new BillboardShader * [m_nBillboardShaders];

	BillboardParticleShader* pBillboardParticleShader = new BillboardParticleShader();
	pBillboardParticleShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBillboardParticleShader->SetCurScene(INGAME_SCENE);
	pBillboardParticleShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[0] = pBillboardParticleShader;

	HealPackBillboardShader* pCrossHairShader = new HealPackBillboardShader();
	pCrossHairShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pCrossHairShader->SetCurScene(INGAME_SCENE);
	pCrossHairShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[1] = pCrossHairShader;

	BloodHittingBillboard* pBloodHittingBillboard = new BloodHittingBillboard();
	pBloodHittingBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBloodHittingBillboard->SetCurScene(INGAME_SCENE);
	pBloodHittingBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[2] = pBloodHittingBillboard;

	SparkBillboard* pHelicopterSparkBillboard = new SparkBillboard();
	pHelicopterSparkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHelicopterSparkBillboard->SetCurScene(INGAME_SCENE);
	pHelicopterSparkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHelicopterSparkBillboard->m_bActive = true;
	m_pBillboardShader[3] = pHelicopterSparkBillboard;

	BulletMarkBillboard* pBulletMarkBillboard = new BulletMarkBillboard();
	pBulletMarkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBulletMarkBillboard->SetCurScene(INGAME_SCENE);
	pBulletMarkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[4] = pBulletMarkBillboard;

	HeliHittingMarkBillboard* pHeliHittingMarkBillboard = new HeliHittingMarkBillboard();
	pHeliHittingMarkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHeliHittingMarkBillboard->SetCurScene(INGAME_SCENE);
	pHeliHittingMarkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHeliHittingMarkBillboard->m_bActive = true;
	m_pBillboardShader[5] = pHeliHittingMarkBillboard;

	MuzzleFrameBillboard* pMuzzleFrameBillboard = new MuzzleFrameBillboard();
	pMuzzleFrameBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pMuzzleFrameBillboard->SetCurScene(INGAME_SCENE);
	pMuzzleFrameBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[6] = pMuzzleFrameBillboard;

	MuzzleFrameBillboard* pMuzzleFrameBillboard2 = new MuzzleFrameBillboard();
	pMuzzleFrameBillboard2->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pMuzzleFrameBillboard2->SetCurScene(INGAME_SCENE);
	pMuzzleFrameBillboard2->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[7] = pMuzzleFrameBillboard2;

	HeliHittingMarkBillboard* pOtherPlyHeliHittingMarkBillboard = new HeliHittingMarkBillboard();
	pOtherPlyHeliHittingMarkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pOtherPlyHeliHittingMarkBillboard->SetCurScene(INGAME_SCENE);
	pOtherPlyHeliHittingMarkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pOtherPlyHeliHittingMarkBillboard->m_bActive = true;
	m_pBillboardShader[8] = pOtherPlyHeliHittingMarkBillboard;

	HeliHittingMarkBillboard* pNPCHeliHittingMarkBillboard = new HeliHittingMarkBillboard();
	pNPCHeliHittingMarkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pNPCHeliHittingMarkBillboard->SetCurScene(INGAME_SCENE);
	pNPCHeliHittingMarkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pNPCHeliHittingMarkBillboard->m_bActive = true;
	m_pBillboardShader[9] = pNPCHeliHittingMarkBillboard;

	HealPackBillboardShader* pHealPack1Shader = new HealPackBillboardShader();
	pHealPack1Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack1Shader->SetCurScene(INGAME_SCENE);
	pHealPack1Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHealPack1Shader->m_bActive = true;
	m_pBillboardShader[10] = pHealPack1Shader;

	HealPackBillboardShader* pHealPack2Shader = new HealPackBillboardShader();
	pHealPack2Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack2Shader->SetCurScene(INGAME_SCENE);
	pHealPack2Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHealPack2Shader->m_bActive = true;
	m_pBillboardShader[11] = pHealPack2Shader;

	HealPackBillboardShader* pHealPack3Shader = new HealPackBillboardShader();
	pHealPack3Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack3Shader->SetCurScene(INGAME_SCENE);
	pHealPack3Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[12] = pHealPack3Shader;

	HealPackBillboardShader* pHealPack4Shader = new HealPackBillboardShader();
	pHealPack4Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack4Shader->SetCurScene(INGAME_SCENE);
	pHealPack4Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHealPack4Shader->m_bActive = true;
	m_pBillboardShader[13] = pHealPack4Shader;

	HealPackBillboardShader* pHealPack5Shader = new HealPackBillboardShader();
	pHealPack5Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack5Shader->SetCurScene(INGAME_SCENE);
	pHealPack5Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHealPack5Shader->m_bActive = true;
	m_pBillboardShader[14] = pHealPack5Shader;

	HealPackBillboardShader* pHealPack6Shader = new HealPackBillboardShader();
	pHealPack6Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack6Shader->SetCurScene(INGAME_SCENE);
	pHealPack6Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHealPack6Shader->m_bActive = true;
	m_pBillboardShader[15] = pHealPack6Shader;

	HealPackBillboardShader* pHealPack7Shader = new HealPackBillboardShader();
	pHealPack7Shader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHealPack7Shader->SetCurScene(INGAME_SCENE);
	pHealPack7Shader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHealPack7Shader->m_bActive = true;
	m_pBillboardShader[16] = pHealPack7Shader;

	m_nSpriteBillboards = 1;
	m_ppSpriteBillboard = new CSpriteObjectsShader * [m_nSpriteBillboards];
	SpriteAnimationBillboard* pSpriteAnimationBillboard = new SpriteAnimationBillboard();
	pSpriteAnimationBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pSpriteAnimationBillboard->SetCurScene(INGAME_SCENE);
	pSpriteAnimationBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pSpriteAnimationBillboard->SetActive(false);
	m_ppSpriteBillboard[0] = pSpriteAnimationBillboard;

	m_nFragShaders = 3;
	m_ppFragShaders = new CFragmentsShader * [m_nFragShaders];
	CFragmentsShader* pFragmentsShader = new CFragmentsShader();
	pFragmentsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pFragmentsShader->SetCurScene(INGAME_SCENE);
	pFragmentsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppFragShaders[0] = pFragmentsShader;

	CHelicopterBulletMarkParticleShader* pValkanHittingPointShader = new CHelicopterBulletMarkParticleShader();
	pValkanHittingPointShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pValkanHittingPointShader->SetCurScene(INGAME_SCENE);
	pValkanHittingPointShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppFragShaders[1] = pValkanHittingPointShader;

	CHumanBulletMarkParticleShader* pRifleHittingPointShader = new CHumanBulletMarkParticleShader();
	pRifleHittingPointShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pRifleHittingPointShader->SetCurScene(INGAME_SCENE);
	pRifleHittingPointShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppFragShaders[2] = pRifleHittingPointShader;

	m_nShaders = 1;
	m_ppShaders = new CObjectsShader * [m_nShaders];

	CObjectsShader* pObjectShader = new CObjectsShader();
	pObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pObjectShader->SetCurScene(INGAME_SCENE);
	pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders[0] = pObjectShader;

	m_pDepthRenderShader = new CDepthRenderShader(pObjectShader, m_pLights->m_pLights);
	m_pDepthRenderShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	m_pDepthRenderShader->SetCurScene(INGAME_SCENE);
	m_pDepthRenderShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);

	m_pShadowShader = new CShadowMapShader(pObjectShader);
	m_pShadowShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	m_pShadowShader->SetCurScene(INGAME_SCENE);
	m_pShadowShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pDepthRenderShader->GetDepthTexture());

	m_pTreeBlendShadowShader = new CTreeBlendingShadowShader(pObjectShader);
	m_pTreeBlendShadowShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	m_pTreeBlendShadowShader->SetCurScene(INGAME_SCENE);
	m_pTreeBlendShadowShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pDepthRenderShader->GetDepthTexture());

	pBCBulletEffectShader = new CBulletEffectShader();
	pBCBulletEffectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBCBulletEffectShader->SetCurScene(INGAME_SCENE);

	CGameObject* pBulletMesh = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", pBCBulletEffectShader);
	for (int i = 0; i < HELIBULLETS; i++)
	{
		pBulletObject = new CValkanObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh, false);
		pBulletObject->SetMovingSpeed(200.0f);
		pBulletObject->SetActive(false);
		pBulletObject->SetCurScene(INGAME_SCENE);
		m_ppBullets[i] = pBulletObject;
		pBulletMesh->AddRef();
	}
	CGameObject* pCartrudgetMesh = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bullet4.bin", pBCBulletEffectShader);
	for (int i = 0; i < CARTRIDGES; i++)
	{
		pCartridge = new CValkanObject(1.5);
		pCartridge->SetChild(pCartrudgetMesh, false);
		pCartridge->SetMovingSpeed(6.5f);
		pCartridge->SetActive(false);
		pCartridge->SetCurScene(INGAME_SCENE);
		m_ppCartridge[i] = pCartridge;
		pCartrudgetMesh->AddRef();
	}
	for (int i = 0; i < HELICOPTERVALKANS; i++)
	{
		CGameObject* pHelicopterValkanMesh = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bullet4.bin", pBCBulletEffectShader);
		pValkan = new CValkanObject(300.0);
		pValkan->SetChild(pHelicopterValkanMesh, false);
		pValkan->SetMovingSpeed(1000.0f);
		pValkan->SetActive(false);
		pValkan->SetCurScene(INGAME_SCENE);
		m_ppValkan[i] = pValkan;
		pHelicopterValkanMesh->AddRef();
	}
	OtherHeliPlayerTransformStore();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void MainGameScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	for (int i = 0; i < HELIBULLETS; i++) if (m_ppBullets[i]) delete[] m_ppBullets[i];
	for (int i = 0; i < CARTRIDGES; i++) if (m_ppCartridge[i]) delete[] m_ppCartridge[i];
	for (int i = 0; i < HELICOPTERVALKANS; i++) if (m_ppValkan[i]) delete[] m_ppValkan[i];

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

	if (m_ppSpriteBillboard)
	{
		for (int i = 0; i < m_nSpriteBillboards; i++)
		{
			m_ppSpriteBillboard[i]->ReleaseShaderVariables();
			m_ppSpriteBillboard[i]->ReleaseObjects();
			m_ppSpriteBillboard[i]->Release();
		}
		delete[] m_ppSpriteBillboard;
	}

	if (m_ppShaders)delete m_ppShaders;
	if (m_ppFragShaders)
	{
		for (int i = 0; i < m_nFragShaders; i++)
		{
			m_ppFragShaders[i]->ReleaseShaderVariables();
			m_ppFragShaders[i]->ReleaseObjects();
			m_ppFragShaders[i]->Release();
		}
		delete[] m_ppFragShaders;
	}
	if (m_pDepthRenderShader)
	{
		m_pDepthRenderShader->ReleaseShaderVariables();
		m_pDepthRenderShader->ReleaseObjects();
		m_pDepthRenderShader->Release();

	}

	if (m_pShadowShader)delete m_pShadowShader;
	if (m_pTreeBlendShadowShader)delete m_pTreeBlendShadowShader;
	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;
	ReleaseShaderVariables();
	if (m_pLights) delete m_pLights;
	if (m_pMaterials) delete m_pMaterials;

}

ID3D12RootSignature* MainGameScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[18];

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

	pd3dDescriptorRanges[14].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[14].NumDescriptors = MAX_DEPTH_TEXTURES;
	pd3dDescriptorRanges[14].BaseShaderRegister = 18; //t18~34 (18+MAX LIGHT): Depth Buffer
	pd3dDescriptorRanges[14].RegisterSpace = 0;
	pd3dDescriptorRanges[14].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[15].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[15].NumDescriptors = 1;
	pd3dDescriptorRanges[15].BaseShaderRegister = 9; // SpriteAnimation b9
	pd3dDescriptorRanges[15].RegisterSpace = 0;
	pd3dDescriptorRanges[15].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[16].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[16].NumDescriptors = 1;
	pd3dDescriptorRanges[16].BaseShaderRegister = 27; //t27: Texture2DArray
	pd3dDescriptorRanges[16].RegisterSpace = 0;
	pd3dDescriptorRanges[16].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[17].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[17].NumDescriptors = 1;
	pd3dDescriptorRanges[17].BaseShaderRegister = 28; //t28: Texture2D<float4>
	pd3dDescriptorRanges[17].RegisterSpace = 0;
	pd3dDescriptorRanges[17].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[24];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 34;
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
	pd3dRootParameters[19].DescriptorTable.NumDescriptorRanges = 1; // SpriteAnimation b9
	pd3dRootParameters[19].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[15];
	pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[20].Descriptor.ShaderRegister = 10; //Material
	pd3dRootParameters[20].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[21].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[21].Descriptor.ShaderRegister = 11; //FrameWorkInfo
	pd3dRootParameters[21].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[21].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[22].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[22].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[22].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[14]; //Depth Buffer
	pd3dRootParameters[22].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[23].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[23].Descriptor.ShaderRegister = 12; //toshadow
	pd3dRootParameters[23].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[23].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[4];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
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
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0.0f;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;// D3D12_COMPARISON_FUNC_LESS;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;//  D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].MipLODBias = 0.0f;
	pd3dSamplerDescs[3].MaxAnisotropy = 1;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].ShaderRegister = 3;
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


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
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void MainGameScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);
}

void MainGameScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(m_pcbMappedMaterials, &m_pMaterials, sizeof(MATERIALS));
}

void MainGameScene::ReleaseShaderVariables()
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

void MainGameScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHumanShaders; i++) m_ppHumanShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nBillboardShaders; i++) if (m_pBillboardShader[i]) m_pBillboardShader[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nSpriteBillboards; i++) if (m_ppSpriteBillboard[i]) m_ppSpriteBillboard[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nFragShaders; i++) if (m_ppFragShaders[i]) m_ppFragShaders[i]->ReleaseUploadBuffers();
}

void MainGameScene::CreateDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

void MainGameScene::CreateCBV(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE MainGameScene::CreateCBVs(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
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
	case RESOURCE_TEXTURE2D: 
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}
void MainGameScene::CreateSRVs(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * i;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * i;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

void MainGameScene::Firevalkan(CGameObject* Objects, XMFLOAT3 ToPlayerLook)
{
	CValkanObject* pBulletObject = NULL;
	for (int i = 0; i < HELIBULLETS; i++)
	{
		if (!m_ppBullets[i]->m_bActive)
		{
			pBulletObject = m_ppBullets[i];
			pBulletObject->Reset();
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 PlayerLook = ToPlayerLook;
		XMFLOAT3 xmf3Position = Objects->GetPosition();
		XMFLOAT3 xmf3Direction = PlayerLook;
		pBulletObject->m_xmf4x4ToParent = Objects->m_xmf4x4World;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 0.0f, false));
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->m_xmf4x4ToParent._31 = ToPlayerLook.x;
		pBulletObject->m_xmf4x4ToParent._32 = ToPlayerLook.y;
		pBulletObject->m_xmf4x4ToParent._33 = ToPlayerLook.z;
		pBulletObject->Rotate(90.0, 0.0, 0.0);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(0.5, 6.0, 0.5);
		pBulletObject->SetActive(true);
	}
}
void MainGameScene::ParticleCollisionResult()
{

	XMFLOAT4 Oriented = XMFLOAT4(0, 0, 0, 1);
	BoundingOrientedBox HeliPlayeroobb = BoundingOrientedBox(((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->GetPosition(),XMFLOAT3(8.0, 8.0, 10.0), XMFLOAT4(0, 0, 0, 1));
	BoundingOrientedBox HumanPlayeroobb = BoundingOrientedBox(((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition(),XMFLOAT3(2.0, 4.0, 2.0), XMFLOAT4(0, 0, 0, 1));
	for (int i = 12; i < 17; i++)
	{
		BoundingOrientedBox P1 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj1->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P2 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj2->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P3 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj3->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P4 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj4->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P5 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj5->GetPosition(), XMFLOAT3(5.0, 5.0, 5.0), Oriented);
		BoundingOrientedBox P6 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj6->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P7 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj7->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P8 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj8->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P9 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj9->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P10 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pTailRotorFrame->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);
		BoundingOrientedBox P11 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pMainRotorFrame->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Oriented);

		if (HeliPlayeroobb.Intersects(P1) || HeliPlayeroobb.Intersects(P2) || HeliPlayeroobb.Intersects(P3) || HeliPlayeroobb.Intersects(P4)
			|| HeliPlayeroobb.Intersects(P5) || HeliPlayeroobb.Intersects(P6) || HeliPlayeroobb.Intersects(P7) || HeliPlayeroobb.Intersects(P8)
			|| HeliPlayeroobb.Intersects(P9) || HeliPlayeroobb.Intersects(P10) || HeliPlayeroobb.Intersects(P11))
		{
			cout << "Heli Collision Particle!" << endl;
			m_bHeliParticleCollisionCheck = true;
		}

		if (HumanPlayeroobb.Intersects(P1) || HumanPlayeroobb.Intersects(P2) || HumanPlayeroobb.Intersects(P3) || HumanPlayeroobb.Intersects(P4)
			|| HumanPlayeroobb.Intersects(P5) || HumanPlayeroobb.Intersects(P6) || HumanPlayeroobb.Intersects(P7) || HumanPlayeroobb.Intersects(P8)
			|| HumanPlayeroobb.Intersects(P9) || HumanPlayeroobb.Intersects(P10) || HumanPlayeroobb.Intersects(P11))
		{
			cout << "Human Collision Particle!" << endl;
			m_bHumanParticleCollisionCheck = true;
		}

		if (((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_bPartitionfalldownEnd == true)
		{
			m_bPartitionEnd = true;
		}
	}
}
void MainGameScene::OtherPlayerFirevalkan(CGameObject* Objects, XMFLOAT3 ToPlayerLook)
{
	CValkanObject* pBulletObject = NULL;
	for (int i = 0; i < HELICOPTERVALKANS; i++)
	{
		if (!m_ppValkan[i]->m_bActive)
		{
			pBulletObject = m_ppValkan[i];
			pBulletObject->Reset();
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 PlayerLook = ToPlayerLook;

		XMFLOAT3 xmf3Position = Objects->GetPosition();
		XMFLOAT3 xmf3Direction = PlayerLook;

		pBulletObject->m_xmf4x4ToParent = Objects->m_xmf4x4World;


		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 0.0f, false));
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->m_xmf4x4ToParent._31 = ToPlayerLook.x;
		pBulletObject->m_xmf4x4ToParent._32 = ToPlayerLook.y;
		pBulletObject->m_xmf4x4ToParent._33 = ToPlayerLook.z;
		pBulletObject->Rotate(90.0, 0.0, 0.0);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(0.5, 6.0, 0.5);
		pBulletObject->SetActive(true);

	}
}
void MainGameScene::PlayerFirevalkan(CCamera* pCamera, XMFLOAT3 Look)
{
	CValkanObject* pBulletObject = NULL;
	for (int i = 0; i < HELICOPTERVALKANS; i++)
	{
		if (!m_ppValkan[i]->m_bActive)
		{
			pBulletObject = m_ppValkan[i];
			pBulletObject->Reset();
			break;
		}
	}
	if (pBulletObject)
	{
		XMFLOAT3 PlayerLook = pCamera->GetLookVector();
		XMFLOAT3 xmf3Position = pCamera->GetPosition();
		XMFLOAT3 xmf3Direction = PlayerLook;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 60.0f, false));
		pBulletObject->m_xmf4x4ToParent = m_pPlayer->m_xmf4x4World;
		pBulletObject->m_xmf4x4ToParent._31 = PlayerLook.x;
		pBulletObject->m_xmf4x4ToParent._32 = PlayerLook.y;
		pBulletObject->m_xmf4x4ToParent._33 = PlayerLook.z;
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->Rotate(90.0, 0.0, 0.0);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(3.5, 7.0, 3.5);
		pBulletObject->SetActive(true);

	}
}

void MainGameScene::Reflectcartridgecase(CGameObject* Objects)
{
	CValkanObject* pBulletObject = NULL;
	for (int i = 0; i < CARTRIDGES; i++)
	{
		if (!m_ppCartridge[i]->m_bActive)
		{
			pBulletObject = m_ppCartridge[i];
			pBulletObject->Reset();
			break;
		}
	}
	if (pBulletObject)
	{
		XMFLOAT3 PlayerLook = ((CHumanPlayer*)m_pPlayer)->m_pBulletFindFrame->GetRight();
		XMFLOAT3 xmf3Position = ((CHumanPlayer*)m_pPlayer)->m_pBulletFindFrame->GetPosition();
		XMFLOAT3 xmf3Direction = RandomDirection(1, 5);
		pBulletObject->m_xmf4x4ToParent = m_pPlayer->m_xmf4x4World;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 0.0f, false));
		xmf3FirePosition.y += 0.7f;
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(0.05, 0.05, 0.05);
		pBulletObject->SetActive(true);
	}
}
bool MainGameScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}
bool MainGameScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_CONTROL:
			break;
		case 'F':
			m_ppSpriteBillboard[0]->SetActive(!m_ppSpriteBillboard[0]->GetActive());
			break;
		case 'K':
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
void MainGameScene::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}
bool MainGameScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void MainGameScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	XMFLOAT3 xmfPosition = m_pPlayer->GetPosition();
	for (int i = 0; i < m_nBillboardShaders; i++) if (m_pBillboardShader[i]) m_pBillboardShader[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nSpriteBillboards; i++) if (m_ppSpriteBillboard[i]) m_ppSpriteBillboard[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nFragShaders; i++) if (m_ppFragShaders[i]) m_ppFragShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nHumanShaders; i++) if (m_ppHumanShaders[i]) m_ppHumanShaders[i]->AnimateObjects(fTimeElapsed);

	for (int i = 0; i < HELIBULLETS; i++)
	{
		if (m_ppBullets[i]->m_bActive)
		{
			m_ppBullets[i]->Animate(fTimeElapsed);
		}
	}
	for (int i = 0; i < CARTRIDGES; i++)
	{
		if (m_ppCartridge[i]->m_bActive)
		{
			m_ppCartridge[i]->Animate(fTimeElapsed);
			m_ppCartridge[i]->Rotate(8, 5, 0);
		}
	}
	for (int i = 0; i < HELICOPTERVALKANS; i++)
	{
		if (m_ppValkan[i]->m_bActive)
		{
			m_ppValkan[i]->Animate(fTimeElapsed);
		}
	}

	XMFLOAT3 Position2P = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->GetPosition();
	XMFLOAT3 Look2P = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->GetLook();
	if (m_pLights)
	{
		m_pLights->m_pLights[2].m_xmf3Position = XMFLOAT3(Position2P.x, Position2P.y, Position2P.z + 0.0f);
		m_pLights->m_pLights[2].m_xmf3Direction = Look2P;
		m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(xmfPosition.x, xmfPosition.y + 8.0, xmfPosition.z);
		m_pLights->m_pLights[3].m_xmf3Direction = m_pPlayer->GetLook();

	}

	NpcByPlayerCollsiion();
	PlayerByPlayerCollision();
	ParticleCollisionResult();
	SetPositionPilotHuman();
}

void MainGameScene::HealPackZoneInfo()
{
	((HealPackBillboardShader*)m_pBillboardShader[1])->HealParticlePosition = XMFLOAT3(141.30, 6.00, 169.50);
	((HealPackBillboardShader*)m_pBillboardShader[10])->HealParticlePosition = XMFLOAT3(560.00, 6.00, 938.00);
	((HealPackBillboardShader*)m_pBillboardShader[11])->HealParticlePosition = XMFLOAT3(141.30, 6.00, -138.00);
	((HealPackBillboardShader*)m_pBillboardShader[12])->HealParticlePosition = XMFLOAT3(560.00, 6.00, -902.00);
	((HealPackBillboardShader*)m_pBillboardShader[13])->HealParticlePosition = XMFLOAT3(-217.00, 6.00, 169.50);
	((HealPackBillboardShader*)m_pBillboardShader[14])->HealParticlePosition = XMFLOAT3(-630.00, 6.00, 938.00);
	((HealPackBillboardShader*)m_pBillboardShader[15])->HealParticlePosition = XMFLOAT3(-634.00, 6.00, -905.00);
	((HealPackBillboardShader*)m_pBillboardShader[16])->HealParticlePosition = XMFLOAT3(-215.30, 6.00, -136.50);
}

void MainGameScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	UpdateShaderVariables(pd3dCommandList);

	m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nBillboardShaders; i++) if (i!= 6 && m_pBillboardShader[i]) m_pBillboardShader[i]->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < m_nFragShaders; i++) if (m_ppFragShaders[i]) m_ppFragShaders[i]->Render(pd3dCommandList, pCamera, 0);
	if (pBCBulletEffectShader) pBCBulletEffectShader->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < HELIBULLETS; i++)if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->Render(pd3dCommandList, pCamera); }
	for (int i = 0; i < CARTRIDGES; i++) if (m_ppCartridge[i]->m_bActive) { m_ppCartridge[i]->Render(pd3dCommandList, pCamera); }
	for (int i = 0; i < HELICOPTERVALKANS; i++)if (m_ppValkan[i]->m_bActive) { m_ppValkan[i]->Render(pd3dCommandList, pCamera); }
	if (m_pShadowShader) m_pShadowShader->Render(pd3dCommandList, pCamera, 0);
	if (m_pTreeBlendShadowShader) m_pTreeBlendShadowShader->Render(pd3dCommandList, pCamera, 0);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

}

void MainGameScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	m_pDepthRenderShader->PrepareShadowMap(pd3dCommandList);
}

void MainGameScene::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

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

void MainGameScene::OtherHeliPlayerTransformStore()
{
	//m_pMainRotorFrameP = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pMainRotorFrame->m_xmf4x4ToParent;
	//m_pTailRotorFrameP = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pTailRotorFrame->m_xmf4x4ToParent;
	//m_pFrameFragObj1P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj1->m_xmf4x4ToParent;
	//m_pFrameFragObj2P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj2->m_xmf4x4ToParent;
	//m_pFrameFragObj3P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj3->m_xmf4x4ToParent;
	//m_pFrameFragObj4P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj4->m_xmf4x4ToParent;
	//m_pFrameFragObj5P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj5->m_xmf4x4ToParent;
	//m_pFrameFragObj6P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj6->m_xmf4x4ToParent;
	//m_pFrameFragObj7P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj7->m_xmf4x4ToParent;
	//m_pFrameFragObj8P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj8->m_xmf4x4ToParent;
	//m_pFrameFragObj9P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj9->m_xmf4x4ToParent;
	//m_pFrameFragObj10P = ((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj10->m_xmf4x4ToParent;
}

void MainGameScene::OtherHeliPlayerTransfromReset()
{
	/*((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pMainRotorFrame->m_xmf4x4ToParent = m_pMainRotorFrameP;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pTailRotorFrame->m_xmf4x4ToParent = m_pTailRotorFrameP;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj1->m_xmf4x4ToParent = m_pFrameFragObj1P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj2->m_xmf4x4ToParent = m_pFrameFragObj2P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj3->m_xmf4x4ToParent = m_pFrameFragObj3P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj4->m_xmf4x4ToParent = m_pFrameFragObj4P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj5->m_xmf4x4ToParent = m_pFrameFragObj5P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj6->m_xmf4x4ToParent = m_pFrameFragObj6P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj7->m_xmf4x4ToParent = m_pFrameFragObj7P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj8->m_xmf4x4ToParent = m_pFrameFragObj8P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj9->m_xmf4x4ToParent = m_pFrameFragObj9P;
	((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[7])->m_pFrameFragObj10->m_xmf4x4ToParent = m_pFrameFragObj10P;*/
}

void MainGameScene::BillBoardRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position)
{
	if (((BulletMarkBillboard*)m_pBillboardShader[4])) ((BulletMarkBillboard*)m_pBillboardShader[4])->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < m_nSpriteBillboards; i++) if (m_ppSpriteBillboard[i]) m_ppSpriteBillboard[i]->Render(pd3dCommandList, pCamera, 0);
}

void MainGameScene::MuzzleFlameRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position)
{
	XMFLOAT3 MuzzleFrameLook = pCamera->GetLookVector();
	XMFLOAT3 MuzzleFramePosition = pCamera->GetPosition();
	if (((MuzzleFrameBillboard*)m_pBillboardShader[6]))
	{
		((MuzzleFrameBillboard*)m_pBillboardShader[6])->Render(pd3dCommandList, pCamera, 0, MuzzleFrameLook, ((CHumanPlayer*)m_pPlayer)->m_pBulletFindFrame->GetPosition());
	}
}

void MainGameScene::NPCMuzzleFlamedRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position)
{
	XMFLOAT3 MuzzleFrameLook = pCamera->GetLookVector();
	XMFLOAT3 MuzzleFramePosition = pCamera->GetPosition();
	if (((MuzzleFrameBillboard*)m_pBillboardShader[7]))
	{
		((MuzzleFrameBillboard*)m_pBillboardShader[7])->Render(pd3dCommandList, pCamera, 0, MuzzleFrameLook, Position);
	}

}

void MainGameScene::PlayerByPlayerCollision()
{
	XMFLOAT3 HumanSize = XMFLOAT3(3.0, 5.0, 3.0);
	XMFLOAT4 Orientation = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT3 P1Pos = XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->m_xmf4x4ToParent._41,
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->m_xmf4x4ToParent._42,
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->m_xmf4x4ToParent._43);
	XMFLOAT3 P2Pos = XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->m_xmf4x4ToParent._41,
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->m_xmf4x4ToParent._42,
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->m_xmf4x4ToParent._43);
	XMFLOAT3 P0Pos = XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->m_xmf4x4ToParent._41,
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->m_xmf4x4ToParent._42,
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->m_xmf4x4ToParent._43);
	XMFLOAT3 MyPos = XMFLOAT3(((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition().x,
		((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition().y,
		((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition().z);

	BoundingOrientedBox Other0Poobb = BoundingOrientedBox(XMFLOAT3(P0Pos), HumanSize, Orientation);
	BoundingOrientedBox Other1Poobb = BoundingOrientedBox(XMFLOAT3(P1Pos), HumanSize, Orientation);
	BoundingOrientedBox Other2Poobb = BoundingOrientedBox(XMFLOAT3(P2Pos), HumanSize, Orientation);
	BoundingOrientedBox MyPoobb = BoundingOrientedBox(XMFLOAT3(MyPos), HumanSize, Orientation);

	if (MyPoobb.Intersects(Other0Poobb)) {
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->SetPosition(XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->GetPosition().x,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->GetPosition().y,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[5])->m_xmf4x4ToParent._43 - 1.0f));
	}
	if (MyPoobb.Intersects(Other2Poobb)) {
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->SetPosition(XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->GetPosition().x,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->GetPosition().y,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[6])->m_xmf4x4ToParent._43 - 1.0f));
	}
	if (MyPoobb.Intersects(Other1Poobb)) {
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->SetPosition(XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->GetPosition().x,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->GetPosition().y,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[4])->m_xmf4x4ToParent._43 - 1.0f));
	}
}

void MainGameScene::SetPositionPilotHuman()
{
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._11 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._11;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._12 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._12;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._13 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._13;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._21 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._21;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._22 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._22;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._23 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._23;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._31 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._31;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._32 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._32;
	((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_xmf4x4ToParent._33 = ((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_xmf4x4ToParent._33;

	if (((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_FallSwitch == false)
	{
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 9);
		((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[0])->SetPosition(XMFLOAT3(
			((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_pChairPoint->GetPosition().x,
			((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_pChairPoint->GetPosition().y - 2.0f,
			((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->m_pChairPoint->GetPosition().z
		));
	}
}

void MainGameScene::NpcByPlayerCollsiion()
{
	for (int i = 21; i < 42; i++)
	{
		XMFLOAT3 NpcPos = XMFLOAT3(((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[i])->m_xmf4x4ToParent._41,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[i])->m_xmf4x4ToParent._42,
			((CSoldiarOtherPlayerObjects*)m_ppShaders[0]->m_ppObjects[i])->m_xmf4x4ToParent._43);
		XMFLOAT3 MyPos = XMFLOAT3(((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition());

		BoundingOrientedBox Npcoobb = BoundingOrientedBox(XMFLOAT3(NpcPos), XMFLOAT3(3.0, 5.0, 3.0), XMFLOAT4(0, 0, 0, 1));
		BoundingOrientedBox MyPoobb = BoundingOrientedBox(XMFLOAT3(MyPos), XMFLOAT3(3.0, 5.0, 3.0), XMFLOAT4(0, 0, 0, 1));
		if (MyPoobb.Intersects(Npcoobb)) {
			(((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->SetPosition(XMFLOAT3(
				((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->m_xmf4x4ToParent._41 - 2.0f,
				((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition().y,
				((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition().z)));
		}
	}


}
