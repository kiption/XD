#include "stdafx.h"
#include "BillboardObjects.h"
#include "Shader.h"
#include "Scene.h"
#include "Stage1.h"
CBillboardObject::CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed)
{

	if (m_fRotationAngle <= -1.5f) m_fRotationDelta = 1.0f;
	if (m_fRotationAngle >= +1.5f) m_fRotationDelta = -1.0f;
	m_fRotationAngle += m_fRotationDelta * fTimeElapsed;
	Move(XMFLOAT3(0.0f, m_fRotationAngle, 0.0));


	CGameObject::Animate(fTimeElapsed);
}

CBillboardParticleObject::CBillboardParticleObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CBillboardParticleObject::~CBillboardParticleObject()
{
}

void CBillboardParticleObject::Animate(float fTimeElapsed)
{
	/*random_device rd;
	uniform_int_distribution<int>dre(1, 5);
	XMFLOAT3 xf_Position;
	xf_Position.x = m_xmf4x4ToParent._41;
	xf_Position.y = m_xmf4x4ToParent._42;
	xf_Position.z= m_xmf4x4ToParent._43;
	XMFLOAT3 xf_Velocity= XMFLOAT3(0.01,0.0,1.0);
	XMFLOAT3 xf_GravityAccel =XMFLOAT3(0.0,9.8,0.0);
	float f_EmmitTime={};
	float a_LifeTime={};
	float f_ResetTime = {};
	float NewY{};

	XMFLOAT4 newPosition = XMFLOAT4(0, 0, 0, 1);
	float Time = fTimeElapsed - f_EmmitTime;
	if (Time < 0.0)
	{

	}
	else
	{
		NewY = a_LifeTime * XMScalarModAngle(Time / a_LifeTime);
		newPosition.x = xf_Position.x + xf_Velocity.x * Time + 0.5 * xf_GravityAccel.x * Time * Time;
		newPosition.y = xf_Position.y + xf_Velocity.y * Time + 0.5 * xf_GravityAccel.y * Time * Time;
		newPosition.z = xf_Position.z + xf_Velocity.z * Time + 0.5 * xf_GravityAccel.z * Time * Time;

		m_xmf4x4ToParent._41 = newPosition.x;
		m_xmf4x4ToParent._42 = newPosition.y;
		m_xmf4x4ToParent._43 = newPosition.z;
	}*/
	CGameObject::Animate(fTimeElapsed);
}

CMultiSpriteObject::CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CMultiSpriteObject::~CMultiSpriteObject()
{
}

void CMultiSpriteObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbGameObject = ::CreateBufResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dcbGameObject->Map(0, NULL, (void**)&m_pcbMappedGameObject);
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CMultiSpriteObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
	CGameObject::ReleaseShaderVariables();
}

void CMultiSpriteObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	SceneManager* pScene = NULL;
	CGameObject::UpdateShaderVariable(pd3dCommandList, pxmf4x4World);
	if (m_pcbMappedGameObject) XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
}

void CMultiSpriteObject::Animate(float fTimeElapsed)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
		{
			if (m_ppMaterials[i] && m_ppMaterials[i]->m_ppTextures[j])
			{
				m_fTime += fTimeElapsed * 2.0f;
				if (m_fTime >= m_fSpeed) m_fTime = 0.0f;
				m_ppMaterials[i]->m_ppTextures[j]->AnimateRowColumn(m_fTime);
			}
		}
	}

	CGameObject::Animate(fTimeElapsed);
}

void CMultiSpriteObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	SceneManager* m_pScene = NULL;

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{

			if (m_ppMaterials[i]->m_pShader)
			{
				m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera, 0);
				m_ppMaterials[i]->m_pShader->UpdateShaderVariables(pd3dCommandList);

				UpdateShaderVariables(pd3dCommandList);
			}
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{

				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					m_ppMaterials[i]->m_ppTextures[j]->UpdateShaderVariables(pd3dCommandList);
					if (m_pcbMappedGameObject) XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4Texture, XMMatrixTranspose(XMLoadFloat4x4(&m_ppMaterials[i]->m_ppTextures[j]->m_xmf4x4Texture)));
				}
			}
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(19, ((Stage1*)m_pScene)->m_d3dCbvGPUDescriptorStartHandle);

	if (m_pMesh)
	{

		if (m_pMesh) m_pMesh->Render(pd3dCommandList);
	}

	CGameObject::Render(pd3dCommandList, pCamera);

}
