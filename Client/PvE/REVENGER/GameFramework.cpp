//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
#ifdef _WITH_DIRECT2D
		m_ppd3d11WrappedBackBuffers[i] = NULL;
		m_ppd2dRenderTargets[i] = NULL;
#endif

	}

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("REVENGER :"));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	m_nMode = SCENE1STAGE;
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);
#ifdef _WITH_DIRECT2D
	CreateDirect2DDevice();
#endif
	BuildObjects();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}
	if (!m_pd3dDevice)
	{
		hResult = m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}
	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_d3dScissorRect = { 0, 0, m_nWndClientWidth, m_nWndClientHeight };
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		case '1':
			ChangeScene(SCENE1STAGE);
			break;
		case '2':
			ChangeScene(SCENE2STAGE);
			break;
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:/*
			if (m_nMode == SCENE2STAGE)((CHumanPlayer*)m_pPlayer)->FireBullet(NULL);
			if (m_nMode == SCENE1STAGE)((CAirplanePlayer*)m_pPlayer)->Firevalkan(NULL);*/
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}



void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::BuildObjects()
{

	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	
	
	m_pScene = new SceneManager();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain);
		
	m_pScene->m_pPlayer = m_pPlayer = pPlayer;
	m_pCamera = m_pPlayer->GetCamera();
	m_pScene->m_pPlayer->SetPosition(XMFLOAT3(500.0,200.0,500.0));
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
	gamesound.backGroundMusic();
	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{

	if (m_pPlayer) m_pPlayer->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

// key value
constexpr char INPUT_SPACEBAR = 0b1000000;
constexpr char INPUT_KEY_W = 0b0100000;
constexpr char INPUT_KEY_S = 0b0010000;
constexpr char INPUT_KEY_D = 0b0001000;
constexpr char INPUT_KEY_A = 0b0000100;
constexpr char INPUT_KEY_E = 0b0000010;
constexpr char INPUT_KEY_Q = 0b0000001;

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		char inputKeyValue = 0;//S
		DWORD dwDirection = 0;

		if (pKeysBuffer[KEY_W] & 0xF0) {
			dwDirection |= DIR_FORWARD;
			if (m_nMode == SCENE2STAGE)
			{
			}
			if (m_nMode != SCENE2STAGE)
			{
				inputKeyValue += INPUT_KEY_W;//S
				m_pCamera->SetTimeLag(0.02);
			}
		}
		if (pKeysBuffer[KEY_S] & 0xF0) {
			dwDirection |= DIR_BACKWARD;
			if (m_nMode == SCENE2STAGE)
			{
			}
			if (m_nMode != SCENE2STAGE)
			{
				inputKeyValue += INPUT_KEY_S;//S
				m_pCamera->SetTimeLag(0.02);
			}
		}
		if (pKeysBuffer[KEY_D] & 0xF0) {
				
			if (m_nMode == SCENE2STAGE)
			{
			}
			dwDirection |= DIR_RIGHT;
			if (m_nMode != SCENE2STAGE)
			{
				inputKeyValue += INPUT_KEY_D;//S
				m_pCamera->SetTimeLag(0.05);
			}
		}
		if (pKeysBuffer[KEY_A] & 0xF0) {
			if (m_nMode == SCENE2STAGE)
			{
			}
			dwDirection |= DIR_LEFT;
			
			inputKeyValue += INPUT_KEY_A;//S
			m_pCamera->SetTimeLag(0.05);
			
		}

		if (pKeysBuffer[KEY_Q] & 0xF0) {
			inputKeyValue += INPUT_KEY_Q;//S
			dwDirection |= DIR_UP;
			m_pCamera->SetTimeLag(0.05);
		}
		if (pKeysBuffer[KEY_E] & 0xF0) {
			inputKeyValue += INPUT_KEY_E;//S
			dwDirection |= DIR_DOWN;
			m_pCamera->SetTimeLag(0.05);
		}

		if (pKeysBuffer[VK_SPACE] & 0xF0) {
			inputKeyValue += INPUT_SPACEBAR;//S
			
		}

		// Server
		if (inputKeyValue != 0) {
			q_keyboardInput.push(inputKeyValue);
		}//

		float cxDelta = 0.0f, cyDelta = 0.0f, czDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 2.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 2.0f;
			czDelta = ((float)(ptCursorPos.y - m_ptOldCursorPos.y) + (float)(ptCursorPos.x - m_ptOldCursorPos.x)) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				// Server
				MouseInputVal inputMouseValue;

				if (pKeysBuffer[VK_LBUTTON] & 0xF0)
					inputMouseValue.button = L_BUTTON;
				else if (pKeysBuffer[VK_RBUTTON] & 0xF0)
					inputMouseValue.button = R_BUTTON;

				inputMouseValue.delX = cxDelta;
				inputMouseValue.delY = cyDelta;

				q_mouseInput.push(inputMouseValue);
				//====
				if (m_nMode==SCENE2STAGE)
				{
					if (pKeysBuffer[VK_RBUTTON] & 0xF0)
						m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
					else
						m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
				}
			}
			if (m_nMode == SCENE2STAGE)
				if (dwDirection) m_pPlayer->Move(dwDirection, 12.25f, true);

		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);

	m_pPlayer->Animate(fTimeElapsed);
	m_pPlayer->Animate(fTimeElapsed,NULL);
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP
#define _WITH_PLAYER_TOP
float g_time = 0.0f;
float g_reverse_time = 0.0f;
void CGameFramework::FrameAdvance()
{
	SleepEx(1, TRUE);//Server
	m_GameTimer.Tick(30.0f);

	ProcessInput();

	AnimateObjects();

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_DIRECT2D
	//Direct2D Drawing
	m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
	ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
	m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

	m_pd2dDeviceContext->BeginDraw();

	m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	D2D_POINT_2F d2dPoint = { 0.0f, 0.0f };
	D2D_RECT_F d2dRect = { 100.0f, 100.0f, 250.0f, 250.0f };
	m_pd2dDeviceContext->DrawImage((m_nDrawEffectImage == 0) ? m_pd2dfxGaussianBlur : m_pd2dfxGaussianBlur, &d2dPoint, &d2dRect);
#endif
	D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();
	D2D1_RECT_F rcUpperText = D2D1::RectF(0, 0, szRenderTarget.width, szRenderTarget.height * 0.25f);
	m_pd2dDeviceContext->DrawTextW(L"Locking...", (UINT32)wcslen(L"Locking..."), m_pdwFont, &rcUpperText, m_pd2dbrText);

	D2D1_RECT_F rcLowerText = D2D1::RectF(0, szRenderTarget.height * 0.8f, szRenderTarget.width, szRenderTarget.height);
	m_pd2dDeviceContext->DrawTextW(L" ", (UINT32)wcslen(L" "), m_pdwFont, &rcLowerText, m_pd2dbrText);

	D2D1_RECT_F rcBulletText = D2D1::RectF(750, 750, szRenderTarget.width, szRenderTarget.height * 0.5f);
	m_pd2dDeviceContext->DrawTextW(m_myBullet, (UINT32)wcslen(m_myBullet), m_pdwFont, &rcBulletText, m_pd2dbrText);

	D2D1_RECT_F rcMaxBulletText = D2D1::RectF(850, 750, szRenderTarget.width, szRenderTarget.height * 0.5f);
	m_pd2dDeviceContext->DrawTextW(L"/100", (UINT32)wcslen(L"/100"), m_pdwFont, &rcMaxBulletText, m_pd2dbrText);

	m_pd2dDeviceContext->EndDraw();

	m_pd3d11On12Device->ReleaseWrappedResources(ppd3dResources, _countof(ppd3dResources));

	m_pd3d11DeviceContext->Flush();
#endif
#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::ChangeScene(DWORD nMode)
{
	if (nMode != m_nMode)
	{
		ReleaseObjects();
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
		switch (nMode)
		{
		case SCENE1STAGE:
		{
			m_nMode = nMode;

			m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
			m_pScene = new Stage1();
			if (m_pScene) ((Stage1 *)m_pScene)->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
			CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), ((Stage1*)m_pScene)->m_pTerrain);
			m_pScene->m_pPlayer = m_pPlayer = pPlayer;
			m_pCamera = m_pPlayer->GetCamera();
			m_pScene->SetCurScene(SCENE1STAGE);
			break;
		}
		case SCENE2STAGE:
		{
			m_nMode = nMode;
			gamesound.m_bStopSound = true;
			m_pScene = new Stage2();
			if (m_pScene) ((Stage2*)m_pScene)->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
			CHumanPlayer* pPlayer = new CHumanPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), ((Stage2*)m_pScene)->m_pTerrain);
			m_pScene->m_pPlayer = m_pPlayer = pPlayer;
			m_pCamera = m_pPlayer->GetCamera();
			m_pScene->SetCurScene(SCENE2STAGE);
			break;
		}
		}
	}
}

