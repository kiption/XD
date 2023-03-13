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
	m_nObjects = 17;
	m_ppObjects = new CGameObject * [m_nObjects];


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CPlaneMeshIlluminated* pPlaneMesh = new CPlaneMeshIlluminated(pd3dDevice, pd3dCommandList, _PLANE_WIDTH + 1000.0, 0.0f, _PLANE_HEIGHT + 1000.0, 0.0f, 0.0f, 0.0f);
	CCubeMeshIlluminated* pCubeMesh = new CCubeMeshIlluminated(pd3dDevice, pd3dCommandList, 100.0f, 100.0f, 100.0f);

	CMaterial* pPlaneMaterial = new CMaterial(1);
	pPlaneMaterial->SetReflection(1);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	m_ppObjects[0] = new CGameObject(1);
	m_ppObjects[0]->SetMesh(pPlaneMesh);
	m_ppObjects[0]->SetMaterial(pPlaneMaterial);
	m_ppObjects[0]->SetPosition(XMFLOAT3(100.0f, -65.0, 100.0f));

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetReflection(1);
	CGameObject* pGeneratorModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Stage1.bin", NULL);

	m_ppObjects[1] = new CGameObject(1);
	m_ppObjects[1]->SetChild(pGeneratorModel, false);
	m_ppObjects[1]->SetScale(1.0, 1.0, 1.0);
	m_ppObjects[1]->SetMaterial(pMaterial);
	m_ppObjects[1]->SetPosition(-200.0f, -20.0, 500.0f);

	CPlaneMeshIlluminated* pPlaneMesh2 = new CPlaneMeshIlluminated(pd3dDevice, pd3dCommandList, _PLANE_WIDTH + 10.0, 0.0f, _PLANE_HEIGHT + 10.0, 0.0f, 0.0f, 0.0f);
	CMaterial* pPlaneMaterial2 = new CMaterial(1);
	pPlaneMaterial2->SetReflection(1);
	m_ppObjects[2] = new CGameObject(1);
	m_ppObjects[2]->SetMesh(pPlaneMesh2);
	m_ppObjects[2]->SetMaterial(pPlaneMaterial2);

	m_ppObjects[2]->SetPosition(XMFLOAT3(2000.0f, pTerrain->GetHeight(2000.0, 1500.0f) + 200.0, 1500.0f));


	pMaterial = new CMaterial(3);
	pMaterial->SetReflection(3);

	m_ppObjects[3] = new CGameObject(1);
	m_ppObjects[3]->SetMesh(pCubeMesh);
	m_ppObjects[3]->SetMaterial(pMaterial);
	m_ppObjects[3]->SetPosition(-500.0f, -20.0, 500.0f);

	XMFLOAT3 xmf3Scale(5.0f, 4.0f, 5.0f);
	XMFLOAT3 xmf3Normal(1.0f, 1.0f, 1.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, _T("Terrain/Stage2.raw"), 257, 257, xmf3Scale, xmf3Normal);
	m_pTerrain->SetMaterial(pMaterial);
	m_pTerrain->SetPosition(-500.0f, -90.0, -500.0f);
	m_ppObjects[4] = m_pTerrain;

	m_nHierarchicalGameObjects = 5;
	m_ppHierarchicalGameObjects = new CGameObject * [m_nHierarchicalGameObjects];
	CMaterial* pOtherPlayerMaterial = new CMaterial(5);
	pOtherPlayerMaterial->SetReflection(5);

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{

		m_ppHierarchicalGameObjects[i] = new CMi24Object(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		CGameObject* pOtherPlayerModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
		m_ppHierarchicalGameObjects[i]->SetChild(pOtherPlayerModel, false);
		m_ppHierarchicalGameObjects[i]->SetMaterial(pOtherPlayerMaterial);
		m_ppHierarchicalGameObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppHierarchicalGameObjects[i]->SetScale(10.0, 10.0, 10.0);
		m_ppHierarchicalGameObjects[i]->OnPrepareAnimate();
		pOtherPlayerModel->AddRef();
	}
	m_ppObjects[5] = m_ppHierarchicalGameObjects[0];
	m_ppObjects[6] = m_ppHierarchicalGameObjects[1];
	m_ppObjects[7] = m_ppHierarchicalGameObjects[2];
	m_ppObjects[8] = m_ppHierarchicalGameObjects[3];
	m_ppObjects[9] = m_ppHierarchicalGameObjects[4];

	m_nNpcObjects = 7;
	m_ppNpcObjects = new CGameObject * [m_nNpcObjects];
	CMaterial* pNpcMaterial = new CMaterial(5);
	pNpcMaterial->SetReflection(5);
	for (int i = 0; i < m_nNpcObjects; i++)
	{

		m_ppNpcObjects[i] = new CMi24Object(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		CGameObject* pNpcModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
		m_ppNpcObjects[i]->SetChild(pNpcModel, false);
		m_ppNpcObjects[i]->SetMaterial(pNpcMaterial);
		m_ppNpcObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppNpcObjects[i]->SetScale(10.0, 10.0, 10.0);
		m_ppNpcObjects[i]->OnPrepareAnimate();
		pNpcModel->AddRef();
	}
	m_ppObjects[10] = m_ppNpcObjects[0];
	m_ppObjects[11] = m_ppNpcObjects[1];
	m_ppObjects[12] = m_ppNpcObjects[2];
	m_ppObjects[13] = m_ppNpcObjects[3];
	m_ppObjects[14] = m_ppNpcObjects[4];
	m_ppObjects[15] = m_ppNpcObjects[5];
	m_ppObjects[16] = m_ppNpcObjects[6];

	//CGameObject* pBulletMesh = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Bullet1(1).bin", NULL);
	//CMaterial* pBulletMaterial = new CMaterial(100);
	//pBulletMaterial->SetReflection(100);
	//for (int i = 0; i < 3; i++)
	//{
	//	m_ppBullets[i] = new CBulletObject(NULL);
	//	m_ppBullets[i]->SetChild(pBulletMesh, false);
	//	m_ppBullets[i]->SetMaterial(pNpcMaterial);
	//	m_ppBullets[i]->SetScale(10.0, 10.0, 10.0);
	//	m_ppBullets[i]->OnPrepareAnimate();
	//	pBulletMesh->AddRef();
	//	/*	pBulletObject->SetChild(pBulletMesh->m_pModelRootObject, true);*/
	//	//	pBulletObject->m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 0, pBulletMesh);
	//	for (int j = 15; j <= ; j++)m_ppObjects[j] = m_ppBullets[i];
	//}

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

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CIlluminatedShader::Render(pd3dCommandList, pCamera);

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

BoundingBox CObjectsShader::CalculateBoundingBox()
{

	BoundingBox xmBoundingBox;
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->CalculateBoundingBox();
		xmBoundingBox = m_ppObjects[i]->m_xmBoundingBox;
		for (int i = 1; i < m_nObjects; i++)BoundingBox::CreateMerged(xmBoundingBox, xmBoundingBox, m_ppObjects[i]->m_xmBoundingBox);
	}
	return(xmBoundingBox);
}

CShadowMapShader::CShadowMapShader(CObjectsShader* pObjectsShader)
{
	m_pObjectsShader = pObjectsShader;
}

CShadowMapShader::~CShadowMapShader()
{
}

D3D12_DEPTH_STENCIL_DESC CShadowMapShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
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

D3D12_SHADER_BYTECODE CShadowMapShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSShadowMapShadow", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSShadowMapShadow", "ps_5_1", &m_pd3dPixelShaderBlob));
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
	m_pDepthTexture->AddRef();
	SceneManager* m_pScene = NULL;
	//SceneManager::CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, m_pDepthTexture->GetTextures());
	SceneManager::CreateShaderResourceViews(pd3dDevice, m_pDepthTexture, 0, 22);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void CShadowMapShader::ReleaseObjects()
{

	if (m_pDepthTexture) m_pDepthTexture->Release();

}

