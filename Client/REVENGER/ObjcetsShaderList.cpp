#include "stdafx.h"
#include "ObjcetsShaderList.h"
float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}
inline float RandF(float fMin, float fMax)
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}

float Random()
{
	return(rand() / float(RAND_MAX));
}
XMVECTOR RandomUnitVectorOnSphere()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}
XMVECTOR RandomMarkDir()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandF(-0.2f, 0.2f), RandF(-1.f, 1.5f), RandF(-0.2f, 0.2f), 0.0f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}
XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
	float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
	xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
	xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
	xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}
CHellicopterObjectsShader::CHellicopterObjectsShader()
{
}

CHellicopterObjectsShader::~CHellicopterObjectsShader()
{
}

void CHellicopterObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationObjectsShader::CSkinnedAnimationObjectsShader()
{
}

CSkinnedAnimationObjectsShader::~CSkinnedAnimationObjectsShader()
{
}

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext)
{
}

void CSkinnedAnimationObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CSkinnedAnimationObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	//for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Animate(fTimeElapsed);
}

void CSkinnedAnimationObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera, nPipelineState, true);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->Render(pd3dCommandList, pCamera, true);
		}
	}
}


BulletEffectShader::BulletEffectShader()
{
}

BulletEffectShader::~BulletEffectShader()
{
}


D3D12_INPUT_LAYOUT_DESC BulletEffectShader::CreateInputLayout(int nPipelineState)
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

D3D12_SHADER_BYTECODE BulletEffectShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE BulletEffectShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "PSBulletStandard", "ps_5_1", ppd3dShaderBlob));
}

void BulletEffectShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	CStandardObjectsShader::Render(pd3dCommandList, pCamera, nPipelineState, false);
}

D3D12_SHADER_BYTECODE CFragmentsShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleStandard", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CFragmentsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "PSParticleStandard", "ps_5_1", ppd3dShaderBlob));
}


void CFragmentsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_nObjects = EXPLOSION_DEBRISES;
	m_ppObjects = new GameObjectMgr * [m_nObjects];
	GameObjectMgr* pFragmentModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Wing2_1_Variant.bin", this);
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CExplosiveObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pFragmentModel, false);
		m_ppObjects[i]->SetScale(0.5, 1.5, 0.5);
		pFragmentModel->AddRef();
	}
	for (int i = 0; i < EXPLOSION_DEBRISES; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomUnitVectorOnSphere());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CFragmentsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CFragmentsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bActive == true)
	{
		ShaderMgr::Render(pd3dCommandList, pCamera, 0, false);

		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->UpdateTransform(NULL);
				m_ppObjects[j]->Render(pd3dCommandList, pCamera, false);
			}
		}
	}

}

D3D12_INPUT_LAYOUT_DESC CFragmentsShader::CreateInputLayout(int nPipelineState)
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

void CFragmentsShader::AnimateObjects(float fTimeElapsed)
{
	if (m_bActive == true)
	{

		XMFLOAT3 gravity = XMFLOAT3(0.0, -3.8, 0);
		m_fElapsedTimes += fTimeElapsed * 6.05f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < EXPLOSION_DEBRISES; i++)
			{
				m_fExplosionSpeed = Random(1.0f, 15.0f);
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes + gravity.x;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.y * m_fElapsedTimes * m_fElapsedTimes;;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes + gravity.z;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->m_xmf4x4ToParent._41 = m_pxmf4x4Transforms[i]._41;
				m_ppObjects[i]->m_xmf4x4ToParent._42 = m_pxmf4x4Transforms[i]._42;
				m_ppObjects[i]->m_xmf4x4ToParent._43 = m_pxmf4x4Transforms[i]._43;
				m_ppObjects[i]->Rotate(2.0, 8.0, 0.0);
			}
		}
		else
		{
			m_bActive = false;
			m_fElapsedTimes = 0.0f;
		}

	}



}

void CFragmentsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CHelicopterBulletMarkParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_nObjects = HELICOPTEREXPLOSION_PARTICLES;
	m_ppObjects = new GameObjectMgr * [m_nObjects];
	GameObjectMgr* pParticleFragModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Blood_particle.bin", this);
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CExplosiveObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pParticleFragModel, false);
		m_ppObjects[i]->SetScale(0.5, 0.5, 0.5);
		pParticleFragModel->AddRef();
	}
	for (int i = 0; i < HELICOPTEREXPLOSION_PARTICLES; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomUnitVectorOnSphere());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CHelicopterBulletMarkParticleShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CHelicopterBulletMarkParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bActive == true)
	{

		ShaderMgr::Render(pd3dCommandList, pCamera, 0, false);

		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->UpdateTransform(NULL);
				m_ppObjects[j]->Render(pd3dCommandList, pCamera, false);
			}
		}
	}

}
D3D12_BLEND_DESC CHelicopterBulletMarkParticleShader::CreateBlendState(int nPipelineState)
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
D3D12_INPUT_LAYOUT_DESC CHelicopterBulletMarkParticleShader::CreateInputLayout(int nPipelineState)
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