#ifdef _WITH_DIRECT2D
void CGameFramework::CreateDirect2DDevice()
{
	UINT nD3D11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG) || defined(DBG)
	nD3D11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device* pd3d11Device = NULL;
	ID3D12CommandQueue* ppd3dCommandQueues[] = { m_pd3dCommandQueue };
	HRESULT hResult = ::D3D11On12CreateDevice(m_pd3dDevice, nD3D11DeviceFlags, NULL, 0, reinterpret_cast<IUnknown**>(ppd3dCommandQueues), _countof(ppd3dCommandQueues), 0, &pd3d11Device, &m_pd3d11DeviceContext, NULL);
	hResult = pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&m_pd3d11On12Device);
	if (pd3d11Device) pd3d11Device->Release();

	D2D1_FACTORY_OPTIONS nD2DFactoryOptions = { D2D1_DEBUG_LEVEL_NONE };
#if defined(_DEBUG) || defined(DBG)
	nD2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	ID3D12InfoQueue* pd3dInfoQueue = NULL;
	if (SUCCEEDED(m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue))))
	{
		D3D12_MESSAGE_SEVERITY pd3dSeverities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO,
		};

		D3D12_MESSAGE_ID pd3dDenyIds[] =
		{
			D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
		};

		D3D12_INFO_QUEUE_FILTER d3dInforQueueFilter = { };
		d3dInforQueueFilter.DenyList.NumSeverities = _countof(pd3dSeverities);
		d3dInforQueueFilter.DenyList.pSeverityList = pd3dSeverities;
		d3dInforQueueFilter.DenyList.NumIDs = _countof(pd3dDenyIds);
		d3dInforQueueFilter.DenyList.pIDList = pd3dDenyIds;

		pd3dInfoQueue->PushStorageFilter(&d3dInforQueueFilter);
	}
	pd3dInfoQueue->Release();
