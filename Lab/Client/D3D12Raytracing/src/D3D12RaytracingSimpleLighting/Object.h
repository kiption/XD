#pragma once
#include "DXSampleHelper.h"
#include "DeviceResources.h"


//class CMesh
//{
//public:
//	CMesh() { }
//	virtual ~CMesh() { }
//private:
//	int								m_nReferences = 0;
//
//public:
//	void AddRef() { m_nReferences++; }
//	void Release() { if (--m_nReferences <= 0) delete this; }
//
//	virtual void ReleaseUploadBuffers() { }
//
//protected:
//	ID3D12Resource* m_pd3dIndexBuffer = NULL;
//	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
//	ID3D12Resource* m_pd3dVertexBuffer = NULL;
//	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;
//	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
//	/*인덱스 버퍼(인덱스의 배열)와 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터이다. 인덱스 버퍼는 정점버퍼(배열)에 대한 인덱스를 가진다.*/
//	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
//	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	UINT							m_nSlot = 0;
//	UINT							m_nVertices = 0;
//	UINT							m_nOffset = 0;
//	UINT							m_nStride = 1;
//	UINT							m_nType = 0;
//	UINT							m_nIndices = 0;
//	//인덱스 버퍼에 포함되는 인덱스의 개수이다. 
//	UINT m_nStartIndex = 0;
//	//인덱스 버퍼에서 메쉬를 그리기 위해 사용되는 시작 인덱스이다. 
//	int m_nBaseVertex = 0;
//	//인덱스 버퍼의 인덱스에 더해질 인덱스이다. 
//public:
//	UINT GetType() { return(m_nType); }
//
//};
//
////Load Model
//class CMeshLoadInfo
//{
//public:
//	CMeshLoadInfo() { }
//	~CMeshLoadInfo();
//
//public:
//	char							m_pstrMeshName[256] = { 0 };
//
//	UINT							m_nType = 0x00;
//
//	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
//	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
//
//	int								m_nVertices = 0;
//	XMFLOAT3* m_pxmf3Positions = NULL;
//	XMFLOAT4* m_pxmf4Colors = NULL;
//	XMFLOAT3* m_pxmf3Normals = NULL;
//
//	int								m_nIndices = 0;
//	UINT* m_pnIndices = NULL;
//
//	int								m_nSubMeshes = 0;
//	int* m_pnSubSetIndices = NULL;
//	UINT** m_ppnSubSetIndices = NULL;
//};
//
//
//class CMeshFromFile : public CMesh
//{
//public:
//	CMeshFromFile(CMeshLoadInfo* pMeshInfo);
//	virtual ~CMeshFromFile();
//
//public:
//	virtual void ReleaseUploadBuffers();
//
//protected:
//	ID3D12Resource* m_pd3dPositionBuffer = NULL;
//	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
//	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;
//
//	int								m_nSubMeshes = 0;
//	int* m_pnSubSetIndices = NULL;
//
//	ID3D12Resource** m_ppd3dSubSetIndexBuffers = NULL;
//	ID3D12Resource** m_ppd3dSubSetIndexUploadBuffers = NULL;
//	D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews = NULL;
//
//public:
//	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
//};
//
//class CGameObject
//{
//public:
//	char							m_pstrFrameName[64];
//
//	CMesh* m_pMesh = NULL;
//	float m_fRotationSpeed = 0.0f;
//	int								m_nMaterials = 0;
//	CMeshLoadInfo* m_pMeshInfo = NULL;
//	XMFLOAT4X4						m_xmf4x4Transform;
//	XMFLOAT4X4						m_xmf4x4World;
//	CGameObject* m_pParent = NULL;
//	CGameObject* m_pChild = NULL;
//	CGameObject* m_pSibling = NULL;
//	static CGameObject* LoadGeometryFromFile(char* pstrFileName);
//	static CGameObject* LoadFrameHierarchyFromFile(FILE* pInFile);
//
//};