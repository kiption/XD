#include "stdafx.h"
#include "Stage1.h"
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
ID3D12DescriptorHeap* Stage1::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	Stage1::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	Stage1::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	Stage1::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	Stage1::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	Stage1::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	Stage1::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	Stage1::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	Stage1::m_d3dSrvGPUDescriptorNextHandle;

Stage1::Stage1() : SceneManager()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
}

Stage1::~Stage1()
{
}

void Stage1::BuildDefaultLightsAndMaterials()
{
	m_nLights = 7;
	m_pLights = new LIGHTS[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHTS) * m_nLights);


	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	// 방향 조절 -> 2개 쓰면 프레임 떨어짐
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
	m_pLights->m_pLights[1].m_fRange = 30000.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.6, 0.6, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-550, 600.0f, 1100.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.4f, -1.0f, -0.9f);

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[2].m_fRange = 500.0f;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -0.2f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[2].m_fFalloff = 6.0f;
	m_pLights->m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 600.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -0.2f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 12.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[4].m_bEnable = false;
	m_pLights->m_pLights[4].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[4].m_fRange = 30000.0f;
	m_pLights->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5, 0.5, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Specular = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Position = XMFLOAT3(-600, 300.0f, 900.0f);
	m_pLights->m_pLights[4].m_xmf3Direction = XMFLOAT3(+0.4f, -0.3f, -1.0f);

	m_pLights->m_pLights[5].m_bEnable = false;
	m_pLights->m_pLights[5].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[5].m_fRange = 35000.0f;
	m_pLights->m_pLights[5].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[5].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.6, 0.6, 1.0f);
	m_pLights->m_pLights[5].m_xmf4Specular = XMFLOAT4(0.2f, 0.2, 0.2f, 1.0f);
	m_pLights->m_pLights[5].m_xmf3Position = XMFLOAT3(-600, 300.0f, 900.0f);
	m_pLights->m_pLights[5].m_xmf3Direction = XMFLOAT3(+0.2f, -0.3f, -1.0f);

	m_pLights->m_pLights[6].m_bEnable = false;
	m_pLights->m_pLights[6].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[6].m_fRange = 60.0f;
	m_pLights->m_pLights[6].m_xmf4Ambient = XMFLOAT4(0.6f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Diffuse = XMFLOAT4(0.6f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Specular = XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[6].m_xmf4Emissive = XMFLOAT4(0.9f, 0.2f, 0.2f, 1.0f);
	m_pLights->m_pLights[6].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[6].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.5f);
	m_pLights->m_pLights[6].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[6].m_fFalloff = 1.0f;
	m_pLights->m_pLights[6].m_fPhi = (float)cos(XMConvertToRadians(359.0f));
	m_pLights->m_pLights[6].m_fTheta = (float)cos(XMConvertToRadians(15.0));

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

void Stage1::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, ID3D12Resource* pd3dDepthStencilBuffer)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 800);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_pSkyBox->SetCurScene(SCENE1STAGE);

	m_nBillboardShaders = 8;
	m_pBillboardShader = new BillboardShader * [m_nBillboardShaders];

	BillboardParticleShader* pBillboardParticleShader = new BillboardParticleShader();
	pBillboardParticleShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBillboardParticleShader->SetCurScene(SCENE1STAGE);
	pBillboardParticleShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[0] = pBillboardParticleShader;

	BloodMarkShader* pCrossHairShader = new BloodMarkShader();
	pCrossHairShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pCrossHairShader->SetCurScene(SCENE1STAGE);
	pCrossHairShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[1] = pCrossHairShader;

	BloodHittingBillboard* pBloodHittingBillboard = new BloodHittingBillboard();
	pBloodHittingBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBloodHittingBillboard->SetCurScene(SCENE1STAGE);
	pBloodHittingBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[2] = pBloodHittingBillboard;

	SparkBillboard* pHelicopterSparkBillboard = new SparkBillboard();
	pHelicopterSparkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHelicopterSparkBillboard->SetCurScene(SCENE1STAGE);
	pHelicopterSparkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHelicopterSparkBillboard->m_bActive = true;
	m_pBillboardShader[3] = pHelicopterSparkBillboard;

	BulletMarkBillboard* pBulletMarkBillboard = new BulletMarkBillboard();
	pBulletMarkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBulletMarkBillboard->SetCurScene(SCENE1STAGE);
	pBulletMarkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[4] = pBulletMarkBillboard;

	HeliHittingMarkBillboard* pHeliHittingMarkBillboard = new HeliHittingMarkBillboard();
	pHeliHittingMarkBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pHeliHittingMarkBillboard->SetCurScene(SCENE1STAGE);
	pHeliHittingMarkBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pHeliHittingMarkBillboard->m_bActive = true;
	m_pBillboardShader[5] = pHeliHittingMarkBillboard;

	MuzzleFrameBillboard* pMuzzleFrameBillboard = new MuzzleFrameBillboard();
	pMuzzleFrameBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pMuzzleFrameBillboard->SetCurScene(SCENE1STAGE);
	pMuzzleFrameBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[6] = pMuzzleFrameBillboard;

	MuzzleFrameBillboard* pMuzzleFrameBillboard2 = new MuzzleFrameBillboard();
	pMuzzleFrameBillboard2->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pMuzzleFrameBillboard2->SetCurScene(SCENE1STAGE);
	pMuzzleFrameBillboard2->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_pBillboardShader[7] = pMuzzleFrameBillboard2;


	m_nSpriteBillboards = 1;
	m_ppSpriteBillboard = new CSpriteObjectsShader * [m_nSpriteBillboards];
	SpriteAnimationBillboard* pSpriteAnimationBillboard = new SpriteAnimationBillboard();
	pSpriteAnimationBillboard->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pSpriteAnimationBillboard->SetCurScene(SCENE1STAGE);
	pSpriteAnimationBillboard->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pSpriteAnimationBillboard->SetActive(false);
	m_ppSpriteBillboard[0] = pSpriteAnimationBillboard;

	m_nFragShaders = 2;
	m_ppFragShaders = new CFragmentsShader * [m_nFragShaders];
	CFragmentsShader* pFragmentsShader = new CFragmentsShader();
	pFragmentsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pFragmentsShader->SetCurScene(SCENE1STAGE);
	pFragmentsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppFragShaders[0] = pFragmentsShader;

	CHelicopterBulletMarkParticleShader* pBloodFragmentsShader = new CHelicopterBulletMarkParticleShader();
	pBloodFragmentsShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBloodFragmentsShader->SetCurScene(SCENE1STAGE);
	pBloodFragmentsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	m_ppFragShaders[1] = pBloodFragmentsShader;

	m_nShaders = 1;
	m_ppShaders = new CObjectsShader * [m_nShaders];

	CObjectsShader* pObjectShader = new CObjectsShader();
	pObjectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pObjectShader->SetCurScene(SCENE1STAGE);
	pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pTerrain);
	m_ppShaders[0] = pObjectShader;

	m_pDepthRenderShader = new CDepthRenderShader(pObjectShader, m_pLights->m_pLights);
	m_pDepthRenderShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	m_pDepthRenderShader->SetCurScene(SCENE1STAGE);
	m_pDepthRenderShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);

	m_pShadowShader = new CShadowMapShader(pObjectShader);
	m_pShadowShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	m_pShadowShader->SetCurScene(SCENE1STAGE);
	m_pShadowShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pDepthRenderShader->GetDepthTexture());

	m_pTreeBlendShadowShader = new CTreeBlendingShadowShader(pObjectShader);
	m_pTreeBlendShadowShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	m_pTreeBlendShadowShader->SetCurScene(SCENE1STAGE);
	m_pTreeBlendShadowShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pDepthRenderShader->GetDepthTexture());

	pBCBulletEffectShader = new CBulletEffectShader();
	pBCBulletEffectShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0);
	pBCBulletEffectShader->SetCurScene(SCENE1STAGE);

	CGameObject* pBulletMesh = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", pBCBulletEffectShader);
	for (int i = 0; i < HELIBULLETS; i++)
	{
		pBulletObject = new CValkanObject(m_fBulletEffectiveRange);
		pBulletObject->SetChild(pBulletMesh, false);
		pBulletObject->SetMovingSpeed(200.0f);
		pBulletObject->SetActive(false);
		pBulletObject->SetCurScene(SCENE1STAGE);
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
		pCartridge->SetCurScene(SCENE1STAGE);
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
		pValkan->SetCurScene(SCENE1STAGE);
		m_ppValkan[i] = pValkan;
		pHelicopterValkanMesh->AddRef();
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void Stage1::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();
	//if (m_pBoundingBoxShader) m_pBoundingBoxShader->Release();
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
	if (m_ppTreeObjectShader)
	{
		for (int i = 0; i < 1; i++)
		{
			m_ppTreeObjectShader[i]->ReleaseShaderVariables();
			m_ppTreeObjectShader[i]->ReleaseObjects();
			m_ppTreeObjectShader[i]->Release();
		}
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
	if (m_pLights) delete[] m_pLights;
	if (m_pMaterials) delete m_pMaterials;

}

ID3D12RootSignature* Stage1::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
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

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
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
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;// D3D12_COMPARISON_FUNC_LESS;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;//  D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
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

void Stage1::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);


}

