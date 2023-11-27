//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "ObjcetsShaderList.h"
#include "MissileObject.h"
#include "SceneMgr.h"
#include "StageScene.h"





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
GameObjectMgr::GameObjectMgr()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

GameObjectMgr::GameObjectMgr(int nMaterials) : GameObjectMgr()
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial * [m_nMaterials];
		for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}
}

GameObjectMgr::GameObjectMgr(int nMeshes, int nMaterials)
{

	m_nMeshes = nMeshes;
	m_pMesh = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new Mesh * [m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;
	}

	m_nMaterials = nMaterials;
	m_ppMaterials = NULL;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial * [m_nMaterials];
		for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}
}

GameObjectMgr::~GameObjectMgr()
{
	//ReleaseShaderVariables();
	if (m_pMesh) m_pMesh->Release();

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}
	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
		delete[] m_ppMaterials;
	}
	//if (m_pMaterials) m_pMaterials->Release();
	if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
}

void GameObjectMgr::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void GameObjectMgr::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

void GameObjectMgr::SetChild(GameObjectMgr* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void GameObjectMgr::UpdateBoundingBox()
{
	OnPrepareRender();
	//if (m_pMesh)
	//{
	//	m_pMesh->m_xmBoundingOrientedBox.Transform(m_xoobb, XMLoadFloat4x4(&m_xmf4x4World));
	//	XMStoreFloat4(&m_xoobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xoobb.Orientation)));
	//}
}

void GameObjectMgr::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pBoundingBoxMesh)
	{
		m_pBoundingBoxMesh->UpdateVertexPosition(&m_xoobb);
		m_pBoundingBoxMesh->Render(pd3dCommandList);
	}
}

bool GameObjectMgr::IsVisible(CCamera* pCamera)
{
	OnPrepareRender();
	bool bIsVisible = false;
	BoundingBox xmBoundingBox = m_pMesh->GetBoundingBox();
	//모델 좌표계의 바운딩 박스를 월드 좌표계로 변환한다. 
	xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
	if (pCamera) bIsVisible = pCamera->IsInFrustum(xmBoundingBox);
	return(bIsVisible);
}

void GameObjectMgr::SetMovingDirection(const XMFLOAT3& xmf3MovingDirection)
{
	XMStoreFloat3(&m_xmf3MovingDirection, XMVector3Normalize(XMLoadFloat3(&xmf3MovingDirection)));
}

void GameObjectMgr::SetLookAt(XMFLOAT3 xmf3Target, XMFLOAT3 xmf3Up)
{
	XMFLOAT3 xmf3Position(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43);
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(xmf3Position, xmf3Target, xmf3Up);
	m_xmf4x4ToParent._11 = mtxLookAt._11; m_xmf4x4ToParent._12 = mtxLookAt._21; m_xmf4x4ToParent._13 = mtxLookAt._31;
	m_xmf4x4ToParent._21 = mtxLookAt._12; m_xmf4x4ToParent._22 = mtxLookAt._22; m_xmf4x4ToParent._23 = mtxLookAt._32;
	m_xmf4x4ToParent._31 = mtxLookAt._13; m_xmf4x4ToParent._32 = mtxLookAt._23; m_xmf4x4ToParent._33 = mtxLookAt._33;

}

void GameObjectMgr::SetMesh(Mesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void GameObjectMgr::SetMesh(int nIndex, Mesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}
}

void GameObjectMgr::SetShader(ShaderMgr* pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	m_ppMaterials[0] = new CMaterial(0);
	m_ppMaterials[0]->SetShader(pShader);
}

void GameObjectMgr::SetShader(int nMaterial, ShaderMgr* pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);

	if (!m_pMaterials)
	{
		CMaterial* pMaterial = new CMaterial(nMaterial);
		SetMaterial(pMaterial);
	}
	if (m_pMaterials) m_pMaterials->SetShader(pShader);

}

void GameObjectMgr::SetMaterial(int nMaterial, CMaterial* pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}

