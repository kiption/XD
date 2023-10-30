#include "stdafx.h"
#include "ShadowShader.h"
#include "DDSTextureLoader12.h"
#include "SceneMgr.h"
#include "Terrain.h"
#include "SkinAnimationShader.h"


ObjectStore::ObjectStore()
{
}

ObjectStore::~ObjectStore()
{
}

void ObjectStore::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	if (m_nCurScene == OPENING_SCENE)
	{
		m_nObjects = 3;
		m_ppObjects = new GameObjectMgr * [m_nObjects];
		CMaterial* pMaterial = new CMaterial(4);
		pMaterial->SetReflection(4);

		GameObjectMgr* pRobbiMapModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Stage1_(1).bin", NULL);
		GameObjectMgr* pChairModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Car_B.bin", NULL);

		m_ppObjects[0] = new CBilldingObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[0]->SetChild(pRobbiMapModel, false);
		m_ppObjects[0]->SetMaterial(0, pMaterial);
		m_ppObjects[0]->SetPosition(-150, -20, -155);
		m_ppObjects[0]->OnPrepareAnimate();
		pRobbiMapModel->AddRef();

		m_ppObjects[1] = new CBilldingObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[1]->SetChild(pChairModel, false);
		m_ppObjects[1]->SetMaterial(0, pMaterial);
		m_ppObjects[1]->SetPosition(11.0, -11, -11);
		m_ppObjects[1]->SetScale(4, 3.8, 4);
		m_ppObjects[1]->Rotate(0.0, 70, 0.0);
		m_ppObjects[1]->OnPrepareAnimate();
		pChairModel->AddRef();

		CMaterial* pHumanMaterial = new CMaterial(1);
		pHumanMaterial->SetReflection(1);
		CLoadedModelInfo* psHumanModel = GameObjectMgr::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/rifle_soldier_(2)_Variant.bin", NULL);
		m_ppObjects[2] = new COpeningHuman(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, psHumanModel, 3);
		m_ppObjects[2]->SetMaterial(0, pHumanMaterial);
		m_ppObjects[2]->SetScale(5.0, 5.0, 5.0);
		m_ppObjects[2]->SetPosition(7.7, -7.5, -12.5);
		m_ppObjects[2]->Rotate(0.0, -20, 0.0);
		psHumanModel->m_pModelRootObject->AddRef();
		if (psHumanModel) delete psHumanModel;

	}
	if (m_nCurScene == INGAME_SCENE)
	{
		m_nObjects = 45;
		m_ppObjects = new GameObjectMgr * [m_nObjects];
		m_ppObjects[3] = new GameObjectMgr(1);

		/////////////////////////////////////////MY_PLAYER_LOAD & OTHER_PLAYER_LOAD////////////////////////////////////////////////
		{
			CMaterial* pOtherPlayerMaterial = new CMaterial(6);
			pOtherPlayerMaterial->SetReflection(6);
			CLoadedModelInfo* pPlayerModel = GameObjectMgr::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Rifle_Soldier_(1).bin", NULL);
			
			m_ppObjects[0] = new CSoldiarOtherPlayerObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pPlayerModel, NULL);
			m_ppObjects[0]->SetMaterial(0, pOtherPlayerMaterial);
			m_ppObjects[0]->SetScale(3, 3, 3);
			m_ppObjects[0]->SetPosition(XMFLOAT3(150.0, -60.0, 830.0));
			pPlayerModel->m_pModelRootObject->AddRef();

			m_ppObjects[1] = new CHumanPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pPlayerModel, NULL);
			m_ppObjects[1]->SetMaterial(0, pOtherPlayerMaterial);
			pPlayerModel->m_pModelRootObject->AddRef();

			m_ppObjects[2] = new CSoldiarOtherPlayerObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pPlayerModel, NULL);
			m_ppObjects[2]->SetMaterial(0, pOtherPlayerMaterial);
			m_ppObjects[2]->SetScale(3, 3, 3);
			m_ppObjects[2]->SetPosition(XMFLOAT3(150.0, -60.0, 830.0));
			pPlayerModel->m_pModelRootObject->AddRef();

			for (int i = 4; i <=6; i++)
			{
				m_ppObjects[i] = new CSoldiarOtherPlayerObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pPlayerModel, NULL);
				m_ppObjects[i]->SetMaterial(0, pOtherPlayerMaterial);
				m_ppObjects[i]->SetScale(5, 5, 5);
				m_ppObjects[i]->SetPosition(XMFLOAT3(150.0, -60.0, 800.0));
				pPlayerModel->m_pModelRootObject->AddRef();
			}
			if (pPlayerModel) delete pPlayerModel;

			GameObjectMgr* pPlayerHelicopterModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
			m_ppObjects[7] = new CHelicopterObjects(pd3dDevice, pd3dCommandList, pPlayerHelicopterModel, pd3dGraphicsRootSignature);
			m_ppObjects[7]->SetChild(pPlayerHelicopterModel, false);
			m_ppObjects[7]->SetMaterial(0, pOtherPlayerMaterial);
			m_ppObjects[7]->OnPrepareAnimate();
			m_ppObjects[7]->SetScale(5, 5, 5);
			m_ppObjects[7]->SetPosition(XMFLOAT3(120.0, -100.0, 700.0));
			pPlayerHelicopterModel->AddRef();
		}
		//MY_PLAYER_LOAD & OTHER_PLAYER_LOAD-END//

		////////////////////////////////////////////////////MAP_LOAD///////////////////////////////////////////////////////////////
		{
			CMaterial* pCityMaterial;
			pCityMaterial = new CMaterial(4);
			pCityMaterial->SetReflection(4);
			int Cities = 4;
			m_ppCityGameObjects = new GameObjectMgr * [Cities];
			for (int i = 0; i < Cities; i++)
			{
				string filename{ "Model/Stage1_(" };
				string num = to_string(i + 1);
				filename += num;
				filename += ").bin";
				char* c_filename = const_cast<char*>(filename.c_str());
				GameObjectMgr* pGeneratorModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, c_filename, NULL);
				m_ppCityGameObjects[i] = new CCityObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppCityGameObjects[i]->SetChild(pGeneratorModel, false);
				m_ppCityGameObjects[i]->SetMaterial(0, pCityMaterial);
				m_ppCityGameObjects[i]->SetScale(1.0f, 1.0f, 1.0f);
				m_ppCityGameObjects[i]->OnPrepareAnimate();
				pGeneratorModel->AddRef();
			}

			m_ppObjects[8] = m_ppCityGameObjects[0];
			m_ppObjects[9] = m_ppCityGameObjects[1];
			m_ppObjects[10] = m_ppCityGameObjects[2];
			m_ppObjects[11] = m_ppCityGameObjects[3];
		}
		//MAP_LOAD-END//
		
		////////////////////////////////////////////////////HELI_LOAD///////////////////////////////////////////////////////////////
		{
			m_nHeliNpcObjects = 5;
			m_ppNpc_Heli_Objects = new CHelicopterObjects * [m_nHeliNpcObjects];
			CMaterial* pNpcHeliMaterial = new CMaterial(m_nHeliNpcObjects);
			pNpcHeliMaterial->SetReflection(m_nHeliNpcObjects);

			for (int i = 0; i < m_nHeliNpcObjects; i++)
			{
				GameObjectMgr* pNPCHelicopterModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
				m_ppNpc_Heli_Objects[i] = new CHelicopterObjects(pd3dDevice, pd3dCommandList, pNPCHelicopterModel, pd3dGraphicsRootSignature);
				m_ppNpc_Heli_Objects[i]->SetChild(pNPCHelicopterModel, false);
				m_ppNpc_Heli_Objects[i]->SetMaterial(0, pNpcHeliMaterial);
				m_ppNpc_Heli_Objects[i]->OnPrepareAnimate();
				m_ppNpc_Heli_Objects[i]->SetPosition(XMFLOAT3(50.0 + i * 15, 70.0, 500.0));
				pNPCHelicopterModel->AddRef();
			}

			m_ppObjects[12] = m_ppNpc_Heli_Objects[0];
			m_ppObjects[13] = m_ppNpc_Heli_Objects[1];
			m_ppObjects[14] = m_ppNpc_Heli_Objects[2];
			m_ppObjects[15] = m_ppNpc_Heli_Objects[3];
			m_ppObjects[16] = m_ppNpc_Heli_Objects[4];
			m_ppObjects[17] = new CValkanObject(1);
			m_ppObjects[18] = new CValkanObject(1);
			m_ppObjects[19] = new CValkanObject(1);
			m_ppObjects[20] = new CValkanObject(1);
			m_ppObjects[21] = new CValkanObject(1);
		}
		//HELI_LOAD-END//

		////////////////////////////////////////////////SOLDIAR_NPC_LOAD///////////////////////////////////////////////////////////
		{
			m_nSoldiarNpcObjects = 22;
			CMaterial* pSoldiarNpcMaterial = new CMaterial(m_nSoldiarNpcObjects);
			pSoldiarNpcMaterial->SetReflection(m_nSoldiarNpcObjects);
			m_ppSoldiarNpcObjects = new GameObjectMgr * [m_nSoldiarNpcObjects];

			CLoadedModelInfo* psNpcSoldiarModel = GameObjectMgr::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/fixed.bin", NULL);
			for (int i = 0; i < m_nSoldiarNpcObjects; i++)
			{
				m_ppSoldiarNpcObjects[i] = new CSoldiarNpcObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, psNpcSoldiarModel, 4);
				m_ppSoldiarNpcObjects[i]->SetMaterial(0, pSoldiarNpcMaterial);
				m_ppSoldiarNpcObjects[i]->SetScale(5.0, 5.0, 5.0);
				if (i != 21) m_ppSoldiarNpcObjects[i]->SetPosition(210.0, 6.3, 300.0 + i * 20);
				psNpcSoldiarModel->m_pModelRootObject->AddRef();
			}

			m_ppObjects[22] = m_ppSoldiarNpcObjects[0];
			m_ppObjects[23] = m_ppSoldiarNpcObjects[1];
			m_ppObjects[24] = m_ppSoldiarNpcObjects[2];
			m_ppObjects[25] = m_ppSoldiarNpcObjects[3];
			m_ppObjects[26] = m_ppSoldiarNpcObjects[4];
			m_ppObjects[27] = m_ppSoldiarNpcObjects[5];
			m_ppObjects[28] = m_ppSoldiarNpcObjects[6];
			m_ppObjects[29] = m_ppSoldiarNpcObjects[7];
			m_ppObjects[30] = m_ppSoldiarNpcObjects[8];
			m_ppObjects[31] = m_ppSoldiarNpcObjects[9];
			m_ppObjects[32] = m_ppSoldiarNpcObjects[11];
			m_ppObjects[33] = m_ppSoldiarNpcObjects[12];
			m_ppObjects[34] = m_ppSoldiarNpcObjects[13];
			m_ppObjects[35] = m_ppSoldiarNpcObjects[14];
			m_ppObjects[36] = m_ppSoldiarNpcObjects[15];
			m_ppObjects[37] = m_ppSoldiarNpcObjects[16];
			m_ppObjects[38] = m_ppSoldiarNpcObjects[17];
			m_ppObjects[39] = m_ppSoldiarNpcObjects[18];
			m_ppObjects[40] = m_ppSoldiarNpcObjects[19];
			m_ppObjects[41] = m_ppSoldiarNpcObjects[20];
			m_ppObjects[42] = new GameObjectMgr(1);
			if (psNpcSoldiarModel) delete psNpcSoldiarModel;
		}
		//SOLDIAR_NPC_LOAD-END//
		////////////////////////////////////////////////HELIPLAYER_LOAD////////////////////////////////////////////////////////////
		{
			CMaterial* pHeliMaterial = new CMaterial(1);
			pHeliMaterial->SetReflection(1);
			GameObjectMgr* pHelicopterModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
			m_ppObjects[43] = new HeliPlayer(pd3dDevice, pd3dCommandList, pHelicopterModel, pd3dGraphicsRootSignature);
			m_ppObjects[43]->SetMaterial(0, pHeliMaterial);
			m_ppObjects[43]->OnPrepareAnimate();
			pHelicopterModel->AddRef();
		}
		//HELIPLAYER_LOAD-END//
	
		////////////////////////////////////////////////TREE_LOAD//////////////////////////////////////////////////////////////////
		{	
			CMaterial* pTreeMaterial = new CMaterial(1);
			pTreeMaterial->SetReflection(1);
			GameObjectMgr* pTreesModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/ALL_Tree.bin", NULL);
			m_ppObjects[44] = new CCityObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
			m_ppObjects[44]->SetChild(pTreesModel, false);
			m_ppObjects[44]->SetMaterial(0, pTreeMaterial);
			m_ppObjects[44]->OnPrepareAnimate();
			m_ppObjects[44]->SetScale(1.0, 1.0, 1.0);
			m_ppObjects[44]->SetPosition(0, 0, 0);
			pTreesModel->AddRef();
		}
		///TREE_LOAD-END//
	}
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void ObjectStore::AnimateObjects(float fTimeElapsed)
{

	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void ObjectStore::ReleaseObjects()
{

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void ObjectStore::ReleaseUploadBuffers()
{
	if (m_ppObjects) for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void ObjectStore::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	ShaderMgr::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);
}


CShadowMapShader::CShadowMapShader(ObjectStore* pObjectsShader)
{
	m_pObjectsShader = pObjectsShader;
}

CShadowMapShader::~CShadowMapShader()
{
}

D3D12_INPUT_LAYOUT_DESC CShadowMapShader::CreateInputLayout(int nPipelineState)
{

	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
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
	return(ShaderMgr::CompileShaderFromFile(L"Shadow.hlsl", "VSShadowMapShadow", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowMapShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shadow.hlsl", "PSShadowMapShadow", "ps_5_1", ppd3dShaderBlob));
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
	ShaderMgr::ReleaseShaderVariables();
}

void CShadowMapShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{

	m_pDepthTexture = (Texture*)pContext;
	m_pDepthTexture->AddRef();
	SceneMgr::CreateSRVs(pd3dDevice, m_pDepthTexture, 0, 22, false);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void CShadowMapShader::ReleaseObjects()
{
	if (m_pDepthTexture) m_pDepthTexture->Release();
	ShaderMgr::ReleaseObjects();
}

void CShadowMapShader::ReleaseUploadBuffers()
{
}

D3D12_BLEND_DESC CShadowMapShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_SUBTRACT;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return(d3dBlendDesc);
}

void CShadowMapShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	ShaderMgr::Render(pd3dCommandList, pCamera, nPipelineState, false);
	UpdateShaderVariables(pd3dCommandList);
	if (m_nCurScene == OPENING_SCENE)
	{
		for (int i = 0; i < m_pObjectsShader->m_nObjects; i++)
		{
			m_pObjectsShader->m_ppObjects[i]->Animate(m_fElapsedTime);
			m_pObjectsShader->m_ppObjects[i]->UpdateShaderVariables(pd3dCommandList);
			m_pObjectsShader->m_ppObjects[i]->ShadowRender(pd3dCommandList, pCamera, true, this);
		}
	}
	if (m_nCurScene == INGAME_SCENE)
	{
		for (int i = 0; i < m_pObjectsShader->m_nObjects - 1; i++)
		{
			if (i != 1)
			{
				m_pObjectsShader->m_ppObjects[i]->Animate(m_fElapsedTime);
				m_pObjectsShader->m_ppObjects[i]->UpdateShaderVariables(pd3dCommandList);
				m_pObjectsShader->m_ppObjects[i]->ShadowRender(pd3dCommandList, pCamera, true, this);
			}
		}
		if (((CHumanPlayer*)m_pObjectsShader->m_ppObjects[1])->m_bZoomMode == false)
		{
			m_pObjectsShader->m_ppObjects[1]->Animate(m_fElapsedTime);
			m_pObjectsShader->m_ppObjects[1]->UpdateShaderVariables(pd3dCommandList);
			m_pObjectsShader->m_ppObjects[1]->ShadowRender(pd3dCommandList, pCamera, true, this);
		}
	}
}

void CShadowMapShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
	int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	ShaderMgr::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);
}



