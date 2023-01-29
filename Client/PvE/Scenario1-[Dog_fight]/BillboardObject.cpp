#include "stdafx.h"
#include "BillboardObject.h"
#include "Shader.h"

CBillboardObject::CBillboardObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1, 1)
{
}

CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_fRotationAngle <= -1.5f) m_fRotationDelta = 1.0f;
	if (m_fRotationAngle >= +1.5f) m_fRotationDelta = -1.0f;
	m_fRotationAngle += m_fRotationDelta * fTimeElapsed;

	Rotate(0.0f, 0.0f, m_fRotationAngle);

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

CMultiSpriteObject::CMultiSpriteObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) : CGameObject(1, 1)
{
}

CMultiSpriteObject::~CMultiSpriteObject()
{
}

void CMultiSpriteObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{

	if (m_ppMaterials[0] && m_ppMaterials[0]->m_pTexture)
	{
		m_fTime += fTimeElapsed * 0.5f;
		if (m_fTime >= m_fSpeed) m_fTime = 0.0f;
		m_ppMaterials[0]->m_pTexture->AnimateRowColumn(m_fTime);
	}

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

void CMultiSpriteObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_ppMaterials)
	{
		if (m_ppMaterials[0]->m_pShader)//->½¦ÀÌ´õ 
		{
			m_ppMaterials[0]->m_pShader->Render(pd3dCommandList, pCamera);
			m_ppMaterials[0]->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_ppMaterials[0]->m_pTexture)
		{
			m_ppMaterials[0]->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			if (m_pcbMappedSpriteGameObject) XMStoreFloat4x4(&m_pcbMappedSpriteGameObject->m_xmf4x4Texture,
				XMMatrixTranspose(XMLoadFloat4x4(&m_ppMaterials[0]->m_pTexture->m_xmf4x4Texture)));
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(23, m_d3dCbvGPUDescriptorHandle);
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[0]) m_ppMeshes[0]->Render(pd3dCommandList);
		}
	}


}