void Stage1::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(m_pcbMappedMaterials, &m_pMaterials, sizeof(MATERIALS));
}

void Stage1::ReleaseShaderVariables()
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

void Stage1::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHumanShaders; i++) m_ppHumanShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nBillboardShaders; i++) if (m_pBillboardShader[i]) m_pBillboardShader[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nSpriteBillboards; i++) if (m_ppSpriteBillboard[i]) m_ppSpriteBillboard[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nFragShaders; i++) if (m_ppFragShaders[i]) m_ppFragShaders[i]->ReleaseUploadBuffers();

}

void Stage1::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
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

void Stage1::CreateConstantBufferView(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
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

D3D12_GPU_DESCRIPTOR_HANDLE Stage1::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
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
void Stage1::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
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

void Stage1::Firevalkan(CGameObject* Objects, XMFLOAT3 ToPlayerLook)
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
void Stage1::ParticleCollisionResult()
{
	
	BoundingOrientedBox HeliPlayeroobb = BoundingOrientedBox(((HeliPlayer*)m_ppShaders[0]->m_ppObjects[43])->GetPosition(),
		XMFLOAT3(8.0, 8.0, 10.0), XMFLOAT4(0, 0, 0, 1));

	BoundingOrientedBox HumanPlayeroobb = BoundingOrientedBox(((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->GetPosition(),
		XMFLOAT3(2.0, 4.0, 2.0), XMFLOAT4(0, 0, 0, 1));

	XMFLOAT4 Quaternion = XMFLOAT4(0, 0, 0, 1);
	for (int i = 12; i < 17; i++)
	{
		P1 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj1->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P2 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj2->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P3 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj3->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P4 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj4->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P5 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj5->GetPosition(), XMFLOAT3(5.0, 5.0, 5.0), Quaternion);
		P6 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj6->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P7 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj7->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P8 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj8->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P9 =  BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pFrameFragObj9->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P10 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pTailRotorFrame->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);
		P11 = BoundingOrientedBox(((CHelicopterObjects*)m_ppShaders[0]->m_ppObjects[i])->m_pMainRotorFrame->GetPosition(), XMFLOAT3(3.0, 3.0, 3.0), Quaternion);

		if (HeliPlayeroobb.Intersects(P1)) cout << "Heli Collision P1!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P2)) cout << "Heli Collision P2!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P3)) cout << "Heli Collision P3!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P4)) cout << "Heli Collision P4!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P5)) cout << "Heli Collision P5!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P6)) cout << "Heli Collision P6!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P7)) cout << "Heli Collision P7!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P8)) cout << "Heli Collision P8!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P9)) cout << "Heli Collision P9!" << endl; m_bHeliParticleCollisionCheck = true;	
		if (HeliPlayeroobb.Intersects(P10)) cout << "Heli Collision P10!" << endl; m_bHeliParticleCollisionCheck = true;
		if (HeliPlayeroobb.Intersects(P11)) cout << "Heli Collision P11!" << endl; m_bHeliParticleCollisionCheck = true;

		if (HumanPlayeroobb.Intersects(P1)) cout << "Human Collision P1!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P2)) cout << "Human Collision P2!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P3)) cout << "Human Collision P3!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P4)) cout << "Human Collision P4!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P5)) cout << "Human Collision P5!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P6)) cout << "Human Collision P6!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P7)) cout << "Human Collision P7!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P8)) cout << "Human Collision P8!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P9)) cout << "Human Collision P9!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P10)) cout << "Human Collision P10!" << endl; m_bHumanParticleCollisionCheck = true;
		if (HumanPlayeroobb.Intersects(P11)) cout << "Human Collision P11!" << endl; m_bHumanParticleCollisionCheck = true;
	}



}
void Stage1::OtherPlayerFirevalkan(CGameObject* Objects, XMFLOAT3 ToPlayerLook)
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
void Stage1::PlayerFirevalkan(CCamera* pCamera, XMFLOAT3 Look)
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
		xmf3Direction.y += 0.0f;
		pBulletObject->m_xmf4x4ToParent = m_pPlayer->m_xmf4x4World;
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 60.0f, false));
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->Rotate(90.0, 0.0, 0.0);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(1.5, 7.0, 1.5);
		pBulletObject->SetActive(true);

	}
}