void GameObjectMgr::SetMaterial(CMaterial* pMaterial)
{
	if (m_pMaterials) m_pMaterials->Release();
	m_pMaterials = pMaterial;
	if (m_pMaterials) m_pMaterials->AddRef();
}

CSkinnedMesh* GameObjectMgr::FindSkinnedMesh(char* pstrSkinnedMeshName)
{
	CSkinnedMesh* pSkinnedMesh = NULL;
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT))
	{
		pSkinnedMesh = (CSkinnedMesh*)m_pMesh;
		if (!strncmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName, strlen(pstrSkinnedMeshName))) return(pSkinnedMesh);
	}

	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);

	return(NULL);
}

void GameObjectMgr::FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh*)m_pMesh;

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

GameObjectMgr* GameObjectMgr::FindFrame(char* pstrFrameName)
{
	GameObjectMgr* pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void GameObjectMgr::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void GameObjectMgr::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void GameObjectMgr::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void GameObjectMgr::Animate(float fTimeElapsed)
{
	OnPrepareRender();
	//UpdateBoundingBox();
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void GameObjectMgr::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	OnPrepareRender();
	//UpdateBoundingBox();
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}

void GameObjectMgr::AnimateObject(CCamera* pCamera, float fTimeElapsed)
{
	OnPrepareRender();
	//UpdateBoundingBox();
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void GameObjectMgr::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPrerender, ShaderMgr* pShader)
{
	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	OnPrepareRender();

	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
	if (m_pMesh) {
		if (m_nMaterials > 0) {
			for (int i = 0; i < m_nMaterials; i++) {

				if (m_ppMaterials[i]) {

					if (m_ppMaterials[i]->m_pShader) {

						m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera, 0, false);
						pShader->SetPipelineState(pd3dCommandList, 0);
						UpdateShaderVariables(pd3dCommandList);
					}
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
					for (int k = 0; k < m_ppMaterials[i]->m_nTextures; k++) {

						if (m_ppMaterials[i]->m_ppTextures[k]) m_ppMaterials[i]->m_ppTextures[k]->UpdateShaderVariables(pd3dCommandList);
					}
				}
				m_pMesh->Render(pd3dCommandList, i);
			}
		}
	}
	if (m_pSibling) m_pSibling->ShadowRender(pd3dCommandList, pCamera, bPrerender, pShader);
	if (m_pChild) m_pChild->ShadowRender(pd3dCommandList, pCamera, bPrerender, pShader);
}

void GameObjectMgr::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPrerender)
{

	if (m_pSkinnedAnimationController)
		m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	OnPrepareRender();


	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

	if (m_pMesh)
	{
		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) {

						m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera, 0, bPrerender);
						UpdateShaderVariables(pd3dCommandList);
					}
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);

					for (int k = 0; k < m_ppMaterials[i]->m_nTextures; k++)
					{
						if (m_ppMaterials[i]->m_ppTextures[k]) m_ppMaterials[i]->m_ppTextures[k]->UpdateShaderVariables(pd3dCommandList);
					}
				}
				m_pMesh->Render(pd3dCommandList, i);
			}
		}
	}
	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera, bPrerender);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera, bPrerender);

}

void GameObjectMgr::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{



}

void GameObjectMgr::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void GameObjectMgr::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);


	for (int i = 0; i < m_nMaterials; i++)
		if (m_ppMaterials[i]) pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_ppMaterials[i]->m_nReflection, 16);

}

void GameObjectMgr::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial)
{
}

void GameObjectMgr::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMMATRIX* pxmf4x4Shadow)
{
	XMFLOAT4X4 xmf4x4PlanarShadow;
	XMStoreFloat4x4(&xmf4x4PlanarShadow, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World) * (*pxmf4x4Shadow)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4PlanarShadow, 0);

	for (int i = 0; i < m_nMaterials; i++)
		if (m_ppMaterials[i]) pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_ppMaterials[i]->m_nReflection, 16);


}

void GameObjectMgr::ReleaseShaderVariables()
{

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseShaderVariables();
	}
}

