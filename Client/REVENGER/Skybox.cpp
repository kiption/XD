#include "stdafx.h"
#include "Skybox.h"
#include "SceneMgr.h"
#include "StageScene.h"
CSkyBox::CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, SceneMgr* pScene) : GameObjectMgr(1), m_pScene(pScene)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 200.0f, 200.0f, 0.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	Texture* pSkyBoxTexture[1];
	pSkyBoxTexture[0] = new Texture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Skybox/pinksky.dds", RESOURCE_TEXTURE_CUBE, 0);

	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,0);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pScene) pScene->CreateSRVs(pd3dDevice, pSkyBoxTexture[0], 0, 10);

	CMaterial* pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture[0],0);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0,pSkyBoxMaterial);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	MainGameScene* pMain = m_pScene ? dynamic_cast<MainGameScene*>(m_pScene) : NULL;
	if (pMain && m_nMaterials > 0 && m_ppMaterials[0] && m_ppMaterials[0]->m_nTextures > 0 && m_ppMaterials[0]->m_ppTextures[0])
		pMain->CreateTextureSRVAtSlotZero(m_ppMaterials[0]->m_ppTextures[0], 10);
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	GameObjectMgr::Render(pd3dCommandList, pCamera);
}

COpeningBackScene::COpeningBackScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, SceneMgr* pScene) : GameObjectMgr(1)
{
	CSkyBoxMesh* pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	Texture* pSkyBoxTexture = new Texture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Skybox/moonburst.dds", RESOURCE_TEXTURE_CUBE, 0);
	DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	CSkyBoxShader* pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 0);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pScene) pScene->CreateSRVs(pd3dDevice, pSkyBoxTexture, 0, 10);

	CMaterial* pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture,0);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

COpeningBackScene::~COpeningBackScene()
{
}

void COpeningBackScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	GameObjectMgr::Render(pd3dCommandList, pCamera);
}
