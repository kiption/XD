#pragma once
#include "SkinAnimationShader.h"
#include "StandardShader.h"
#include "Terrain.h"



class CMapObjectShader : public CStandardShader
{
public:
	CMapObjectShader() {};
	virtual ~CMapObjectShader() {};

	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	BoundingBox CalculateBoundingBox();
};

class CStage2MapObjectShader : public CStandardShader
{
public:
	CStage2MapObjectShader() {};
	virtual ~CStage2MapObjectShader() {};

	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseUploadBuffers();

	BoundingOrientedBox m_xoobb = BoundingOrientedBox(XMFLOAT3(),XMFLOAT3(),XMFLOAT4());
};