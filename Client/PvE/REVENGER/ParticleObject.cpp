#include "stdafx.h"
#include "Scene.h"
#include "ParticleMesh.h"
#include "ParticleObject.h"
#include "ParticleShader.h"

CParticleObject::CParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Velocity, float fLifetime, XMFLOAT3 xmf3Acceleration, XMFLOAT3 xmf3Color, XMFLOAT2 xmf2Size, UINT nMaxParticles) : CGameObject(1)
{
	//CParticleMesh* pMesh = new CParticleMesh(pd3dDevice, pd3dCommandList, xmf3Position, xmf3Velocity, fLifetime, xmf3Acceleration, xmf3Color, xmf2Size, nMaxParticles);
	//SetMesh(pMesh);

	//CTexture* pParticleTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	//pParticleTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Billboard/RoundSoftParticle.dds", RESOURCE_TEXTURE2D, 0);

	//CMaterial* pMaterial = new CMaterial(1);
	//pMaterial->SetTextures(pParticleTexture);  // D

	//srand((unsigned)time(NULL));
	//XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	//for (int i = 0; i < 1024; i++) { pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f; }
	//DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	//m_pRandowmValueTexture = new CTexture(1, RESOURCE_TEXTURE1D, 0, 1);
	////	m_pRandowmValueTexture = new CTexture(1, RESOURCE_BUFFER, 0, 1);
	//m_pRandowmValueTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	//m_pRandowmValueOnSphereTexture = new CTexture(1, RESOURCE_TEXTURE1D, 0, 1);
	//m_pRandowmValueOnSphereTexture->CreateBuffer(pd3dDevice, pd3dCommandList, pxmf4RandomValues, 256, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//ParticleShader* pShader = new ParticleShader();
	//pShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,
	//	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 0, pdxgiRtvBaseFormats, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	//pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SceneManager* pScene = NULL;
	//
	//pScene->CreateConstantBufferViews(pd3dDevice, 0, m_pd3dcbGameObject, ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255));
	//pScene->CreateShaderResourceViews(pd3dDevice, pParticleTexture, 0, 16);
	//pScene->CreateShaderResourceViews(pd3dDevice, m_pRandowmValueTexture, 0, 17);
	//pScene->CreateShaderResourceViews(pd3dDevice, m_pRandowmValueOnSphereTexture, 0, 18);

	//SetCbvGPUDescriptorHandle(pScene->GetGPUCbvDescriptorStartHandle());

	//pMaterial->SetShader(pShader);
	//SetMaterial(0, pMaterial);
}


CParticleObject::~CParticleObject()
{
	if (m_pRandowmValueTexture) m_pRandowmValueTexture->Release();
	if (m_pRandowmValueOnSphereTexture) m_pRandowmValueOnSphereTexture->Release();
}


void CParticleObject::ReleaseUploadBuffers()
{
	if (m_pRandowmValueTexture) m_pRandowmValueTexture->ReleaseUploadBuffers();
	if (m_pRandowmValueOnSphereTexture) m_pRandowmValueOnSphereTexture->ReleaseUploadBuffers();

	CGameObject::ReleaseUploadBuffers();
}

void CParticleObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent); //D
}

void CParticleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{

			//if (m_ppMaterials[i]->m_pShader)  m_ppMaterials[i]->m_pShader->OnPrepareRender(pd3dCommandList, 0);
			if (m_ppMaterials[i]->m_pTexture) m_ppMaterials[i]->m_pTexture->UpdateShaderVariables(pd3dCommandList);

			if (m_pRandowmValueTexture) m_pRandowmValueTexture->UpdateShaderVariables(pd3dCommandList);
			if (m_pRandowmValueOnSphereTexture) m_pRandowmValueOnSphereTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	UpdateShaderVariables(pd3dCommandList);


	if (m_pMesh) m_pMesh->PreRender(pd3dCommandList, 0); //Stream Output
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, 0); //Stream Output
	if (m_pMesh) m_pMesh->PostRender(pd3dCommandList, 0); //Stream Output
	for (int i = 0; i < m_nMaterials; i++)
		//if (m_ppMaterials[i] && m_ppMaterials[i]->m_pShader)m_ppMaterials[i]->m_pShader->OnPrepareRender(pd3dCommandList, 1);
	if (m_pMesh)m_pMesh->PreRender(pd3dCommandList, 1); //Draw
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, 1); //Draw
}

void CParticleObject::OnPostRender()
{
	if (m_pMesh) m_pMesh->OnPostRender(0); //Read Stream Output Buffer Filled Size
}


CExplosiveObject::CExplosiveObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CExplosiveObject::~CExplosiveObject()
{
}

void CExplosiveObject::Animate(float fElapsedTime)
{


	CGameObject::Animate(fElapsedTime);

}

void CExplosiveObject::PrepareExplosion()
{
	
}


