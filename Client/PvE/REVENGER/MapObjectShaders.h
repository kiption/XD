#pragma once
#include "SkinAnimationShader.h"
#include "Terrain.h"

class WallObjectShaders : public CSkinnedAnimationObjectsShader
{
public:
	WallObjectShaders();
	virtual ~WallObjectShaders();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};

class BuildingObjectShader : public CSkinnedAnimationObjectsShader
{
public:
	BuildingObjectShader() {};
	virtual ~BuildingObjectShader() {};

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
};