//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "SkinMesh.h"
#include "TerrainMesh.h"
#include "Texture.h"
#include "Animation.h"
#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20
#define DIR_SLIDEVEC				0X40

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
	MATERIAL						m_material;

	XMFLOAT4X4						m_xmf4x4Texture;
	XMINT2							m_xmi2TextureTiling;
	XMFLOAT2						m_xmf2TextureOffset;
	UINT							m_nType;

};

class ShaderMgr;
class StandardShader;
class BulletEffectShader;
class CtridgeObject;
class PlayerMgr;


struct CB_FRAMEWORK_INFO
{
	bool					m_bAnimationShader = false;
};



class GameObjectMgr
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	GameObjectMgr();
	GameObjectMgr(int nMaterials);
	GameObjectMgr(int nMeshes, int nMaterials);
	virtual ~GameObjectMgr();

public:
	char							m_pstrFrameName[64];

	Mesh* m_pMesh = NULL;
	Mesh** m_ppMeshes = NULL;
	int m_nMeshes = NULL;

	int								m_nMaterials = 0;
	CMaterial** m_ppMaterials = NULL;
	CMaterial* m_pMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	GameObjectMgr* m_pParent = NULL;
	GameObjectMgr* m_pChild = NULL;
	GameObjectMgr* m_pSibling = NULL;
	ShaderMgr* m_pShaderInfo = NULL;
	Texture* m_pTextureInfo = NULL;


	BoundingBox	m_xmBoundingBox;
	CBoundingBoxMesh* m_pBoundingBoxMesh = NULL;
	BoundingOrientedBox m_xoobb = BoundingOrientedBox(XMFLOAT3(), XMFLOAT3(), XMFLOAT4());
	BoundingOrientedBox GetBoundingBox() { return(m_xoobb); }
	void UpdateBoundingBox();
	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
	//게임 객체가 카메라에 보인는 가를 검사한다.
	bool IsVisible(CCamera* pCamera = NULL);
public:
	ID3D12Resource* m_pd3dcbGameObject = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;
public:
	float m_fMovingSpeed = 0.0f;
	float m_fMovingRange = 0.0f;
	float m_fRotationSpeed = 0.0f;
	bool m_bActive = false;
	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection);
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetActive(bool bActive) { m_bActive = bActive; }

public:

	XMFLOAT3 m_xmf3RotationAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);

	CAnimationController* m_pSkinnedAnimationController = NULL;
	CAnimationTrack* m_ppAnimationTrack = NULL;
	virtual void SetMesh(Mesh* pMesh);
	virtual void SetMesh(int nIndex, Mesh* pMesh);
	void SetShader(ShaderMgr* pShader);
	void SetShader(int nMaterial, ShaderMgr* pShader);
	virtual void SetMaterial(int nMaterial, CMaterial* pMaterial);
	virtual void SetMaterial(CMaterial* pMaterial);


	void SetChild(GameObjectMgr* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent);
	virtual void AnimateObject(CCamera* pCamera, float fTimeElapsed);
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPreRender = false);
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPrerender, ShaderMgr* pShaderComponent);
	virtual void OnLateUpdate() { }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMMATRIX* pxmf4x4Shadow);

	virtual void ReleaseUploadBuffers();

	void CalculateBoundingBox();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	XMFLOAT3 GetToParentPosition();
	void Move(XMFLOAT3 xmf3Offset);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);
	void SetLookAt(XMFLOAT3 xmf3Target, XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void Move(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	GameObjectMgr* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	GameObjectMgr* FindFrame(char* pstrFrameName);

	Texture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

	void SetCurScene(int nCurScene) { m_nCurScene = nCurScene; }
	int m_nCurScene = INGAME_SCENE;
	SceneMgr* m_pScene = NULL;
public:
	CSkinnedMesh* FindSkinnedMesh(char* pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, GameObjectMgr* pParent, FILE* pInFile, ShaderMgr* pShader);

	static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);
	static GameObjectMgr* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, GameObjectMgr* pParent, FILE* pInFile, ShaderMgr* pShader, int* pnSkinnedMeshes);
	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, ShaderMgr* pShader);

	static GameObjectMgr* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, ShaderMgr* pShader);
	static GameObjectMgr* LoadGeometryHierachyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, ShaderMgr* pShader);

	static void PrintFrameInfo(GameObjectMgr* pGameObject, GameObjectMgr* pParent);

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }

public:
	// Server
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf4x4ToParent._21 = xmf3Up.x, m_xmf4x4ToParent._22 = xmf3Up.y, m_xmf4x4ToParent._23 = xmf3Up.z; }
	void SetRight(XMFLOAT3 xmf3right) { m_xmf4x4ToParent._11 = xmf3right.x; m_xmf4x4ToParent._12 = xmf3right.y; m_xmf4x4ToParent._13 = xmf3right.z; }
	void SetLook(XMFLOAT3 xmf3look) { m_xmf4x4ToParent._31 = xmf3look.x; m_xmf4x4ToParent._32 = xmf3look.y; m_xmf4x4ToParent._33 = xmf3look.z; }

};

class CNpcHelicopterObject : public GameObjectMgr
{
public:
	CNpcHelicopterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CNpcHelicopterObject();

private:
	GameObjectMgr* m_FrameTopRotor = NULL;
	GameObjectMgr* m_FrameTailRotor = NULL;
	GameObjectMgr* m_pTail2RotorFrame = NULL;
public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);
};