#endif

	hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &nD2DFactoryOptions, (void**)&m_pd2dFactory);

	IDXGIDevice* pdxgiDevice = NULL;
	hResult = m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pdxgiDevice);
	hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, &m_pd2dDevice);
	hResult = m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
	hResult = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_pdWriteFactory);
	if (pdxgiDevice) pdxgiDevice->Release();

	m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.0f, 0.0f, 0.5f), &m_pd2dbrBackground);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(0x9ACD32, 1.0f)), &m_pd2dbrBorder);

	hResult = m_pdWriteFactory->CreateTextFormat(L"텍스트 레이아웃", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"en-US", &m_pdwFont);
	hResult = m_pdwFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	hResult = m_pdwFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue, 0.8f), &m_pd2dbrText);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont, 1024, 1024, &m_pdwTextLayout);

	float fDpi = (float)GetDpiForWindow(m_hWnd);
	D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fDpi, fDpi);

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		m_pd3d11On12Device->CreateWrappedResource(m_ppd3dSwapChainBackBuffers[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_ppd3d11WrappedBackBuffers[i]));
		IDXGISurface* pdxgiSurface = NULL;
		m_ppd3d11WrappedBackBuffers[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&pdxgiSurface);
		m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_ppd2dRenderTargets[i]);
		if (pdxgiSurface) pdxgiSurface->Release();
	}

#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	CoInitialize(NULL);
	hResult = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&m_pwicImagingFactory);

	hResult = m_pd2dFactory->CreateDrawingStateBlock(&m_pd2dsbDrawingState);
	hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource);
	hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur);
	hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxEdgeDetection);

	IWICBitmapDecoder* pwicBitmapDecoder;
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/MiniMap.jpg", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	IWICBitmapFrameDecode* pwicFrameDecode;
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur->SetInputEffect(0, m_pd2dfxBitmapSource);
	m_pd2dfxGaussianBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection->SetInputEffect(0, m_pd2dfxBitmapSource);
	m_pd2dfxEdgeDetection->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
	if (pwicFrameDecode) pwicFrameDecode->Release();