void GameObjectMgr::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void GameObjectMgr::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void GameObjectMgr::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void GameObjectMgr::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4ToParent._41 += xmf3Offset.x;
	m_xmf4x4ToParent._42 += xmf3Offset.y;
	m_xmf4x4ToParent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}

void GameObjectMgr::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void GameObjectMgr::CalculateBoundingBox()
{
	m_xmBoundingBox.Transform(m_xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
}

XMFLOAT3 GameObjectMgr::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 GameObjectMgr::GetToParentPosition()
{
	return(XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43));
}

XMFLOAT3 GameObjectMgr::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 GameObjectMgr::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 GameObjectMgr::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void GameObjectMgr::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	GameObjectMgr::SetPosition(xmf3Position);
}

void GameObjectMgr::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	GameObjectMgr::SetPosition(xmf3Position);
}

void GameObjectMgr::Move(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	GameObjectMgr::SetPosition(xmf3Position);
}

void GameObjectMgr::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void GameObjectMgr::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void GameObjectMgr::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

//#define _WITH_DEBUG_FRAME_HIERARCHY

Texture* GameObjectMgr::FindReplicatedTexture(_TCHAR* pstrTextureName)
{
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i])
		{
			for (int j = 0; j < m_ppMaterials[i]->m_nTextures; j++)
			{
				if (m_ppMaterials[i]->m_ppTextures[j])
				{
					if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName))) return(m_ppMaterials[i]->m_ppTextures[j]);
				}
			}
		}
	}
	Texture* pTexture = NULL;
	if (m_pSibling) if (pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName)) return(pTexture);
	if (m_pChild) if (pTexture = m_pChild->FindReplicatedTexture(pstrTextureName)) return(pTexture);

	return(NULL);
}

int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void GameObjectMgr::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, GameObjectMgr* pParent, FILE* pInFile, ShaderMgr* pShader)
{
	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	if (m_nCurScene == INGAME_SCENE) {
		m_pScene = ((MainGameScene*)m_pScene);
	}
	else
	{
		m_pScene = ((SceneMgr*)m_pScene);
	}
	m_nMaterials = ReadIntegerFromFile(pInFile);

	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial* pMaterial = NULL;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);

			pMaterial = new CMaterial(7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

			if (!pShader)
			{
				UINT nMeshType = GetMeshType();
				if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE)
				{
					if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT)
					{
						pMaterial->SetSkinnedAnimationShader();


					}
					else
					{
						pMaterial->SetStandardShader();
					}
				}
			}

			pMaterial->SetReflection(nMaterial);
			SetMaterial(nMaterial, pMaterial);

		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_ALBEDO_MAP, 3, pMaterial->m_ppstrTextureNames[0], &(pMaterial->m_ppTextures[0]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_SPECULAR_MAP, 4, pMaterial->m_ppstrTextureNames[1], &(pMaterial->m_ppTextures[1]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_NORMAL_MAP, 5, pMaterial->m_ppstrTextureNames[2], &(pMaterial->m_ppTextures[2]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_METALLIC_MAP, 6, pMaterial->m_ppstrTextureNames[3], &(pMaterial->m_ppTextures[3]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_EMISSION_MAP, 7, pMaterial->m_ppstrTextureNames[4], &(pMaterial->m_ppTextures[4]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_ALBEDO_MAP, 8, pMaterial->m_ppstrTextureNames[5], &(pMaterial->m_ppTextures[5]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		{
			m_ppMaterials[nMaterial]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, MATERIAL_DETAIL_NORMAL_MAP, 9, pMaterial->m_ppstrTextureNames[6], &(pMaterial->m_ppTextures[6]), pParent, pInFile, pShader, m_pScene);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{

			break;
		}
	}
}

GameObjectMgr* GameObjectMgr::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObjectMgr* pParent, FILE* pInFile, ShaderMgr* pShader, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	GameObjectMgr* pGameObject = new GameObjectMgr();
	ObjectStore* pObjectShader = new ObjectStore();
	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{

			CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);


		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					GameObjectMgr* pChild = GameObjectMgr::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

void GameObjectMgr::PrintFrameInfo(GameObjectMgr* pGameObject, GameObjectMgr* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) GameObjectMgr::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) GameObjectMgr::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}
GameObjectMgr* GameObjectMgr::LoadGeometryHierachyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, ShaderMgr* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);
	GameObjectMgr* pGameObject = NULL;
	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pGameObject = GameObjectMgr::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, NULL);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
		}
		else
		{
			break;
		}
	}

	return(pGameObject);
}

