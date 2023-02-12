#pragma once
#include "SkinAnimationShader.h"
#include "Terrain.h"
class CHellicopterObjectsShader : public CStandardObjectsShader
{
public:
	CHellicopterObjectsShader();
	virtual ~CHellicopterObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

class CHumanObjectsShader : public CSkinnedAnimationObjectsShader
{
public:
	CHumanObjectsShader();
	virtual ~CHumanObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

class CAngrybotObjectsShader : public CSkinnedAnimationObjectsShader
{
public:
	CAngrybotObjectsShader();
	virtual ~CAngrybotObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CEthanObjectsShader : public CSkinnedAnimationObjectsShader
{
public:
	CEthanObjectsShader();
	virtual ~CEthanObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