void Stage1::Reflectcartridgecase(CGameObject* Objects)
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
		//xmf3FirePosition.z += 0.8f;
		pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition));
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetScale(0.05, 0.05, 0.05);
		pBulletObject->SetActive(true);

	}
}

bool Stage1::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool Stage1::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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
		case 'J':
			((BloodMarkShader*)m_pBillboardShader[1])->m_bActiveMark = false;
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

void Stage1::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	/*m_pBoundingBoxShader->Render(pd3dCommandList, pCamera,0);
	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_ppShaders[i]) m_ppShaders[i]->RenderBoundingBox(pd3dCommandList, pCamera);
	}

	m_pPlayer->RenderBoundingBox(pd3dCommandList, pCamera);*/
}

bool Stage1::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void Stage1::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	for (int i = 0; i < m_nBillboardShaders; i++) if (m_pBillboardShader[i]) m_pBillboardShader[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nSpriteBillboards; i++) if (m_ppSpriteBillboard[i]) m_ppSpriteBillboard[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nFragShaders; i++) if (m_ppFragShaders[i]) m_ppFragShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	for (int i = 0; i < m_nHumanShaders; i++) if (m_ppHumanShaders[i]) m_ppHumanShaders[i]->AnimateObjects(fTimeElapsed);
	XMFLOAT3 xmfPosition = m_pPlayer->GetPosition();

	((BloodMarkShader*)m_pBillboardShader[1])->m_bActiveMark = true;

	/*((CFragmentsShader*)m_ppFragShaders[0])->m_bActive = true;
	((CFragmentsShader*)m_ppFragShaders[0])->ParticlePosition = XMFLOAT3(120.0, 50.0, 700.0);*/

	m_pBillboardShader[1]->m_ppObjects[0]->SetPosition(m_ppShaders[0]->m_ppObjects[30]->GetPosition());



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
	XMFLOAT3 Position2P = m_ppShaders[0]->m_ppObjects[5]->GetPosition();
	XMFLOAT3 Look2P = m_ppShaders[0]->m_ppObjects[5]->GetLook();
	if (m_pLights)
	{
		m_pLights->m_pLights[2].m_xmf3Position = XMFLOAT3(Position2P.x, Position2P.y + 8.0, Position2P.z);
		m_pLights->m_pLights[2].m_xmf3Direction = Look2P;

		m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(xmfPosition.x, xmfPosition.y + 8.0, xmfPosition.z);
		m_pLights->m_pLights[3].m_xmf3Direction = m_pPlayer->GetLook();

	}
	m_fLightRotationAngle += fTimeElapsed;
	XMMATRIX xmmtxRotation = XMMatrixRotationY(fTimeElapsed * 0.02f);
	for (int i = 22; i < 42; i++)
	{
		XMFLOAT3 xmf3PlayerPosition = m_ppShaders[0]->m_ppObjects[i]->GetPosition();
		float fHeight = 6.15f + 0.05f;
		if (xmf3PlayerPosition.y < fHeight)
		{
			xmf3PlayerPosition.y = 0.0f;
			xmf3PlayerPosition.y = fHeight;
			m_ppShaders[0]->m_ppObjects[i]->SetPosition(xmf3PlayerPosition);
		}
	}


	if (m_bHeliParticleCollisionCheck == true)
	{
		((CHumanPlayer*)m_ppShaders[0]->m_ppObjects[1])->m_xmf4x4ToParent._41 -= 4.0f;
		m_bHumanParticleCollisionCheck = false;
	}
	if (m_bHeliParticleCollisionCheck == true)
	{
		
		m_bHeliParticleCollisionCheck = false;
	}
	ParticleCollisionResult();
	ParticleAnimation();
}

