#include "stdafx.h"
#include "Object.h"

CMeshLoadInfo::~CMeshLoadInfo()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;

	if (m_pnIndices) delete[] m_pnIndices;

	if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;

	for (int i = 0; i < m_nSubMeshes; i++) if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
	if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CMeshFromFile::CMeshFromFile(CMeshLoadInfo* pMeshInfo)
{
	m_nVertices = pMeshInfo->m_nVertices;
	m_nType = pMeshInfo->m_nType;

	//m_pd3dPositionBuffer = ::CreateBufferSRV(pd3dDevice, pd3dCommandList, pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = pMeshInfo->m_nSubMeshes;
	if (m_nSubMeshes > 0)
	{
		m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
		m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
		m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

		m_pnSubSetIndices = new int[m_nSubMeshes];

		for (int i = 0; i < m_nSubMeshes; i++)
		{
			m_pnSubSetIndices[i] = pMeshInfo->m_pnSubSetIndices[i];
			//m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);
		
			m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
			m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
			m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * pMeshInfo->m_pnSubSetIndices[i];
		}
	}
}

CMeshFromFile::~CMeshFromFile()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
	}
}

void CMeshFromFile::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

CGameObject* CGameObject::LoadGeometryFromFile(char* pstrFileName)
{
//	char pstrToken[64] = { '\0' };
//	UINT nReads = 0;
//
//	int nFrame = 0;
//
CGameObject* pGameObject = NULL;
//
//	for (; ; )
//	{
//		::ReadStringFromFile(pInFile, pstrToken);
//		if (!strcmp(pstrToken, "<Frame>:"))
//		{
//			pGameObject = new CGameObject();
//
//			nFrame = ::ReadIntegerFromFile(pInFile);
//			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
//		}
//		else if (!strcmp(pstrToken, "<Transform>:"))
//		{
//			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
//			XMFLOAT4 xmf4Rotation;
//			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
//			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
//			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
//			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
//		}
//		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
//		{
//			nReads = (UINT)::fread(&pGameObject->m_xmf4x4Transform, sizeof(float), 16, pInFile);
//		}
//		else if (!strcmp(pstrToken, "<Mesh>:"))
//		{
//			CMeshLoadInfo* pMeshInfo = pGameObject->LoadMeshInfoFromFile(pInFile);
//			if (pMeshInfo)
//			{
//				CMesh* pMesh = NULL;
//				if (pMeshInfo->m_nType & VERTEXT_NORMAL)
//				{
//					pMesh = new CMeshIlluminatedFromFile(pMeshInfo);
//				}
//				if (pMesh) pGameObject->SetMesh(pMesh);
//				delete pMeshInfo;
//			}
//		}
//		else if (!strcmp(pstrToken, "<Materials>:"))
//		{
//			MATERIALSLOADINFO* pMaterialsInfo = pGameObject->LoadMaterialsInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
//			if (pMaterialsInfo && (pMaterialsInfo->m_nMaterials > 0))
//			{
//				pGameObject->m_nMaterials = pMaterialsInfo->m_nMaterials;
//				pGameObject->m_ppMaterials = new CMaterial * [pMaterialsInfo->m_nMaterials];
//
//				for (int i = 0; i < pMaterialsInfo->m_nMaterials; i++)
//				{
//					pGameObject->m_ppMaterials[i] = NULL;
//
//					CMaterial* pMaterial = new CMaterial();
//
//					CMaterialColors* pMaterialColors = new CMaterialColors(&pMaterialsInfo->m_pMaterials[i]);
//					pMaterial->SetMaterialColors(pMaterialColors);
//
//					if (pGameObject->GetMeshType() & VERTEXT_NORMAL) pMaterial->SetIlluminatedShader();
//
//					pGameObject->SetMaterial(i, pMaterial);
//				}
//			}
//		}
//		else if (!strcmp(pstrToken, "<Children>:"))
//		{
//			int nChilds = ::ReadIntegerFromFile(pInFile);
//			if (nChilds > 0)
//			{
//				for (int i = 0; i < nChilds; i++)
//				{
//					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pInFile);
//					if (pChild) pGameObject->SetChild(pChild);
//#ifdef _WITH_DEBUG_RUNTIME_FRAME_HIERARCHY
//					TCHAR pstrDebug[256] = { 0 };
//					_stprintf_s(pstrDebug, 256, _T("(Child Frame: %p) (Parent Frame: %p)\n"), pChild, pGameObject);
//					OutputDebugString(pstrDebug);
//#endif
//				}
//			}
//		}
//		else if (!strcmp(pstrToken, "</Frame>"))
//		{
//			break;
//		}
//	}
	return(pGameObject);
}
