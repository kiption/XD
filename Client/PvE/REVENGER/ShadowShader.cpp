#include "stdafx.h"
#include "ShadowShader.h"
#include "DDSTextureLoader12.h"
#include "Scene.h"
#include "Terrain.h"



CObjectsShader::CObjectsShader()
{
}

CObjectsShader::~CObjectsShader()
{
}

void CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	if (m_nCurScene == SCENE1STAGE)
	{
		m_nObjects = 19;
		m_ppObjects = new CGameObject * [m_nObjects];

		CPlaneMeshIlluminated* pPlaneMesh = new CPlaneMeshIlluminated(pd3dDevice, pd3dCommandList, _PLANE_WIDTH + 2000.0, 0.0f, _PLANE_HEIGHT + 2000.0, 0.0f, 0.0f, 0.0f);
		CCubeMeshIlluminated* pCubeMesh = new CCubeMeshIlluminated(pd3dDevice, pd3dCommandList, 300.0f, 300.0f, 300.0f);
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

		CMaterial* pPlaneMaterial = new CMaterial(2);
		pPlaneMaterial->SetReflection(2);
		m_ppObjects[0] = new CGameObject(1);
		m_ppObjects[0]->SetMesh(pPlaneMesh);
		m_ppObjects[0]->SetMaterial(0, pPlaneMaterial);
		m_ppObjects[0]->SetPosition(XMFLOAT3(100.0f, 1.0, 100.0f));

		m_ppObjects[2] = new CGameObject(1);
		m_ppObjects[2]->SetMesh(NULL);
		m_ppObjects[2]->SetMaterial(0, pPlaneMaterial);
		m_ppObjects[2]->SetPosition(XMFLOAT3(100.0f, 0.0, 100.0f));

		CMaterial* pMaterial = new CMaterial(3);
		pMaterial->SetReflection(3);
		m_ppObjects[1] = new CGameObject(1);
		m_ppObjects[1]->SetMesh(NULL);
		m_ppObjects[1]->SetMaterial(0, pMaterial);
		m_ppObjects[1]->Rotate(0, 90, 0);
		m_ppObjects[1]->SetPosition(-400.0f, 50.0, 1300.0f);

		m_ppObjects[3] = new CGameObject(1);
		m_ppObjects[3]->SetMesh(NULL);
		m_ppObjects[3]->SetMaterial(0, pMaterial);
		m_ppObjects[3]->Rotate(0, 90, 0);
		m_ppObjects[3]->SetPosition(-400.0f, 50.0, 1300.0f);

		m_ppObjects[4] = new CGameObject(1);
		m_ppObjects[4]->SetMesh(NULL);
		m_ppObjects[4]->SetMaterial(0, pMaterial);
		m_ppObjects[4]->Rotate(0, 90, 0);
		m_ppObjects[4]->SetPosition(-400.0f, 50.0, 1300.0f);

		int OtherPlayers = 5;
		CMaterial* pOtherPlayerMaterial = new CMaterial(5);
		pOtherPlayerMaterial->SetReflection(5);

		for (int x = 5; x < OtherPlayers + 5; x++)
		{
			CLoadedModelInfo* pSoldierModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/MODEL.bin", NULL);
			m_ppObjects[x] = new CHumanoidObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSoldierModel, 7);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);

			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(1, false);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(3, false);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(4, false);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(5, false);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackEnable(6, false);

		/*	m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.15f);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackSpeed(1, 0.25f);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackSpeed(2, 0.35f);*/
			m_ppObjects[x]->SetMaterial(0, pMaterial);
			m_ppObjects[x]->SetScale(7.0,7.0,7.0);
			m_ppObjects[x]->SetPosition(XMFLOAT3(0.0 + x * 10, 5.0, 0.0 + x * 10));
			if (pSoldierModel) delete pSoldierModel;
		}

		m_nNpcObjects = 5;
		m_ppNpcObjects = new CGameObject * [m_nNpcObjects];
		CMaterial* pNpcMaterial = new CMaterial(5);
		pNpcMaterial->SetReflection(5);
		for (int i = 0; i < m_nNpcObjects; i++)
		{

			m_ppNpcObjects[i] = new CNpcHelicopterObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			m_ppNpcObjects[i]->SetMaterial(0, pNpcMaterial);
			m_ppNpcObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppNpcObjects[i]->OnPrepareAnimate();
		}


		m_ppObjects[10] = m_ppNpcObjects[0];
		m_ppObjects[11] = m_ppNpcObjects[1];
		m_ppObjects[12] = m_ppNpcObjects[2];
		m_ppObjects[13] = m_ppNpcObjects[3];
		m_ppObjects[14] = m_ppNpcObjects[4];


		CMaterial* pCityMaterial[6];
		pCityMaterial[0] = new CMaterial(1);
		pCityMaterial[1] = new CMaterial(1);
		pCityMaterial[2] = new CMaterial(1);
		pCityMaterial[3] = new CMaterial(1);
		pCityMaterial[4] = new CMaterial(1);
		pCityMaterial[5] = new CMaterial(1);

		pCityMaterial[0]->SetReflection(1);
		pCityMaterial[1]->SetReflection(1);
		pCityMaterial[2]->SetReflection(1);
		pCityMaterial[3]->SetReflection(1);
		pCityMaterial[4]->SetReflection(1);
		pCityMaterial[5]->SetReflection(1);

		int Cities = 4;
		m_ppCityGameObjects = new CGameObject * [Cities];
		CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
		for (int i = 0; i < Cities; i++)
		{
			string filename{ "Model/Stage1_(" };
			string num = to_string(i + 1);
			filename += num;
			filename += ").bin";
			cout << filename << endl;

			char* c_filename = const_cast<char*>(filename.c_str());
			m_ppCityGameObjects[i] = new CCityObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			//CGameObject* pGeneratorModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Stage1.bin", NULL);
			CGameObject* pGeneratorModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, c_filename, NULL);
			m_ppCityGameObjects[i]->SetChild(pGeneratorModel, false);
			m_ppCityGameObjects[i]->SetMaterial(0, pCityMaterial[i]);
			m_ppCityGameObjects[i]->Rotate(0.0f, 0.0f, 0.0f);
			m_ppCityGameObjects[i]->SetScale(1.0f, 1.0f, 1.0f);
			m_ppCityGameObjects[i]->OnPrepareAnimate();
			pGeneratorModel->AddRef();
		}

		m_ppObjects[15] = m_ppCityGameObjects[0];
		m_ppObjects[16] = m_ppCityGameObjects[1];
		m_ppObjects[17] = m_ppCityGameObjects[2];
		m_ppObjects[18] = m_ppCityGameObjects[3];
		/*m_ppObjects[19] = m_ppCityGameObjects[4];
		m_ppObjects[20] = m_ppCityGameObjects[5];*/

		m_ppObjects[15]->SetPosition(0.0f, 0.0f, 0.0f);
		m_ppObjects[16]->SetPosition(0.0f, 0.0f, 0.0f);
		m_ppObjects[17]->SetPosition(0.0f, 0.0f, 0.0f);
		m_ppObjects[18]->SetPosition(0.0f, 0.0f, 0.0f);
		//m_ppObjects[19]->SetPosition(0.0f, 1.0f, 0.0f);
		//m_ppObjects[20]->SetPosition(0.0f, 1.0f, 0.0f);
	}
	if (m_nCurScene == SCENE2STAGE)
	{
		m_nObjects = 12;
		m_ppObjects = new CGameObject * [m_nObjects];

		CPlaneMeshIlluminated* pPlaneMesh = new CPlaneMeshIlluminated(pd3dDevice, pd3dCommandList, _PLANE_WIDTH + 2000.0, 0.0f, _PLANE_HEIGHT + 2000.0, 0.0f, 0.0f, 0.0f);
		CCubeMeshIlluminated* pCubeMesh = new CCubeMeshIlluminated(pd3dDevice, pd3dCommandList, 300.0f, 300.0f, 300.0f);
		CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

		CMaterial* pPlaneMaterial = new CMaterial(1);
		pPlaneMaterial->SetReflection(1);
		m_ppObjects[0] = new CGameObject(1);
		m_ppObjects[0]->SetMesh(pPlaneMesh);
		m_ppObjects[0]->SetMaterial(0, pPlaneMaterial);
		m_ppObjects[0]->SetPosition(XMFLOAT3(1500.0f, 6.0, 2000.0f));


		CMaterial* pMaterial = new CMaterial(10);
		pMaterial->SetReflection(10);
		for (int x = 1; x < m_nObjects; x++)
		{
			CLoadedModelInfo* pSoldierModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Soldier_demo.bin", NULL);
			m_ppObjects[x] = new CHumanoidObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSoldierModel, 3);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.85f);
			m_ppObjects[x]->m_pSkinnedAnimationController->SetTrackPosition(0, 0);

			XMFLOAT3 xmf3Position = XMFLOAT3(1610.0, 146.0f, 2250.0f);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[x]->SetMaterial(0, pMaterial);
			m_ppObjects[x]->SetScale(7.0, 7.0, 7.0);
			m_ppObjects[x]->SetPosition(xmf3Position.x + x * 10.0f, xmf3Position.y, xmf3Position.z);
			if (pSoldierModel) delete pSoldierModel;
		}

		CMaterial* pMapMaterial = new CMaterial(1);
		pMapMaterial->SetReflection(1);
		CGameObject* pSceneModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GameObject.bin", NULL);

		m_ppObjects[11] = new CHelicopterObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[11]->SetChild(pSceneModel, false);
		m_ppObjects[11]->SetMaterial(0, pMapMaterial);
		m_ppObjects[11]->Rotate(0.0f, 0.0f, 0.0f);
		m_ppObjects[11]->SetScale(20.0, 20.0, 20.0);
		m_ppObjects[11]->SetPosition(1500.0, -3.5, 1500.0);
		pSceneModel->AddRef();

	}
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{

	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CObjectsShader::ReleaseObjects()
{

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CObjectsShader::ReleaseUploadBuffers()
{

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CIlluminatedShader::Render(pd3dCommandList, pCamera, nPipelineState);
	CPlayer* pPlayer = pCamera->GetPlayer();

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{

			m_ppObjects[j]->UpdateShaderVariables(pd3dCommandList);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CObjectsShader::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
}
#define _WITH_SCENE_ROOT_SIGNATURE
void CObjectsShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{

#ifdef _WITH_SCENE_ROOT_SIGNATURE
	m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
	m_pd3dGraphicsRootSignature->AddRef();
#else
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
#endif

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, d3dPrimitiveTopology,
		nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat, nPipelineState);

}

BoundingBox CObjectsShader::CalculateBoundingBox()
{

	BoundingBox xmBoundingBox;
	for (int i = 0; i < m_nObjects; i++)
	{
		for (int j = 0; j < m_ppObjects[i]->m_nMeshes; j++)
		{
			m_ppObjects[i]->CalculateBoundingBox();
			xmBoundingBox = m_ppObjects[i]->m_pMesh->m_xmBoundingBox;
			BoundingBox::CreateMerged(xmBoundingBox, xmBoundingBox, m_ppObjects[i]->m_pMesh->m_xmBoundingBox);
		}
	}
	return(xmBoundingBox);
}

D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{

	return(CShader::CompileShaderFromFile(L"Post.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", ppd3dShaderBlob));
}

void CObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		for (int k = 0; k < m_ppObjects[j]->m_nMaterials; k++)
		{

			CB_GAMEOBJECT_INFO* pbMappedcbGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
			XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
#ifdef _WITH_BATCH_MATERIAL
			if (m_ppMaterials[k]) pbMappedcbGameObject->m_nMaterialID = m_ppMaterials[k]->m_nReflection;
			if (m_ppMaterials[k]) pbMappedcbGameObject->m_nObjectID = j;
#endif
		}
	}
}

void CObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	CIlluminatedShader::ReleaseShaderVariables();
}

CShadowMapShader::CShadowMapShader(CObjectsShader* pObjectsShader)
{
	m_pObjectsShader = pObjectsShader;
}

CShadowMapShader::~CShadowMapShader()
{
}

D3D12_INPUT_LAYOUT_DESC CShadowMapShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0,32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}



D3D12_DEPTH_STENCIL_DESC CShadowMapShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_RASTERIZER_DESC CShadowMapShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSShadowMapShadow", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSShadowMapShadow", "ps_5_1", ppd3dShaderBlob));
}

void CShadowMapShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShadowMapShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

	if (m_pDepthTexture) m_pDepthTexture->UpdateShaderVariables(pd3dCommandList);


}

void CShadowMapShader::ReleaseShaderVariables()
{
}

void CShadowMapShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{

	m_pDepthTexture = (CTexture*)pContext;

	SceneManager* m_pScene = NULL;
	if (m_nCurScene == SCENE1STAGE)
	{
		m_pDepthTexture->AddRef();
		m_pScene->CreateShaderResourceViews(pd3dDevice, m_pDepthTexture, 0, 22);
	}
	if (m_nCurScene == SCENE2STAGE)
	{
		m_pDepthTexture->AddRef();
		m_pScene->CreateShaderResourceViews(pd3dDevice, m_pDepthTexture, 0, 16);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void CShadowMapShader::ReleaseObjects()
{

	if (m_pDepthTexture) m_pDepthTexture->Release();

}

void CShadowMapShader::ReleaseUploadBuffers()
{
}

void CShadowMapShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera, nPipelineState);

	UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_pObjectsShader->m_nObjects; i++)
	{
		if (m_pObjectsShader->m_ppObjects[i])
		{
			m_pObjectsShader->m_ppObjects[i]->UpdateShaderVariables(pd3dCommandList);
			m_pObjectsShader->m_ppObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CShadowMapShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, d3dPrimitiveTopology,
		nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat, nPipelineState);
}

