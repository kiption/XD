//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "SkinMesh.h"
#include "TerrainMesh.h"

#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20
#define DIR_SLIDEVEC				0X40

#define RESOURCE_TEXTURE1D			0x01
#define RESOURCE_TEXTURE2D			0x02
#define RESOURCE_TEXTURE2D_ARRAY	0x03	//[]
#define RESOURCE_TEXTURE2DARRAY		0x04
#define RESOURCE_TEXTURE_CUBE		0x05
#define RESOURCE_BUFFER				0x06
#define RESOURCE_STRUCTURED_BUFFER	0x07
//
//struct EXPLOSIONMATERIAL
//{
//	XMFLOAT4						m_xmf4Ambient;
//	XMFLOAT4						m_xmf4Diffuse;
//	XMFLOAT4						m_xmf4Specular; //(r,g,b,a=power)
//	XMFLOAT4						m_xmf4Emissive;
//};
struct MATERIAL
{
	XMFLOAT4						m_xmf4Ambient;
	XMFLOAT4						m_xmf4Diffuse;
	XMFLOAT4						m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4						m_xmf4Emissive;
};
struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
	MATERIAL						m_material;

	XMFLOAT4X4						m_xmf4x4Texture;
	XMINT2							m_xmi2TextureTiling;
	XMFLOAT2						m_xmf2TextureOffset;
	UINT							m_nType;

};

class CShader;
class CStandardShader;
class CBulletEffectShader;
class CValkanObject;
class CPlayer;
struct SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};
class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters, int nRows = 1, int nCols = 1);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType;

	int								m_nTextures = 0;
	ID3D12Resource** m_ppd3dTextures = NULL;
	ID3D12Resource** m_ppd3dTextureUploadBuffers;
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	UINT* m_pnResourceTypes = NULL;

	DXGI_FORMAT* m_pdxgiBufferFormats = NULL;
	int* m_pnBufferElements = NULL;
	int* m_pnBufferStrides = NULL;

	int								m_nRootParameters = 0;
	UINT* m_pnRootParameterIndices = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

	int 							m_nRow = 0;
	int 							m_nCol = 0;
public:
	SRVROOTARGUMENTINFO* m_pRootArgumentInfos = NULL;
public:
	int 							m_nRows = 1;
	int 							m_nCols = 1;

	XMFLOAT4X4						m_xmf4x4Texture;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();



	//	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex);
	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	//	void LoadBufferFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex);
	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
	virtual ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);
	virtual ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex);


	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);
	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);

	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	ID3D12Resource* GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);
	void CreateBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT dxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, UINT nIndex);

	void ReleaseUploadBuffers();

	void Animate() { }
	void AnimateRowColumn(float fTime = 0.0f);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_MAP			0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_METALLIC_MAP			0x08
#define MATERIAL_EMISSION_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

class CGameObject;
struct CB_FRAMEWORK_INFO
{
	bool					m_bAnimationShader = false;
};
class CMaterial
{
public:
	CMaterial(int nTextures);
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CShader* m_pShader = NULL;
	UINT							m_nReflection = 0;
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	void SetShader(CShader* pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	virtual void SetTextures(CTexture* pTexture);
	virtual void SetTexture(CTexture* pTexture, UINT nTexture = 0);
	void SetReflection(UINT nReflection) { m_nReflection = nReflection; }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void ReleaseUploadBuffers();

public:
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	bool m_isAnimationShader = false;
public:
	int 							m_nTextures = 0;
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	CTexture** m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
	CTexture* m_pTexture = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader);

public:
	static CShader* m_pStandardShader;
	static CShader* m_pSkinnedAnimationShader;

	static void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); m_isAnimationShader = true; }


};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct CALLBACKKEY
{
	float  							m_fTime = 0.0f;
	void* m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	~CAnimationCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition) { }
};

//#define _WITH_ANIMATION_SRT

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
	~CAnimationSet();