D3D12_SHADER_BYTECODE CHelicopterBulletMarkParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleStandard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CHelicopterBulletMarkParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "PSBloodParticleStandard", "ps_5_1", ppd3dShaderBlob));
}
void CHelicopterBulletMarkParticleShader::AnimateObjects(float fTimeElapsed)
{

	if (m_bActive == true)
	{

		XMFLOAT3 gravity = XMFLOAT3(0.0, -9.8, 0);
		m_fElapsedTimes += fTimeElapsed * 5.5f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < HELICOPTEREXPLOSION_PARTICLES; i++)
			{
				m_fExplosionSpeed = Random(5.0f, 10.0f);
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;// + gravity.x;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes; +0.5f * gravity.y * m_fElapsedTimes * m_fElapsedTimes;;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;// + gravity.z;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->m_xmf4x4ToParent._41 = m_pxmf4x4Transforms[i]._41;
				m_ppObjects[i]->m_xmf4x4ToParent._42 = m_pxmf4x4Transforms[i]._42;
				m_ppObjects[i]->m_xmf4x4ToParent._43 = m_pxmf4x4Transforms[i]._43;
				m_ppObjects[i]->Rotate(10, 10, 10);
			}
		}
		else
		{
			m_bActive = false;
			m_fElapsedTimes = 0.0f;
		}

	}
}
void CHelicopterBulletMarkParticleShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}



void CHumanBulletMarkParticleShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_nObjects = HELICOPTEREXPLOSION_PARTICLES;
	m_ppObjects = new GameObjectMgr * [m_nObjects];
	GameObjectMgr* pFragmentModel = GameObjectMgr::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Sphere.bin", this);
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i] = new CExplosiveObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_ppObjects[i]->SetChild(pFragmentModel, false);
		m_ppObjects[i]->SetScale(6.1, 6.1, 6.1);
		pFragmentModel->AddRef();
	}
	for (int i = 0; i < HELICOPTEREXPLOSION_PARTICLES; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomUnitVectorOnSphere());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CHumanBulletMarkParticleShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}
void CHumanBulletMarkParticleShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int nPipelineState)
{
	if (m_bActive == true)
	{

		ShaderMgr::Render(pd3dCommandList, pCamera, 0, false);

		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->UpdateTransform(NULL);
				m_ppObjects[j]->Render(pd3dCommandList, pCamera, false);
			}
		}
	}
}
D3D12_INPUT_LAYOUT_DESC CHumanBulletMarkParticleShader::CreateInputLayout(int nPipelineState)
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
D3D12_SHADER_BYTECODE CHumanBulletMarkParticleShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "VSParticleStandard", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CHumanBulletMarkParticleShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob, int nPipelineState)
{
	return(ShaderMgr::CompileShaderFromFile(L"Shaders.hlsl", "PSRiflePointingStandard", "ps_5_1", ppd3dShaderBlob));

}

D3D12_BLEND_DESC CHumanBulletMarkParticleShader::CreateBlendState(int nPipelineState)
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
void CHumanBulletMarkParticleShader::AnimateObjects(float fTimeElapsed)
{
	if (m_bActive == true)
	{

		XMFLOAT3 gravity = XMFLOAT3(0.0, -9.8, 0);
		m_fElapsedTimes += fTimeElapsed * 5.5f;
		if (m_fElapsedTimes <= m_fDuration)
		{
			for (int i = 0; i < BULLETTEREXPLOSION_PARTICLES; i++)
			{
				m_fExplosionSpeed = Random(5.0f, 6.0f);
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = ParticlePosition.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;// + gravity.x;
				m_pxmf4x4Transforms[i]._42 = ParticlePosition.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes + 0.5f * gravity.y * m_fElapsedTimes * m_fElapsedTimes;;
				m_pxmf4x4Transforms[i]._43 = ParticlePosition.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;// + gravity.z;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);

				m_ppObjects[i]->SetPosition(m_pxmf4x4Transforms[i]._41, m_pxmf4x4Transforms[i]._42, m_pxmf4x4Transforms[i]._43);
				m_ppObjects[i]->Rotate(10.0,10.0,10.0);
			}
		}
		else
		{
			m_bActive = false;
			m_fElapsedTimes = 0.0f;
		}

	}
}

void CHumanBulletMarkParticleShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}