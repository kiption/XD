#pragma once
#include "Object.h"

class SceneMgr;
class CSkyBoxShader;

class MainGameScene;
class CSkyBox : public GameObjectMgr
{
	SceneMgr* m_pScene = NULL;
public:
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, SceneMgr* pScene = NULL);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};
class COpeningBackScene : public GameObjectMgr
{
public:
	COpeningBackScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, SceneMgr* pScene = NULL);
	virtual ~COpeningBackScene();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};