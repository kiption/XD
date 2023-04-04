#include "stdafx.h"
#include "DynamicMappingShader.h"
#include "DynamicMappingObject.h"
#include "TerrainObject.h"

CDynamicCubeMappingShader::CDynamicCubeMappingShader(UINT nCubeMapSize)
{
	m_nCubeMapSize = nCubeMapSize;
}
CDynamicCubeMappingShader::~CDynamicCubeMappingShader()
{
}

D3D12_INPUT_LAYOUT_DESC CDynamicCubeMappingShader::CreateInputLayout(int PipelineState)
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CDynamicCubeMappingShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int PipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSCubeMapping", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CDynamicCubeMappingShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int PipelineState)
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSCubeMapping", "ps_5_1", ppd3dShaderBlob));
}

void CDynamicCubeMappingShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	m_pd3dCommandList->Close();

	m_nObjects = 6;
	m_ppObjects = new CGameObject * [m_nObjects];

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, m_nObjects);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ((sizeof(CB_DYNAMICOBJECT_INFO) + 255) & ~255));

	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorStartHandle = m_d3dCbvGPUDescriptorStartHandle;
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = m_nObjects;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dDescriptorHeapDesc.NumDescriptors = m_nObjects * 6;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	CMesh* pMeshIlluminated = new CSphereMeshIlluminated(pd3dDevice, pd3dCommandList, 80.0f, 20, 20);

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	XMFLOAT2 xmf2TerrainCenter = XMFLOAT2(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CDynamicCubeMappingObject(pd3dDevice, pd3dCommandList, m_nCubeMapSize, d3dDsvCPUDescriptorHandle, d3dRtvCPUDescriptorHandle, this);

		m_ppObjects[i]->SetMesh(0, pMeshIlluminated);
		m_ppObjects[i]->SetPosition(500.0, 1100.0, 500.0);
		SavePointx[i] = m_ppObjects[i]->m_xmf4x4Transform._41;
		SavePointz[i] = m_ppObjects[i]->m_xmf4x4Transform._43;
		m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(d3dCbvGPUDescriptorStartHandle.ptr);

		d3dCbvGPUDescriptorStartHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		d3dDsvCPUDescriptorHandle.ptr += ::gnDsvDescriptorIncrementSize;
		d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * 6);
	}
}
void CDynamicCubeMappingShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);

		XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0), XMConvertToRadians(0.5), XMConvertToRadians(0.0));
		m_ppObjects[j]->m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_ppObjects[j]->m_xmf4x4World);
		m_ppObjects[j]->m_xmf4x4World._41 = 500.0 * cos(theta) + SavePointx[j];
		m_ppObjects[j]->m_xmf4x4World._43 = 500.0 * sin(theta) + SavePointz[j];
		theta += fTimeElapsed / 30.0;
	}
}
void CDynamicCubeMappingShader::ReleaseObjects()
{
	CBaseObjectShader::ReleaseObjects();

	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
}

void CDynamicCubeMappingShader::ReleaseUploadBuffers()
{
	CBaseObjectShader::ReleaseUploadBuffers();
}

void CDynamicCubeMappingShader::OnPreRender(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, HANDLE hFenceEvent, SceneManager* pScene)
{
	for (int i = 0; i < m_nObjects; i++)
	{
		m_pd3dCommandAllocator->Reset();
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

		m_ppObjects[i]->OnPreRender(m_pd3dCommandList, pScene);

		m_pd3dCommandList->Close();

		ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
		pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		UINT64 nFenceValue = pd3dFence->GetCompletedValue();
		::WaitForGpuComplete(pd3dCommandQueue, pd3dFence, nFenceValue + 1, hFenceEvent);
	}
}

void CDynamicCubeMappingShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int PipelinState)
{
	CBaseObjectShader::Render(pd3dCommandList, pCamera, PipelinState);
}