void Stage1::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	UpdateShaderVariables(pd3dCommandList);

	m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);


	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nBillboardShaders; i++) if (i != 1 && i != 6 && m_pBillboardShader[i]) m_pBillboardShader[i]->Render(pd3dCommandList, pCamera, 0);

	for (int i = 0; i < m_nFragShaders; i++) if (m_ppFragShaders[i]) m_ppFragShaders[i]->Render(pd3dCommandList, pCamera, 0);
	if (pBCBulletEffectShader) pBCBulletEffectShader->Render(pd3dCommandList, pCamera, 0);
	for (int i = 0; i < HELIBULLETS; i++)if (m_ppBullets[i]->m_bActive) { m_ppBullets[i]->Render(pd3dCommandList, pCamera); }
	for (int i = 0; i < CARTRIDGES; i++)if (m_ppCartridge[i]->m_bActive) { m_ppCartridge[i]->Render(pd3dCommandList, pCamera); }
	for (int i = 0; i < HELICOPTERVALKANS; i++)if (m_ppValkan[i]->m_bActive) { m_ppValkan[i]->Render(pd3dCommandList, pCamera); }

	if (m_pShadowShader) m_pShadowShader->Render(pd3dCommandList, pCamera, 0);
	if (m_pTreeBlendShadowShader) m_pTreeBlendShadowShader->Render(pd3dCommandList, pCamera, 0);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

}

