#pragma once

#include "Mesh.h"

class CGeometryShadowVertex : public CVertex
{
public:
	XMFLOAT2						m_xmf2Size;

public:
	CGeometryShadowVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf2Size = XMFLOAT2(5.0f, 10.0f); }
	CGeometryShadowVertex(float x, float y, float z, XMFLOAT2 xmf2Size = XMFLOAT2(5.0f, 10.0f), UINT nTexture = 0) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf2Size = xmf2Size; }
	CGeometryShadowVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size = XMFLOAT2(5.0f, 10.0f)) { m_xmf3Position = xmf3Position; m_xmf2Size = xmf2Size; }
	~CGeometryShadowVertex() { }
};

class CPlaneMeshIlluminated : public CMeshIlluminated
{
public:
	CPlaneMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f, float fxPosition = 0.0f, float fyPosition = 0.0f, float fzPosition = 0.0f);
	virtual ~CPlaneMeshIlluminated();
}; 
class CCubeMeshIlluminated : public CMeshIlluminated
{
public:
	CCubeMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshIlluminated();
};