void GameObjectMgr::LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets->m_ppAnimatedBoneFrameCaches = new GameObjectMgr * [pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames];

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppAnimatedBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_ppAnimatedBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					CAnimationSet* pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet];

					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);

#ifdef _WITH_ANIMATION_SRT
					m_pfKeyFrameScaleTimes[i] = fKeyTime;
					m_pfKeyFrameRotationTimes[i] = fKeyTime;
					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pInFile);
#else
					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nAnimatedBoneFrames, pInFile);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

CLoadedModelInfo* GameObjectMgr::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, ShaderMgr* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pLoadedModel->m_pModelRootObject = GameObjectMgr::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				GameObjectMgr::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}


CNpcHelicopterObject::CNpcHelicopterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) :GameObjectMgr(10)
{
	//CGameObject* pOtherPlayerModel = CGameObject::LoadGeometryHierachyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Military_Helicopter.bin", NULL);
	//SetChild(pOtherPlayerModel, false);
	//SetScale(1.0, 1.0, 1.0);
	//pOtherPlayerModel->AddRef();
}

CNpcHelicopterObject::~CNpcHelicopterObject()
{
}

void CNpcHelicopterObject::OnPrepareAnimate()
{
	m_FrameTailRotor = FindFrame("rescue_2");
	m_FrameTopRotor = FindFrame("rescue_1");
}

void CNpcHelicopterObject::Animate(float fTimeElapsed)
{
	if (m_FrameTopRotor)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 10.0f) * fTimeElapsed);
		m_FrameTopRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTopRotor->m_xmf4x4ToParent);
	}
	if (m_FrameTailRotor)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 10.0f) * fTimeElapsed);
		m_FrameTailRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTailRotor->m_xmf4x4ToParent);
	}

	GameObjectMgr::Animate(fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////


inline float RandomValue(float fMin, float fMax)
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}
XMVECTOR RandomScatter()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandomValue(-1.0f, 1.0f), RandomValue(-1.0f, 1.0f), RandomValue(-1.0f, 1.0f), 0.5f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}
CHelicopterObjects::CHelicopterObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, GameObjectMgr* pmodel, ID3D12RootSignature* pd3dGraphicsRootSignature) :GameObjectMgr(10)
{

	OnPrepareAnimate();
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}
void CHelicopterObjects::Resetpartition()
{
	//m_pMainRotorFrame->m_xmf4x4ToParent = m_pMainRotorFrameP;
	//m_pTailRotorFrame->m_xmf4x4ToParent = m_pTailRotorFrameP;
	//m_pFrameFragObj1->m_xmf4x4ToParent = m_pFrameFragObj1P;
	//m_pFrameFragObj2->m_xmf4x4ToParent = m_pFrameFragObj2P;
	//m_pFrameFragObj3->m_xmf4x4ToParent = m_pFrameFragObj3P;
	//m_pFrameFragObj4->m_xmf4x4ToParent = m_pFrameFragObj4P;
	//m_pFrameFragObj5->m_xmf4x4ToParent = m_pFrameFragObj5P;
	//m_pFrameFragObj6->m_xmf4x4ToParent = m_pFrameFragObj6P;
	//m_pFrameFragObj7->m_xmf4x4ToParent = m_pFrameFragObj7P;
	//m_pFrameFragObj8->m_xmf4x4ToParent = m_pFrameFragObj8P;
	//m_pFrameFragObj9->m_xmf4x4ToParent = m_pFrameFragObj9P;
	//m_pFrameFragObj10->m_xmf4x4ToParent = m_pFrameFragObj10P;

}
CHelicopterObjects::~CHelicopterObjects()
{

}