#endif
}
#endif




//==================================================
//			    Function for Networking
//==================================================
bool CGameFramework::CheckNewInputExist_Keyboard() {
	return q_keyboardInput.empty();
}
bool CGameFramework::CheckNewInputExist_Mouse() {
	return q_mouseInput.empty();
}

short CGameFramework::PopInputVal_Keyboard() {
	short val = q_keyboardInput.front();
	q_keyboardInput.pop();

	return val;
}
MouseInputVal CGameFramework::PopInputVal_Mouse() {
	MouseInputVal val = q_mouseInput.front();
	q_mouseInput.pop();

	return val;
}

void CGameFramework::SetPosition_PlayerObj(XMFLOAT3 pos) {
	m_pPlayer->m_xmf3Position = pos;
}
void CGameFramework::SetVectors_PlayerObj(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec) {
	m_pPlayer->SetVectorsByServer(rightVec, upVec, lookVec);
}

void CGameFramework::SetPosition_OtherPlayerObj(int id, XMFLOAT3 pos) {

	m_pScene->m_ppShaders[0]->m_ppObjects[id]->m_xmf4x4ToParent._41 = pos.x;
	m_pScene->m_ppShaders[0]->m_ppObjects[id]->m_xmf4x4ToParent._42 = pos.y;
	m_pScene->m_ppShaders[0]->m_ppObjects[id]->m_xmf4x4ToParent._43 = pos.z;
	if (m_nMode == SCENE2STAGE)
	{
		((Stage2*)m_pScene)->m_ppShaders[0]->m_ppObjects[id]->m_xmf4x4ToParent._41 = pos.x;
		((Stage2*)m_pScene)->m_ppShaders[0]->m_ppObjects[id]->m_xmf4x4ToParent._42 = pos.y;
		((Stage2*)m_pScene)->m_ppShaders[0]->m_ppObjects[id]->m_xmf4x4ToParent._43 = pos.z;
	}
}
void CGameFramework::SetVectors_OtherPlayerObj(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec) {
	m_pScene->m_ppShaders[0]->m_ppObjects[id]->SetUp(upVec);
	m_pScene->m_ppShaders[0]->m_ppObjects[id]->SetRight(rightVec);
	m_pScene->m_ppShaders[0]->m_ppObjects[id]->SetLook(lookVec);
	if (m_nMode == SCENE2STAGE)
	{
		((Stage2*)m_pScene)->m_ppShaders[0]->m_ppObjects[id]->SetUp(upVec);
		((Stage2*)m_pScene)->m_ppShaders[0]->m_ppObjects[id]->SetRight(rightVec);
		((Stage2*)m_pScene)->m_ppShaders[0]->m_ppObjects[id]->SetLook(lookVec);
	}
}
void CGameFramework::Remove_OtherPlayerObj(int id) {
	if (m_pScene->m_ppShaders[0]->m_ppObjects[id] ) {
		m_pScene->m_ppShaders[0]->m_ppObjects[id]->SetScale(0.0, 0.0, 0.0);
	}
}

void CGameFramework::Create_Bullet(int id, XMFLOAT3 pos, XMFLOAT3 xmf3look)
{
	
}

void CGameFramework::SetPosition_Bullet(int id, XMFLOAT3 pos, XMFLOAT3 xmf3right, XMFLOAT3 xmf3up, XMFLOAT3 xmf3look)
{
	m_pScene->m_ppBullets[id]->SetPosition(pos);
	m_pScene->m_ppBullets[id]->SetRight(xmf3right);
	m_pScene->m_ppBullets[id]->SetUp(xmf3up);
	m_pScene->m_ppBullets[id]->SetLook(xmf3look);
	
}

void CGameFramework::SetPosition_NPC(int id, XMFLOAT3 pos)
{
}

//void CGameFramework::SetVectors_NPC(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec)
//{
//}