class CBulletObject;
class CNPCbulletObject;
class CHelicopterObjects : public GameObjectMgr
{
public:
	CHelicopterObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, GameObjectMgr* pmodel, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CHelicopterObjects();

public:
	GameObjectMgr* m_FrameTopRotor = NULL;
	GameObjectMgr* m_FrameTailRotor = NULL;
	GameObjectMgr* m_FrameHeliglass = NULL;
	GameObjectMgr* m_FrameCleanse = NULL;
	GameObjectMgr* m_FrameLefttyre = NULL;
	GameObjectMgr* m_FrameCleanser_2 = NULL;
	GameObjectMgr* m_FrameHeliBody = NULL;
	GameObjectMgr* m_FrameRightDoor = NULL;
	GameObjectMgr* m_FrameBackDoor = NULL;
	GameObjectMgr* m_FrameLeftDoor = NULL;
	GameObjectMgr* m_FrameRighttyre = NULL;
	GameObjectMgr* m_FrameBacktyre = NULL;
	GameObjectMgr* m_pFrameFragObj11 = NULL;
	GameObjectMgr* m_pChairPoint = NULL;

	BoundingOrientedBox ParticleFrame1;
	BoundingOrientedBox ParticleFrame2;
	BoundingOrientedBox ParticleFrame3;
	BoundingOrientedBox ParticleFrame4;
	BoundingOrientedBox ParticleFrame5;
	BoundingOrientedBox ParticleFrame6;
	BoundingOrientedBox ParticleFrame7;
	BoundingOrientedBox ParticleFrame8;
	BoundingOrientedBox ParticleFrame9;
	BoundingOrientedBox ParticleFrame10;
	BoundingOrientedBox ParticleFrame11;
	BoundingOrientedBox ParticleFrame12;

	bool m_bPartitionfalldownEnd = false;
	XMFLOAT3 ParticlePosition{};
	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_HELICOPTER];
	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 25.0f;
	float						m_fExplosionSpeed = 1.0f;
	float						m_fExplosionRotation = 5.0f;
	XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_HELICOPTER];
public:
	XMFLOAT4X4 m_pMainRotorFrameP{};
	XMFLOAT4X4 m_pTailRotorFrameP{};
	XMFLOAT4X4 m_pFrameFragObj1P{};
	XMFLOAT4X4 m_pFrameFragObj2P{};
	XMFLOAT4X4 m_pFrameFragObj3P{};
	XMFLOAT4X4 m_pFrameFragObj4P{};
	XMFLOAT4X4 m_pFrameFragObj5P{};
	XMFLOAT4X4 m_pFrameFragObj6P{};
	XMFLOAT4X4 m_pFrameFragObj7P{};
	XMFLOAT4X4 m_pFrameFragObj8P{};
	XMFLOAT4X4 m_pFrameFragObj9P{};
	XMFLOAT4X4 m_pFrameFragObj10P{};
	XMFLOAT4X4 m_pFrameFragObj11P{};
	XMFLOAT4X4 m_pResetCameraPos;
	void Resetpartition();
	CCamera* m_pCamera = NULL;
public:
	float m_fBulletEffectiveRange = 2000.0f;
	BulletEffectShader* pBCBulletEffectShader = NULL;
	CtridgeObject* pBulletObject = NULL;
	CtridgeObject* m_ppBullets[HELIBULLETS];
	void Firevalkan(XMFLOAT3 ToPlayerLook);
public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);
	virtual void FallDown(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
public:
	bool m_bDyingstate = false;
	bool m_bDyingMotion = false;
};
class CCityObject : public GameObjectMgr
{
public:
	CCityObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CCityObject();

public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);

};


class CSoldiarNpcObjects : public GameObjectMgr
{
public:
	CSoldiarNpcObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CSoldiarNpcObjects();
	void Move(float EleapsedTime);
	void HittingState(float EleapsedTime);
	void ReloadState(float EleapsedTime);
	void DyingMotion(float EleapsedTime);
	void ShotState(float EleapsedTime);
	void IdleState(float EleapsedTime);
	virtual void Animate(float fTimeElapsed);


public:
	float m_fBulletEffectiveRange = 2000.0f;
	BulletEffectShader* pBCBulletEffectShader = NULL;
	CNPCbulletObject* pBulletObject = NULL;
	CNPCbulletObject* m_ppBullets[HUMANBULLETS];
	void Firevalkan(XMFLOAT3 ToPlayerLook);

};


class COpeningHuman : public GameObjectMgr
{
public:
	COpeningHuman(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~COpeningHuman();
	virtual void Animate(float fTimeElapsed);

};
class CInsideHelicopterHuman : public GameObjectMgr
{
public:
	CInsideHelicopterHuman(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CInsideHelicopterHuman();
	virtual void Animate(float fTimeElapsed);

};
class CSoldiarOtherPlayerObjects : public GameObjectMgr
{
public:
	CSoldiarOtherPlayerObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CSoldiarOtherPlayerObjects();
	void Move(float EleapsedTime);
	void MoveBackward(float EleapsedTime);
	void MoveLeft(float EleapsedTime);
	void MoveRight(float EleapsedTime);
	void ReloadState(float EleapsedTime);
	void JumpState(float EleapsedTime);
	void DyingMotion(float EleapsedTime);
	void ShotState(float EleapsedTime);
	void IdleState(float EleapsedTime);
	virtual void Animate(float fTimeElapsed);

};
class CBilldingObject : public GameObjectMgr
{
public:
	CBilldingObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBilldingObject();
};


