#include "stdafx.h"
#include "TerrainObject.h"
#include "TerrainShader.h"

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT3 xmf3Normal) : CGameObject(0, 1)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh * [m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf3Normal, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}


	CTexture* pTerrainTexture = new CTexture(5, RESOURCE_TEXTURE2D, 0, 1);

	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Glass.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Glass.dds", RESOURCE_TEXTURE2D, 1);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/PlanetPlan.dds", RESOURCE_TEXTURE2D, 2);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Glass.dds", RESOURCE_TEXTURE2D, 3);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/BasePlane.dds", RESOURCE_TEXTURE2D, 4);

	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
	pTerrainShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 5);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pTerrainTexture, 0, 11);

	CMaterial* pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);
	pTerrainMaterial->SetShader(pTerrainShader);
	SetMaterial(0, pTerrainMaterial);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}


CUseWaterMoveTerrain::CUseWaterMoveTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT3 xmf3Normal) :CGameObject(0, 1)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh * [m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf3Normal, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}



	CTexture* pTerrainTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0, 1);

	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/DetailWater.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Water.dds", RESOURCE_TEXTURE2D, 1);

	CWaterMoveShader* pTerrainShader = new CWaterMoveShader();
	pTerrainShader->CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
	pTerrainShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 2);
	//pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pTerrainTexture, 0, 11);

	CMaterial* pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);
	pTerrainMaterial->SetShader(pTerrainShader);
	SetMaterial(0, pTerrainMaterial);

}

CUseWaterMoveTerrain::~CUseWaterMoveTerrain()
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

void CUseWaterMoveTerrain::Animate(float fTimeElapsed)
{
	float MAX_MoveWavePoint = 100.0;
	float Min_MoveWavePoint = -100.0;
	if (m_bSurfacePoint == false)
	{

		this->m_xmf4x4World._41 += fTimeElapsed * 10.5f;
		this->m_xmf4x4World._42 -= fTimeElapsed * 5.5f;
		if (this->m_xmf4x4World._41 > MAX_MoveWavePoint)
		{
			m_bSurfacePoint = true;
		}
	}
	if (m_bSurfacePoint == true)
	{
		this->m_xmf4x4World._41 -= fTimeElapsed * 10.5f;
		this->m_xmf4x4World._42 += fTimeElapsed * 5.5f;
		if (this->m_xmf4x4World._41 < Min_MoveWavePoint)
		{
			m_bSurfacePoint = false;
		}
	}
}
