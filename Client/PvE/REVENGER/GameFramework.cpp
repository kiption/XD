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

	gnRtvDescriptorIncrementSize = 0;
	gnDsvDescriptorIncrementSize = 0;

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

	loginpos[0].sx = 460.0f;
	loginpos[0].sy = 660.0f;
	loginpos[0].lx = 695.0f;
	loginpos[0].ly = 735.0f;

	loginpos[1].sx = 460.0f;
	loginpos[1].sy = 752.0;
	loginpos[1].lx = 695.0f;
	loginpos[1].ly = 820.0f;

	loginpos[2].sx = 355.0f;
	loginpos[2].sy = 845.0f;
	loginpos[2].lx = 520.0f;
	loginpos[2].ly = 915.0f;

	loginpos[3].sx = 540.0f;
	loginpos[3].sy = 845.0f;
	loginpos[3].lx = 705.0f;
	loginpos[3].ly = 915.0f;

	gamepos[0].sx = 107.0f;
	gamepos[0].sy = 705.0f;
	gamepos[0].lx = 280.0f;
	gamepos[0].ly = 775.0f;

	gamepos[1].sx = 107.0f;
	gamepos[1].sy = 840.0f;
	gamepos[1].lx = 280.0f;
	gamepos[1].ly = 910.0f;

	lobbypos[0].sx = 1015.0f;
	lobbypos[0].sy = 245.0f;
	lobbypos[0].lx = 1275.0f;
	lobbypos[0].ly = 350.0f;

	lobbypos[1].sx = 1315.0f;
	lobbypos[1].sy = 245.0f;
	lobbypos[1].lx = 1580.0f;
	lobbypos[1].ly = 350.0f;

	lobbypos[2].sx = 1565.0f;
	lobbypos[2].sy = 190.0f;
	lobbypos[2].lx = 1605.0f;
	lobbypos[2].ly = 230.0f;

	lobbypos[3].sx = 885.0f;
	lobbypos[3].sy = 895.0f;
	lobbypos[3].lx = 905.0f;
	lobbypos[3].ly = 920.0f;

	lobbypos[4].sx = 1010.0f;
	lobbypos[4].sy = 895.0f;
	lobbypos[4].lx = 1025.0f;
	lobbypos[4].ly = 920.0f;

	roompos[0].sx = 975.0f;
	roompos[0].sy = 390.0f;
	roompos[0].lx = 1225.0f;
	roompos[0].ly = 485.0f;

	roompos[1].sx = 1295.0f;
	roompos[1].sy = 390.0f;
	roompos[1].lx = 1545.0f;
	roompos[1].ly = 485.0f;

	roompos[2].sx = 1560.0f;
	roompos[2].sy = 350.0f;
	roompos[2].lx = 1605.0f;
	roompos[2].ly = 390.0f;

	createpos[0].sx = 1200.0f;
	createpos[0].sy = 640.0f;
	createpos[0].lx = 1335.0f;
	createpos[0].ly = 700.0f;

	createpos[1].sx = 1355.0f;
	createpos[1].sy = 640.0f;
	createpos[1].lx = 1490.0f;
	createpos[1].ly = 700.0f;

	choicejob[0].sx = 1155.0f;
	choicejob[0].sy = 595.0f;
	choicejob[0].lx = 1229.9f;
	choicejob[0].ly = 645.0f;

	choicejob[1].sx = 1230.0f;
	choicejob[1].sy = 595.0f;
	choicejob[1].lx = 1305.0f;
	choicejob[1].ly = 645.0f;

	choicejob[2].sx = 1155.0f;
	choicejob[2].sy = 650.0f;
	choicejob[2].lx = 1229.9f;
	choicejob[2].ly = 700.0f;

	choicejob[3].sx = 1230.0f;
	choicejob[3].sy = 650.0f;
	choicejob[3].lx = 1305.0f;
	choicejob[3].ly = 700.0f;

	choicejob[4].sx = 1155.0f;
	choicejob[4].sy = 710.0f;
	choicejob[4].lx = 1229.9f;
	choicejob[4].ly = 760.0f;

	choicejob[5].sx = 1230.0f;
	choicejob[5].sy = 710.0f;
	choicejob[5].lx = 1305.0f;
	choicejob[5].ly = 760.0f;
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	m_nMode = OPENINGSCENE;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CoInitialize(NULL);

#ifdef _WITH_DIRECT2D
	CreateDirect2DDevice();
