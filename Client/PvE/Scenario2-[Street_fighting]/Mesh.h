//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once

class CGameObject;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define VERTEXT_POSITION				0x0001
#define VERTEXT_COLOR					0x0002
#define VERTEXT_NORMAL					0x0004
#define VERTEXT_TANGENT					0x0008
#define VERTEXT_TEXTURE_COORD0			0x0010
#define VERTEXT_TEXTURE_COORD1			0x0020

#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

#define VERTEXT_TEXTURE					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_DETAIL					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TEXTURE			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_TANGENT_TEXTURE	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_DETAIL			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TANGENT__DETAIL	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual ~CMesh();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	char							m_pstrMeshName[64] = { 0 };

	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nOffset = 0;
	UINT							m_nIndices = 0;
	UINT* m_pnIndices = NULL;

protected:
	int								m_nVertices = 0;

	XMFLOAT3* m_pxmf3Positions = NULL;
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dPositionBufferView;
	UINT							m_nSubsets = 0;
	int	m_nSubMeshes = 0;
	UINT* m_pnSubSetIndices = NULL;
	UINT** m_ppnSubSetIndices = NULL;
	ID3D12Resource** m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource** m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews = NULL;
	UINT* m_pnSubSetStartIndices = NULL;

	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;
	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dVertexBufferViews = NULL;

	ID3D12Resource** m_pd3dIndexBuffer = NULL;
	ID3D12Resource** m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW*			m_d3dIndexBufferView = NULL;

	XMFLOAT3* m_pxmf3Normals = NULL;
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView;

	XMFLOAT2* m_pxmf2TextureCoords = NULL;
	ID3D12Resource* m_pd3dTextureCoordBuffer = NULL;
	ID3D12Resource* m_pd3dTextureCoordUploadBuffer = NULL;
	BoundingBox						m_xmBoundingBox;

public:
	UINT GetType() { return(m_nType); }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);

	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapImage
{
private:
	BYTE* m_pHeightMapPixels;

	int								m_nWidth;
	int								m_nLength;
	XMFLOAT3						m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z, bool bReverseQuad = false);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }

	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	int								m_nWidth;
	int								m_nLength;
	XMFLOAT3						m_xmf3Scale;

protected:
	XMFLOAT4* m_pxmf4Colors = NULL;
	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2* m_pxmf2TextureCoords1 = NULL;
	XMFLOAT3* m_pxmf3Normal = NULL;
	ID3D12Resource* m_pd3dColorBuffer = NULL;
	ID3D12Resource* m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

	ID3D12Resource* m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource* m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource* m_pd3dTextureCoord1Buffer = NULL;
	ID3D12Resource* m_pd3dTextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void* pContext = NULL);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void* pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void* pContext);

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxMesh : public CMesh
{
public:
	CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardMesh : public CMesh
{
public:
	CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CStandardMesh();

protected:
	XMFLOAT4* m_pxmf4Colors = NULL;
	XMFLOAT3* m_pxmf3Normals = NULL;
	XMFLOAT3* m_pxmf3Tangents = NULL;
	XMFLOAT3* m_pxmf3BiTangents = NULL;
	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2* m_pxmf2TextureCoords1 = NULL;

	ID3D12Resource* m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource* m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource* m_pd3dTextureCoord1Buffer = NULL;
	ID3D12Resource* m_pd3dTextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	ID3D12Resource* m_pd3dTangentBuffer = NULL;
	ID3D12Resource* m_pd3dTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTangentBufferView;

	ID3D12Resource* m_pd3dBiTangentBuffer = NULL;
	ID3D12Resource* m_pd3dBiTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBiTangentBufferView;

public:
	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define SKINNED_ANIMATION_BONES		128

class CSkinnedMesh : public CStandardMesh
{
public:
	CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CSkinnedMesh();

protected:
	char							m_pstrSkinnedMeshName[64] = { 0 };

	int								m_nBonesPerVertex = 4;

	// 뼈들의 인덱스와 웨이트를 버텍스 버퍼로 만들어 입력조립기로 처리한다.
	XMUINT4* m_pxmu4BoneIndices = NULL;
	XMFLOAT4* m_pxmf4BoneWeights = NULL;

	ID3D12Resource* m_pd3dBoneIndexBuffer = NULL;
	ID3D12Resource* m_pd3dBoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneIndexBufferView;

	ID3D12Resource* m_pd3dBoneWeightBuffer = NULL;
	ID3D12Resource* m_pd3dBoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneWeightBufferView;

public:
	int								m_nSkinningBones = 0;

	char(*m_ppstrSkinningBoneNames)[64];
	// 뼈들의 개수만큼 캐싱을 해서 넣어놓겠다.
	CGameObject** m_ppSkinningBoneFrameCaches = NULL;

	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL;

	// 스킨 메쉬를 스키닝해서 정점블렌딩으로 그리기위해 본 오프셋 행렬이 필요하다.
	ID3D12Resource* m_pd3dcbBoneOffsets = NULL;
	XMFLOAT4X4* m_pcbxmf4x4BoneOffsets = NULL;

	// 상수 버퍼로 넘기기위해 변환형태의 행렬이 필요하다.
	ID3D12Resource* m_pd3dcbBoneTransforms = NULL;
	XMFLOAT4X4* m_pcbxmf4x4BoneTransforms = NULL;

public:
	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
};



class CTexturedRectMesh : public CMesh
{
public:
	CTexturedRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition);
	virtual ~CTexturedRectMesh();

protected:
	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	ID3D12Resource* m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource* m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

public:
	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};


class CSceneMesh : public CMesh
{
public:
	CSceneMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName = NULL);
	virtual ~CSceneMesh();
	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubset);

};