CDepthRenderShader::CDepthRenderShader(CObjectsShader* pObjectsShader, LIGHT* pLights)
{
	m_pObjectsShader = pObjectsShader;

	m_pLights = pLights;
	m_pToLightSpaces = new TOLIGHTSPACES;

	XMFLOAT4X4 xmf4x4ToTexture = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f };
	m_xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);
}

CDepthRenderShader::~CDepthRenderShader()
{
	if (m_pToLightSpaces) delete m_pToLightSpaces;
}

D3D12_DEPTH_STENCIL_DESC CDepthRenderShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS_EQUAL
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}
#define _WITH_RASTERIZER_DEPTH_BIAS
D3D12_RASTERIZER_DESC CDepthRenderShader::CreateRasterizerState(int nPipelineState)
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
#ifdef _WITH_RASTERIZER_DEPTH_BIAS
	d3dRasterizerDesc.DepthBias = 250000;
#endif
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDepthWriteShader", "ps_5_1", ppd3dShaderBlob));
}

void CDepthRenderShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbDepthElementBytes;

	ncbDepthElementBytes = ((sizeof(TOLIGHTSPACES) + 255) & ~255); //256의 배수
	m_pd3dcbToLightSpaces = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbDepthElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbToLightSpaces->Map(0, NULL, (void**)&m_pcbMappedToLightSpaces);
}

void CDepthRenderShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedToLightSpaces, m_pToLightSpaces, sizeof(TOLIGHTSPACES));

	if (m_nCurScene == SCENE1STAGE)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbToLightGpuVirtualAddress = m_pd3dcbToLightSpaces->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(23, d3dcbToLightGpuVirtualAddress); //ToLight
	}
	if (m_nCurScene == SCENE2STAGE)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbToLightGpuVirtualAddress = m_pd3dcbToLightSpaces->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(18, d3dcbToLightGpuVirtualAddress); //ToLight
	}
}

void CDepthRenderShader::ReleaseShaderVariables()
{
	if (m_pd3dcbToLightSpaces)
	{
		m_pd3dcbToLightSpaces->Unmap(0, NULL);
		m_pd3dcbToLightSpaces->Release();
	}

}

void CDepthRenderShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	SceneManager* pScene = NULL;
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = MAX_DEPTH_TEXTURES;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

	m_pDepthTexture = new CTexture(MAX_DEPTH_TEXTURES, RESOURCE_TEXTURE2D_ARRAY, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R32_FLOAT, { 1.0f,1.0f, 1.0, 1.0f } };
	for (UINT i = 0; i < MAX_DEPTH_TEXTURES; i++)
		m_pDepthTexture->CreateTexture(pd3dDevice, pd3dCommandList, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, RESOURCE_TEXTURE2D, i);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < MAX_DEPTH_TEXTURES; i++)
	{
		ID3D12Resource* pd3dTextureResource = m_pDepthTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);

	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = _DEPTH_BUFFER_WIDTH;
	d3dResourceDesc.Height = _DEPTH_BUFFER_HEIGHT;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_d3dDsvDescriptorCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pd3dDevice->CreateDepthStencilView(m_pd3dDepthBuffer, &d3dDepthStencilViewDesc, m_d3dDsvDescriptorCPUHandle);

	for (int i = 0; i < MAX_DEPTH_TEXTURES; i++)
	{
		m_ppDepthRenderCameras[i] = new CCamera();
		m_ppDepthRenderCameras[i]->SetViewport(0, 0, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_ppDepthRenderCameras[i]->SetScissorRect(0, 0, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT);
		m_ppDepthRenderCameras[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CDepthRenderShader::ReleaseObjects()
{
	for (int i = 0; i < MAX_DEPTH_TEXTURES; i++)
	{
		if (m_ppDepthRenderCameras[i])
		{
			m_ppDepthRenderCameras[i]->ReleaseShaderVariables();
			delete m_ppDepthRenderCameras[i];
		}
	}

	if (m_pDepthTexture) m_pDepthTexture->Release();
	if (m_pd3dDepthBuffer) m_pd3dDepthBuffer->Release();

	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
}

void CDepthRenderShader::PrepareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList)
{

	for (int j = 0; j < MAX_LIGHTS; j++)
	{

		if (m_pLights[j].m_bEnable)
		{
			XMFLOAT3 xmf3Position = m_pLights[j].m_xmf3Position;
			XMFLOAT3 xmf3Look = m_pLights[j].m_xmf3Direction;
			XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, +1.0f, 0.0f);

			XMMATRIX xmmtxView = XMMatrixLookToLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Up));

			float fNearPlaneDistance = 20.0f, fFarPlaneDistance = m_pLights[j].m_fRange;

			XMMATRIX xmmtxProjection{};
			if (m_pLights[j].m_nType == DIRECTIONAL_LIGHT)
			{
				float fWidth = _PLANE_WIDTH, fHeight = _PLANE_HEIGHT;
				float fAspectRatio = _PLANE_WIDTH / _PLANE_HEIGHT;
				xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(120.0), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
				//float fLeft = -(_PLANE_WIDTH * 0.5f), fRight = +(_PLANE_WIDTH * 0.5f), fTop = +(_PLANE_HEIGHT * 0.5f), fBottom = -(_PLANE_HEIGHT * 0.5f);
				//xmmtxProjection = XMMatrixOrthographicOffCenterLH(fLeft * 6.0f, fRight * 6.0f, fBottom * 6.0f, fTop * 6.0f, fBack, fFront);
			}
			else if (m_pLights[j].m_nType == SPOT_LIGHT)
			{
				/*float fFovAngle = 60.0f; */m_pLights[j].m_fPhi = cos(120.0f);
				float fAspectRatio = float(_DEPTH_BUFFER_WIDTH) / float(_DEPTH_BUFFER_HEIGHT);
				xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_pLights[j].m_fPhi), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
			}
			else if (m_pLights[j].m_nType == POINT_LIGHT)
			{
				//ShadowMap[6]
			}

			m_ppDepthRenderCameras[j]->SetPosition(xmf3Position);
			XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4View, xmmtxView);
			XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4Projection, xmmtxProjection);
			//
			XMMATRIX xmmtxToTexture = XMMatrixTranspose(xmmtxView * xmmtxProjection * m_xmProjectionToTexture);
			XMStoreFloat4x4(&m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4x4ToTexture, xmmtxToTexture);

			m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position = XMFLOAT4(xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);

			::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

			FLOAT pfClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			pd3dCommandList->ClearRenderTargetView(m_pd3dRtvCPUDescriptorHandles[j], pfClearColor, 0, NULL);

			pd3dCommandList->ClearDepthStencilView(m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);

			pd3dCommandList->OMSetRenderTargets(1, &m_pd3dRtvCPUDescriptorHandles[j], TRUE, &m_d3dDsvDescriptorCPUHandle);

			Render(pd3dCommandList, m_ppDepthRenderCameras[j], 0);

			::SynchronizeResourceTransition(pd3dCommandList, m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		}
		else
		{
			m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position.w = 0.0f;
		}
	}

}

void CDepthRenderShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelinestates)
{
	CShader::Render(pd3dCommandList, pCamera, nPipelinestates);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_pObjectsShader->m_nObjects; i++)
	{
		if (m_pObjectsShader->m_ppObjects[i])
		{
			m_pObjectsShader->m_ppObjects[i]->UpdateShaderVariables(pd3dCommandList);
			m_pObjectsShader->m_ppObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CDepthRenderShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, d3dPrimitiveTopology,
		nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat, nPipelineState);

}

CTextureToViewportShader::CTextureToViewportShader()
{
}

CTextureToViewportShader::~CTextureToViewportShader()
{
}

D3D12_DEPTH_STENCIL_DESC CTextureToViewportShader::CreateDepthStencilState(int nPipelineState)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextureToViewport", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextureToViewport", "ps_5_1", ppd3dShaderBlob));
}

void CTextureToViewportShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CTextureToViewportShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CTextureToViewportShader::ReleaseObjects()
{
}

void CTextureToViewportShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera, nPipelineState);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void CTextureToViewportShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	DXGI_FORMAT pdxgiRtvBaseFormats[1] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, d3dPrimitiveTopology,
		nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat, nPipelineState);
}