void CHelicopterObjects::Firevalkan(XMFLOAT3 ToPlayerLook)
{

}

void CHelicopterObjects::OnPrepareAnimate()
{

	GameObjectMgr::OnPrepareAnimate();
	m_FrameHeliglass = FindFrame("glass");
	m_FrameCleanse = FindFrame("cleanser");
	m_FrameCleanser_2 = FindFrame("cleanser_1");
	m_FrameLefttyre = FindFrame("left_tyre");
	m_FrameHeliBody = FindFrame("helicopter");
	m_FrameRightDoor = FindFrame("right_door");
	m_FrameBackDoor = FindFrame("back_door");
	m_FrameLeftDoor = FindFrame("left_door");
	m_FrameRighttyre = FindFrame("right_tyre");
	m_FrameBacktyre = FindFrame("back_tyre");
	m_FrameTailRotor = FindFrame("rescue_2");
	m_FrameTopRotor = FindFrame("rescue_1");
	m_pChairPoint = FindFrame("ChairPoint");
}

void CHelicopterObjects::Animate(float fTimeElapsed)
{
	XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 7.1) * fTimeElapsed);
	m_xoobb = BoundingOrientedBox(GetToParentPosition(), { 4,6,8 }, { 0, 0, 0, 1 });
	if (m_FrameTopRotor)
		m_FrameTopRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTopRotor->m_xmf4x4ToParent);

	if (m_FrameTailRotor)
		m_FrameTailRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTailRotor->m_xmf4x4ToParent);

	if (m_bDyingstate == true)
		m_bDyingMotion = true;

	if (m_bDyingMotion == true)
		FallDown(fTimeElapsed);

	ParticlePosition = this->GetPosition();

	float FallingMaxHeight = -18.5f;
	if (m_FrameTailRotor->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameTopRotor->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameCleanser_2->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameHeliBody->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameRightDoor->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameLeftDoor->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameRighttyre->m_xmf4x4ToParent._42 > FallingMaxHeight &&
		m_FrameBacktyre->m_xmf4x4ToParent._42 > FallingMaxHeight)
	{
		m_bPartitionfalldownEnd = true;
	}

	GameObjectMgr::Animate(fTimeElapsed);
}