#endif
	CreateDepthStencilView();
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
	if (!m_pdxgiSwapChain)
	{
		MessageBox(NULL, L"Swap Chain Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

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
		m_pd3dSwapChainBackBufferRTVCPUHandles[i] = d3dRtvCPUDescriptorHandle;
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

	//m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);
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
	// key Delay
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
		if (m_nMode == SCENE1STAGE) {
			if (m_ingame_role == R_RIFLE) {
				m_SniperOn = true;
				((CHumanPlayer*)m_pScene->m_pPlayer)->m_bZoomMode = true;
				m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 40.0f);
			}
			if (m_ingame_role == R_HELI) {
				m_pCamera->m_bHelicopterFreedom = true;
			}
		}

		break;
		//::ReleaseCapture();
	case WM_RBUTTONUP:
		if (m_nMode == SCENE1STAGE) {
			if (m_ingame_role == R_RIFLE) {
				m_SniperOn = false;
				((CHumanPlayer*)m_pScene->m_pPlayer)->m_bZoomMode = false;
				m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			}
			if (m_ingame_role == R_HELI) {
				m_pCamera->m_bHelicopterFreedom = false;
			}
		}

		break;

	case WM_LBUTTONUP:
		break;
	case WM_LBUTTONDOWN:
	{
		::GetCursorPos(&m_ptOldCursorPos);
		if (m_nMode == OPENINGSCENE) {
			cout << "x: " << m_ptOldCursorPos.x << ", y: " << m_ptOldCursorPos.y << endl;
			switch (m_LoginScene)
			{
			case LS_LOGIN: // 로그인 클릭 창
				if (loginpos[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < loginpos[0].lx && loginpos[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < loginpos[0].ly) {
					memset(m_LoginClick, 0, sizeof(m_LoginClick));
					m_LoginClick[0] = true; // ID 입력 활성화
				}
				else if (loginpos[1].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < loginpos[1].lx && loginpos[1].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < loginpos[1].ly) {
					memset(m_LoginClick, 0, sizeof(m_LoginClick));
					m_LoginClick[1] = true; // PW 입력 활성화
				}
				else if (loginpos[2].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < loginpos[2].lx && loginpos[2].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < loginpos[2].ly) {
					memset(m_LoginClick, 0, sizeof(m_LoginClick));
					m_LoginClick[2] = true; // Regist
				}
				else if (loginpos[3].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < loginpos[3].lx && loginpos[3].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < loginpos[3].ly) {
					memset(m_LoginClick, 0, sizeof(m_LoginClick));
					m_LoginClick[3] = true;

					m_bLoginInfoSend = true;

					m_LoginScene = LS_OPENING; // Login 클릭, 다음 UI 전환
				}
				else {
					memset(m_LoginClick, 0, sizeof(m_LoginClick));
				}
				break;
			case LS_OPENING: // 게임 시작, 설정, 종료 
				if (gamepos[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < gamepos[0].lx && gamepos[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < gamepos[0].ly) {
					memset(m_GameClick, 0, sizeof(m_GameClick));
					m_GameClick[0] = true;
					m_LoginScene = LS_LOBBY; // 게임 시작 클릭
				}
				else if (gamepos[1].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < gamepos[1].lx && gamepos[1].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < gamepos[1].ly) {
					memset(m_GameClick, 0, sizeof(m_GameClick));
					m_GameClick[1] = true; // 종료 클릭 --> 종료되는 프로그램 넣어야 함.
				}
				else {
					memset(m_GameClick, 0, sizeof(m_GameClick));
				}
				break;
			case LS_LOBBY: // 로비 UI
				if (lobbypos[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < lobbypos[0].lx && lobbypos[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < lobbypos[0].ly) {
					m_LobbyClick[0] = true;		// '빠른시작' 서버전송용 트리거
				}
				else if (lobbypos[1].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < lobbypos[1].lx && lobbypos[1].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < lobbypos[1].ly) {
					random_device rd;
					default_random_engine dre(rd());
					uniform_int_distribution <int> uid(1, 3);

					int ran = uid(dre);
					switch (ran)
					{
					case 1:
						createRoomName = RoomnameList.str1;
						break;
					case 2:
						createRoomName = RoomnameList.str2;
						break;
					case 3:
						createRoomName = RoomnameList.str3;
						break;
					}

					m_LoginScene = LS_CREATE_ROOM;	// 방 생성 UI를 띄운다.
				}
				else if (lobbypos[2].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < lobbypos[2].lx && lobbypos[2].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < lobbypos[2].ly) {
					m_LoginScene = LS_OPENING; // 뒤로가기 누름
				}
				else if (lobbypos[3].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < lobbypos[3].lx && lobbypos[3].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < lobbypos[3].ly) {
					memset(m_LobbyClick, 0, sizeof(m_LobbyClick)); // Left Button
					//m_RoomBackButton = true;
					// page--;
				}
				else if (lobbypos[4].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < lobbypos[4].lx && lobbypos[4].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < lobbypos[4].ly) {
					memset(m_LobbyClick, 0, sizeof(m_LobbyClick)); // Right Button
					//m_RoomBackButton = true;
					// page++;
				}
				else {
					memset(m_LobbyClick, 0, sizeof(m_LobbyClick));
				}
				break;
			case LS_ROOM: // 방 내부
				if (roompos[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < roompos[0].lx && roompos[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < roompos[0].ly) {
					m_RoomClick[0] = true;//Start
				}
				else if (roompos[1].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < roompos[1].lx && roompos[1].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < roompos[1].ly) {
					m_RoomClick[1] = true; // Ready
				}
				else if (roompos[2].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < roompos[2].lx && roompos[2].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < roompos[2].ly) {
					memset(m_RoomClick, 0, sizeof(m_RoomClick)); // Back
					m_RoomBackButton = true;
				}
				else if (choicejob[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[1].lx && choicejob[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[5].ly) {
					switch (m_roominMyId)
					{
					case 0:
						if (choicejob[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[0].lx\
							&& choicejob[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[0].ly) {
							// 역핧변경 (사람)
							role_change_h2a_click = true;
						}
						else if (choicejob[1].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[1].lx\
							&& choicejob[1].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[1].ly) {
							// 역핧변경 (헬기)
							role_change_a2h_click = true;
						}
						break;
					case 1:
						if (choicejob[2].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[2].lx\
							&& choicejob[2].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[2].ly) {
							// 헬기 -> 사람
							role_change_h2a_click = true;
						}
						else if (choicejob[3].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[3].lx\
							&& choicejob[3].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[3].ly) {
							// 사람 -> 헬기
							role_change_a2h_click = true;
						}
						break;
					case 2:
						if (choicejob[4].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[4].lx\
							&& choicejob[4].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[4].ly) {
							// 헬기 -> 사람
							role_change_h2a_click = true;
						}
						else if (choicejob[5].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < choicejob[5].lx\
							&& choicejob[5].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < choicejob[5].ly) {
							// 사람 -> 헬기
							role_change_a2h_click = true;
						}
						break;
					default:
						break;
					}
				}
				else {
					memset(m_RoomClick, 0, sizeof(m_RoomClick));
				}
				break;
			case LS_CREATE_ROOM:
				if (createpos[0].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < createpos[0].lx && createpos[0].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < createpos[0].ly) {
					m_CreateRoomOkButton = true;
				}
				else if (createpos[1].sx < m_ptOldCursorPos.x && m_ptOldCursorPos.x < createpos[1].lx && createpos[1].sy < m_ptOldCursorPos.y && m_ptOldCursorPos.y < createpos[1].ly) {
					m_LoginScene = LS_LOBBY; // 취소 누름

				}
				break;
			}

		}
	}
	if (m_nMode == SCENE1STAGE) ::SetCapture(hWnd);
	/*::GetCursorPos(&m_ptOldCursorPos);*/
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
	if (m_LoginScene == 0) {
		switch (nMessageID)
		{
		case WM_CHAR:
		{
			if (m_LoginClick[0]) {
				WCHAR IDchar = static_cast<WCHAR>(wParam);
				if ((IDchar >= L'A' && IDchar <= L'Z') || (IDchar >= L'a' && IDchar <= L'z') ||
					(IDchar >= L'0' && IDchar <= L'9') || (IDchar >= L'!' && IDchar <= L'/') || (IDchar >= L':' && IDchar <= L'@') ||
					(IDchar >= L'[' && IDchar <= L'`') || (IDchar >= L'{' && IDchar <= L'~'))
				{
					size_t IDLength = wcslen(m_LoginID);
					size_t remainingSpace = sizeof(m_LoginID) / sizeof(m_LoginID[0]) - IDLength - 1;


					if (remainingSpace > 0) {
						wcsncat_s(m_LoginID, sizeof(m_LoginID) / sizeof(m_LoginID[0]), &IDchar, remainingSpace);
						m_LoginID[++IDLength] = L'\0';  // 널 종료 문자 위치 업데이트
					}
				}
			}
			else if (m_LoginClick[1]) {
				WCHAR PWchar = static_cast<WCHAR>(wParam);
				if ((PWchar >= L'A' && PWchar <= L'Z') || (PWchar >= L'a' && PWchar <= L'z') ||
					(PWchar >= L'0' && PWchar <= L'9') || (PWchar >= L'!' && PWchar <= L'/') || (PWchar >= L':' && PWchar <= L'@') ||
					(PWchar >= L'[' && PWchar <= L'`') || (PWchar >= L'{' && PWchar <= L'~'))
				{
					size_t PWLength = wcslen(m_LoginPW);
					size_t remainingSpace = sizeof(m_LoginPW) / sizeof(m_LoginPW[0]) - PWLength - 1;
					if (remainingSpace > 0) {
						wcsncat_s(m_LoginPW, sizeof(m_LoginPW) / sizeof(m_LoginPW[0]), &PWchar, remainingSpace);
						m_LoginPW[++PWLength] = L'\0';  // 널 종료 문자 위치 업데이트
					}
				}
			}
			break;
		}
		case WM_KEYUP:
		{
			if (m_LoginClick[0]) {
				if (wParam == VK_BACK) {
					size_t pwLength = wcslen(m_LoginID);
					if (pwLength > 0) {
						m_LoginID[pwLength - 1] = L'\0';
					}
				}
			}
			else if (m_LoginClick[1]) {
				if (wParam == VK_BACK) {
					size_t pwLength = wcslen(m_LoginPW);
					if (pwLength > 0) {
						m_LoginPW[pwLength - 1] = L'\0';
					}
				}
			}
			break;
		}
		}
	}
	else if (UI_Switch) {
		switch (nMessageID) {
		case WM_IME_COMPOSITION:
		{
			WCHAR Chatchar = static_cast<WCHAR>(wParam);//어쨌든 뭔가가 조합된다
			if (lParam == GCS_RESULTSTR) {//얘는 확신
				size_t completeChatLength = wcslen(m_CompleteChat);//아이디 길이
				size_t completeremainingSpace = sizeof(m_CompleteChat) / sizeof(m_CompleteChat[0]) - completeChatLength - 1;//남은 문자					
				if (completeremainingSpace > 0) {
					wcsncat_s(m_CompleteChat, sizeof(m_CompleteChat) / sizeof(m_CompleteChat[0]), &Chatchar, completeremainingSpace);
					m_CompleteChat[++completeChatLength] = L'\0';  // 널 종료 문자 위치 업데이트					
					wcscpy_s(m_InsertChat, sizeof(m_InsertChat) / sizeof(m_InsertChat[0]), m_CompleteChat);
				}
				isComplete = true;
			}
			else {
				size_t ChatLength = wcslen(m_InsertChat);//아이디 길이
				size_t remainingSpace = sizeof(m_InsertChat) / sizeof(m_InsertChat[0]) - ChatLength - 1;//남은 문자

				if (remainingSpace > 0) {
					if (!isComplete && ChatLength > 0) {
						m_InsertChat[ChatLength - 1] = L'\0';
						ChatLength -= 1;
					}
					wcsncat_s(m_InsertChat, sizeof(m_InsertChat) / sizeof(m_InsertChat[0]), &Chatchar, remainingSpace);
					m_InsertChat[++ChatLength] = L'\0';  // 널 종료 문자 위치 업데이트
				}
				isComplete = false;
			}
			break;
		}
		case WM_CHAR:
		{
			WCHAR Chatchar = static_cast<WCHAR>(wParam);

			// 영어 대문자, 영어 소문자, 숫자를 추가합니다.
			if ((Chatchar >= L'A' && Chatchar <= L'Z') || (Chatchar >= L'a' && Chatchar <= L'z') ||
				(Chatchar >= L'0' && Chatchar <= L'9') || (Chatchar >= L'!' && Chatchar <= L'/') || (Chatchar >= L':' && Chatchar <= L'@') ||
				(Chatchar >= L'[' && Chatchar <= L'`') || (Chatchar >= L'{' && Chatchar <= L'~'))
			{
				size_t ChatLength = wcslen(m_InsertChat);
				size_t remainingSpace = sizeof(m_InsertChat) / sizeof(m_InsertChat[0]) - ChatLength - 1;

				size_t ChatCompleteLength = wcslen(m_CompleteChat);
				size_t remainingCompleteSpace = sizeof(m_CompleteChat) / sizeof(m_CompleteChat[0]) - ChatCompleteLength - 1;

				if (remainingSpace > 0) {
					wcsncat_s(m_InsertChat, sizeof(m_InsertChat) / sizeof(m_InsertChat[0]), &Chatchar, remainingSpace);
					m_InsertChat[++ChatLength] = L'\0';  // 널 종료 문자 위치 업데이트
				}


				if (remainingCompleteSpace > 0) {
					wcsncat_s(m_CompleteChat, sizeof(m_CompleteChat) / sizeof(m_CompleteChat[0]), &Chatchar, remainingCompleteSpace);
					m_CompleteChat[++ChatCompleteLength] = L'\0';  // 널 종료 문자 위치 업데이트
				}
			}
			break;
		}
		case WM_KEYUP:
			if (wParam == VK_BACK) {
				size_t ChatLength = wcslen(m_InsertChat);
				size_t CompleteLength = wcslen(m_CompleteChat);

				if (ChatLength > 0) {
					m_InsertChat[ChatLength - 1] = L'\0';
				}
				if (CompleteLength > 0) {
					m_CompleteChat[CompleteLength - 1] = L'\0';
				}
			}
			else if (wParam == VK_ESCAPE) {
				UI_Switch = false;
				m_InsertChat[0] = L'\0';
				m_CompleteChat[0] = L'\0';
			}
			else if (wParam == VK_RETURN) {
				SendChat temp;
				wcstombs_s(nullptr, temp.chatData, sizeof(temp.chatData), m_InsertChat, sizeof(m_InsertChat));

				if (temp.chatData == nullptr || temp.chatData[0] == '\0') {
					cout << "입력된 값이 없습니다." << endl;
				}
				else {
					m_mychat_log.push(temp);
					m_InsertChat[0] = L'\0';
					m_CompleteChat[0] = L'\0';
				}
			}
			else if (wParam == VK_SPACE) {
				size_t ChatLength = wcslen(m_InsertChat);
				size_t remainingChatSpace = sizeof(m_InsertChat) / sizeof(m_InsertChat[0]) - ChatLength - 1;

				//size_t ChatLength = wcslen(m_InsertChat);
				size_t CompleteLength = wcslen(m_CompleteChat);
				size_t remainingCompleteSpace = sizeof(m_CompleteChat) / sizeof(m_CompleteChat[0]) - CompleteLength - 1;

				if (remainingChatSpace > 0) {
					m_InsertChat[ChatLength] = L' ';  // m_InsertChat에 공백 문자 추가
					m_InsertChat[ChatLength + 1] = L'\0';  // 널 종료 문자 위치 업데이트
				}

				if (remainingCompleteSpace > 0) {
					m_CompleteChat[CompleteLength] = L' ';  // m_CompleteChat에 공백 문자 추가
					m_CompleteChat[CompleteLength + 1] = L'\0';
				}
			}
			break;
		}
	}
	else {
		switch (nMessageID)
		{
		case WM_KEYUP:
			switch (wParam)
			{
			case VK_ESCAPE:
				::PostQuitMessage(0);
				break;
			case VK_RETURN:
				if (m_nMode == SCENE1STAGE) {
					UI_Switch = !UI_Switch;
				}
				break;
			case VK_F1:
			case VK_F2:
			case VK_F3:
				if (m_ingame_role == R_RIFLE)
					m_pCamera = ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
				if (m_ingame_role == R_HELI)
					m_pCamera = ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
				else
					m_pCamera = ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
				break;
			case VK_F9:
				ChangeSwapChainState();
				break;
			case VK_END:
				q_keyboardInput.push(SEND_KEY_END);	// 몰살 치트키
				break;
			case VK_PRIOR:	// Page up key
				q_keyboardInput.push(SEND_KEY_PGUP); // 원샷원킬 치트키
				break;
			case VK_NEXT:	// Page down key
				q_keyboardInput.push(SEND_KEY_PGDN); // 원샷원킬 치트키 해제
				break;
			case 'W':
			case 'A':
			case 'S':
			case 'D':
				q_keyboardInput.push(SEND_KEYUP_MOVEKEY);
				break;
			case 'R':
				if (!player_dead && ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_bMoveUpdate == false && m_ingame_role == R_RIFLE)
				{
					((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_bReloadState = true;
					((MuzzleFrameBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[6])->m_bShotActive = false;
					((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->ReloadState();
					q_keyboardInput.push(SEND_KEY_R);
				}
				if (!player_dead && m_ingame_role == R_HELI)
				{
					q_keyboardInput.push(SEND_KEY_R);
				}
				break;
				//case 'H':
				//	//gamesound.HartBeatSound();
				//	gamesound.pauseHeartBeat();
				//	break;
				//case 'K':
				//	//gamesound.HartBeatSound();
				//	gamesound.PlayHearBeatSound();
				//	break;
			case '9':
				if (MouseResponsivenessY > 400 && MouseResponsivenessY < 600) MouseResponsivenessY -= 50.0f;
				break;
			case '0':
				if (MouseResponsivenessY > 400 && MouseResponsivenessY < 600) MouseResponsivenessY += 50.0f;
				break;
			case 'M':
				((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->FallDown(m_GameTimer.GetTimeElapsed());
				break;
			case 'Y':
				((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Resetpartition();
				break;
			default:
				break;
			}
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_CONTROL:
				((Stage1*)m_pScene)->m_ppFragShaders[0]->m_bActive = true;
				break;
			case VK_SPACE:
				break;


			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_IME_COMPOSITION:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_CHAR:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
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

#define _WITH_TERRAIN_PLAYER
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
void CGameFramework::BuildObjects()
{
	gamesound.PlayOpeningSound();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);

	m_pScene = new SceneManager();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, d3dRtvCPUDescriptorHandle, m_pd3dDepthStencilBuffer);
	HeliPlayer* pPlayer = new HeliPlayer(m_pd3dDevice, m_pd3dCommandList, NULL, m_pScene->GetGraphicsRootSignature(), NULL);
	CreateShaderVariables();
	m_pScene->m_pPlayer = m_pScene->m_pPlayer = pPlayer;
	m_pCamera = ((HeliPlayer*)((SceneManager*)m_pScene)->m_pPlayer)->GetCamera();
	m_pCamera->SetMode(CLOSEUP_PERSON_CAMERA);



	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pScene->m_pPlayer) m_pScene->m_pPlayer->ReleaseUploadBuffers();
	m_GameTimer.Reset();
}
void CGameFramework::ReleaseObjects()
{
	if (m_pScene->m_pPlayer) m_pScene->m_pPlayer->Release();
	if (m_pScene) m_pScene->ReleaseObjects();
}

bool ShotKey = false;
bool NpcShotKey = false;
void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;
		if (!UI_Switch && !player_dead) {
			if (pKeysBuffer[KEY_W] & 0xF0) { q_keyboardInput.push(SEND_KEY_W); dwDirection |= DIR_FORWARD; }
			if (pKeysBuffer[KEY_S] & 0xF0) { q_keyboardInput.push(SEND_KEY_S); dwDirection |= DIR_BACKWARD; }
			if (pKeysBuffer[KEY_A] & 0xF0) { q_keyboardInput.push(SEND_KEY_A); dwDirection |= DIR_LEFT; }
			if (pKeysBuffer[KEY_D] & 0xF0) { q_keyboardInput.push(SEND_KEY_D); dwDirection |= DIR_RIGHT; }

			if (m_ingame_role == R_HELI)
			{
				if (pKeysBuffer[KEY_Q] & 0xF0) { q_keyboardInput.push(SEND_KEY_Q); dwDirection |= DIR_UP; }
				if (pKeysBuffer[KEY_E] & 0xF0) { q_keyboardInput.push(SEND_KEY_E); dwDirection |= DIR_DOWN; }
			}
		}

		if (!player_dead) { if (pKeysBuffer[VK_SPACE] & 0xF0) { q_keyboardInput.push(SEND_KEY_SPACEBAR); } }

		float cxDelta = 0.0f, cyDelta = 0.0f, czDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / MouseResponsivenessX;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / MouseResponsivenessY;
			czDelta = ((float)(ptCursorPos.y - m_ptOldCursorPos.y) + (float)(ptCursorPos.x - m_ptOldCursorPos.x)) / MouseResponsivenessX;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if (pKeysBuffer[VK_LBUTTON] & 0xF0) {
			if (m_nMode == SCENE1STAGE && m_ingame_role == R_RIFLE)
			{
				if (((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_fShotDelay < 0.01 && m_currbullet != 0)
				{
					ShotKey = true;
					((Stage1*)m_pScene)->Reflectcartridgecase(NULL);
					MouseInputVal lclick{ SEND_BUTTON_L, 0.f, 0.f };//s
					q_mouseInput.push(lclick);//s
					((CHumanPlayer*)m_pScene->m_pPlayer)->ShotState(m_GameTimer.GetTimeElapsed());

				}
			}
			if (m_nMode == SCENE1STAGE && m_ingame_role == R_HELI)
			{
				if (((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_fShotDelay < 0.004 && m_currbullet != 0)
				{
					ShotKey = true;
					MouseInputVal lclick{ SEND_BUTTON_L, 0.f, 0.f };
					q_mouseInput.push(lclick);

					((Stage1*)m_pScene)->PlayerFirevalkan(m_pCamera, m_pCamera->GetLookVector());

				}
			}
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (!player_dead) {
				if (m_nMode == SCENE1STAGE) {
					if (cxDelta || cyDelta /*|| ((CHumanPlayer*)m_pScene->m_pPlayer)->m_bZoomMode == true*/)
					{
						MouseInputVal mousemove{ SEND_NONCLICK, 0.f, 0.f };//s
						q_mouseInput.push(mousemove);//s
						if (m_ingame_role == R_RIFLE)
							((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Rotate(cyDelta, cxDelta, 0.0f);
						//---------- HeliPlayer
						if (m_ingame_role == R_HELI)
						{
							((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Rotate(cyDelta, cxDelta, 0.0f);
							//((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[0])
								//->Rotate(cyDelta, cxDelta, 0.0f);
							if (m_pCamera->m_bHelicopterFreedom==true)
							{
								((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Rotate(0.0,0.0, 0.0f);
								m_pCamera->Rotate(cyDelta/6, cxDelta/6,0.0f);
							}
						}
						else
							((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Rotate(cyDelta, cxDelta, 0.0f);
					}
					if (dwDirection)
					{
						bool isCollide = false;
						CollideMapInfo temp;
						for (int i{}; i < mapcol_info.size(); ++i) {
							if (mapcol_info[i].m_xoobb.Intersects(((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_xoobb)) {
								temp = mapcol_info[i];
								isCollide = true;
								if (b_heli_mapcollide_cooldown == 0) {
									b_heli_mapcollide = true;	// 헬리 벽 충돌 서버 전송용
									b_heli_mapcollide_cooldown = 100;	// 계속 충돌판정 나는거 방지
								}
								break;
							}
						}

						if (isCollide) {
							XMFLOAT3 normalizedLocalForward;
							XMVECTOR localForwardNormalized = XMVector3Normalize(XMLoadFloat3(&temp.m_local_forward));
							XMStoreFloat3(&normalizedLocalForward, localForwardNormalized);

							XMFLOAT3 normalizedLocalRight;
							XMVECTOR localRightNormalized = XMVector3Normalize(XMLoadFloat3(&temp.m_local_right));
							XMStoreFloat3(&normalizedLocalRight, localRightNormalized);

							XMFLOAT3 Center2PlayerVector = Vector3::Subtract(((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition(), temp.m_xoobb.Center);//벡터
							float Center2PlayerDisrtance = Vector3::Length(Center2PlayerVector);//거리
							Center2PlayerVector = Vector3::Normalize(Center2PlayerVector);


							float forwardDotResult = Vector3::DotProduct(Center2PlayerVector, temp.m_local_forward); //객체의 center와 플레이어와 normal간의 cos값   
							float rightDotResult = Vector3::DotProduct(Center2PlayerVector, temp.m_local_right);

							float forwardDotResultAbs = abs(forwardDotResult);
							float rightDotResultAbs = abs(rightDotResult);

							float radian = XMConvertToRadians(temp.m_angle_aob / 2);

							XMFLOAT3 PlayerMoveDir;

							if (abs(cos(radian)) < forwardDotResultAbs) {
								if (forwardDotResult < 0) {
									XMVECTOR reversedLocalForward = XMVectorNegate(XMLoadFloat3(&normalizedLocalForward));
									XMStoreFloat3(&normalizedLocalForward, reversedLocalForward);

									XMVECTOR AddVector = XMVectorAdd(XMLoadFloat3(&Center2PlayerVector), XMLoadFloat3(&normalizedLocalForward));
									XMStoreFloat3(&PlayerMoveDir, AddVector);
								}
								else {
									XMVECTOR AddVector = XMVectorAdd(XMLoadFloat3(&Center2PlayerVector), XMLoadFloat3(&normalizedLocalForward));
									XMStoreFloat3(&PlayerMoveDir, AddVector);
								}
							}
							else {
								if (rightDotResult < 0) {
									XMVECTOR reversedLocalRight = XMVectorNegate(XMLoadFloat3(&normalizedLocalRight));
									XMStoreFloat3(&normalizedLocalRight, reversedLocalRight);

									XMVECTOR AddVector = XMVectorAdd(XMLoadFloat3(&Center2PlayerVector), XMLoadFloat3(&normalizedLocalRight));
									XMStoreFloat3(&PlayerMoveDir, AddVector);
								}
								else {
									XMVECTOR AddVector = XMVectorAdd(XMLoadFloat3(&Center2PlayerVector), XMLoadFloat3(&normalizedLocalRight));
									XMStoreFloat3(&PlayerMoveDir, AddVector);
								}
							}
							XMVECTOR PlayerMoveNormalized = XMVector3Normalize(XMLoadFloat3(&PlayerMoveDir));
							XMStoreFloat3(&PlayerMoveDir, PlayerMoveNormalized);

							dwDirection = DIR_SLIDEVEC;

							if (m_ingame_role == R_RIFLE)
								((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Move(dwDirection, 850.0f * m_GameTimer.GetTimeElapsed(), true, PlayerMoveDir);
							if (m_ingame_role == R_HELI)
								((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Move(dwDirection, 1750.0f * m_GameTimer.GetTimeElapsed(), true, PlayerMoveDir);
							else
								((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Move(dwDirection, 850.0f * m_GameTimer.GetTimeElapsed(), true, PlayerMoveDir);
						} 	//--------Human Player-----------// 
						else
						{
							if (m_ingame_role == R_RIFLE)
								((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Move(dwDirection, 850.f * m_GameTimer.GetTimeElapsed(), true, { 0,0,0 });

							if (m_ingame_role == R_HELI)
								((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Move(dwDirection, 1750.0f * m_GameTimer.GetTimeElapsed(), true, { 0,0,0 });
							else
								((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Move(dwDirection, 850.f * m_GameTimer.GetTimeElapsed(), true, { 0,0,0 });
						}
						//--------Heli Player-----------// 
						if (m_ingame_role == R_HELI)
							((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Rotate(cyDelta, cxDelta, 0.0f);
						//--------Human Player-----------// 
						if (m_ingame_role == R_RIFLE)
							((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Rotate(cyDelta, cxDelta, 0.0f);

					}
				}
			}
		}
	}
	if (m_nMode == SCENE1STAGE)
	{
		//--------Human Player-----------// 
		if (m_ingame_role == R_RIFLE)
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Update(m_GameTimer.GetTimeElapsed());
		//--------Heli Player-----------// 
		if (m_ingame_role == R_HELI)
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Update(m_GameTimer.GetTimeElapsed());
		else
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Update(m_GameTimer.GetTimeElapsed());
	}

}


void CGameFramework::AnimateObjects()
{
	if (m_nMode == OPENINGSCENE)
	{
		if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
	}
	if (m_nMode == SCENE1STAGE)
	{
		for (int i = 12; i < 16; i++)
		{
			if (((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[i])->m_bDyingstate == true && i)
			{
				((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[i])->FallDown(m_GameTimer.GetTimeElapsed());
			}
		}
		if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
		if (m_ingame_role == R_RIFLE)
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Animate(m_GameTimer.GetTimeElapsed(), NULL);
		if (m_ingame_role == R_HELI)
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Animate(m_GameTimer.GetTimeElapsed(), NULL);

		((Stage1*)m_pScene)->PlayerByPlayerCollision();

		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA && m_bDieMotion == false)
		{
			if (m_ingame_role == R_RIFLE)
				m_pCamera = ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->ChangeCamera(CLOSEUP_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());
			if (m_ingame_role == R_HELI)
				m_pCamera = ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->ChangeCamera(CLOSEUP_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());

		}
		ShotDelay();
		if (m_currbullet <= 0)((MuzzleFrameBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[6])->m_bShotActive = false;


		if (NpcShotKey == true)
		{
			((MuzzleFrameBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[7])->m_bShotActive = true;
		}
		if (NpcShotKey == false)
		{
			((MuzzleFrameBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[7])->m_bShotActive = false;
		}
	}
}
void CGameFramework::ShotDelay()
{
	if (m_ingame_role == R_RIFLE)
	{
		((CHumanPlayer*)m_pScene->m_pPlayer)->m_fShotDelay += m_GameTimer.GetTimeElapsed();
		if (((CHumanPlayer*)m_pScene->m_pPlayer)->m_fShotDelay > 0.17)
		{
			ShotKey = false;
			((CHumanPlayer*)m_pScene->m_pPlayer)->m_fShotDelay = 0.0f;

		}
		if (ShotKey == false)
		{
			m_pCamera->m_xmf4x4View._43 += 0.1f;
			((MuzzleFrameBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[6])->m_bShotActive = false;
			if (((CHumanPlayer*)m_pScene->m_pPlayer)->m_bZoomMode == true)
			{
				m_pCamera->m_xmf4x4View._42 += 0.20f;
				m_pCamera->m_xmf4x4View._43 += 0.75f;
			}
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.7, 0.7, 0.7, 1.0);
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.2, 0.2, 0.2, 1.0);
		}
		if (ShotKey == true)
		{
			((CHumanPlayer*)m_pScene->m_pPlayer)->Rotate(-0.07, 0.0, 0.0);
			m_pCamera->m_xmf4x4View._43 -= 0.07f;
			((MuzzleFrameBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[6])->m_bShotActive = true;
			if (((CHumanPlayer*)m_pScene->m_pPlayer)->m_bZoomMode == true)
			{
				m_pCamera->m_xmf4x4View._42 -= 0.20f;
				m_pCamera->m_xmf4x4View._43 -= 0.75f;
			}
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.9, 0.4, 0.1, 1.0);
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.2, 0.2, 0.2, 1.0);
		}
	}
	if (m_ingame_role == R_HELI)
	{
		((HeliPlayer*)m_pScene->m_pPlayer)->m_fShotDelay += m_GameTimer.GetTimeElapsed();
		if (((HeliPlayer*)m_pScene->m_pPlayer)->m_fShotDelay > 0.1)
		{
			ShotKey = false;
			((HeliPlayer*)m_pScene->m_pPlayer)->m_fShotDelay = 0.0f;
		}
		if (ShotKey == false)
		{
			m_pCamera->m_xmf4x4View._43 += 0.1f;
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.7, 0.7, 0.7, 1.0);
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.2, 0.2, 0.2, 1.0);
		}
		if (ShotKey == true)
		{
			m_pCamera->m_xmf4x4View._43 -= 0.1f;
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.9, 0.4, 0.1, 1.0);
			if (m_nMode == SCENE1STAGE)((Stage1*)m_pScene)->m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.2, 0.2, 0.2, 1.0);
		}
	}
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
void CGameFramework::CreateShaderVariables()
{
}
void CGameFramework::ReleaseShaderVariables()
{
}
void CGameFramework::UpdateShaderVariables()
{
}

float g_time = 0.0f;
float g_reverse_time = 0.0f;

float g_Hittingtime = 0.0f;
float g_WarnningSwitchtime = 0.0f;
void CGameFramework::FrameAdvance()
{

	m_GameTimer.Tick(60);
	AnimateObjects();

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	m_pScene->OnPrepareRender(m_pd3dCommandList, m_pCamera);
	m_pScene->OnPreRender(m_pd3dCommandList, m_pCamera);
	UpdateShaderVariables();

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pd3dCommandList->ClearRenderTargetView(m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], pfClearColor/*Colors::Azure*/, 0, NULL);
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_pd3dCommandList->OMSetRenderTargets(1, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], TRUE, &d3dDsvCPUDescriptorHandle);

	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);
	if (m_nMode == SCENE1STAGE)
	{
		if (m_ingame_role == R_RIFLE)
			((Stage1*)m_pScene)->BillBoardRender(m_pd3dCommandList, m_pCamera, ((CHumanPlayer*)m_pScene->m_pPlayer)->m_pBulletFindFrame->GetPosition());
		((Stage1*)m_pScene)->MuzzleFlameRender(m_pd3dCommandList, m_pCamera, ((CHumanPlayer*)m_pScene->m_pPlayer)->m_pBulletFindFrame->GetPosition());
		if (m_ingame_role == R_HELI)
			((Stage1*)m_pScene)->BillBoardRender(m_pd3dCommandList, m_pCamera, m_pCamera->GetPosition());
	}

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	if (m_nMode == SCENE1STAGE)
		if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, NULL, m_pCamera);
	// Stage2
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();

	// -> DIRECT2D ERROR 송출
#ifdef _WITH_DIRECT2D
	//Direct2D Drawing
	m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);
	ID3D11Resource* ppd3dResources[] = { m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex] };
	m_pd3d11On12Device->AcquireWrappedResources(ppd3dResources, _countof(ppd3dResources));

	m_pd2dDeviceContext->BeginDraw();
	m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	if (m_nMode == OPENINGSCENE) {
		// Logo
		D2D_POINT_2F D2_OpeningUI = { 50.0f, 50.0f };
		D2D_RECT_F D2_OpeningUIRect = { 0.0f, 0.0f, 840.0f, 184.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[25], &D2_OpeningUI, &D2_OpeningUIRect);

		// Game Clear 시 UI --> 내일 작업 예정


		// Game Failed 시 UI --> 내일 작업 예정


		// 로비 안내 UI --> 80(인 게임 진입), 81(방장에게만 보낼 것. 준비되지 않은 플레이어 있다고 표시), 82(각자 직업 선택하라고 알림)


		if (m_LoginScene == 0) {
			D2D_POINT_2F D2_LoginUI = { (FRAME_BUFFER_WIDTH / 6) + 12.0f, FRAME_BUFFER_HEIGHT / 16 * 9 };
			D2D_RECT_F D2_LoginUIRect = { 0.0f, 0.0f, 381.0f, 297.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[26], &D2_LoginUI, &D2_LoginUIRect);

			POINT CurrMousePoint;
			GetCursorPos(&CurrMousePoint);

			bool RegistOff = true;
			bool LoginOff = true;

			if (loginpos[2].sx < CurrMousePoint.x && CurrMousePoint.x < loginpos[2].lx && loginpos[2].sy < CurrMousePoint.y && CurrMousePoint.y < loginpos[2].ly) {
				RegistOff = false;
				LoginOff = true;
			}
			else if (loginpos[3].sx < CurrMousePoint.x && CurrMousePoint.x < loginpos[3].lx && loginpos[3].sy < CurrMousePoint.y && CurrMousePoint.y < loginpos[3].ly) {
				RegistOff = true;
				LoginOff = false;
			}


			D2D_POINT_2F D2_RegistButtonUI = { (FRAME_BUFFER_WIDTH / 6) + 22.5f, D2_LoginUI.y + 210.0f };
			D2D_RECT_F D2_RegistButtonUIRect = { 0.0f, 71.5f * RegistOff, 171.0f, 71.5f * (RegistOff + 1) };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[69], &D2_RegistButtonUI, &D2_RegistButtonUIRect);


			D2D_POINT_2F D2_LoginButtonUI = { D2_RegistButtonUI.x + 190.0f, D2_LoginUI.y + 210.0f };
			D2D_RECT_F D2_LoginButtonUIRect = { 0.0f, 71.5f * LoginOff, 168.0f, 71.5f * (LoginOff + 1) };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[68], &D2_LoginButtonUI, &D2_LoginButtonUIRect);



		}
		else if (m_LoginScene == 1) {
			POINT CurrMousePoint;
			GetCursorPos(&CurrMousePoint);

			bool StartOn = false;
			bool ExitOn = false;

			if (gamepos[0].sx < CurrMousePoint.x && CurrMousePoint.x < gamepos[0].lx && gamepos[0].sy < CurrMousePoint.y && CurrMousePoint.y < gamepos[0].ly) {
				ExitOn = false;
				StartOn = true;
			}
			else if (gamepos[1].sx < CurrMousePoint.x && CurrMousePoint.x < gamepos[1].lx && gamepos[1].sy < CurrMousePoint.y && CurrMousePoint.y < gamepos[1].ly) {
				ExitOn = true;
				StartOn = false;
			}

			D2D_POINT_2F D2_GameStartUI = { 100.0f, FRAME_BUFFER_HEIGHT / 8 * 5 };
			D2D_RECT_F D2_GameStartUIRect = { 0.0f, 70.0f * StartOn, 171.0f, 70.0f * (StartOn + 1) };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[54], &D2_GameStartUI, &D2_GameStartUIRect);

			D2D_POINT_2F D2_GameExitUI = { 100.0f, FRAME_BUFFER_HEIGHT / 4 * 3 };
			D2D_RECT_F D2_GameExitUIRect = { 0.0f, 70.0f * ExitOn, 171.0f, 70.0f * (ExitOn + 1) };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[55], &D2_GameExitUI, &D2_GameExitUIRect);
		}
		else {
			// 로비 ui
			D2D_POINT_2F D2_RobbyUI = { FRAME_BUFFER_WIDTH / 2 - 640.0f, FRAME_BUFFER_HEIGHT / 2 - 381.5f };
			D2D_RECT_F D2_RobbyUIRect = { 0.0f, 0.0f, 1280.0f, 763.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[27], &D2_RobbyUI, &D2_RobbyUIRect);

			POINT CurrMousePoint;
			GetCursorPos(&CurrMousePoint);

			bool Roomin = false;
			bool CreateRoom = false;
			bool LeftOn = false;
			bool RightOn = false;
			if (m_LoginScene == 2) {
				if (lobbypos[0].sx < CurrMousePoint.x && CurrMousePoint.x < lobbypos[0].lx && lobbypos[0].sy < CurrMousePoint.y && CurrMousePoint.y < lobbypos[0].ly) {
					Roomin = true;
					CreateRoom = false;
					LeftOn = false;
					RightOn = false;
				}
				else if (lobbypos[1].sx < CurrMousePoint.x && CurrMousePoint.x < lobbypos[1].lx && lobbypos[1].sy < CurrMousePoint.y && CurrMousePoint.y < lobbypos[1].ly) {
					Roomin = false;
					CreateRoom = true;
					LeftOn = false;
					RightOn = false;
				}
				else if (lobbypos[3].sx < CurrMousePoint.x && CurrMousePoint.x < lobbypos[3].lx && lobbypos[3].sy < CurrMousePoint.y && CurrMousePoint.y < lobbypos[3].ly) {
					Roomin = false;
					CreateRoom = false;
					LeftOn = true;
					RightOn = false;
				}
				else if (lobbypos[4].sx < CurrMousePoint.x && CurrMousePoint.x < lobbypos[4].lx && lobbypos[4].sy < CurrMousePoint.y && CurrMousePoint.y < lobbypos[4].ly) {
					Roomin = false;
					CreateRoom = false;
					LeftOn = false;
					RightOn = true;
				}
			}

			// 빠른시작
			D2D_POINT_2F D2_RapidStartUI = { 1004.0f, 215.0f };
			D2D_RECT_F D2_RapidStartUIRect = { 0.0f, 102.5f * Roomin, 260.0f, 102.5f * (Roomin + 1) };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[56], &D2_RapidStartUI, &D2_RapidStartUIRect);

			// 방만들기
			D2D_POINT_2F D2_CreateRoomUI = { 1310.0f, 215.0f };
			D2D_RECT_F D2_CreateRoomUIRect = { 0.0f, 102.5f * CreateRoom, 261.0f, 102.5f * (CreateRoom + 1) };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[57], &D2_CreateRoomUI, &D2_CreateRoomUIRect);


			// 왼쪽 페이지
			D2D_POINT_2F D2_LeftPageUI = { FRAME_BUFFER_WIDTH / 2 - 100.0f, FRAME_BUFFER_HEIGHT / 2 + 315.0f };
			D2D_RECT_F D2_LeftPageUIRect = { 0.0f, 0.0f, 46.0f, 50.0f };
			if (!LeftOn) m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[58], &D2_LeftPageUI, &D2_LeftPageUIRect);
			else m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[59], &D2_LeftPageUI, &D2_LeftPageUIRect);
			// 오른쪽 페이지
			D2D_POINT_2F D2_RightPageUI = { FRAME_BUFFER_WIDTH / 2 + 26.0f, FRAME_BUFFER_HEIGHT / 2 + 315.0f };
			D2D_RECT_F D2_RightPageUIRect = { 0.0f, 0.0f, 46.0f, 50.0f };

			if (!RightOn) m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[60], &D2_RightPageUI, &D2_RightPageUIRect);
			else m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[61], &D2_RightPageUI, &D2_RightPageUIRect);

			D2D_POINT_2F D2_RobbyPeopleUI[8];
			D2D_RECT_F D2_RobbyPeopleUIRect[8];

			D2D_POINT_2F D2_RobbyReadyUI[8];
			D2D_RECT_F D2_RobbyReadyUIRect[8];

			for (int i = 0; i < m_LobbyRoom_Info.size(); ++i) {
				int numberRoom = i;
				if (m_LobbyPage != 0) {
					numberRoom = m_LobbyPage * 8 + i;
				}
				int resulty = 400 + 54 * i;
				float textypos = (((float)(FRAME_BUFFER_HEIGHT)) / ((float)(resulty)));

				// 각 방 현재 인원
				D2_RobbyPeopleUI[i] = { FRAME_BUFFER_WIDTH / 1.60f, FRAME_BUFFER_HEIGHT / textypos - 10.0f };
				D2_RobbyPeopleUIRect[i] = { 0.0f, (m_LobbyRoom_Info[numberRoom].currnum_of_people - 1) * 48.0f, 120.0f, m_LobbyRoom_Info[numberRoom].currnum_of_people * 48.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[28 + i], &D2_RobbyPeopleUI[i], &D2_RobbyPeopleUIRect[i]);

				// 각 방 준비 상태
				D2_RobbyReadyUI[i] = { FRAME_BUFFER_WIDTH / 1.42f, FRAME_BUFFER_HEIGHT / textypos };
				D2_RobbyReadyUIRect[i] = { 0.0f, (m_LobbyRoom_Info[numberRoom].ready_state - 1) * 50.0f,225.0f, m_LobbyRoom_Info[numberRoom].ready_state * 50.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[36 + i], &D2_RobbyReadyUI[i], &D2_RobbyReadyUIRect[i]);
			}
			if (m_LoginScene == 3) {
				// 방 ui
				D2D_POINT_2F D2_RoomUI = { (FRAME_BUFFER_WIDTH / 6), FRAME_BUFFER_HEIGHT / 2 - 223.5f };
				D2D_RECT_F D2_RoomUIRect = { 0.0f, 0.0f, 1280.0f, 447.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[44], &D2_RoomUI, &D2_RoomUIRect);

				// 시작 버튼
				if (m_RoomClick[0]) m_StartKey = 0.0f;
				else m_StartKey = 102.5f;

				D2D_POINT_2F D2_RoomStartUI = { FRAME_BUFFER_WIDTH / 2, D2_RoomUI.y + 40.0f };
				D2D_RECT_F D2_RoomStartUIRect = { 0.0f , 0.0f + m_StartKey, 261.0f, 102.5f + m_StartKey };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[45], &D2_RoomStartUI, &D2_RoomStartUIRect);

				// 준비 버튼
				if (m_RoomClick[1]) m_ReadyKey = 102.5f;
				else m_ReadyKey = 0.0f;

				D2D_POINT_2F D2_RoomReadyUI = { D2_RoomStartUI.x + 320.0f, D2_RoomStartUI.y };
				D2D_RECT_F D2_RoomReadyUIRect = { 0.0f, 0.0f + m_ReadyKey, 261.0f, 102.5f + m_ReadyKey };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[46], &D2_RoomReadyUI, &D2_RoomReadyUIRect);

				// 모든 인원 준비 표시
				D2D_POINT_2F D2_RoomReadyNumUI[3];
				D2D_RECT_F D2_RoomReadyNumUIRect[3];

				for (int i = 0; i < m_MAX_USER; ++i) {
					int resulty = 560 + 58 * i;
					float textypos = (((float)(FRAME_BUFFER_HEIGHT)) / ((float)(resulty)));

					D2_RoomReadyNumUI[i] = { FRAME_BUFFER_WIDTH / 1.48f, FRAME_BUFFER_HEIGHT / textypos };
					D2_RoomReadyNumUIRect[i] = { 0.0f, (m_MyRoom_Info[i].ready_state - 1) * 60.0f, 284.0f, m_MyRoom_Info[i].ready_state * 60.0f };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[47 + i], &D2_RoomReadyNumUI[i], &D2_RoomReadyNumUIRect[i]);
				}

				// 역할 선택
				D2D_POINT_2F D2_JobArmyUI[3];
				D2D_RECT_F D2_JobArmyRect[3];

				D2D_POINT_2F D2_JobHeliUI[3];
				D2D_RECT_F D2_JobHeliRect[3];

				D2D_POINT_2F D2_MyMarkUI;
				D2D_RECT_F D2_MyMarkRect;

				for (int i = 0; i < m_MAX_USER; ++i) {
					int resulty = 565 + 58 * i;
					float textypos = (((float)(FRAME_BUFFER_HEIGHT)) / ((float)(resulty)));

					D2_JobArmyUI[i] = { FRAME_BUFFER_WIDTH * 0.6f + 5.0f, FRAME_BUFFER_HEIGHT / textypos };
					D2_JobArmyRect[i] = { 0.0f, (m_MyRoom_Info[i].armyCheck) * 50.0f, 50.0f, (m_MyRoom_Info[i].armyCheck + 1) * 50.0f };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[62 + i], &D2_JobArmyUI[i], &D2_JobArmyRect[i]);

					D2_JobHeliUI[i] = { D2_JobArmyUI[i].x + 75.0f, FRAME_BUFFER_HEIGHT / textypos };
					D2_JobHeliRect[i] = { 0.0f, (m_MyRoom_Info[i].HeliCheck) * 50.0f, 50.0f, (m_MyRoom_Info[i].HeliCheck + 1) * 50.0f };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[65 + i], &D2_JobHeliUI[i], &D2_JobHeliRect[i]);

					if (i == m_roominMyId) {
						D2_MyMarkUI = { (FRAME_BUFFER_WIDTH / 5.0f), FRAME_BUFFER_HEIGHT / textypos + 5.0f };
						D2_MyMarkRect = { 0.0f, 0.0f, 25.0f, 37.0f };
						m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[83], &D2_MyMarkUI, &D2_MyMarkRect);
					}
				}



				if (m_ingame) {
					D2D_POINT_2F D2_GameLoadingUI = { FRAME_BUFFER_WIDTH / 2 - 404.0f, FRAME_BUFFER_HEIGHT / 2 - 78.0f };
					D2D_RECT_F D2_GameLoadingUIRect = { 0.0f, 0.0f, 808.0f, 156.0f };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[80], &D2_GameLoadingUI, &D2_GameLoadingUIRect);
				}
				if (m_infoReady) {
					D2D_POINT_2F D2_NotAllReadyUI = { FRAME_BUFFER_WIDTH / 2 - 404.0f, FRAME_BUFFER_HEIGHT / 2 - 84.0f };
					D2D_RECT_F D2_NotAllReadyUIRect = { 0.0f, 0.0f, 808.0f, 168.0f };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[81], &D2_NotAllReadyUI, &D2_NotAllReadyUIRect);

					m_infoReadyTime += 0.1f;
				}
				if (m_infoChoose) {
					D2D_POINT_2F D2_ChoiceJobUI = { FRAME_BUFFER_WIDTH / 2 - 404.0f, FRAME_BUFFER_HEIGHT / 2 - 84.0f };
					D2D_RECT_F D2_ChoiceJobUIRect = { 0.0f, 0.0f, 808.0f, 168.0f };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[82], &D2_ChoiceJobUI, &D2_ChoiceJobUIRect);

					m_infoChooseTime += 0.1f;
				}

			}
			if (m_LoginScene == 4) {
				D2D_POINT_2F D2_RoomUI = { (FRAME_BUFFER_WIDTH / 6) + 100.0f, FRAME_BUFFER_HEIGHT / 2 - 146.5f };
				D2D_RECT_F D2_RoomUIRect = { 0.0f, 0.0f, 1080.0f, 293.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[53], &D2_RoomUI, &D2_RoomUIRect);
			}
		}
	}
	else if (m_nMode = SCENE1STAGE) {
		// Time 
		D2D_POINT_2F D2_RemainTime = { FRAME_BUFFER_WIDTH / 128, FRAME_BUFFER_HEIGHT / 128 };
		D2D_RECT_F D2_RemainTimeRect = { 0.0f, 0.0f, 182.0f, 47.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[0], &D2_RemainTime, &D2_RemainTimeRect);

		D2D_POINT_2F D2_RemainTimeDot = { D2_RemainTime.x + 70.0f ,D2_RemainTime.y + 52.5f };
		D2D_RECT_F D2_RemainTimeDotRect = { 56.0f, 64.0f, 63.0f, 108.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[1], &D2_RemainTimeDot, &D2_RemainTimeDotRect);

		D2D_POINT_2F D2_RemainTime10Min = { D2_RemainTime.x, D2_RemainTime.y + 50.0f };
		D2D_RECT_F D2_RemainTime10MinRect = { m_10MinOfTime * 31.3f, 0.0f, 31.3f + m_10MinOfTime * 31.3f, 50.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[2], &D2_RemainTime10Min, &D2_RemainTime10MinRect);

		D2D_POINT_2F D2_RemainTime1Min = { D2_RemainTime10Min.x + 31.3f, D2_RemainTime.y + 50.0f };
		D2D_RECT_F D2_RemainTime1MinRect = { m_1MinOfTime * 31.3f, 0.0f, 31.3f + m_1MinOfTime * 31.3f, 50.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[3], &D2_RemainTime1Min, &D2_RemainTime1MinRect);

		D2D_POINT_2F D2_RemainTime10Sec = { D2_RemainTimeDot.x + 14.4f, D2_RemainTime.y + 50.0f };
		D2D_RECT_F D2_RemainTime10SecRect = { m_10SecOftime * 31.3f, 0.0f, 31.3f + m_10SecOftime * 31.3f, 50.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[4], &D2_RemainTime10Sec, &D2_RemainTime10SecRect);

		D2D_POINT_2F D2_RemainTime1Sec = { D2_RemainTime10Sec.x + 31.3f, D2_RemainTime.y + 50.0f };
		D2D_RECT_F D2_RemainTime1SecRect = { m_1SecOfTime * 31.3f, 0.0f, 31.3f + m_1SecOfTime * 31.3f, 50.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[5], &D2_RemainTime1Sec, &D2_RemainTime1SecRect);

		// Progress
		if (m_mainmissionnum == 1) {
			D2D_POINT_2F D2_ProgressUI = { FRAME_BUFFER_WIDTH / 2 - 113.0f, 160.0f };
			D2D_RECT_F D2_ProgressUIRect = { 0.0f, 0.0f, 226.0f, 112.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[6], &D2_ProgressUI, &D2_ProgressUIRect);

			D2D_POINT_2F D2_ProgressBG = { FRAME_BUFFER_WIDTH / 2 - 451.0f, D2_ProgressUI.y + 120.0f };
			D2D_RECT_F D2_ProgressBGRect = { 0.0f, 0.0f, 902.0f, 66.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[7], &D2_ProgressBG, &D2_ProgressBGRect);

			D2D_POINT_2F D2_CurrentProgress = { FRAME_BUFFER_WIDTH / 2 - 451.0f, D2_ProgressUI.y + 122.0f };
			D2D_RECT_F D2_CurrentProgressRect = { 0.0f, 0.0f, (902.0f / 100 * m_occupationnum), 62.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[8], &D2_CurrentProgress, &D2_CurrentProgressRect);

			if (m_occupationnum >= 99 && !m_missionFailed) {
				m_missionClear = true;
			}
		}
		// Compress
		D2D_POINT_2F D2_CompressArrow = { FRAME_BUFFER_WIDTH / 2 - 61.0f, 0.0f };
		D2D_RECT_F D2_CompressArrowRect = { 0.0f, 0.0f, 122.0f, 115.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[9], &D2_CompressArrow, &D2_CompressArrowRect);
		float myAngle{};

		if (m_ingame_role == R_RIFLE)
		{
			myAngle = abs(((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_fYaw);
			if (myAngle > 315.0f) {
				myAngle -= 360.0f;
			}
		}
		if (m_ingame_role == R_HELI)
		{
			myAngle = abs(((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_fYaw);
			if (myAngle > 315.0f) {
				myAngle -= 360.0f;
			}
		}
		D2D_POINT_2F D2_Compress = { FRAME_BUFFER_WIDTH / 2 - 150.5f, D2_CompressArrow.y + 50.0f };
		D2D_RECT_F D2_CompressRect = { 75.3f + (myAngle * 1.6741f) , 0.0f, 376.3f + (myAngle * 1.6741f), 150.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[10], &D2_Compress, &D2_CompressRect);

		// My
		D2D_POINT_2F D2_BulletUIBG = { FRAME_BUFFER_WIDTH - 420.0f, FRAME_BUFFER_HEIGHT / 4 * 3 };
		D2D_RECT_F D2_BulletUIBGRect = { 0.0f, 0.0f, 402.0f, 65.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[11], &D2_BulletUIBG, &D2_BulletUIBGRect);

		D2D_POINT_2F D2_HPUIBG = { D2_BulletUIBG.x, D2_BulletUIBG.y + D2_BulletUIBGRect.bottom };
		D2D_RECT_F D2_HPUIBGRect = { 0.0f, 0.0f, 402.0f, 48.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[12], &D2_HPUIBG, &D2_HPUIBGRect);

		D2D_POINT_2F D2_BulletIcon = { FRAME_BUFFER_WIDTH / 16 * 13, D2_BulletUIBG.y + (D2_BulletUIBGRect.bottom / 2) - 16.0f };
		D2D_RECT_F D2_BulletIconRect = { 0.0f, 0.0f, 32.0f, 32.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[13], &D2_BulletIcon, &D2_BulletIconRect);

		D2D_POINT_2F D2_Bullet10Num = { FRAME_BUFFER_WIDTH - 255.3f, D2_BulletUIBG.y + (D2_BulletUIBGRect.bottom / 2) - 30.0f };
		D2D_RECT_F D2_Bullet10NumRect = { 45.3f * (m_currbullet / 10), 0.0f, 45.3f + (m_currbullet / 10) * 45.3f, 60.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[14], &D2_Bullet10Num, &D2_Bullet10NumRect);

		D2D_POINT_2F D2_Bullet1Num = { D2_Bullet10Num.x + 45.3f, D2_BulletUIBG.y + (D2_BulletUIBGRect.bottom / 2) - 30.0f };
		D2D_RECT_F D2_Bullet1NumRect = { 45.3f * (m_currbullet % 10), 0.0f, 45.3f + (m_currbullet % 10) * 45.3f, 60.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[15], &D2_Bullet1Num, &D2_Bullet1NumRect);

		D2D_POINT_2F D2_HPBar = { D2_HPUIBG.x + 2.5f, D2_HPUIBG.y + (D2_HPUIBGRect.bottom / 2) - 20.0f };
		D2D_RECT_F D2_HPBarRect = { 0.0f, 0.0f, 3.97f * m_currHp, 40.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[16], &D2_HPBar, &D2_HPBarRect);

		// Remain NPC
		D2D_POINT_2F D2_RemainNPCBG = { FRAME_BUFFER_WIDTH - 191.0f, FRAME_BUFFER_HEIGHT / 128 };
		D2D_RECT_F D2_RemainNPCBGRect = { 0.0f, 0.0f, 135.0f, 50.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[17], &D2_RemainNPCBG, &D2_RemainNPCBGRect);

		// Mission
		D2D_POINT_2F D2_MainMissionUI = { FRAME_BUFFER_WIDTH - 300.0f, D2_RemainNPCBG.y + 65.0f };
		D2D_RECT_F D2_MainMissionUIRect = { 271.5f * m_mainmissionnum, 0.0f, 271.5f + (271.5f * m_mainmissionnum), 167.0f };
		m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[18], &D2_MainMissionUI, &D2_MainMissionUIRect);

		/*if (UI_Switch) {
			D2D_POINT_2F D2_SubMissionUI = { FRAME_BUFFER_WIDTH - 300.0f, D2_MainMissionUI.y + 200.0f };
			D2D_RECT_F D2_SubMissionUIRect = { 265.7f * m_submissionnum, 0.0f, 265.7f + (265.7f * m_submissionnum), 159.5f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[19], &D2_SubMissionUI, &D2_SubMissionUIRect);
		}*/

		// Cross Hair
		if (!m_SniperOn) {
			D2D_POINT_2F D2_CrossHairUI = { (FRAME_BUFFER_WIDTH / 2) - 166.5f, (FRAME_BUFFER_HEIGHT / 2) - 125.0f };
			D2D_RECT_F D2_CrossHairRect = { 0.0f, 0.0f, 333.0f, 250.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[20], &D2_CrossHairUI, &D2_CrossHairRect);
		}

		// Team
		if (m_CurrentPlayerNum > 1) {
			D2D_POINT_2F D2_Team1UI = { FRAME_BUFFER_WIDTH / 128, FRAME_BUFFER_HEIGHT / 5 * 3 };
			D2D_RECT_F D2_Team1UIRect = { 0.0f, 0.0f, 323.0f, 60.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[21], &D2_Team1UI, &D2_Team1UIRect);

			D2D_POINT_2F D2_Team1UIHP = { FRAME_BUFFER_WIDTH / 128 + 14.0f, D2_Team1UI.y + 44.0f };
			D2D_RECT_F D2_Team1UIHPRect = { 0.0f, 0.0f, m_otherHP[0] * 2.95f, 11.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[23], &D2_Team1UIHP, &D2_Team1UIHPRect);
		}

		if (m_CurrentPlayerNum > 2) {
			D2D_POINT_2F D2_Team2UI = { FRAME_BUFFER_WIDTH / 128, FRAME_BUFFER_HEIGHT / 5 * 3 + 75.0f };
			D2D_RECT_F D2_Team2UIRect = { 0.0f, 0.0f, 323.0f, 60.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[22], &D2_Team2UI, &D2_Team2UIRect);

			D2D_POINT_2F D2_Team2UIHP = { FRAME_BUFFER_WIDTH / 128 + 14.0f, D2_Team2UI.y + 44.0f };
			D2D_RECT_F D2_Team2UIHPRect = { 0.0f, 0.0f, m_otherHP[1] * 2.95f, 11.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[24], &D2_Team2UIHP, &D2_Team2UIHPRect);
		}

		// Chat UI
		if (UI_Switch) {
			D2D_POINT_2F D2_ChatUI = { FRAME_BUFFER_WIDTH - 782.0f, (FRAME_BUFFER_HEIGHT / 2) - 145.5f };
			D2D_RECT_F D2_ChatUIRect = { 0.0f, 0.0f, 782.0f, 291.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[50], &D2_ChatUI, &D2_ChatUIRect);

			D2D_POINT_2F D2_InsertChatUI = { FRAME_BUFFER_WIDTH - 782.0f, D2_ChatUI.y + D2_ChatUIRect.bottom + 5.0f };
			D2D_RECT_F D2_InsertChatUIRect = { 0.0f, 0.0f, 782.0f, 69.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[51], &D2_InsertChatUI, &D2_InsertChatUIRect);
		}

		// Sniper UI
		if (m_SniperOn) {
			D2D_POINT_2F D2_SniperAimUI = { FRAME_BUFFER_WIDTH / 2 - 457.5f, FRAME_BUFFER_HEIGHT - 725.0f };
			D2D_RECT_F D2_SniperAimUIRect = { 0.0f, 0.0f, 950.0f, 787.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[52], &D2_SniperAimUI, &D2_SniperAimUIRect);

		}

		if (m_ingame_role == R_RIFLE)
		{
			// Splatter 
			if (m_BloodSplatterOn) {
				D2D_POINT_2F D2_HumanSplatterUI = { (FRAME_BUFFER_WIDTH - 2000.0f) / 2, (FRAME_BUFFER_HEIGHT - 1491.0f) / 2 };
				D2D_RECT_F D2_Splatter = { 0.0f, 0.0f, 2000.0f, 1491.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[70], &D2_HumanSplatterUI, &D2_Splatter);
			}

			if (m_BloodSplatterOn == true) g_Hittingtime++;
			if (g_Hittingtime > 6.0f)
			{
				g_Hittingtime = 0.0f;
				m_BloodSplatterOn = false;
			}
		}
		if (m_ingame_role == R_HELI)
		{
			//if(/*헬기 30퍼 일때 */)
			// Warrning
			if (m_HeliPlayerWarnningUISwitch) {
				if (g_WarnningSwitchtime > 3.0f)
				{
					D2D_POINT_2F D2_HeliWarrningBannerUI = { (FRAME_BUFFER_WIDTH - 2500.0) / 2, (FRAME_BUFFER_HEIGHT - 1559.0) / 2 };
					D2D_RECT_F D2_HeliWarrningRECT = { 0.0f, 0.0f, 2500.0, 1559.0 };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[72], &D2_HeliWarrningBannerUI, &D2_HeliWarrningRECT);

					D2D_POINT_2F D2_HeliWarrningIconUI = { 20.0, 150.0 };
					D2D_RECT_F D2_HeliWarrningIconRECT = { 0.0f, 0.0f, 200.0, 57.0 };
					m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[73], &D2_HeliWarrningIconUI, &D2_HeliWarrningIconRECT);
				}
				g_WarnningSwitchtime++;
				if (g_WarnningSwitchtime > 12.0f)
				{
					g_WarnningSwitchtime = 0.0f;
				}
			}
		}
		// DyingBanner
		if (player_dead == true)
		{
			D2D_POINT_2F D2_DyingBannerUI = { (FRAME_BUFFER_WIDTH - 2500.0) / 2, (FRAME_BUFFER_HEIGHT - 1730.0) / 2 };
			D2D_RECT_F D2_DyingBanner = { 0.0f, 0.0f, 2500.0, 1730.0 };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[71], &D2_DyingBannerUI, &D2_DyingBanner);
		}
		if (m_missionClear && !m_missionFailed) {
			D2D_POINT_2F D2_MissionclearedBGUI = { 0.0f, FRAME_BUFFER_HEIGHT / 2 - 163.0f };
			D2D_RECT_F D2_MissionclearedBGUIRect = { 0.0f, 0.0f, m_missionClearUI * 19.6f, 326.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[74], &D2_MissionclearedBGUI, &D2_MissionclearedBGUIRect);

			if (m_missionClearUI >= 100) {
				D2D_POINT_2F D2_MissionclearedTextUI = { FRAME_BUFFER_WIDTH / 2 - 416.0f, FRAME_BUFFER_HEIGHT / 2 - 170.0f };
				D2D_RECT_F D2_MissionclearedTextUIRect = { 0.0f, 0.0f, 832.0f, 176.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[75], &D2_MissionclearedTextUI, &D2_MissionclearedTextUIRect);

				D2D_POINT_2F D2_CongratulationUI = { FRAME_BUFFER_WIDTH / 2 - 399.0f, FRAME_BUFFER_HEIGHT / 2 };
				D2D_RECT_F D2_CongratulationUIRect = { 0.0f, 0.0f, 792.0f, 176.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[76], &D2_CongratulationUI, &D2_CongratulationUIRect);
			}
			else {
				m_missionClearUI += 2.5f;
				if (m_missionClearUI >= 100) {
					m_missionClearUI = 100.0f;
				}
			}
		}
		if (m_missionFailed) {
			D2D_POINT_2F D2_MissionFailedBGUI = { 0.0f, FRAME_BUFFER_HEIGHT / 2 - 181.5f };
			D2D_RECT_F D2_MissionFailedBGUIRect = { 0.0f, 0.0f, m_missionFailedUI * 19.6f, 363.0f };
			m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[77], &D2_MissionFailedBGUI, &D2_MissionFailedBGUIRect);

			if (m_missionFailedUI >= 100) {
				D2D_POINT_2F D2_MissionFailedTextUI = { FRAME_BUFFER_WIDTH / 2 - 345.0f, FRAME_BUFFER_HEIGHT / 2 - 170.0f };
				D2D_RECT_F D2_MissionFailedTextUIRect = { 0.0f, 0.0f, 690.0f, 176.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[78], &D2_MissionFailedTextUI, &D2_MissionFailedTextUIRect);

				D2D_POINT_2F D2_TimeOverUI = { FRAME_BUFFER_WIDTH / 2 - 314.0f, FRAME_BUFFER_HEIGHT / 2 + 10.0f };
				D2D_RECT_F D2_TimeOverUIRect = { 0.0f, 0.0f, 628.0f, 175.0f };
				m_pd2dDeviceContext->DrawImage(m_pd2dfxGaussianBlur[79], &D2_TimeOverUI, &D2_TimeOverUIRect);
			}
			else {
				m_missionFailedUI += 2.5f;
				if (m_missionFailedUI >= 100) {
					m_missionFailedUI = 100.0f;
				}
			}
		}
	}

#endif
	if (m_nMode == OPENINGSCENE) {
		if (m_LoginScene == 0) {
			D2D_RECT_F D2_LoginIDText = D2D1::RectF((FRAME_BUFFER_WIDTH / 4.2), (FRAME_BUFFER_HEIGHT / 1.63f), (FRAME_BUFFER_WIDTH / 2.8), (FRAME_BUFFER_HEIGHT / 1.63f));
			m_pd2dDeviceContext->DrawTextW(m_LoginID, (UINT32)wcslen(m_LoginID), m_pdwFont[1], &D2_LoginIDText, m_pd2dbrText[1]);

			D2D_RECT_F D2_LoginPWText = D2D1::RectF((FRAME_BUFFER_WIDTH / 4.2), (FRAME_BUFFER_HEIGHT / 1.43f), (FRAME_BUFFER_WIDTH / 2.8), (FRAME_BUFFER_HEIGHT / 1.43f));
			m_pd2dDeviceContext->DrawTextW(m_LoginPW, (UINT32)wcslen(m_LoginPW), m_pdwFont[1], &D2_LoginPWText, m_pd2dbrText[1]);
		}
		else if (m_LoginScene == 2) {
			D2D_RECT_F D2_LoginRoomNumText[8];
			D2D_RECT_F D2_LoginRoomNameText[8];
			for (int i{}; i < m_LobbyRoom_Info.size(); ++i) {
				int resultY = 420 + 55 * i;
				float textypos = (((float)FRAME_BUFFER_HEIGHT) / ((float)resultY));

				wchar_t roomnum1[20];
				_itow_s(m_LobbyRoom_Info[i].num, roomnum1, 20, 10);

				D2_LoginRoomNumText[i] = D2D1::RectF((FRAME_BUFFER_WIDTH / 5.2), (FRAME_BUFFER_HEIGHT / textypos), (FRAME_BUFFER_WIDTH / 4.0), (FRAME_BUFFER_HEIGHT / textypos)); //2.7, 2.37, 2.12, 1.92
				m_pd2dDeviceContext->DrawTextW(roomnum1, (UINT32)wcslen(roomnum1), m_pdwFont[2], &D2_LoginRoomNumText[i], m_pd2dbrText[2]);

				D2_LoginRoomNameText[i] = D2D1::RectF((FRAME_BUFFER_WIDTH / 4.1), (FRAME_BUFFER_HEIGHT / textypos), (FRAME_BUFFER_WIDTH / 1.61), (FRAME_BUFFER_HEIGHT / textypos));
				m_pd2dDeviceContext->DrawTextW(m_LobbyRoom_Info[i].name, (UINT32)wcslen(m_LobbyRoom_Info[i].name), m_pdwFont[2], &D2_LoginRoomNameText[i], m_pd2dbrText[2]);
			}
		}
		else if (m_LoginScene == 3) {
			wchar_t roomnum[20];
			_itow_s(m_myRoomNum, roomnum, 20, 10);
			D2D_RECT_F D2_RoomnumText = D2D1::RectF((FRAME_BUFFER_WIDTH * 0.22f), (FRAME_BUFFER_HEIGHT * 0.3f), (FRAME_BUFFER_WIDTH * 0.25f), (FRAME_BUFFER_HEIGHT * 0.35f));
			m_pd2dDeviceContext->DrawTextW(roomnum, (UINT32)wcslen(roomnum), m_pdwFont[2], &D2_RoomnumText, m_pd2dbrText[2]);

			D2D_RECT_F D2_RoomNameText = D2D1::RectF((FRAME_BUFFER_WIDTH * 0.22f), (FRAME_BUFFER_HEIGHT * 0.4f), (FRAME_BUFFER_WIDTH * 0.5f), (FRAME_BUFFER_HEIGHT * 0.4f));
			m_pd2dDeviceContext->DrawTextW(currRoomName, (UINT32)wcslen(currRoomName), m_pdwFont[5], &D2_RoomNameText, m_pd2dbrText[5]);

			D2D_RECT_F D2_UserNameText[3];
			for (int i{}; i < m_MAX_USER; ++i) {
				int resultY = 590 + 60 * i;
				float textypos = (((float)FRAME_BUFFER_HEIGHT) / ((float)resultY));

				//wchar_t roomnum_tmp[20];
				//_itow_s(m_LobbyRoom_Info[i].num, roomnum_tmp, 20, 10);

				D2_UserNameText[i] = D2D1::RectF((FRAME_BUFFER_WIDTH * 0.22f), (FRAME_BUFFER_HEIGHT / textypos), (FRAME_BUFFER_WIDTH * 0.5f), (FRAME_BUFFER_HEIGHT / textypos));
				m_pd2dDeviceContext->DrawTextW(m_MyRoom_Info[i].User_name, (UINT32)wcslen(m_MyRoom_Info[i].User_name), m_pdwFont[2], &D2_UserNameText[i], m_pd2dbrText[2]);
			}

		}
		else if (m_LoginScene == 4) {
			D2D_RECT_F D2_CreateRoomText = D2D1::RectF((FRAME_BUFFER_WIDTH * 0.34), (FRAME_BUFFER_HEIGHT * 0.44f), (FRAME_BUFFER_WIDTH * 0.6), (FRAME_BUFFER_HEIGHT * 0.54f));
			m_pd2dDeviceContext->DrawTextW(createRoomName, (UINT32)wcslen(createRoomName), m_pdwFont[4], &D2_CreateRoomText, m_pd2dbrText[4]);
		}
	}
	else if (m_nMode == SCENE1STAGE) {
		D2D1_RECT_F D2_RemainNPCText = D2D1::RectF((FRAME_BUFFER_WIDTH / 64) * 57, 0.0f, (FRAME_BUFFER_WIDTH / 16) * 15, (FRAME_BUFFER_HEIGHT / 16) * 1);
		m_pd2dDeviceContext->DrawTextW(m_remainNPCPrint, (UINT32)wcslen(m_remainNPCPrint), m_pdwFont[0], &D2_RemainNPCText, m_pd2dbrText[0]);

		switch (m_mainmissionnum)
		{
		case 0:
		{
			int killNPC = m_Max_NPCs - m_remainNPC;
			_itow_s(killNPC, killNPCprint, sizeof(killNPCprint), 10);
			D2D1_RECT_F D2_KillNPCText = D2D1::RectF((FRAME_BUFFER_WIDTH / 64) * 59, (FRAME_BUFFER_HEIGHT / 16) * 3, (FRAME_BUFFER_WIDTH / 64) * 61, (FRAME_BUFFER_HEIGHT / 16) * 3);
			m_pd2dDeviceContext->DrawTextW(killNPCprint, (UINT32)wcslen(killNPCprint), m_pdwFont[0], &D2_KillNPCText, m_pd2dbrText[0]);
		}
		break;
		case 1:
		{
			_itow_s(m_occupationnum, occupationPrint, sizeof(occupationPrint), 10);
			D2D1_RECT_F D2_OccupationText = D2D1::RectF((FRAME_BUFFER_WIDTH / 64) * 57, (FRAME_BUFFER_HEIGHT / 16) * 3, (FRAME_BUFFER_WIDTH / 64) * 59, (FRAME_BUFFER_HEIGHT / 16) * 3);
			m_pd2dDeviceContext->DrawTextW(occupationPrint, (UINT32)wcslen(occupationPrint), m_pdwFont[0], &D2_OccupationText, m_pd2dbrText[0]);
		}
		break;
		}

		if (UI_Switch) {
			D2D_RECT_F D2_ChatLogText[10];
			for (int i{}; i < m_chat_info.size(); ++i) {
				int resultY = 430 + 24 * i;
				float textypos = (((float)FRAME_BUFFER_HEIGHT) / ((float)resultY));
				ChatInfo temp;
				wcsncpy_s(temp.chatData, m_chat_info.front().chatData, _TRUNCATE);
				m_chat_info.pop();

				D2_ChatLogText[i] = D2D1::RectF((FRAME_BUFFER_WIDTH * 0.6f), (FRAME_BUFFER_HEIGHT / textypos), FRAME_BUFFER_WIDTH, (FRAME_BUFFER_HEIGHT / textypos)); //2.7, 2.37, 2.12, 1.92
				m_pd2dDeviceContext->DrawTextW(temp.chatData, (UINT32)wcslen(temp.chatData), m_pdwFont[3], &D2_ChatLogText[i], m_pd2dbrText[3]);
				m_chat_info.push(temp);
			}
			D2D_RECT_F D2_ChatInsertText = D2D1::RectF((FRAME_BUFFER_WIDTH * 0.6f), FRAME_BUFFER_HEIGHT * 0.67f, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT * 0.69f);
			m_pd2dDeviceContext->DrawTextW(m_InsertChat, (UINT32)wcslen(m_InsertChat), m_pdwFont[3], &D2_ChatInsertText, m_pd2dbrText[3]);
		}
		/*if (UI_Switch) {
			switch (m_submissionnum)
			{
			case 0:
			{
				_itow_s(m_survive, SurviveSecPrint, sizeof(SurviveSecPrint), 10);
				D2D1_RECT_F D2_Survive30sText = D2D1::RectF((FRAME_BUFFER_WIDTH / 32) * 29, (FRAME_BUFFER_HEIGHT / 64) * 25, (FRAME_BUFFER_WIDTH / 16) * 15, (FRAME_BUFFER_HEIGHT / 64) * 25);
				m_pd2dDeviceContext->DrawTextW(SurviveSecPrint, (UINT32)wcslen(SurviveSecPrint), m_pdwFont[0], &D2_Survive30sText, m_pd2dbrText[0]);
			}
			break;
			case 1:
			{
				_itow_s(m_AttackFly, FlyAtkPrint, sizeof(FlyAtkPrint), 10);
				D2D1_RECT_F D2_FlyAttackText = D2D1::RectF((FRAME_BUFFER_WIDTH / 32) * 29, (FRAME_BUFFER_HEIGHT / 64) * 25, (FRAME_BUFFER_WIDTH / 16) * 15, (FRAME_BUFFER_HEIGHT / 64) * 25);
				m_pd2dDeviceContext->DrawTextW(FlyAtkPrint, (UINT32)wcslen(FlyAtkPrint), m_pdwFont[0], &D2_FlyAttackText, m_pd2dbrText[0]);
			}
			break;
			case 2:
			{
				_itow_s(m_killArmy, KillArmyPrint, sizeof(KillArmyPrint), 10);
				D2D1_RECT_F D2_ExecutionText = D2D1::RectF((FRAME_BUFFER_WIDTH / 32) * 29, (FRAME_BUFFER_HEIGHT / 64) * 25, (FRAME_BUFFER_WIDTH / 16) * 15, (FRAME_BUFFER_HEIGHT / 64) * 25);
				m_pd2dDeviceContext->DrawTextW(KillArmyPrint, (UINT32)wcslen(KillArmyPrint), m_pdwFont[0], &D2_ExecutionText, m_pd2dbrText[0]);
			}
			break;
			}
		}*/

		if (m_CurrentPlayerNum > 1) {
			D2D1_RECT_F Friend1Text = D2D1::RectF((FRAME_BUFFER_WIDTH / 64) * 1, (FRAME_BUFFER_HEIGHT / 64) * 42, (FRAME_BUFFER_WIDTH / 64) * 7, (FRAME_BUFFER_HEIGHT / 64) * 42);
			m_pd2dDeviceContext->DrawTextW(m_OtherName[0], (UINT32)wcslen(m_OtherName[0]), m_pdwFont[0], &Friend1Text, m_pd2dbrText[0]);
		}

		if (m_CurrentPlayerNum > 2) {
			D2D1_RECT_F Friend2Text = D2D1::RectF((FRAME_BUFFER_WIDTH / 64) * 1, (FRAME_BUFFER_HEIGHT / 128) * 93, (FRAME_BUFFER_WIDTH / 64) * 7, (FRAME_BUFFER_HEIGHT / 128) * 93);
			m_pd2dDeviceContext->DrawTextW(m_OtherName[1], (UINT32)wcslen(m_OtherName[1]), m_pdwFont[0], &Friend2Text, m_pd2dbrText[0]);
		}
	}

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

	if (m_nMode == SCENE1STAGE)
	{
		m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
		size_t nLength = _tcslen(m_pszFrameRate);
		XMFLOAT3 xmf3Position = ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition();
		_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%5.1f, %5.1f, %5.1f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
		::SetWindowText(m_hWnd, m_pszFrameRate);
	}
	}

void CGameFramework::ChangeScene(DWORD nMode)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);
	if (nMode != m_nMode)
	{
		ReleaseObjects();
		m_pd3dCommandAllocator->Reset();
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
		switch (nMode)
		{
		case SCENE1STAGE:
		{
			m_nMode = nMode;
			m_pScene = new Stage1();
			if (m_pScene) ((Stage1*)m_pScene)->BuildObjects(m_pd3dDevice, m_pd3dCommandList, d3dRtvCPUDescriptorHandle, m_pd3dDepthStencilBuffer);
			if (m_ingame_role == R_RIFLE)
			{
				m_pScene->m_pPlayer = ((CHumanPlayer*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[1]);
				m_pCamera = ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetCamera();
				((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->HeliPitch = false;
				((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SoldiarPitch = true;
			}
			if (m_ingame_role == R_HELI)
			{
				m_pScene->m_pPlayer = ((HeliPlayer*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[43]);
				m_pCamera = ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetCamera();
				gamesound.HelicopterLoop();
				((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->HeliPitch = true;
				((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SoldiarPitch = false;
			}
			else
			{
				m_pScene->m_pPlayer = ((CHumanPlayer*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[1]);
				m_pCamera = ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetCamera();
			}
			m_pScene->SetCurScene(SCENE1STAGE);
			m_pd3dCommandList->Close();
			ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
			m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

			WaitForGpuComplete();

			gamesound.SpeakMusic();

			gamesound.pauseOpeningSound();
			if (m_pScene) m_pScene->ReleaseUploadBuffers();
			if (m_pScene->m_pPlayer)m_pScene->m_pPlayer->ReleaseUploadBuffers();
			if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
			m_GameTimer.Reset();
			break;
		}
		case SCENE2STAGE:
		{
			gamesound.m_bStopSound = true;
			gamesound.SpeakMusic();
			gamesound.speakChannel->setVolume(0.0f);
			m_nMode = nMode;
			m_pScene = new Stage2();
			if (m_pScene) ((Stage2*)m_pScene)->BuildObjects(m_pd3dDevice, m_pd3dCommandList, d3dRtvCPUDescriptorHandle, m_pd3dDepthStencilBuffer);
			CHumanPlayer* pPlayer = new CHumanPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL, ((Stage2*)m_pScene)->m_pTerrain);
			m_pScene->m_pPlayer = m_pPlayer = pPlayer;
			m_pCamera = m_pPlayer->GetCamera();
			m_pScene->SetCurScene(SCENE2STAGE);

			m_pd3dCommandList->Close();
			ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
			m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

			WaitForGpuComplete();

			if (m_pScene) m_pScene->ReleaseUploadBuffers();
			if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
			m_GameTimer.Reset();
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

	hResult = m_pdWriteFactory->CreateTextFormat(L"NanumSquare_acEB.ttf", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 35.0f, L"ko-kr", &m_pdwFont[0]);
	hResult = m_pdwFont[0]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	hResult = m_pdwFont[0]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 5.0f), &m_pd2dbrText[0]);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont[0], 1024, 1024, &m_pdwTextLayout[0]);

	hResult = m_pdWriteFactory->CreateTextFormat(L"NanumSquare_acEB.ttf", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"ko-kr", &m_pdwFont[1]);
	hResult = m_pdwFont[1]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pdwFont[1]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 5.0f), &m_pd2dbrText[1]);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont[1], 1024, 1024, &m_pdwTextLayout[1]);

	hResult = m_pdWriteFactory->CreateTextFormat(L"NanumSquare_acEB.ttf", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"ko-kr", &m_pdwFont[2]);
	hResult = m_pdwFont[2]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pdwFont[2]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 5.0f), &m_pd2dbrText[2]);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont[2], 1024, 1024, &m_pdwTextLayout[2]);

	hResult = m_pdWriteFactory->CreateTextFormat(L"NanumSquare_acEB.ttf", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"ko-kr", &m_pdwFont[3]);
	hResult = m_pdwFont[3]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pdwFont[3]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 5.0f), &m_pd2dbrText[3]);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont[3], 1024, 1024, &m_pdwTextLayout[3]);

	hResult = m_pdWriteFactory->CreateTextFormat(L"NanumSquare_acEB.ttf", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 36.0f, L"ko-kr", &m_pdwFont[4]);
	hResult = m_pdwFont[4]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pdwFont[4]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 5.0f), &m_pd2dbrText[4]);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont[4], 1024, 1024, &m_pdwTextLayout[4]);

	hResult = m_pdWriteFactory->CreateTextFormat(L"NanumSquare_acEB.ttf", NULL, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 36.0f, L"ko-kr", &m_pdwFont[5]);
	hResult = m_pdwFont[5]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	hResult = m_pdwFont[5]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 5.0f), &m_pd2dbrText[5]);
	hResult = m_pdWriteFactory->CreateTextLayout(L"텍스트 레이아웃", 6, m_pdwFont[5], 1024, 1024, &m_pdwTextLayout[5]);


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

	for (int i{}; i < m_NumOfUI; ++i) {
		hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource[i]);
		hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pd2dfxGaussianBlur[i]);
		hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1EdgeDetection, &m_pd2dfxEdgeDetection[i]);
	}

	// Time

	IWICBitmapDecoder* pwicBitmapDecoder;
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RemainTime.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);	IWICBitmapFrameDecode* pwicFrameDecode;
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[0]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[0]->SetInputEffect(0, m_pd2dfxBitmapSource[0]);
	m_pd2dfxGaussianBlur[0]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[0]->SetInputEffect(0, m_pd2dfxBitmapSource[0]);
	m_pd2dfxEdgeDetection[0]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[0]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[0]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[0]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[0]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/TimenumberDot.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[1]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[1]->SetInputEffect(0, m_pd2dfxBitmapSource[1]);
	m_pd2dfxGaussianBlur[1]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[1]->SetInputEffect(0, m_pd2dfxBitmapSource[1]);
	m_pd2dfxEdgeDetection[1]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[1]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[1]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[1]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[1]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Timenumber.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[2]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[2]->SetInputEffect(0, m_pd2dfxBitmapSource[2]);
	m_pd2dfxGaussianBlur[2]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[2]->SetInputEffect(0, m_pd2dfxBitmapSource[2]);
	m_pd2dfxEdgeDetection[2]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[2]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[2]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[2]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[2]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	m_pd2dfxBitmapSource[3]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	m_pd2dfxGaussianBlur[3]->SetInputEffect(0, m_pd2dfxBitmapSource[3]);
	m_pd2dfxGaussianBlur[3]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[3]->SetInputEffect(0, m_pd2dfxBitmapSource[3]);
	m_pd2dfxEdgeDetection[3]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[3]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[3]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[3]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[3]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	m_pd2dfxBitmapSource[4]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	m_pd2dfxGaussianBlur[4]->SetInputEffect(0, m_pd2dfxBitmapSource[4]);
	m_pd2dfxGaussianBlur[4]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[4]->SetInputEffect(0, m_pd2dfxBitmapSource[4]);
	m_pd2dfxEdgeDetection[4]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[4]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[4]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[4]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[4]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	m_pd2dfxBitmapSource[5]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	m_pd2dfxGaussianBlur[5]->SetInputEffect(0, m_pd2dfxBitmapSource[5]);
	m_pd2dfxGaussianBlur[5]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[5]->SetInputEffect(0, m_pd2dfxBitmapSource[5]);
	m_pd2dfxEdgeDetection[5]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[5]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[5]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[5]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[5]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Progress
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Progress.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[6]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[6]->SetInputEffect(0, m_pd2dfxBitmapSource[6]);
	m_pd2dfxGaussianBlur[6]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[6]->SetInputEffect(0, m_pd2dfxBitmapSource[6]);
	m_pd2dfxEdgeDetection[6]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[6]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[6]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[6]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[6]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);


	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/ProgressBG.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[7]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[7]->SetInputEffect(0, m_pd2dfxBitmapSource[7]);
	m_pd2dfxGaussianBlur[7]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[7]->SetInputEffect(0, m_pd2dfxBitmapSource[7]);
	m_pd2dfxEdgeDetection[7]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[7]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[7]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[7]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[7]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/ProgressCurrent.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[8]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[8]->SetInputEffect(0, m_pd2dfxBitmapSource[8]);
	m_pd2dfxGaussianBlur[8]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[8]->SetInputEffect(0, m_pd2dfxBitmapSource[8]);
	m_pd2dfxEdgeDetection[8]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[8]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[8]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[8]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[8]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Compress
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/CurrentCompress.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[9]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[9]->SetInputEffect(0, m_pd2dfxBitmapSource[9]);
	m_pd2dfxGaussianBlur[9]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[9]->SetInputEffect(0, m_pd2dfxBitmapSource[9]);
	m_pd2dfxEdgeDetection[9]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[9]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[9]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[9]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[9]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/CompressUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[10]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[10]->SetInputEffect(0, m_pd2dfxBitmapSource[10]);
	m_pd2dfxGaussianBlur[10]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[10]->SetInputEffect(0, m_pd2dfxBitmapSource[10]);
	m_pd2dfxEdgeDetection[10]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[10]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[10]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[10]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[10]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// My_Info - bg
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/bullet_UIBG.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[11]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[11]->SetInputEffect(0, m_pd2dfxBitmapSource[11]);
	m_pd2dfxGaussianBlur[11]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[11]->SetInputEffect(0, m_pd2dfxBitmapSource[11]);
	m_pd2dfxEdgeDetection[11]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[11]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[11]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[11]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[11]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/UserHPBG.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[12]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[12]->SetInputEffect(0, m_pd2dfxBitmapSource[12]);
	m_pd2dfxGaussianBlur[12]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[12]->SetInputEffect(0, m_pd2dfxBitmapSource[12]);
	m_pd2dfxEdgeDetection[12]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[12]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[12]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[12]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[12]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// My_Info - bullet
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/bullet_icon.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[13]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[13]->SetInputEffect(0, m_pd2dfxBitmapSource[13]);
	m_pd2dfxGaussianBlur[13]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[13]->SetInputEffect(0, m_pd2dfxBitmapSource[13]);
	m_pd2dfxEdgeDetection[13]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[13]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[13]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[13]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[13]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/bullet_number.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[14]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[14]->SetInputEffect(0, m_pd2dfxBitmapSource[14]);
	m_pd2dfxGaussianBlur[14]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[14]->SetInputEffect(0, m_pd2dfxBitmapSource[14]);
	m_pd2dfxEdgeDetection[14]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[14]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[14]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[14]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[14]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	m_pd2dfxBitmapSource[15]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	m_pd2dfxGaussianBlur[15]->SetInputEffect(0, m_pd2dfxBitmapSource[15]);
	m_pd2dfxGaussianBlur[15]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[15]->SetInputEffect(0, m_pd2dfxBitmapSource[15]);
	m_pd2dfxEdgeDetection[15]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[15]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[15]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[15]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[15]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// My_Info - hp
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/UserHP.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[16]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[16]->SetInputEffect(0, m_pd2dfxBitmapSource[16]);
	m_pd2dfxGaussianBlur[16]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[16]->SetInputEffect(0, m_pd2dfxBitmapSource[16]);
	m_pd2dfxEdgeDetection[16]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[16]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[16]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[16]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[16]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Remain NPC
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RemainNPC.jpg", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[17]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[17]->SetInputEffect(0, m_pd2dfxBitmapSource[17]);
	m_pd2dfxGaussianBlur[17]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[17]->SetInputEffect(0, m_pd2dfxBitmapSource[17]);
	m_pd2dfxEdgeDetection[17]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[17]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[17]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[17]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[17]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Mission
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/MainMission.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[18]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[18]->SetInputEffect(0, m_pd2dfxBitmapSource[18]);
	m_pd2dfxGaussianBlur[18]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[18]->SetInputEffect(0, m_pd2dfxBitmapSource[18]);
	m_pd2dfxEdgeDetection[18]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[18]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[18]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[18]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[18]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/SuddenMission.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[19]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[19]->SetInputEffect(0, m_pd2dfxBitmapSource[19]);
	m_pd2dfxGaussianBlur[19]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[19]->SetInputEffect(0, m_pd2dfxBitmapSource[19]);
	m_pd2dfxEdgeDetection[19]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[19]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[19]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[19]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[19]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Cross hair
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/CrossHair2.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[20]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[20]->SetInputEffect(0, m_pd2dfxBitmapSource[20]);
	m_pd2dfxGaussianBlur[20]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[20]->SetInputEffect(0, m_pd2dfxBitmapSource[20]);
	m_pd2dfxEdgeDetection[20]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[20]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[20]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[20]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[20]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Team_Info
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/TeamUIBG.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[21]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[21]->SetInputEffect(0, m_pd2dfxBitmapSource[21]);
	m_pd2dfxGaussianBlur[21]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[21]->SetInputEffect(0, m_pd2dfxBitmapSource[21]);
	m_pd2dfxEdgeDetection[21]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[21]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[21]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[21]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[21]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	m_pd2dfxBitmapSource[22]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	m_pd2dfxGaussianBlur[22]->SetInputEffect(0, m_pd2dfxBitmapSource[22]);
	m_pd2dfxGaussianBlur[22]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[22]->SetInputEffect(0, m_pd2dfxBitmapSource[22]);
	m_pd2dfxEdgeDetection[22]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[22]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[22]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[22]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[22]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/TeamHP.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[23]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[23]->SetInputEffect(0, m_pd2dfxBitmapSource[23]);
	m_pd2dfxGaussianBlur[23]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[23]->SetInputEffect(0, m_pd2dfxBitmapSource[23]);
	m_pd2dfxEdgeDetection[23]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[23]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[23]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[23]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[23]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	m_pd2dfxBitmapSource[24]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	m_pd2dfxGaussianBlur[24]->SetInputEffect(0, m_pd2dfxBitmapSource[24]);
	m_pd2dfxGaussianBlur[24]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[24]->SetInputEffect(0, m_pd2dfxBitmapSource[24]);
	m_pd2dfxEdgeDetection[24]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[24]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[24]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[24]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[24]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Logo
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Logo.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	//hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Opening.jpg", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[25]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[25]->SetInputEffect(0, m_pd2dfxBitmapSource[25]);
	m_pd2dfxGaussianBlur[25]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[25]->SetInputEffect(0, m_pd2dfxBitmapSource[25]);
	m_pd2dfxEdgeDetection[25]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[25]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[25]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[25]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[25]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// LoginUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LoginUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[26]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[26]->SetInputEffect(0, m_pd2dfxBitmapSource[26]);
	m_pd2dfxGaussianBlur[26]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[26]->SetInputEffect(0, m_pd2dfxBitmapSource[26]);
	m_pd2dfxEdgeDetection[26]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[26]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[26]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[26]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[26]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LobbyUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[27]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[27]->SetInputEffect(0, m_pd2dfxBitmapSource[27]);
	m_pd2dfxGaussianBlur[27]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[27]->SetInputEffect(0, m_pd2dfxBitmapSource[27]);
	m_pd2dfxEdgeDetection[27]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[27]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[27]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[27]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[27]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby People UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LobbyPeopleUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[28]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[28]->SetInputEffect(0, m_pd2dfxBitmapSource[28]);
	m_pd2dfxGaussianBlur[28]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[28]->SetInputEffect(0, m_pd2dfxBitmapSource[28]);
	m_pd2dfxEdgeDetection[28]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[28]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[28]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[28]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[28]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	for (int i{}; i < 7; ++i) {
		m_pd2dfxBitmapSource[29 + i]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
		m_pd2dfxGaussianBlur[29 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[29 + i]);
		m_pd2dfxGaussianBlur[29 + i]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

		m_pd2dfxEdgeDetection[29 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[29 + i]);
		m_pd2dfxEdgeDetection[29 + i]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
		m_pd2dfxEdgeDetection[29 + i]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
		m_pd2dfxEdgeDetection[29 + i]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
		m_pd2dfxEdgeDetection[29 + i]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
		m_pd2dfxEdgeDetection[29 + i]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);
	}


	// Lobby Ready UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/ReadyLoobyUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[36]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[36]->SetInputEffect(0, m_pd2dfxBitmapSource[36]);
	m_pd2dfxGaussianBlur[36]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[36]->SetInputEffect(0, m_pd2dfxBitmapSource[36]);
	m_pd2dfxEdgeDetection[36]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[36]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[36]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[36]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[36]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	for (int i{}; i < 7; ++i) {
		m_pd2dfxBitmapSource[37 + i]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
		m_pd2dfxGaussianBlur[37 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[37 + i]);
		m_pd2dfxGaussianBlur[37 + i]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

		m_pd2dfxEdgeDetection[37 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[37 + i]);
		m_pd2dfxEdgeDetection[37 + i]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
		m_pd2dfxEdgeDetection[37 + i]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
		m_pd2dfxEdgeDetection[37 + i]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
		m_pd2dfxEdgeDetection[37 + i]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
		m_pd2dfxEdgeDetection[37 + i]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);
	}

	// Room UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RoomUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[44]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[44]->SetInputEffect(0, m_pd2dfxBitmapSource[44]);
	m_pd2dfxGaussianBlur[44]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[44]->SetInputEffect(0, m_pd2dfxBitmapSource[44]);
	m_pd2dfxEdgeDetection[44]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[44]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[44]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[44]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[44]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Room Start UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RoomStartUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[45]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[45]->SetInputEffect(0, m_pd2dfxBitmapSource[45]);
	m_pd2dfxGaussianBlur[45]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[45]->SetInputEffect(0, m_pd2dfxBitmapSource[45]);
	m_pd2dfxEdgeDetection[45]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[45]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[45]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[45]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[45]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Room Ready UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RoomReadyUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[46]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[46]->SetInputEffect(0, m_pd2dfxBitmapSource[46]);
	m_pd2dfxGaussianBlur[46]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[46]->SetInputEffect(0, m_pd2dfxBitmapSource[46]);
	m_pd2dfxEdgeDetection[46]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[46]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[46]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[46]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[46]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Room Ready Status UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RoomReadyStatusUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[47]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[47]->SetInputEffect(0, m_pd2dfxBitmapSource[47]);
	m_pd2dfxGaussianBlur[47]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[47]->SetInputEffect(0, m_pd2dfxBitmapSource[47]);
	m_pd2dfxEdgeDetection[47]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[47]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[47]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[47]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[47]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	for (int i{}; i < 2; ++i) {
		m_pd2dfxBitmapSource[48 + i]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
		m_pd2dfxGaussianBlur[48 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[48 + i]);
		m_pd2dfxGaussianBlur[48 + i]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

		m_pd2dfxEdgeDetection[48 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[48 + i]);
		m_pd2dfxEdgeDetection[48 + i]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
		m_pd2dfxEdgeDetection[48 + i]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
		m_pd2dfxEdgeDetection[48 + i]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
		m_pd2dfxEdgeDetection[48 + i]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
		m_pd2dfxEdgeDetection[48 + i]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);
	}

	// Room Ready Status UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/ChattingUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[50]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[50]->SetInputEffect(0, m_pd2dfxBitmapSource[50]);
	m_pd2dfxGaussianBlur[50]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[50]->SetInputEffect(0, m_pd2dfxBitmapSource[50]);
	m_pd2dfxEdgeDetection[50]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[50]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[50]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[50]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[50]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// InsertChattingUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/InsertChattingUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[51]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[51]->SetInputEffect(0, m_pd2dfxBitmapSource[51]);
	m_pd2dfxGaussianBlur[51]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[51]->SetInputEffect(0, m_pd2dfxBitmapSource[51]);
	m_pd2dfxEdgeDetection[51]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[51]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[51]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[51]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[51]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// SniperAim
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Hologram_2.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[52]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[52]->SetInputEffect(0, m_pd2dfxBitmapSource[52]);
	m_pd2dfxGaussianBlur[52]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[52]->SetInputEffect(0, m_pd2dfxBitmapSource[52]);
	m_pd2dfxEdgeDetection[52]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[52]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[52]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[52]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[52]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Create Room UI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/CreateRoomUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[53]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[53]->SetInputEffect(0, m_pd2dfxBitmapSource[53]);
	m_pd2dfxGaussianBlur[53]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[53]->SetInputEffect(0, m_pd2dfxBitmapSource[53]);
	m_pd2dfxEdgeDetection[53]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[53]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[53]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[53]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[53]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Game Start Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/GameLoginAfterStartUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[54]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[54]->SetInputEffect(0, m_pd2dfxBitmapSource[54]);
	m_pd2dfxGaussianBlur[54]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[54]->SetInputEffect(0, m_pd2dfxBitmapSource[54]);
	m_pd2dfxEdgeDetection[54]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[54]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[54]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[54]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[54]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Game Exit Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/GameLoginAfterExitUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[55]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[55]->SetInputEffect(0, m_pd2dfxBitmapSource[55]);
	m_pd2dfxGaussianBlur[55]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[55]->SetInputEffect(0, m_pd2dfxBitmapSource[55]);
	m_pd2dfxEdgeDetection[55]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[55]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[55]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[55]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[55]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby Game in Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LobbyRoomInUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[56]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[56]->SetInputEffect(0, m_pd2dfxBitmapSource[56]);
	m_pd2dfxGaussianBlur[56]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[56]->SetInputEffect(0, m_pd2dfxBitmapSource[56]);
	m_pd2dfxEdgeDetection[56]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[56]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[56]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[56]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[56]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby Game Create Room Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LobbyRoomCreateUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[57]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[57]->SetInputEffect(0, m_pd2dfxBitmapSource[57]);
	m_pd2dfxGaussianBlur[57]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[57]->SetInputEffect(0, m_pd2dfxBitmapSource[57]);
	m_pd2dfxEdgeDetection[57]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[57]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[57]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[57]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[57]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby Game Left Page Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LeftPageButtonNonActiveUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[58]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[58]->SetInputEffect(0, m_pd2dfxBitmapSource[58]);
	m_pd2dfxGaussianBlur[58]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[58]->SetInputEffect(0, m_pd2dfxBitmapSource[58]);
	m_pd2dfxEdgeDetection[58]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[58]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[58]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[58]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[58]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby Game Left Page Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LeftPageButtonActiveUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[59]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[59]->SetInputEffect(0, m_pd2dfxBitmapSource[59]);
	m_pd2dfxGaussianBlur[59]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[59]->SetInputEffect(0, m_pd2dfxBitmapSource[59]);
	m_pd2dfxEdgeDetection[59]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[59]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[59]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[59]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[59]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby Game Right Page Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RightPageButtonNonActiveUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[60]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[60]->SetInputEffect(0, m_pd2dfxBitmapSource[60]);
	m_pd2dfxGaussianBlur[60]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[60]->SetInputEffect(0, m_pd2dfxBitmapSource[60]);
	m_pd2dfxEdgeDetection[60]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[60]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[60]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[60]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[60]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby Game Right Page Button
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RightPageButtonActiveUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[61]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[61]->SetInputEffect(0, m_pd2dfxBitmapSource[61]);
	m_pd2dfxGaussianBlur[61]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[61]->SetInputEffect(0, m_pd2dfxBitmapSource[61]);
	m_pd2dfxEdgeDetection[61]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[61]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[61]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[61]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[61]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// job Choice - Army
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Job_Army.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[62]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[62]->SetInputEffect(0, m_pd2dfxBitmapSource[62]);
	m_pd2dfxGaussianBlur[62]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[62]->SetInputEffect(0, m_pd2dfxBitmapSource[62]);
	m_pd2dfxEdgeDetection[62]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[62]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[62]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[62]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[62]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	for (int i{}; i < 2; ++i) {
		m_pd2dfxBitmapSource[63 + i]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
		m_pd2dfxGaussianBlur[63 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[63 + i]);
		m_pd2dfxGaussianBlur[63 + i]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

		m_pd2dfxEdgeDetection[63 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[63 + i]);
		m_pd2dfxEdgeDetection[63 + i]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
		m_pd2dfxEdgeDetection[63 + i]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
		m_pd2dfxEdgeDetection[63 + i]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
		m_pd2dfxEdgeDetection[63 + i]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
		m_pd2dfxEdgeDetection[63 + i]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);
	}

	// job Choice - Helicopter
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Job_Helicopter.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[65]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[65]->SetInputEffect(0, m_pd2dfxBitmapSource[65]);
	m_pd2dfxGaussianBlur[65]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[65]->SetInputEffect(0, m_pd2dfxBitmapSource[65]);
	m_pd2dfxEdgeDetection[65]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[65]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[65]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[65]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[65]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	for (int i{}; i < 2; ++i) {
		m_pd2dfxBitmapSource[66 + i]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
		m_pd2dfxGaussianBlur[66 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[66 + i]);
		m_pd2dfxGaussianBlur[66 + i]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

		m_pd2dfxEdgeDetection[66 + i]->SetInputEffect(0, m_pd2dfxBitmapSource[66 + i]);
		m_pd2dfxEdgeDetection[66 + i]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
		m_pd2dfxEdgeDetection[66 + i]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
		m_pd2dfxEdgeDetection[66 + i]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
		m_pd2dfxEdgeDetection[66 + i]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
		m_pd2dfxEdgeDetection[66 + i]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);
	}

	// loginButtonUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/loginButtonUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[68]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[68]->SetInputEffect(0, m_pd2dfxBitmapSource[68]);
	m_pd2dfxGaussianBlur[68]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[68]->SetInputEffect(0, m_pd2dfxBitmapSource[68]);
	m_pd2dfxEdgeDetection[68]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[68]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[68]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[68]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[68]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// RegistButtonUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/RegistButtonUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[69]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);

	m_pd2dfxGaussianBlur[69]->SetInputEffect(0, m_pd2dfxBitmapSource[69]);
	m_pd2dfxGaussianBlur[69]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);

	m_pd2dfxEdgeDetection[69]->SetInputEffect(0, m_pd2dfxBitmapSource[69]);
	m_pd2dfxEdgeDetection[69]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[69]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[69]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[69]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[69]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// HittingSplatter
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/BloodSplatter51.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[70]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[70]->SetInputEffect(0, m_pd2dfxBitmapSource[70]);
	m_pd2dfxGaussianBlur[70]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[70]->SetInputEffect(0, m_pd2dfxBitmapSource[70]);
	m_pd2dfxEdgeDetection[70]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[70]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[70]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[70]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[70]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// HittingSplatter
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/Respawning_BannerUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[71]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[71]->SetInputEffect(0, m_pd2dfxBitmapSource[71]);
	m_pd2dfxGaussianBlur[71]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[71]->SetInputEffect(0, m_pd2dfxBitmapSource[71]);
	m_pd2dfxEdgeDetection[71]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[71]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[71]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[71]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[71]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// HelicopterWarnningBanner
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/HelicopterWarnning_Banner.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[72]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[72]->SetInputEffect(0, m_pd2dfxBitmapSource[72]);
	m_pd2dfxGaussianBlur[72]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[72]->SetInputEffect(0, m_pd2dfxBitmapSource[72]);
	m_pd2dfxEdgeDetection[72]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[72]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[72]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[72]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[72]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// HelicopterWarnningIcon
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/HelicopterWarnning.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[73]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[73]->SetInputEffect(0, m_pd2dfxBitmapSource[73]);
	m_pd2dfxGaussianBlur[73]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[73]->SetInputEffect(0, m_pd2dfxBitmapSource[73]);
	m_pd2dfxEdgeDetection[73]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[73]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[73]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[73]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[73]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear Background
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/ClearBackGroundUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[74]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[74]->SetInputEffect(0, m_pd2dfxBitmapSource[74]);
	m_pd2dfxGaussianBlur[74]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[74]->SetInputEffect(0, m_pd2dfxBitmapSource[74]);
	m_pd2dfxEdgeDetection[74]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[74]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[74]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[74]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[74]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear MissionClearTextUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/MissionClearTextUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[75]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[75]->SetInputEffect(0, m_pd2dfxBitmapSource[75]);
	m_pd2dfxGaussianBlur[75]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[75]->SetInputEffect(0, m_pd2dfxBitmapSource[75]);
	m_pd2dfxEdgeDetection[75]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[75]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[75]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[75]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[75]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear MissionClearTextUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/CongratulationUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[76]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[76]->SetInputEffect(0, m_pd2dfxBitmapSource[76]);
	m_pd2dfxGaussianBlur[76]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[76]->SetInputEffect(0, m_pd2dfxBitmapSource[76]);
	m_pd2dfxEdgeDetection[76]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[76]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[76]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[76]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[76]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);
	if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
	if (pwicFrameDecode) pwicFrameDecode->Release();

	// Fail Background
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/FailedBackGroundUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[77]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[77]->SetInputEffect(0, m_pd2dfxBitmapSource[77]);
	m_pd2dfxGaussianBlur[77]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[77]->SetInputEffect(0, m_pd2dfxBitmapSource[77]);
	m_pd2dfxEdgeDetection[77]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[77]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[77]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[77]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[77]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear MissionClearTextUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/MissionFailedUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[78]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[78]->SetInputEffect(0, m_pd2dfxBitmapSource[78]);
	m_pd2dfxGaussianBlur[78]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[78]->SetInputEffect(0, m_pd2dfxBitmapSource[78]);
	m_pd2dfxEdgeDetection[78]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[78]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[78]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[78]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[78]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear MissionClearTextUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/TimeOverTextUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[79]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[79]->SetInputEffect(0, m_pd2dfxBitmapSource[79]);
	m_pd2dfxGaussianBlur[79]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[79]->SetInputEffect(0, m_pd2dfxBitmapSource[79]);
	m_pd2dfxEdgeDetection[79]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[79]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[79]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[79]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[79]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Fail Background
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/info_InGameLoadingUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[80]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[80]->SetInputEffect(0, m_pd2dfxBitmapSource[80]);
	m_pd2dfxGaussianBlur[80]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[80]->SetInputEffect(0, m_pd2dfxBitmapSource[80]);
	m_pd2dfxEdgeDetection[80]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[80]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[80]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[80]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[80]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear MissionClearTextUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/info_NotReadyAllPlayerUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[81]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[81]->SetInputEffect(0, m_pd2dfxBitmapSource[81]);
	m_pd2dfxGaussianBlur[81]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[81]->SetInputEffect(0, m_pd2dfxBitmapSource[81]);
	m_pd2dfxEdgeDetection[81]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[81]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[81]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[81]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[81]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Clear MissionClearTextUI
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/info_PressReadybeforeChooseUI.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[82]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[82]->SetInputEffect(0, m_pd2dfxBitmapSource[82]);
	m_pd2dfxGaussianBlur[82]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[82]->SetInputEffect(0, m_pd2dfxBitmapSource[82]);
	m_pd2dfxEdgeDetection[82]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[82]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[82]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[82]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[82]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	// Lobby My Mark
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"UI/XDUI/LobbyMymark.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource[83]->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	m_pd2dfxGaussianBlur[83]->SetInputEffect(0, m_pd2dfxBitmapSource[83]);
	m_pd2dfxGaussianBlur[83]->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.0f);
	m_pd2dfxEdgeDetection[83]->SetInputEffect(0, m_pd2dfxBitmapSource[83]);
	m_pd2dfxEdgeDetection[83]->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.5f);
	m_pd2dfxEdgeDetection[83]->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 0.0f);
	m_pd2dfxEdgeDetection[83]->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_SOBEL);
	m_pd2dfxEdgeDetection[83]->SetValue(D2D1_EDGEDETECTION_PROP_OVERLAY_EDGES, false);
	m_pd2dfxEdgeDetection[83]->SetValue(D2D1_EDGEDETECTION_PROP_ALPHA_MODE, D2D1_ALPHA_MODE_PREMULTIPLIED);

	if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
	if (pwicFrameDecode) pwicFrameDecode->Release();
#endif
}
#endif


//==================================================
//			  서버 통신에 필요한 함수들
//==================================================
bool CGameFramework::checkNewInput_Keyboard()
{
	if (!q_keyboardInput.empty()) {
		return true;
	}
	return false;
}
bool CGameFramework::checkNewInput_Mouse()
{
	if (!q_mouseInput.empty()) {
		return true;
	}
	return false;
}
short CGameFramework::popInputVal_Keyboard()
{
	if (!q_keyboardInput.empty()) {
		short val = q_keyboardInput.front();
		q_keyboardInput.pop();
		return val;
	}
	return -1;
}
MouseInputVal CGameFramework::popInputVal_Mouse()
{
	MouseInputVal val;
	if (!q_mouseInput.empty()) {
		val = q_mouseInput.front();
		q_mouseInput.pop();
		return val;
	}
	val.button = -1;
	return val;
}

XMFLOAT3 CGameFramework::getMyPosition()
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE) return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition();
		if (m_ingame_role == R_HELI) return ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition();
		else return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition();
	}

}
XMFLOAT3 CGameFramework::getMyRightVec()
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE) return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetRightVector();
		if (m_ingame_role == R_HELI) return ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetRightVector();
		else return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetRightVector();
	}

}
XMFLOAT3 CGameFramework::getMyUpVec()
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE) return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetUpVector();
		if (m_ingame_role == R_HELI) return ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetUpVector();
		else return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetUpVector();
	}

}
XMFLOAT3 CGameFramework::getMyLookVec()
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE) return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetLookVector();
		if (m_ingame_role == R_HELI) return ((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetLookVector();
		else return ((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetLookVector();
	}

}
XMFLOAT3 CGameFramework::getMyCameraLookVec()
{
	if (m_nMode == SCENE1STAGE) {
		return m_pCamera->GetLookVector();
	}

}
void CGameFramework::setPosition_Self(XMFLOAT3 pos)
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE)
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetPosition(pos);
		if (m_ingame_role == R_HELI)
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetPosition(pos);
		else
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetPosition(pos);
	}
}
void CGameFramework::setVectors_Self(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec)
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE)
		{
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetUp(upVec);
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetRight(rightVec);
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetLook(lookVec);
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetScale(XMFLOAT3(4.0, 4.0, 4.0));
		}
		if (m_ingame_role == R_HELI)
		{
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetUp(upVec);
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetRight(rightVec);
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetLook(lookVec);
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetScale(XMFLOAT3(4.0, 4.0, 4.0));
		}
		else
		{
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetUp(upVec);
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetRight(rightVec);
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetLook(lookVec);
			((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->SetScale(XMFLOAT3(4.0, 4.0, 4.0));
		}
	}

}
void CGameFramework::setPosition_SoldiarOtherPlayer(int id, XMFLOAT3 pos)
{
	if (m_nMode == SCENE1STAGE) {
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->SetPosition(pos);
	}
}
void CGameFramework::setVectors_SoldiarOtherPlayer(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec)
{
	if (m_nMode == SCENE1STAGE) {

		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4+id])->SetRight(rightVec);
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4+id])->SetUp(upVec);
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4+id])->SetLook(lookVec);
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4+id])->SetScale(5.0, 5.0, 5.0);

	}
}
void CGameFramework::setPosition_HeliOtherPlayer(XMFLOAT3 pos)
{
	if (m_nMode == SCENE1STAGE)
	{
		((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->SetPosition(pos);
	}

}
void CGameFramework::setVectors_HeliOtherPlayer(XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec)
{
	if (m_nMode == SCENE1STAGE)
	{
		((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->SetRight(rightVec);
		((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->SetUp(upVec);
		((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->SetLook(lookVec);
		((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->SetScale(5.0, 5.0, 5.0);
	}
}
void CGameFramework::remove_OtherPlayer(int id)
{
	if (m_nMode == SCENE1STAGE) {
		if (m_ingame_role == R_RIFLE)
		{
			if (((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4+id]) {
				((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4+id]->SetScale(0.0, 0.0, 0.0);
			}
		}
		if (m_ingame_role == R_HELI)
		{
			if ((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7]) {
				((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->SetScale(0.0, 0.0, 0.0);
			}
		}
	}
}
void CGameFramework::setPosition_Npc(int id, XMFLOAT3 pos)
{
	/* 12~16 = 헬리콥터NPC , 22~41 = 사람NPC */
	if (m_nMode == SCENE1STAGE)
	{
		if (0 <= id && id < 5) {	// 헬기
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id]->SetPosition(pos);
		}
		else {	// 사람
			int indexnum = id - 5;	// id = 5 ~ 24, Object인덱스 = 22 ~ 41
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->SetPosition(pos);
		}
	}
}
void CGameFramework::setVectors_Npc(int id, XMFLOAT3 rightVec, XMFLOAT3 upVec, XMFLOAT3 lookVec)
{
	/* 12~16 = 헬리콥터NPC , 22~41 = 사람NPC */
	if (m_nMode == SCENE1STAGE)
	{
		if (0 <= id && id < 5) {	// 헬기
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id]->SetRight(rightVec);
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id]->SetUp(upVec);
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id]->SetLook(lookVec);
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id]->SetScale(3.0, 3.0, 3.0);
		}
		else {	// 사람
			int indexnum = id - 5;	// id = 5 ~ 24, Object인덱스 = 22 ~ 41
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->SetRight(rightVec);
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->SetUp(upVec);
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->SetLook(lookVec);
			((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->SetScale(5.0, 5.0, 5.0);
		}
	}
}
void CGameFramework::remove_Npcs(int id)
{
	//	if (id < 0 || id > 5) return;	// 배열 범위 벗어나는 거 방지
}

bool m_bFirstCollision = true;
bool m_bPrevCollisionCheck;

bool CGameFramework::CollisionMap_by_PLAYER(XMFLOAT3 pos, XMFLOAT3 extents, CGameObject* pTargetGameObject)
{
	pTargetGameObject->m_xoobb = BoundingOrientedBox(XMFLOAT3(m_pPlayer->GetPosition()), XMFLOAT3(5.0, 7.0, 5.0), XMFLOAT4(0, 0, 0, 1));
	m_mapxmoobb = BoundingOrientedBox(pos, XMFLOAT3(extents), XMFLOAT4(0, 0, 0, 1));

	if (m_mapxmoobb.Intersects(pTargetGameObject->m_xoobb))
	{
		return(true);
	}

	return(false);
}
void CGameFramework::CollisionMap_by_BULLET(XMFLOAT3 mappos)
{
	if (m_ingame_role == R_RIFLE)
	{
		((BulletMarkBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[4])->m_bActive = true;
		((BulletMarkBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[4])->ParticlePosition = (mappos);
	}
	if (m_ingame_role == R_HELI)
	{

		((CSpriteObjectsShader*)((Stage1*)m_pScene)->m_ppSpriteBillboard[0])->m_bActive = true;
		((CSpriteObjectsShader*)((Stage1*)m_pScene)->m_ppSpriteBillboard[0])->m_ppObjects[0]->SetPosition(mappos);
		((CHelicopterBulletMarkParticleShader*)((Stage1*)m_pScene)->m_ppFragShaders[1])->m_bActive = true;
		((CHelicopterBulletMarkParticleShader*)((Stage1*)m_pScene)->m_ppFragShaders[1])->ParticlePosition = mappos;
		((HeliHittingMarkBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[5])->m_bActive = true;
		((HeliHittingMarkBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[5])->ParticlePosition = mappos;

	}


}
void CGameFramework::CollisionNPC_by_PLAYER(XMFLOAT3 npcpos, XMFLOAT3 npcextents)
{
	m_pScene->m_pPlayer->m_xoobb = BoundingOrientedBox(XMFLOAT3(((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition()), XMFLOAT3(2.5, 2.0, 4.0), XMFLOAT4(0, 0, 0, 1));
	m_npcoobb = BoundingOrientedBox(npcpos, npcextents, XMFLOAT4(0, 0, 0, 1));

	if (m_npcoobb.Intersects(((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_xoobb))
	{
		// 충돌 모션
		//cout << "CollisionCheck!" << m_npcoobb.Center.x << m_npcoobb.Center.z << endl;
	}

}
void CGameFramework::CollisionNPC_by_MAP(XMFLOAT3 npcpos, XMFLOAT3 npcextents, XMFLOAT3 mapcenter, XMFLOAT3 mapextents)
{
	m_npcoobb = BoundingOrientedBox(npcpos, npcextents, XMFLOAT4(0, 0, 0, 1));
	m_mapxmoobb = BoundingOrientedBox(mapcenter, XMFLOAT3(mapextents.x / 2.5f, mapextents.y / 2.0f, mapextents.z / 2.5f), XMFLOAT4(0, 0, 0, 1));
	if (m_mapxmoobb.Intersects(m_npcoobb))
	{
		//cout << "CollisionCheck!" << m_npcoobb.Center.x << m_npcoobb.Center.z <<m_xmoobb.mapcenter.z endl;
	}

}
void CGameFramework::CollisionNPC_by_BULLET(XMFLOAT3 npcpos, XMFLOAT3 npcextents)
{
	m_npcoobb = BoundingOrientedBox(npcpos, npcextents, XMFLOAT4(0, 0, 0, 1));

}

void CGameFramework::otherPlayerReturnToIdle(int id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id]))
		{
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->IdleState(m_GameTimer.GetTimeElapsed());
		}
	}
}
void CGameFramework::otherPlayerForwardMotion(int id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id]))
		{
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->MoveForward(m_GameTimer.GetTimeElapsed());
		}
	}

}
void CGameFramework::otherPlayerBackwardMotion(int id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id]))
		{
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->MoveBackward(m_GameTimer.GetTimeElapsed());
		}
	}
}
void CGameFramework::otherPlayerSfrateMotion(int id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id]))
		{
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->MoveLeft(m_GameTimer.GetTimeElapsed());
		}
	}
}
void CGameFramework::otherPlayerShootingMotion(int id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id]))
		{
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->ShotState(m_GameTimer.GetTimeElapsed());
		}
	}
}

