#include "stdafx.h"
#include "BillboardObjects.h"
#include "Shader.h"
#include "SceneMgr.h"
#include "StageScene.h"
CBillboardObject::CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed)
{

	/*if (m_fRotationAngle <= -1.5f) m_fRotationDelta = 1.0f;
	if (m_fRotationAngle >= +1.5f) m_fRotationDelta = -1.0f;
	m_fRotationAngle += m_fRotationDelta * fTimeElapsed;
	Move(XMFLOAT3(0.0f, m_fRotationAngle, 0.0));*/


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

}

void CMultiSpriteObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
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

void CMultiSpriteObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPreRender)
{
	OnPrepareRender();

	SceneMgr* m_pScene = NULL;

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{

			if (m_ppMaterials[i]->m_pShader)
			{
				m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera, 0, false);
				m_ppMaterials[i]->m_pShader->UpdateShaderVariables(pd3dCommandList);

			}
				UpdateShaderVariables(pd3dCommandList);
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{

				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					m_ppMaterials[i]->m_ppTextures[j]->UpdateShaderVariables(pd3dCommandList);
					if (m_pcbMappedGameObject) XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4Texture,
						XMMatrixTranspose(XMLoadFloat4x4(&m_ppMaterials[i]->m_ppTextures[j]->m_xmf4x4Texture)));
				}
			}
			if (m_pMesh)
			{
				if (m_pMesh) m_pMesh->Render(pd3dCommandList,i);
			}
		}

	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(19, m_pScene->m_d3dCbvGPUDescriptorNextHandle);
	
	
	CGameObject::Render(pd3dCommandList, pCamera, bPreRender);

}

CResponeObject::CResponeObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1)
{
}

CResponeObject::~CResponeObject()
{
}

void CResponeObject::Animate(float fTimeElapsed)
{

	
	CGameObject::Animate(fTimeElapsed);
}