CDepthRenderShader::CDepthRenderShader(ObjectStore* pObjectsShader, LIGHT* pLights)
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
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
#ifdef _WITH_RASTERIZER_DEPTH_BIAS
	d3dRasterizerDesc.DepthBias = 13000;
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

D3D12_INPUT_LAYOUT_DESC CDepthRenderShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	if (nPipelineState == 0) return (ShaderMgr::CompileShaderFromFile(L"Shadow.hlsl", "VSLighting", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CDepthRenderShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shadow.hlsl", "PSDepthWriteShader", "ps_5_1", ppd3dShaderBlob));
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
	pd3dCommandList->SetGraphicsRootConstantBufferView(23, d3dcbToLightGpuVirtualAddress); //ToLight
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
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = MAX_DEPTH_TEXTURES;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

	m_pDepthTexture = new Texture(MAX_DEPTH_TEXTURES, RESOURCE_TEXTURE2D_ARRAY, 0, 1);

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

	ShaderMgr::ReleaseObjects();
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

			float fNearPlaneDistance = 10.0f, fFarPlaneDistance = m_pLights[j].m_fRange;

			XMMATRIX xmmtxProjection{};
			if (m_pLights[j].m_nType == DIRECTIONAL_LIGHT)
			{
				float fWidth = 2000, fHeight = 2000;
				xmmtxProjection = XMMatrixOrthographicLH(fWidth, fHeight, fNearPlaneDistance, fFarPlaneDistance);

			}
			else if (m_pLights[j].m_nType == SPOT_LIGHT)
			{
				float fWidth = _PLANE_WIDTH, fHeight = _PLANE_HEIGHT;
				/*float fFovAngle = 60.0f; */m_pLights[j].m_fPhi = cos(60.0f);
				float fAspectRatio = float(_DEPTH_BUFFER_WIDTH) / float(_DEPTH_BUFFER_HEIGHT);
				//xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_pLights[j].m_fPhi), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
				xmmtxProjection = XMMatrixOrthographicLH(fWidth, fHeight, fNearPlaneDistance, fFarPlaneDistance);
			}
			else if (m_pLights[j].m_nType == POINT_LIGHT)
			{
				//ShadowMap[6]
			}

			m_ppDepthRenderCameras[j]->SetPosition(xmf3Position);
			XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4View, xmmtxView);
			XMStoreFloat4x4(&m_ppDepthRenderCameras[j]->m_xmf4x4Projection, xmmtxProjection);

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

void CDepthRenderShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	ShaderMgr::Render(pd3dCommandList, pCamera, nPipelineState, false);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_pObjectsShader->m_nObjects; i++)
	{
		if (m_pObjectsShader->m_ppObjects[i])
		{
			m_pObjectsShader->m_ppObjects[i]->Animate(m_fElapsedTime);
			m_pObjectsShader->m_ppObjects[i]->UpdateShaderVariables(pd3dCommandList);
			m_pObjectsShader->m_ppObjects[i]->Render(pd3dCommandList, pCamera, true); //true
		}
	}
}

void CDepthRenderShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	ShaderMgr::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 0);
}

TreeBlendingShader::TreeBlendingShader(ObjectStore* pObjectsShader)
{
	m_pObjectsShader = pObjectsShader;
}

TreeBlendingShader::~TreeBlendingShader()
{
}

D3D12_BLEND_DESC TreeBlendingShader::CreateBlendState(int nPipelineState)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_INPUT_LAYOUT_DESC TreeBlendingShader::CreateInputLayout(int nPipelineState)
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC TreeBlendingShader::CreateDepthStencilState(int nPipelineState)
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

D3D12_RASTERIZER_DESC TreeBlendingShader::CreateRasterizerState(int nPipelineState)
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

D3D12_SHADER_BYTECODE TreeBlendingShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shadow.hlsl", "VSShadowMapShadow", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE TreeBlendingShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shadow.hlsl", "PSShadowMapShadow", "ps_5_1", ppd3dShaderBlob));
}

void TreeBlendingShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void TreeBlendingShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pDepthTexture) m_pDepthTexture->UpdateShaderVariables(pd3dCommandList);
}

void TreeBlendingShader::ReleaseShaderVariables()
{
}

void TreeBlendingShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_pDepthTexture = (Texture*)pContext;
	m_pDepthTexture->AddRef();
	SceneMgr::CreateSRVs(pd3dDevice, m_pDepthTexture, 0, 22, false);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void TreeBlendingShader::ReleaseObjects()
{
	if (m_pDepthTexture) m_pDepthTexture->Release();
	ShaderMgr::ReleaseObjects();
}

void TreeBlendingShader::ReleaseUploadBuffers()
{
}

void TreeBlendingShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	ShaderMgr::Render(pd3dCommandList, pCamera, nPipelineState, false);

	UpdateShaderVariables(pd3dCommandList);
	m_pObjectsShader->m_ppObjects[44]->Animate(m_fElapsedTime);
	m_pObjectsShader->m_ppObjects[44]->UpdateShaderVariables(pd3dCommandList);
	m_pObjectsShader->m_ppObjects[44]->ShadowRender(pd3dCommandList, pCamera, true, this);
}

void TreeBlendingShader::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
	ShaderMgr::CreateGraphicsPipelineState(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nPipelineState);
}