public:
	char							m_pstrAnimationSetName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond

	int								m_nKeyFrames = 0;
	float* m_pfKeyFrameTimes = NULL;
	XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL;
	CAnimationCallbackHandler* m_pAnimationCallbackHandler = NULL;
	int 							m_nCallbackKeys = 0;
	CALLBACKKEY* m_pCallbackKeys = NULL;
	float 							m_fPosition = 0.0f;

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float* m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float* m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float* m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif

public:
	XMFLOAT4X4 GetSRT(int nBone, float fPosition);
	void HandleCallback();
};

class CAnimationSets
{
public:
	CAnimationSets(int nAnimationSets);
	~CAnimationSets();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	int								m_nAnimationSets = 0;
	CAnimationSet** m_pAnimationSets = NULL;

	int								m_nAnimatedBoneFrames = 0;
	CGameObject** m_ppAnimatedBoneFrameCaches = NULL; //[m_nAnimatedBoneFrames]
};

class CAnimationTrack
{
public:
	CAnimationTrack() { }
	~CAnimationTrack();

public:
	BOOL 							m_bEnable = true;
	float 							m_fSpeed = 1.0f;
	float 							m_fPosition = -ANIMATION_CALLBACK_EPSILON;
	float 							m_fWeight = 1.0f;

	int 							m_nAnimationSet = 0;

	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong

	int 							m_nCallbackKeys = 0;
	CALLBACKKEY* m_pCallbackKeys = NULL;

	CAnimationCallbackHandler* m_pAnimationCallbackHandler = NULL;

public:
	void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }

	void SetPosition(float fPosition) { m_fPosition = fPosition; }
	float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);
	bool m_bAnimationEnd = false;

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler);

	void HandleCallback();
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

	CGameObject* m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationSets* m_pAnimationSets = NULL;

public:
	void PrepareSkinning();
};

class CAnimationController
{
public:
	CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CAnimationController();

public:
	float 							m_fTime = 0.0f;

	int 							m_nAnimationTracks = 0;
	CAnimationTrack* m_pAnimationTracks = NULL;

	CAnimationSets* m_pAnimationSets = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	ID3D12Resource** m_ppd3dcbSkinningBoneTransforms = NULL; //[SkinnedMeshes]
	XMFLOAT4X4** m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes]

public:
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
	void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler);

	void AdvanceTime(float fElapsedTime, CGameObject* pRootGameObject);

public:
	bool							m_bRootMotion = false;
	CGameObject* m_pModelRootObject = NULL;

	CGameObject* m_pRootMotionObject = NULL;
	XMFLOAT3						m_xmf3FirstRootMotionPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	void SetRootMotion(bool bRootMotion) { m_bRootMotion = bRootMotion; }

	virtual void OnRootMotion(CGameObject* pRootGameObject) { }
	virtual void OnAnimationIK(CGameObject* pRootGameObject) { }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CGameObject();
	CGameObject(int nMaterials);
	CGameObject(int nMeshes, int nMaterials);
	virtual ~CGameObject();

public:
	char							m_pstrFrameName[64];

	CMesh* m_pMesh = NULL;
	CMesh** m_ppMeshes = NULL;
	int m_nMeshes = NULL;

	int								m_nMaterials = 0;
	CMaterial** m_ppMaterials = NULL;
	CMaterial* m_pMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObject* m_pParent = NULL;
	CGameObject* m_pChild = NULL;
	CGameObject* m_pSibling = NULL;
	CShader* m_pShaderInfo = NULL;
	CTexture* m_pTextureInfo = NULL;


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
	virtual void SetMesh(CMesh* pMesh);
	virtual void SetMesh(int nIndex, CMesh* pMesh);
	void SetShader(CShader* pShader);
	void SetShader(int nMaterial, CShader* pShader);
	virtual void SetMaterial(int nMaterial, CMaterial* pMaterial);
	virtual void SetMaterial(CMaterial* pMaterial);


	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void Animate(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent);
	virtual void AnimateObject(CCamera* pCamera, float fTimeElapsed);
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPreRender = false);
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bPrerender, CShader* pShaderComponent);
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
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	CGameObject* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	CGameObject* FindFrame(char* pstrFrameName);

	CTexture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

	void SetCurScene(int nCurScene) { m_nCurScene = nCurScene; }
	int m_nCurScene = INGAME_SCENE;
