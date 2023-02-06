#include "stdafx.h"
#include "ObjectShader.h"
#include "HelicopterObejct.h"


CStandardShader::CStandardShader()
{
}

CStandardShader::~CStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", ppd3dShaderBlob));
}

void CStandardShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}



CBaseObjectShader::CBaseObjectShader()
{
}

CBaseObjectShader::~CBaseObjectShader()
{
}
void CBaseObjectShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_DYNAMICOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CBaseObjectShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_DYNAMICOBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		CB_DYNAMICOBJECT_INFO* pbMappedcbGameObject = (CB_DYNAMICOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
		if (m_ppObjects[j]->m_pMaterial) pbMappedcbGameObject->m_nMaterial = m_ppObjects[j]->m_pMaterial->m_nReflection;
	}
}

void CBaseObjectShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	CTexturedShader::ReleaseShaderVariables();
}

void CBaseObjectShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CBaseObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CBaseObjectShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CBaseObjectShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void CBaseObjectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CTexturedShader::Render(pd3dCommandList, pCamera, nPipelineState);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}


CObjectsShader::CObjectsShader()
{

}
CObjectsShader::~CObjectsShader()
{
}
D3D12_INPUT_LAYOUT_DESC CObjectsShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}
D3D12_SHADER_BYTECODE CObjectsShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", ppd3dShaderBlob));
}
void CObjectsShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}
D3D12_BLEND_DESC CObjectsShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_DEST_COLOR;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_SUBTRACT;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = 15;
	return(d3dBlendDesc);
}


void CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_nObjects = 10;
	m_ppObjects = new CGameObject * [m_nObjects];

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 40); //SuperCobra(17), Gunship(2)

	CGameObject* pSuperCobraModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GO.bin", this);
	CGameObject* pGunshipModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/GO.bin", this);
	int nObjects = 0;


	for (int i = 0; i < m_nObjects / 2; i++)
	{
		m_ppObjects[i] = new CGOObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pSuperCobraModel);
		m_ppObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppObjects[i]->SetScale(0.1f, 1.05f, 1.1f);

		m_ppObjects[i]->PrepareAnimate();
		pSuperCobraModel->AddRef();
	}

	for (int i = m_nObjects / 2; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CGOObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pGunshipModel);
		m_ppObjects[i]->Rotate(0.0f, 90.0f, 0.0f);
		m_ppObjects[i]->SetScale(0.1, 1.05f, 1.1f);
		m_ppObjects[i]->PrepareAnimate();
		pGunshipModel->AddRef();
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
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CShader::Render(pd3dCommandList, pCamera, nPipelineState);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(0.16f);
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}


CSpriteObjectShader::CSpriteObjectShader()
{
}

CSpriteObjectShader::~CSpriteObjectShader()
{
}

void CSpriteObjectShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	CShader::CreateGraphicsPipelineState(pd3dDevice, pd3dGraphicsRootSignature, 0);
}

void CSpriteObjectShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_SPRITEBILLBOARD_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL,ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CSpriteObjectShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//UINT ncbElementBytes = ((sizeof(CB_SPRITEBILLBOARD_INFO) + 255) & ~255);
	//for (int j = 0; j < m_nObjects; j++)
	//{
	//	CB_SPRITEBILLBOARD_INFO* pbMappedcbGameObject = (CB_SPRITEBILLBOARD_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
	//	XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
	//	if (m_ppObjects[j]->m_pMaterial && m_ppObjects[j]->m_pMaterial->m_pTexture)
	//	{
	//		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4Texture, XMMatrixTranspose(XMLoadFloat4x4(&(m_ppObjects[j]->m_pMaterial->m_pTexture->m_xmf4x4Texture))));
	//	}
	//}

	//UINT ncbElementBytes = ((sizeof(CB_SPRITEBILLBOARD_INFO) + 255) & ~255);
	//for (int j = 0; j < m_nObjects; j++)
	//{
	//	
	//		CB_SPRITEBILLBOARD_INFO* pbMappedcbGameObject = (CB_SPRITEBILLBOARD_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
	//		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
	//		if (m_ppObjects[j]->m_ppMaterials)
	//		{
	//			if (m_ppObjects[j]->m_ppMaterials[0] && m_ppObjects[j]->m_ppMaterials[0]->m_pTexture)//오류
	//			{
	//				XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4Texture, 
	//					XMMatrixTranspose(XMLoadFloat4x4(&(m_ppObjects[j]->m_ppMaterials[0]->m_pTexture->m_xmf4x4Texture))));
	//			}
	//		}
	//	
	//}
}

void CSpriteObjectShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	CTexturedShader::ReleaseShaderVariables();
}

void CSpriteObjectShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
}

void CSpriteObjectShader::ReleaseObjects()
{
	/*if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}*/

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) delete m_pMaterial;
#endif
}

void CSpriteObjectShader::AnimateObjects(float fTimeElapsed)
{
	/*for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}*/
}

void CSpriteObjectShader::ReleaseUploadBuffers()
{
	/*if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}*/
}

void CSpriteObjectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CTexturedShader::Render(pd3dCommandList, pCamera);
	//CTexturedShader::Render(pd3dCommandList, pCamera, nPipelineState);
	/*for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);

		}
	}*/
}