void CGameFramework::otherPlayerDyingMotion(int id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id]))
		{
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->m_pSkinnedAnimationController->m_pAnimationTracks->m_nType
				= ANIMATION_TYPE_ONCE;
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[4 + id])->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 8);
		}
	}
}

void CGameFramework::otherHeliPlayerDyingMotion()
{
	if (m_nMode == SCENE1STAGE)
	{
		if (((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7]))
		{
			((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7])->FallDown(m_GameTimer.GetTimeElapsed());
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[2])->m_pSkinnedAnimationController
				->m_pAnimationTracks->m_nType = ANIMATION_TYPE_ONCE;
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[2])->m_pSkinnedAnimationController
				->SetTrackAnimationSet(0, 8);
			((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[2])->SetPosition(XMFLOAT3(
				((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition().x,
				6.0f,
				((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition().z));
		}
	}
}

void CGameFramework::NpcHittingMotion(int p_id)
{
	if (m_nMode == SCENE1STAGE)
	{
		if (0 <= p_id && p_id < 5) {	// 헬기
			((SparkBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[3])->m_bActive = true;
			((SparkBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[3])->ParticlePosition
				= ((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + p_id]->GetPosition();
		}
		if (p_id >= 5) {	// 사람
			int indexnum = p_id - 5;	// id = 5 ~ 24, Object인덱스 = 22 ~ 41
			if (((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]))
			{

				((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->m_pAnimationTracks->m_nType = ANIMATION_TYPE_LOOP;
				((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
			}
			((BloodHittingBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[2])->m_bActive = true;
			((BloodHittingBillboard*)((Stage1*)m_pScene)->m_pBillboardShader[2])->ParticlePosition =
				XMFLOAT3(((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->GetPosition().x,
					((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->GetPosition().y + 8.0,
					((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]->GetPosition().z);
		}
	}
}
void CGameFramework::HeliPlayerUnderAttack(XMFLOAT3 ToLook)
{
	((Stage1*)m_pScene)->OtherPlayerFirevalkan(((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[7]), ToLook);
}
void CGameFramework::MyPlayerDieMotion()
{
	if (m_ingame_role == R_RIFLE)
	{

		m_bDieMotion = true;
		((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_pSkinnedAnimationController->m_pAnimationTracks->m_nType
			= ANIMATION_TYPE_ONCE;
		((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_bDieState = true;
		((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->DieState();
		m_pCamera->GenerateProjectionMatrix(1.01f, 8000.0f, ASPECT_RATIO, 70.0f);
	}
	if (m_ingame_role == R_HELI)
	{
		((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_FallSwitch = true;
		((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_bDieState = true;
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[0])->m_pSkinnedAnimationController
			->m_pAnimationTracks->m_nType = ANIMATION_TYPE_ONCE;
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[0])->m_pSkinnedAnimationController
			->SetTrackAnimationSet(0, 8);
		((CSoldiarOtherPlayerObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[0])->SetPosition(XMFLOAT3(
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition().x,
			6.0f,
			((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->GetPosition().z));
	}

}
void CGameFramework::MyPlayerRespawnMotion()
{
	if (m_ingame_role == R_RIFLE)
	{
		m_bDieMotion = false;
		((CHumanPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_bDieState = false;
	}
	if (m_ingame_role == R_HELI)
	{
		((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_bDieState = false;
		((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->m_FallSwitch = false;
		((HeliPlayer*)((Stage1*)m_pScene)->m_pPlayer)->Resetpartition();
	}
}
void CGameFramework::OtherPlayerResponeMotion(int id)
{
	if (m_ingame_role == R_HELI)
	{
		((CHelicopterObjects*)((Stage1*)m_pScene)->m_pPlayer)->Resetpartition();
	}
}
void CGameFramework::CollisionEndWorldObject(XMFLOAT3 pos, XMFLOAT3 extents)
{
	m_pScene->m_pPlayer->m_xoobb = BoundingOrientedBox(XMFLOAT3(m_pScene->m_pPlayer->GetPosition()), XMFLOAT3(2.5, 2.0, 4.0), XMFLOAT4(0, 0, 0, 1));

	m_worldmoobb = BoundingOrientedBox(pos, XMFLOAT3(extents), XMFLOAT4(0, 0, 0, 1));

	ContainmentType result = m_worldmoobb.Contains(m_pScene->m_pPlayer->m_xoobb);

	if (result == DISJOINT) {
		m_bCollisionCheck = true;
	}


}
void CGameFramework::AttackMotionNPC(int id)
{

	if (id >= 5) {	// 사람
		int indexnum = id - 5;	// id = 5 ~ 24, Object인덱스 = 22 ~ 41
		if (((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]))
		{
			((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->m_pAnimationTracks->m_nType = ANIMATION_TYPE_ONCE;
			((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
		}
	}

}
void CGameFramework::DyingMotionNPC(int id)
{
	/* 12~16 = 헬리콥터NPC , 22~41 = 사람NPC */
	if (0 <= id && id < 5) {	// 헬기

		((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id])->m_bDyingstate = true;
		((CFragmentsShader*)((Stage1*)m_pScene)->m_ppFragShaders[0])->m_bActive = true;
		((CFragmentsShader*)((Stage1*)m_pScene)->m_ppFragShaders[0])->ParticlePosition =
			((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id])->GetPosition();

		//((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id])->ParticlePosition
			//= ((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + id])->GetPosition();

	}
	else {		// 사람
		int indexnum = id - 5;	// id = 5 ~ 24, Object인덱스 = 22 ~ 41
		if (((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]))
		{
			((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->m_pAnimationTracks->m_nType = ANIMATION_TYPE_ONCE;
			((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
		}
	}

}
void CGameFramework::MoveMotionNPC(int id)
{
	/* 12~16 = 헬리콥터NPC , 22~41 = 사람NPC */
	if (id >= 5) {	// 사람
		int indexnum = id - 5;	// id = 5 ~ 24, Object인덱스 = 22 ~ 41
		if (((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]))
		{
			((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->m_pAnimationTracks->m_nType = ANIMATION_TYPE_LOOP;
			((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
		}

	}

}
void CGameFramework::NpcUnderAttack(XMFLOAT3 ToLook, int npc_id)
{
	//========헬기 NPC========//12
	if (npc_id < 5) {
		((Stage1*)m_pScene)->Firevalkan(((CHelicopterObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[12 + npc_id]), ToLook);
	}

	int indexnum = npc_id - 5;
	if (((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum]))
	{
		NpcShotKey = true;
		XMFLOAT3 SoldiarsPosition = ((CSoldiarNpcObjects*)((Stage1*)m_pScene)->m_ppShaders[0]->m_ppObjects[22 + indexnum])->GetPosition();
		((Stage1*)m_pScene)->NPCMuzzleFlamedRender(m_pd3dCommandList, m_pCamera, XMFLOAT3(SoldiarsPosition.x, SoldiarsPosition.y + 6.0f, SoldiarsPosition.z));
	}

}
void CGameFramework::NpcNoneUnderAttack()
{
	NpcShotKey = false;
}