void CShadowMapShader::ReleaseUploadBuffers()
{
}

void CShadowMapShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

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

D3D12_DEPTH_STENCIL_DESC CDepthRenderShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //D3D12_COMPARISON_FUNC_LESS_EQUAL
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
D3D12_RASTERIZER_DESC CDepthRenderShader::CreateRasterizerState()
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

D3D12_SHADER_BYTECODE CDepthRenderShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDepthWriteShader", "ps_5_1", &m_pd3dPixelShaderBlob));
}

void CDepthRenderShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbDepthElementBytes;

	ncbDepthElementBytes = ((sizeof(TOLIGHTSPACES) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbToLightSpaces = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbDepthElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbToLightSpaces->Map(0, NULL, (void**)&m_pcbMappedToLightSpaces);
}

void CDepthRenderShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedToLightSpaces, m_pToLightSpaces, sizeof(TOLIGHTSPACES));

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbToLightGpuVirtualAddress = m_pd3dcbToLightSpaces->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(24, d3dcbToLightGpuVirtualAddress); //ToLight
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

	//for (int j = 0; j < MAX_LIGHTS; j++)
	//{

	//	if (m_pLights[j].m_bEnable)
	//	{
	//		XMFLOAT3 xmf3Position = m_pLights[j].m_xmf3Position;
	//		XMFLOAT3 xmf3Look = m_pLights[j].m_xmf3Direction;
	//		XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, +1.0f, 0.0f);

	//		XMMATRIX xmmtxView = XMMatrixLookToLH(XMLoadFloat3(&xmf3Position), XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Up));

	//		float fNearPlaneDistance = 10.0f, fFarPlaneDistance = m_pLights[j].m_fRange;

	//		XMMATRIX xmmtxProjection;
	//		if (m_pLights[j].m_nType == DIRECTIONAL_LIGHT)
	//		{
	//			float fWidth = _PLANE_WIDTH, fHeight = _PLANE_HEIGHT;
	//			xmmtxProjection = XMMatrixOrthographicLH(fWidth, fHeight, fNearPlaneDistance, fFarPlaneDistance);
	//			//float fLeft = -(_PLANE_WIDTH * 0.5f), fRight = +(_PLANE_WIDTH * 0.5f), fTop = +(_PLANE_HEIGHT * 0.5f), fBottom = -(_PLANE_HEIGHT * 0.5f);
	//			//xmmtxProjection = XMMatrixOrthographicOffCenterLH(fLeft * 6.0f, fRight * 6.0f, fBottom * 6.0f, fTop * 6.0f, fBack, fFront);
	//		}
	//		else if (m_pLights[j].m_nType == SPOT_LIGHT)
	//		{
	//			/*float fFovAngle = 60.0f; */m_pLights[j].m_fPhi = cos(60.0f);
	//			float fAspectRatio = float(_DEPTH_BUFFER_WIDTH) / float(_DEPTH_BUFFER_HEIGHT);
	//			xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_pLights[j].m_fPhi), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	//		}
	//		else if (m_pLights[j].m_nType == POINT_LIGHT)
	//		{
	//			//ShadowMap[6]
	//		}

	//		m_ppDepthRenderCameras[j]->SetPosition(xmf3Position);
	//		XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4View, xmmtxView);
	//		XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4Projection, xmmtxProjection);
	//		//
	//		XMMATRIX xmmtxToTexture = XMMatrixTranspose(xmmtxView * xmmtxProjection * m_xmProjectionToTexture);
	//		XMStoreFloat4x4(&m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4x4ToTexture, xmmtxToTexture);

	//		m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position = XMFLOAT4(xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);

	//		//::SynchronizeResourceTransition(pd3dCommandList, m_pDepthRenderShader->m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//		//FLOAT pfClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//		//pd3dCommandList->ClearRenderTargetView(m_pDepthRenderShader->m_pd3dRtvCPUDescriptorHandles[j], pfClearColor, 0, NULL);

	//		//pd3dCommandList->ClearDepthStencilView(m_pDepthRenderShader->m_d3dDsvDescriptorCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);

	//		//pd3dCommandList->OMSetRenderTargets(1, &m_pDepthRenderShader->m_pd3dRtvCPUDescriptorHandles[j], TRUE, &m_pDepthRenderShader->m_d3dDsvDescriptorCPUHandle);

	//		//Render(pd3dCommandList, m_pDepthRenderShader->m_ppDepthRenderCameras[j]);

	//		//::SynchronizeResourceTransition(pd3dCommandList, m_pDepthRenderShader->m_pDepthTexture->GetResource(j), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	//	}
	//	else
	//	{
	//		m_pToLightSpaces->m_pToLightSpaces[j].m_xmf4Position.w = 0.0f;
	//	}
	//}
}

void CDepthRenderShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

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

CTextureToViewportShader::CTextureToViewportShader()
{
}

CTextureToViewportShader::~CTextureToViewportShader()
{
}

D3D12_DEPTH_STENCIL_DESC CTextureToViewportShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
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

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTextureToViewport", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTextureToViewportShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTextureToViewport", "ps_5_1", &m_pd3dPixelShaderBlob));
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

void CTextureToViewportShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

