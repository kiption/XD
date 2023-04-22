#include "stdafx.h"
#include "Terrain.h"
#include "Scene.h"

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT3 xmf3Normal) : CGameObject(3)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;
	SceneManager* m_pScene = NULL;
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
	pTerrainShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pScene->CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture[0], 0, 13);
	m_pScene->CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture[1], 0, 14);

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

CUseWaterMoveTerrain::CUseWaterMoveTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT3 xmf3Normal) : CGameObject(1)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh* pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf3Normal, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pTerrainBaseTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainBaseTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/water2.dds", RESOURCE_TEXTURE2D, 0);

	CTexture* pTerrainDetailTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	pTerrainDetailTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Terrain/water2.dds", RESOURCE_TEXTURE2D, 0);

	CWaterShader* pTerrainShader = new CWaterShader();
	pTerrainShader->CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT, 0);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	SceneManager::CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture, 0, 13);
	SceneManager::CreateShaderResourceViews(pd3dDevice, pTerrainDetailTexture, 0, 14);

	CMaterial* pTerrainMaterial = new CMaterial(2);
	pTerrainMaterial->SetTexture(pTerrainBaseTexture, 0);
	pTerrainMaterial->SetTexture(pTerrainDetailTexture, 1);
	pTerrainMaterial->SetShader(pTerrainShader);

	SetMaterial(0, pTerrainMaterial);
}

CUseWaterMoveTerrain::~CUseWaterMoveTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

void CUseWaterMoveTerrain::Animate(float fElapsedTime)
{
	float MAX_MoveWavePoint = 80.0;
	float Min_MoveWavePoint = -80.0;
	if (m_bSurfacePoint == false)
	{

		this->m_xmf4x4World._41 += fElapsedTime * 6.5f;
		this->m_xmf4x4World._42 -= fElapsedTime * 0.5f;
		if (this->m_xmf4x4World._41 > MAX_MoveWavePoint)
		{
			m_bSurfacePoint = true;
		}
	}
	if (m_bSurfacePoint == true)
	{
		this->m_xmf4x4World._41 -= fElapsedTime * 6.5f;
		this->m_xmf4x4World._42 += fElapsedTime * 0.5f;
		if (this->m_xmf4x4World._41 < Min_MoveWavePoint)
		{
			m_bSurfacePoint = false;
		}
	}
}
