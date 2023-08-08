#include "stdafx.h"
#include "Terrain.h"
#include "SceneMgr.h"

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT3 xmf3Normal) : CGameObject(3)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;
	SceneMgr* m_pScene = NULL;
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh* pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf3Normal, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pTerrainBaseTexture[2];
	pTerrainBaseTexture[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainBaseTexture[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Cliff_2.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainBaseTexture[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainBaseTexture[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/Cliff_1.dds", RESOURCE_TEXTURE2D, 0);
	DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	
	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature,0);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pScene->CreateSRVs(pd3dDevice, pTerrainBaseTexture[0], 0, 13);
	m_pScene->CreateSRVs(pd3dDevice, pTerrainBaseTexture[1], 0, 14);

	CMaterial* pTerrainMaterial = new CMaterial(2);
	pTerrainMaterial->SetTexture(pTerrainBaseTexture[0], 0);
	pTerrainMaterial->SetTexture(pTerrainBaseTexture[1], 1);
	pTerrainMaterial->SetReflection(2);
	pTerrainMaterial->SetShader(pTerrainShader);

	SetMaterial(0,pTerrainMaterial);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}