public:
	CSkinnedMesh* FindSkinnedMesh(char* pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);
	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

	static CGameObject* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, CShader* pShader);
	static CGameObject* LoadGeometryHierachyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const char* pstrFileName, CShader* pShader);

	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CNpcHelicopterObject : public CGameObject
{
public:
	CNpcHelicopterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CNpcHelicopterObject();

private:
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;
	CGameObject* m_pTail2RotorFrame = NULL;
public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CBulletObject;
class CValkanObject;
class CNPCbulletObject;
class CHelicopterObjects : public CGameObject
{
public:
	CHelicopterObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pmodel, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CHelicopterObjects();

public:
	CGameObject* m_pMainRotorFrame = NULL;
	CGameObject* m_pTailRotorFrame = NULL;
	CGameObject* m_pFrameFragObj1 = NULL;
	CGameObject* m_pFrameFragObj2 = NULL;
	CGameObject* m_pFrameFragObj3 = NULL;
	CGameObject* m_pFrameFragObj4 = NULL;
	CGameObject* m_pFrameFragObj5 = NULL;
	CGameObject* m_pFrameFragObj6 = NULL;
	CGameObject* m_pFrameFragObj7 = NULL;
	CGameObject* m_pFrameFragObj8 = NULL;
	CGameObject* m_pFrameFragObj9 = NULL;
	CGameObject* m_pFrameFragObj10 = NULL;
	CGameObject* m_pFrameFragObj11 = NULL;
	CGameObject* m_pChairPoint = NULL;

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
	CBulletEffectShader* pBCBulletEffectShader = NULL;
	CValkanObject* pBulletObject = NULL;
	CValkanObject* m_ppBullets[HELIBULLETS];
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
class CCityObject : public CGameObject
{
public:
	CCityObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CCityObject();

public:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);

};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSoldiarNpcObjects : public CGameObject
{
public:
	CSoldiarNpcObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CSoldiarNpcObjects();
	void MoveForward(float EleapsedTime);
	void HittingState(float EleapsedTime);
	void ReloadState(float EleapsedTime);
	void DieState(float EleapsedTime);
	void ShotState(float EleapsedTime);
	void IdleState(float EleapsedTime);
	virtual void Animate(float fTimeElapsed);


public:
	float m_fBulletEffectiveRange = 2000.0f;
	CBulletEffectShader* pBCBulletEffectShader = NULL;
	CNPCbulletObject* pBulletObject = NULL;
	CNPCbulletObject* m_ppBullets[HUMANBULLETS];
	void Firevalkan(XMFLOAT3 ToPlayerLook);

};


class COpeningHuman : public CGameObject
{
public:
	COpeningHuman(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~COpeningHuman();
	virtual void Animate(float fTimeElapsed);

};
class CInsideHelicopterHuman : public CGameObject
{
public:
	CInsideHelicopterHuman(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CInsideHelicopterHuman();
	virtual void Animate(float fTimeElapsed);

};
class CSoldiarOtherPlayerObjects : public CGameObject
{
public:
	CSoldiarOtherPlayerObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CSoldiarOtherPlayerObjects();
	void MoveForward(float EleapsedTime);
	void MoveBackward(float EleapsedTime);
	void MoveLeft(float EleapsedTime);
	void MoveRight(float EleapsedTime);
	void ReloadState(float EleapsedTime);
	void JumpState(float EleapsedTime);
	void DieState(float EleapsedTime);
	void ShotState(float EleapsedTime);
	void IdleState(float EleapsedTime);
	virtual void Animate(float fTimeElapsed);

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CRootMotionCallbackHandler : public CAnimationCallbackHandler
{
public:
	CRootMotionCallbackHandler() { }
	~CRootMotionCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

class CEthanAnimationController : public CAnimationController
{
public:
	CEthanAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CEthanAnimationController();

	virtual void OnRootMotion(CGameObject* pRootGameObject);
};

class CBilldingObject : public CGameObject
{
public:
	CBilldingObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CBilldingObject();
};





class CEthanObject : public CGameObject
{
public:
	CEthanObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CEthanObject();
};