void CHelicopterObjects::FallDown(float fTimeElapsed)
{
	XMFLOAT3 gravity = XMFLOAT3(0.0, -5.5, 0);
	m_fElapsedTimes += fTimeElapsed * 1.1f;

	float FallingMaxHeight = -17.5f;
	float staticValue = 7.3f;
	float staticValueZ = 6.3f;
	XMVECTOR staticDir1 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.95, -0.5, 0.95, 0.0)));
	XMVECTOR staticDir2 = XMVector3Normalize(XMVECTOR(XMVectorSet(-0.95, -0.5, 0.85, 0.0)));
	XMVECTOR staticDir3 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.95, -0.5, 0.95, 0.0)));
	XMVECTOR staticDir4 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.95, -0.5, -0.95, 0.0)));
	XMVECTOR staticDir5 = XMVector3Normalize(XMVECTOR(XMVectorSet(-0.95, -0.5, -0.85, 0.0)));
	XMVECTOR staticDir6 = XMVector3Normalize(XMVECTOR(XMVectorSet(0.95, -0.5, 0.95, 0.0)));
	XMVECTOR staticDir7 = XMVector3Normalize(XMVECTOR(XMVectorSet(-0.93, -0.5, -0.95, 0.0)));
	for (int i = 0; i < 2; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir1);
	for (int i = 2; i < 4; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir2);
	for (int i = 4; i < 5; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir3);
	for (int i = 5; i < 6; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir4);
	for (int i = 6; i < 7; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir5);
	for (int i = 7; i < 9; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir6);
	for (int i = 9; i < EXPLOSION_HELICOPTER; i++)XMStoreFloat3(&m_pxmf3SphereVectors[i], staticDir7);

	float rotrowValue = XMConvertToRadians(360.0f * 0.4) * fTimeElapsed;
	XMMATRIX xmmtxRotateRow = XMMatrixRotationRollPitchYaw(rotrowValue, rotrowValue, rotrowValue);
	float rotFastValue = XMConvertToRadians(360.0f * 1.8) * fTimeElapsed;
	XMMATRIX xmmtxRotateFast = XMMatrixRotationRollPitchYaw(rotFastValue, rotFastValue, rotFastValue);

	if (m_FrameTailRotor->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		float rotValue = XMConvertToRadians(360.0f * 1.8) * fTimeElapsed;
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(rotValue, rotValue, rotValue);
		m_FrameTailRotor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[0].x * staticValue * fTimeElapsed;
		m_FrameTailRotor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[0].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameTailRotor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[0].z * staticValueZ * fTimeElapsed;
		m_FrameTailRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTailRotor->m_xmf4x4ToParent);
	}
	if (m_FrameTopRotor->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		float rotValue = XMConvertToRadians(360.0f * 0.4) * fTimeElapsed;
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(rotValue, rotValue, rotValue);
		m_FrameTopRotor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[1].x * staticValue * fTimeElapsed;
		m_FrameTopRotor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[1].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameTopRotor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[1].z * staticValueZ * fTimeElapsed;
		m_FrameTopRotor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameTopRotor->m_xmf4x4ToParent);
	}
	if (m_FrameCleanser_2->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		float rotValue = XMConvertToRadians(360.0f * 1.8) * fTimeElapsed;
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(rotValue, rotValue, rotValue);
		m_FrameCleanser_2->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[5].x * staticValue * fTimeElapsed;
		m_FrameCleanser_2->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[5].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameCleanser_2->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[5].z * staticValueZ * fTimeElapsed;
		m_FrameCleanser_2->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameCleanser_2->m_xmf4x4ToParent);
	}
	if (m_FrameHeliBody->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		float rotValue = XMConvertToRadians(360.0f * 0.4) * fTimeElapsed;
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(rotValue, rotValue, rotValue);
		m_FrameHeliBody->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[6].x * staticValue * fTimeElapsed;
		m_FrameHeliBody->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[6].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;		//
		m_FrameHeliBody->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[6].z * staticValueZ * fTimeElapsed;
		m_FrameHeliBody->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameHeliBody->m_xmf4x4ToParent);
	}
	if (m_FrameRightDoor->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		float rotValue = XMConvertToRadians(360.0f * 2.8) * fTimeElapsed;
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(rotValue, rotValue, rotValue);
		m_FrameRightDoor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[7].x * staticValue * fTimeElapsed;
		m_FrameRightDoor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[7].z * staticValueZ * fTimeElapsed;
		m_FrameRightDoor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[7].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameRightDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameRightDoor->m_xmf4x4ToParent);
	}
	if (m_FrameLeftDoor->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		float rotValue = XMConvertToRadians(360.0f * 0.4) * fTimeElapsed;
		XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(rotValue, rotValue, rotValue);
		m_FrameLeftDoor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[9].x * staticValue * fTimeElapsed;
		m_FrameLeftDoor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[9].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameLeftDoor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[9].z * staticValueZ * fTimeElapsed;
		m_FrameLeftDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_FrameLeftDoor->m_xmf4x4ToParent);
	}
	if (m_FrameRighttyre->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		m_FrameRighttyre->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[10].x * staticValue * fTimeElapsed;
		m_FrameRighttyre->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[10].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameRighttyre->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[10].z * staticValueZ * fTimeElapsed;
	}
	if (m_FrameBacktyre->m_xmf4x4ToParent._42 > FallingMaxHeight) {
		m_FrameBacktyre->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[11].x * staticValue * fTimeElapsed;
		m_FrameBacktyre->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[11].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
		m_FrameBacktyre->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[11].z * staticValueZ * fTimeElapsed;
	}
	m_FrameBackDoor->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[8].x * staticValue * fTimeElapsed;
	m_FrameBackDoor->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[8].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameBackDoor->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[8].z * staticValueZ * fTimeElapsed;
	m_FrameBackDoor->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateRow, m_FrameBackDoor->m_xmf4x4ToParent);

	m_FrameHeliglass->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[2].x * staticValue * fTimeElapsed;
	m_FrameHeliglass->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[2].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameHeliglass->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[2].z * staticValueZ * fTimeElapsed;
	m_FrameHeliglass->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateRow, m_FrameHeliglass->m_xmf4x4ToParent);


	m_FrameCleanse->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[3].x * staticValue * fTimeElapsed;
	m_FrameCleanse->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[3].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameCleanse->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[3].z * staticValueZ * fTimeElapsed;
	m_FrameCleanse->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateFast, m_FrameCleanse->m_xmf4x4ToParent);


	m_FrameLefttyre->m_xmf4x4ToParent._41 += m_pxmf3SphereVectors[4].x * staticValue * fTimeElapsed;
	m_FrameLefttyre->m_xmf4x4ToParent._42 += m_pxmf3SphereVectors[4].y * staticValue * fTimeElapsed + 0.5f * gravity.y * fTimeElapsed * fTimeElapsed;
	m_FrameLefttyre->m_xmf4x4ToParent._43 += m_pxmf3SphereVectors[4].z * staticValueZ * fTimeElapsed;
	m_FrameLefttyre->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotateFast, m_FrameLefttyre->m_xmf4x4ToParent);

	GameObjectMgr::Animate(fTimeElapsed);
}


void CHelicopterObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	GameObjectMgr::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSoldiarNpcObjects::CSoldiarNpcObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks) : GameObjectMgr(21)
{
	SetChild(pModel->m_pModelRootObject, true);
	pModel->m_pModelRootObject->SetCurScene(INGAME_SCENE);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, ANIMATIONTRACTS_NPCPLAYER, pModel);

	for (int i = 0; i < ANIMATIONTRACTS_NPCPLAYER; i++) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(0, true);
		if (i != 0)m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

CSoldiarNpcObjects::~CSoldiarNpcObjects()
{

}

void CSoldiarNpcObjects::Move(float EleapsedTime)
{

	for (int i = 0; i < ANIMATIONTRACTS_NPCPLAYER; i++) {
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
		if (i != 1)m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);


	GameObjectMgr::Animate(EleapsedTime);
}

void CSoldiarNpcObjects::HittingState(float EleapsedTime)
{

	for (int i = 0; i < ANIMATIONTRACTS_NPCPLAYER; i++) {
		m_pSkinnedAnimationController->SetTrackEnable(3, true);
		if (i != 3)m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	GameObjectMgr::Animate(EleapsedTime);
}

void CSoldiarNpcObjects::ReloadState(float EleapsedTime)
{

	//m_pSkinnedAnimationController->SetTrackEnable(0, false);
	//m_pSkinnedAnimationController->SetTrackEnable(1, false);
	//m_pSkinnedAnimationController->SetTrackEnable(2, false);
	//m_pSkinnedAnimationController->SetTrackEnable(3, false);
	//m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	//CGameObject::Animate(EleapsedTime);
}

void CSoldiarNpcObjects::DyingMotion(float EleapsedTime)
{
	for (int i = 0; i < ANIMATIONTRACTS_NPCPLAYER; i++) {
		m_pSkinnedAnimationController->SetTrackEnable(4, true);
		if (i != 4)m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
	GameObjectMgr::Animate(EleapsedTime);
}

void CSoldiarNpcObjects::ShotState(float EleapsedTime)
{
	for (int i = 0; i < ANIMATIONTRACTS_NPCPLAYER; i++) {
		m_pSkinnedAnimationController->SetTrackEnable(2, true);
		if (i != 2)m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	GameObjectMgr::Animate(EleapsedTime);
}

void CSoldiarNpcObjects::IdleState(float EleapsedTime)
{
	for (int i = 0; i < ANIMATIONTRACTS_NPCPLAYER; i++) {
		m_pSkinnedAnimationController->SetTrackEnable(0, true);
		if (i != 0)m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);

	GameObjectMgr::Animate(EleapsedTime);
}

void CSoldiarNpcObjects::Animate(float fTimeElapsed)
{
	GameObjectMgr::Animate(fTimeElapsed);
}

void CSoldiarNpcObjects::Firevalkan(XMFLOAT3 ToPlayerLook)
{

}

CBilldingObject::CBilldingObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) :GameObjectMgr(2)
{

}

CBilldingObject::~CBilldingObject()
{
}

CCityObject::CCityObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) :GameObjectMgr(5)
{
}

CCityObject::~CCityObject()
{
}
void CCityObject::OnPrepareAnimate()
{
}

void CCityObject::Animate(float fTimeElapsed)
{
}

CSoldiarOtherPlayerObjects::CSoldiarOtherPlayerObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks) : GameObjectMgr(6)
{

	SetChild(pModel->m_pModelRootObject, true);
	pModel->m_pModelRootObject->SetCurScene(INGAME_SCENE);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 11, pModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
	m_pSkinnedAnimationController->SetTrackAnimationSet(6, 6);
	m_pSkinnedAnimationController->SetTrackAnimationSet(7, 7);
	m_pSkinnedAnimationController->SetTrackAnimationSet(8, 8);
	m_pSkinnedAnimationController->SetTrackAnimationSet(9, 9);
	m_pSkinnedAnimationController->SetTrackAnimationSet(10, 10);


	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CSoldiarOtherPlayerObjects::~CSoldiarOtherPlayerObjects()
{
}

void CSoldiarOtherPlayerObjects::Move(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, true);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	GameObjectMgr::Animate(EleapsedTime);
}

