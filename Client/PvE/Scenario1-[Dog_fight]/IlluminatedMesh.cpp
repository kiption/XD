#include "stdafx.h"
#include "IlluminatedMesh.h"



CPlaneMeshIlluminated::CPlaneMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(CIlluminatedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CIlluminatedVertex pVertices[6];

	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	if (fWidth == 0.0f)
	{
		if (fxPosition > 0.0f)
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
		}
	}
	else if (fHeight == 0.0f)
	{
		if (fyPosition > 0.0f)
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
		}
	}
	else if (fDepth == 0.0f)
	{
		if (fzPosition > 0.0f)
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
		}
		else
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
		}
	}

	m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = m_nStride;
	m_d3dPositionBufferView.SizeInBytes = m_nStride * m_nVertices;

	CalculateBoundingBox((XMFLOAT3*)pVertices, m_nStride);
}

CPlaneMeshIlluminated::~CPlaneMeshIlluminated()
{
}


CCubeMeshIlluminated::CCubeMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 24 * 3;
	m_nStride = sizeof(CIlluminatedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);
	int i = 0;

	XMFLOAT3 pxmf3Positions[72];

	//Upper Plane
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, -fz);

	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), -fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, -fz);

	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, -fz);

	//Lower Plane
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), +fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, +fz);

	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), +fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, +fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, +fz);

	//Right Plane
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), -fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), +fz);
	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, -fz);

	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, -fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), +fz);
	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, -fz);

	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x2, +y2, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, +fz);

	//Back/Right Plane
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -y3, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, +fz);

	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, +fz);

	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+x1, -y1, +fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, +fz);

	//Left Plane
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), +fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), -fz);
	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, -fz);

	pxmf3Positions[i++] = XMFLOAT3(0.0f, +(fy + y3), +fz);
	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-x2, +y2, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, +fz);

	//Back/Left Plane
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, -fz);
	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, +fz);

	pxmf3Positions[i++] = XMFLOAT3(0.0f, 0.0f, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-x1, -y1, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -y3, -fz);

	XMFLOAT3 pxmf3Normals[72];
	CalculateVertexNormals(pxmf3Normals, pxmf3Positions, m_nVertices, NULL, 0);

	CIlluminatedVertex pVertices[72];
	for (int i = 0; i < 72; i++) pVertices[i] = CIlluminatedVertex(pxmf3Positions[i], pxmf3Normals[i]);

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = m_nStride;
	m_d3dPositionBufferView.SizeInBytes = m_nStride * m_nVertices;

	CalculateBoundingBox((XMFLOAT3*)pxmf3Positions, sizeof(XMFLOAT3));
}

CCubeMeshIlluminated::~CCubeMeshIlluminated()
{
}