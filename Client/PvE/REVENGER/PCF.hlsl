
#define MAX_LIGHTS			8
#define MAX_MATERIALS		16

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT
#define _WITH_PCF_FILTERING
#define MAX_DEPTH_TEXTURES	MAX_LIGHTS

#define FRAME_BUFFER_WIDTH		1920
#define FRAME_BUFFER_HEIGHT		1080

#define _DEPTH_BUFFER_WIDTH		(FRAME_BUFFER_WIDTH * 4)
#define _DEPTH_BUFFER_HEIGHT	(FRAME_BUFFER_HEIGHT * 4)

#define DELTA_X					(1.0f / _DEPTH_BUFFER_WIDTH)
#define DELTA_Y					(1.0f / _DEPTH_BUFFER_HEIGHT)


#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);
Texture2D gtxtBillboardTexture : register(t14);

TextureCube gtxtSkyCubeTexture : register(t13);
Buffer<float4> gRandomBuffer : register(t16);
Buffer<float4> gRandomSphereBuffer : register(t17);
Texture2D<float4> gtxtParticleTexture : register(t15);
Texture2D<float> gtxtDepthTextures[MAX_DEPTH_TEXTURES] : register(t18);

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);
SamplerComparisonState gssComparisonPCFShadow : register(s2);
SamplerState gssBorder : register(s3);

struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};
cbuffer cbMaterial : register(b10)
{
	MATERIAL			gMaterials[MAX_MATERIALS];
};
struct LIGHT
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular;
	float4					m_cEmissive;
	float3					m_vPosition;
	float 					m_fFalloff;
	float3					m_vDirection;
	float 					m_fTheta; //cos(m_fTheta)
	float3					m_vAttenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int 					m_nType;
	float					m_fRange;
	float					padding;
};
cbuffer cbLights : register(b4)
{
	LIGHT					gLights[MAX_LIGHTS];
	float4					gcGlobalAmbientLight;
	int						gnLights;
};
struct EXPLOSIONMATERIAL
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular; //a = power
	float4				m_cEmissive;

	matrix				gmtxTexture;
	int2				gi2TextureTiling;
	float2				gf2TextureOffset;
};
cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	matrix		gmtxInverseView : packoffset(c8);
	float3		gvCameraPosition : packoffset(c12);
};
cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8);
	uint					gbBoneShader : packoffset(c8.y);
	uint					gnMaterial : packoffset(c12);

};
cbuffer cbFrameworkInfo : register(b11)
{
	float		gfCurrentTime : packoffset(c0.x);
	float		gfElapsedTime : packoffset(c0.y);
	float		gfSecondsPerFirework : packoffset(c0.z);
	int			gnFlareParticlesToEmit : packoffset(c0.w);;
	float3		gf3Gravity : packoffset(c1.x);
	int			gnMaxFlareType2Particles : packoffset(c1.w);;
};
cbuffer cbStreamGameObjectInfo : register(b9)
{
	matrix		gmtxWorld : packoffset(c0);
	EXPLOSIONMATERIAL	gAniMaterial : packoffset(c4);
};
struct CB_TOOBJECTSPACE
{
	matrix		mtxToTexture;
	float4		f4Position;
};
cbuffer cbToLightSpace : register(b12)
{
	CB_TOOBJECTSPACE gcbToLightSpaces[MAX_LIGHTS];
};
struct VS_CIRCULAR_SHADOW_INPUT
{
	float3 center : POSITION;
	float2 size : TEXCOORD;
};
struct GS_CIRCULAR_SHADOW_GEOMETRY_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};
struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float2 uv : TEXCOORD;
	float4 uvs[MAX_LIGHTS] : TEXCOORD1;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};