void CSoldiarOtherPlayerObjects::MoveBackward(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, true);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::MoveLeft(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, true);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::MoveRight(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, true);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::ReloadState(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, true);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 5);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::JumpState(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::DyingMotion(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, true);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 8);

	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::ShotState(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, true);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 6);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::IdleState(float EleapsedTime)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(6, false);
	m_pSkinnedAnimationController->SetTrackEnable(7, false);
	m_pSkinnedAnimationController->SetTrackEnable(8, false);
	m_pSkinnedAnimationController->SetTrackEnable(9, false);
	m_pSkinnedAnimationController->SetTrackEnable(10, false);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	GameObjectMgr::Animate(EleapsedTime);
}
void CSoldiarOtherPlayerObjects::Animate(float fTimeElapsed)
{
	m_xoobb = BoundingOrientedBox(GetToParentPosition(), { 4, 6,4 }, { 0, 0, 0, 1 });
	GameObjectMgr::Animate(fTimeElapsed);
}



COpeningHuman::COpeningHuman(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks) : GameObjectMgr(1)
{
	SetChild(pModel->m_pModelRootObject, true);
	pModel->m_pModelRootObject->SetCurScene(OPENING_SCENE);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 3, pModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);

	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

COpeningHuman::~COpeningHuman()
{
}

void COpeningHuman::Animate(float fTimeElapsed)
{

	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, true);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackPosition(0, 1);

	GameObjectMgr::Animate(fTimeElapsed);
}

CInsideHelicopterHuman::CInsideHelicopterHuman(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks) : GameObjectMgr(1)
{
	SetChild(pModel->m_pModelRootObject, true);
	pModel->m_pModelRootObject->SetCurScene(INGAME_SCENE);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 3, pModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);

	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CInsideHelicopterHuman::~CInsideHelicopterHuman()
{
}

void CInsideHelicopterHuman::Animate(float fTimeElapsed)
{
	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, true);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackPosition(0, 1);

	GameObjectMgr::Animate(fTimeElapsed);
}