void Stage1::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	m_pDepthRenderShader->PrepareShadowMap(pd3dCommandList);

}

void Stage1::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
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

void Stage1::BillBoardRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position)
{
	XMFLOAT3 MuzzleFrameLook = pCamera->GetLookVector();
	XMFLOAT3 MuzzleFramePosition = pCamera->GetPosition();

	if (m_pBillboardShader[1]) m_pBillboardShader[1]->Render(pd3dCommandList, pCamera, 0);
	if (((BulletMarkBillboard*)m_pBillboardShader[4])) ((BulletMarkBillboard*)m_pBillboardShader[4])->Render(pd3dCommandList, pCamera, 0);
	//if (((MuzzleFrameBillboard*)m_pBillboardShader[7])) ((MuzzleFrameBillboard*)m_pBillboardShader[7])->Render(pd3dCommandList, pCamera, Position);

	if (((MuzzleFrameBillboard*)m_pBillboardShader[6])) ((MuzzleFrameBillboard*)m_pBillboardShader[6])
		->Render(pd3dCommandList, pCamera, 0, MuzzleFrameLook, ((CHumanPlayer*)m_pPlayer)->m_pBulletFindFrame->GetPosition());

	for (int i = 0; i < m_nSpriteBillboards; i++) if (m_ppSpriteBillboard[i]) m_ppSpriteBillboard[i]->Render(pd3dCommandList, pCamera, 0);
}

void Stage1::NPCMuzzleFlamedRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT3 Position)
{
	XMFLOAT3 MuzzleFrameLook = pCamera->GetLookVector();
	XMFLOAT3 MuzzleFramePosition = pCamera->GetPosition();



	if (((MuzzleFrameBillboard*)m_pBillboardShader[7])) ((MuzzleFrameBillboard*)m_pBillboardShader[7])
		->Render(pd3dCommandList, pCamera, 0, MuzzleFrameLook, Position);

}

void Stage1::ParticleAnimation()
{
	m_pBillboardShader[0]->NextPosition.x = SmokePosition.x;
	m_pBillboardShader[0]->NextPosition.y = SmokePosition.y + 10.0f;
	m_pBillboardShader[0]->NextPosition.z = SmokePosition.z;
}
