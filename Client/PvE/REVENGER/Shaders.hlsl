#define MAX_LIGHTS 8
//
#define PARTICLE_TYPE_EMITTER		0
#define PARTICLE_TYPE_SHELL			1
#define PARTICLE_TYPE_FLARE01		2
#define PARTICLE_TYPE_FLARE02		3
#define PARTICLE_TYPE_FLARE03		4

#define SHELL_PARTICLE_LIFETIME		3.0f
#define FLARE01_PARTICLE_LIFETIME	2.5f
#define FLARE02_PARTICLE_LIFETIME	1.5f
#define FLARE03_PARTICLE_LIFETIME	2.0f
struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
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
	uint					gnMaterial : packoffset(c12);
	uint					gnObjectID : packoffset(c16);
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

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

Texture2DArray gtxtTextureArray : register(t27);


SamplerState gssWrap : register(s0);

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
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
	float4 uvs[MAX_LIGHTS] : TEXCOORD1;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{

	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}


	float4 uvs[MAX_LIGHTS];
	//float4 cIllumination = Lighting(input.positionW, normalize(input.normalW));
	float4 cIllumination = Lighting(input.positionW, input.normalW, false, uvs);

	return(lerp(cColor, cIllumination, 0.5f));
}

float3 ParticleLogic()
{
	float3 newPosition = float3(0, 0, 0);
	float3 a_Position = float3(0, 0, 0);
	float t = gfElapsedTime - gnFlareParticlesToEmit;

	if (t < 0.0)
	{
	}
	else
	{
		float newT = FLARE01_PARTICLE_LIFETIME * frac(t / FLARE01_PARTICLE_LIFETIME);
		newPosition.xyz = a_Position + 10.0f * newT + 0.5 * 10.0 * newT * newT;
	}
	return newPosition;
}
struct VS_PARTICLES_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_PARTICLES_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_PARTICLES_OUTPUT VSParticleStandard(VS_PARTICLES_INPUT input)
{
	VS_PARTICLES_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalW += ParticleLogic();
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSParticleStandard(VS_PARTICLES_OUTPUT input) : SV_TARGET
{

	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}

	float4 uvs[MAX_LIGHTS];
	float4 cIllumination = ParticleLighting(input.positionW, normalize(input.normalW), false, uvs);

	return(lerp(cColor, cIllumination, 0.5f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float4 PSBulletStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.8f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float3 normalW;
	cEmissionColor = float4(0.9,0.2,0.1,1.0);
	cSpecularColor = float4(0.9,0.2,0.1,1.0);
	cAlbedoColor = float4(0.9,0.2,0.1,1.0);
	cMetallicColor = float4(0.9,0.2,0.1,1.0);
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * +2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}

	float4 uvs[MAX_LIGHTS];
	float4 cIllumination = ParticleLighting(input.positionW, input.normalW, false, uvs);

	return(lerp(cColor, cIllumination, 0.7f));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	//output.positionW = float3(0.0f, 0.0f, 0.0f);
	//output.normalW = float3(0.0f, 0.0f, 0.0f);
	//output.tangentW = float3(0.0f, 0.0f, 0.0f);
	//output.bitangentW = float3(0.0f, 0.0f, 0.0f);
	//matrix mtxVertexToBoneWorld;
	//for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	//{
	//	mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	//	output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	//	output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
	//	output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
	//	output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3)mtxVertexToBoneWorld);
	//}
	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		//		mtxVertexToBoneWorld += input.weights[i] * gpmtxBoneTransforms[input.indices[i]];
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}
	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

	//output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	//float4 color : COLOR;
	float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	//float4 color : COLOR;
	float3 normalW : NORMAL;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.normalW = (float3)mul(input.normal, (float3x3)gmtxProjection);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	//output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 uvs[MAX_LIGHTS];
	input.normalW = normalize(input.normalW);
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	//	float4 cColor = (cBaseTexColor * cDetailTexColor);
		//cIllumination =Lighting(input.positionW, input.normalW);
		float4 cIllumination = Lighting(input.positionW, input.normalW, true, uvs);


		cColor = lerp(cColor, cIllumination, 0.5f);
		return(cColor);
}

struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);
Texture2D gtxtBillboardTexture : register(t14);
Texture2D<float4> gtxtParticleTexture : register(t15);
Buffer<float4> gRandomBuffer : register(t16);
Buffer<float4> gRandomSphereBuffer : register(t17);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input, uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor = gtxtBillboardTexture.Sample(gssWrap, input.uv);

	return(cColor);
}
float4 PSIllumiTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);

	return(cColor);
}

VS_TEXTURED_OUTPUT VSSpritTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSSpritTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtBillboardTexture.Sample(gssWrap, input.uv);

	return(cColor);
}
VS_TEXTURED_OUTPUT VSSpriteAnimation(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.uv = mul(float3(input.uv, 1.0f), (float3x3)(gAniMaterial.gmtxTexture)).xy;

	return(output);
}
VS_TEXTURED_OUTPUT VSBillBoardTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return (output);

}
VS_TEXTURED_OUTPUT VSParticleBillBoardTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return (output);

}
float4 PSBillBoardTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{

	float4 cColor = gtxtBillboardTexture.Sample(gssWrap, input.uv);
	return (cColor);
}



float4 PSSmokeBillBoardTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{

	float4 cColor = gtxtBillboardTexture.Sample(gssWrap, input.uv);

	return (cColor);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//



struct VS_PARTICLE_INPUT
{
	float3 position : POSITION;
	float3 velocity : VELOCITY;
	float lifetime : LIFETIME;
	uint type : PARTICLETYPE;				// 파티클 타입에 따라 포인트 스트림에 넣는다.
};

VS_PARTICLE_INPUT VSParticleStreamOutput(VS_PARTICLE_INPUT input) // 스트림 출력을 위한 쉐이더
{
	return(input);
}


float3 GetParticleColor(float fAge, float fLifetime)
{
	float3 cColor = float3(1.0f, 1.0f, 1.0f);

	if (fAge == 0.0f) cColor = float3(0.0f, 1.0f, 0.0f);
	else if (fLifetime == 0.0f)
		cColor = float3(1.0f, 1.0f, 0.0f);
	else
	{
		float t = fAge / fLifetime;
		cColor = lerp(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), t * 1.0f);
	}

	return(cColor);
}

void GetBillboardCorners(float3 position, float2 size, out float4 pf4Positions[4])
{
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = normalize(gvCameraPosition - position);
	float3 f3Right = normalize(cross(f3Up, f3Look));

	pf4Positions[0] = float4(position + size.x * f3Right - size.y * f3Up, 1.0f);
	pf4Positions[1] = float4(position + size.x * f3Right + size.y * f3Up, 1.0f);
	pf4Positions[2] = float4(position - size.x * f3Right - size.y * f3Up, 1.0f);
	pf4Positions[3] = float4(position - size.x * f3Right + size.y * f3Up, 1.0f);
}

void GetPositions(float3 position, float2 f2Size, out float3 pf3Positions[8])
{
	float3 f3Right = float3(1.0f, 0.0f, 0.0f);
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = float3(0.0f, 0.0f, 1.0f);

	float3 f3Extent = normalize(float3(1.0f, 1.0f, 1.0f));

	pf3Positions[0] = position + float3(-f2Size.x, 0.0f, -f2Size.y);
	pf3Positions[1] = position + float3(-f2Size.x, 0.0f, +f2Size.y);
	pf3Positions[2] = position + float3(+f2Size.x, 0.0f, -f2Size.y);
	pf3Positions[3] = position + float3(+f2Size.x, 0.0f, +f2Size.y);
	pf3Positions[4] = position + float3(-f2Size.x, 0.0f, 0.0f);
	pf3Positions[5] = position + float3(+f2Size.x, 0.0f, 0.0f);
	pf3Positions[6] = position + float3(0.0f, 0.0f, +f2Size.y);
	pf3Positions[7] = position + float3(0.0f, 0.0f, -f2Size.y);
}
//
// 랜덤 값들
float4 RandomDirection(float fOffset)
{
	int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 1024;
	return(normalize(gRandomBuffer.Load(u)));
}

float4 RandomDirectionOnSphere(float fOffset)
{
	int u = uint(gfCurrentTime + fOffset + frac(gfCurrentTime) * 1000.0f) % 256;
	return(normalize(gRandomSphereBuffer.Load(u)));
}

void OutputParticleToStream(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
	input.position += input.velocity * gfElapsedTime;
	input.velocity += gf3Gravity * gfElapsedTime;
	input.lifetime -= gfElapsedTime;

	output.Append(input);
}

void EmmitParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
	float4 f4Random = RandomDirection(input.type);
	if (input.lifetime <= 0.0f)						// 수명이 다 할때
	{
		VS_PARTICLE_INPUT particle = input;

		particle.type = PARTICLE_TYPE_SHELL;
		particle.position = input.position + (input.velocity * gfElapsedTime * f4Random.xyz);
		particle.velocity = input.velocity + (f4Random.xyz * 16.0f);
		particle.lifetime = SHELL_PARTICLE_LIFETIME + (f4Random.y * 0.5f);

		output.Append(particle);					// 새로운 파티클을 형성

		input.lifetime = gfSecondsPerFirework * 0.2f + (f4Random.x * 0.4f);
	}
	else
	{
		input.lifetime -= gfElapsedTime;
	}

	output.Append(input);
}

void ShellParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
	if (input.lifetime <= 0.0f)
	{
		VS_PARTICLE_INPUT particle = input;
		float4 f4Random = float4(0.0f, 0.0f, 0.0f, 0.0f);

		particle.type = PARTICLE_TYPE_FLARE01;
		particle.position = input.position + (input.velocity * gfElapsedTime * 2.0f);
		particle.lifetime = FLARE01_PARTICLE_LIFETIME;

		for (int i = 0; i < gnFlareParticlesToEmit; i++)
		{
			f4Random = RandomDirection(input.type + i);
			particle.velocity = input.velocity + (f4Random.xyz * 18.0f);

			output.Append(particle);
		}

		particle.type = PARTICLE_TYPE_FLARE02;
		particle.position = input.position + (input.velocity * gfElapsedTime);
		for (int j = 0; j < abs(f4Random.x) * gnMaxFlareType2Particles; j++)
		{
			f4Random = RandomDirection(input.type + j);
			particle.velocity = input.velocity + (f4Random.xyz * 10.0f);
			particle.lifetime = FLARE02_PARTICLE_LIFETIME + (f4Random.x * 0.4f);

			output.Append(particle);
		}
	}
	else
	{
		OutputParticleToStream(input, output);
	}
}


void OutputEmberParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
	if (input.lifetime > 0.0f)
	{
		OutputParticleToStream(input, output);
	}
}

void GenerateEmberParticles(VS_PARTICLE_INPUT input, inout PointStream<VS_PARTICLE_INPUT> output)
{
	if (input.lifetime <= 0.0f)
	{
		VS_PARTICLE_INPUT particle = input;

		particle.type = PARTICLE_TYPE_FLARE03;
		particle.position = input.position + (input.velocity * gfElapsedTime);
		particle.lifetime = FLARE03_PARTICLE_LIFETIME;
		for (int i = 0; i < 64; i++)
		{
			float4 f4Random = RandomDirectionOnSphere(input.type + i);
			particle.velocity = input.velocity + (f4Random.xyz * 25.0f);

			output.Append(particle);
		}
	}
	else
	{
		OutputParticleToStream(input, output);
	}
}

[maxvertexcount(128)]
void GSParticleStreamOutput(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> output)
{
	VS_PARTICLE_INPUT particle = input[0];

	if (particle.type == PARTICLE_TYPE_EMITTER) EmmitParticles(particle, output);
	else if (particle.type == PARTICLE_TYPE_SHELL) ShellParticles(particle, output);			// Shell 타입은 파티클의 수명이 다 할떄, FLAR 파티클을 만든다.
	else if ((particle.type == PARTICLE_TYPE_FLARE01) || (particle.type == PARTICLE_TYPE_FLARE03)) OutputEmberParticles(particle, output);
	else if (particle.type == PARTICLE_TYPE_FLARE02) GenerateEmberParticles(particle, output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_PARTICLE_DRAW_OUTPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float size : SCALE;
	uint type : PARTICLETYPE;
};

struct GS_PARTICLE_DRAW_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXTURE;
	uint type : PARTICLETYPE;
};

VS_PARTICLE_DRAW_OUTPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
	VS_PARTICLE_DRAW_OUTPUT output = (VS_PARTICLE_DRAW_OUTPUT)0;

	output.position = input.position;
	output.size = 3.5f;
	output.type = input.type;

	if (input.type == PARTICLE_TYPE_EMITTER) { output.color = float4(1.01f, 0.01f, 0.01f, 1.0f); output.size = 3.0f; }
	else if (input.type == PARTICLE_TYPE_SHELL) { output.color = float4(1.03f, 0.0f, 0.1f, 1.0f); output.size = 3.0f; }
	else if (input.type == PARTICLE_TYPE_FLARE01) { output.color = float4(1.02f, 0.01f, 0.01f, 1.0f); output.color *= (input.lifetime / FLARE01_PARTICLE_LIFETIME); }
	else if (input.type == PARTICLE_TYPE_FLARE02) output.color = float4(1.01f, 0.0f, 0.01f, 1.0f);
	else if (input.type == PARTICLE_TYPE_FLARE03) { output.color = float4(1.01, 0.0f, 0.0f, 1.0f); output.color *= (input.lifetime / FLARE03_PARTICLE_LIFETIME); }

	return(output);
}
//
static float3 gf3Positions[4] = { float3(-1.0f, +1.0f, 0.5f), float3(+1.0f, +1.0f, 0.5f), float3(-1.0f, -1.0f, 0.5f), float3(+1.0f, -1.0f, 0.5f) };
static float2 gf2QuadUVs[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };
//
//
[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_DRAW_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_DRAW_OUTPUT> outputStream)
{
	GS_PARTICLE_DRAW_OUTPUT output = (GS_PARTICLE_DRAW_OUTPUT)0;

	output.type = input[0].type;
	output.color = input[0].color;
	for (int i = 0; i < 4; i++)
	{
		float3 positionW = mul(gf3Positions[i] * input[0].size, (float3x3)gmtxInverseView) + input[0].position;
		output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
		output.uv = gf2QuadUVs[i];

		outputStream.Append(output);
	}
	outputStream.RestartStrip();
}

float4 PSParticleDraw(GS_PARTICLE_DRAW_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtParticleTexture.Sample(gssWrap, input.uv);
	cColor *= input.color;

	return(cColor);
}

Texture2D gtxtCircularShadowTexture : register(t34);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

	return(cColor);
}

VS_CIRCULAR_SHADOW_INPUT VSCircularShadow(VS_CIRCULAR_SHADOW_INPUT input)
{
	return(input);
}

static float2 pf2UVs[4] = { float2(0.0f,1.0f), float2(0.0f,0.0f), float2(1.0f,1.0f), float2(1.0f,0.0f) };


[maxvertexcount(4)]
void GSCircularShadow(point VS_CIRCULAR_SHADOW_INPUT input[1], inout TriangleStream<GS_CIRCULAR_SHADOW_GEOMETRY_OUTPUT> outStream)
{
	float fHalfWidth = input[0].size.x * 0.5f;
	float fHalfDepth = input[0].size.y * 0.5f;

	float3 f3Right = float3(1.0f, 0.0f, 0.0f);
	float3 f3Look = float3(0.0f, 0.0f, 1.0f);

	float4 pf4Vertices[4];
	pf4Vertices[0] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfDepth * f3Look), 1.0f);
	pf4Vertices[1] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfDepth * f3Look), 1.0f);
	pf4Vertices[2] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfDepth * f3Look), 1.0f);
	pf4Vertices[3] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfDepth * f3Look), 1.0f);

	GS_CIRCULAR_SHADOW_GEOMETRY_OUTPUT output;
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
		output.uv = pf2UVs[i];

		outStream.Append(output);
	}
}
float4 PSCircularShadow(GS_CIRCULAR_SHADOW_GEOMETRY_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtCircularShadowTexture.Sample(gssWrap, input.uv);

	return(cColor);
}
VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}
float4 GetColorFromDepth(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth < 0.00625f) cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (fDepth < 0.0125f) cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.025f) cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.05f) cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.075f) cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.1f) cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
	else if (fDepth < 0.4f) cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.6f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.8f) cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
	else if (fDepth < 0.9f) cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
	else if (fDepth < 0.95f) cColor = float4(0.5f, 0.0f, 0.5f, 1.0f);
	else if (fDepth < 0.99f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.999f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth == 1.0f) cColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	else if (fDepth > 1.0f) cColor = float4(0.0f, 0.0f, 0.5f, 1.0f);
	else cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return(cColor);
}
//
SamplerState gssBorder : register(s3);
//
float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtDepthTextures[1].SampleLevel(gssBorder, input.uv, 0).r * 1.0f;

	//cColor = GetColorFromDepth(cColor.r);

	return(cColor);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_PLANAR_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_PLANAR_SHADOW_OUTPUT VSPlanarShadow(VS_LIGHTING_INPUT input)
{
	VS_PLANAR_SHADOW_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

	return(output);
}

float4 PSPlanarShadow(VS_PLANAR_SHADOW_OUTPUT input) : SV_TARGET
{
	return(float4(0.26f, 0.26f, 0.26f, 1.0f));
}


struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;

};

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return(output);
}


////////////////////////////////////////////////////////////////
struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD0;
	float4 uvs[MAX_LIGHTS] : TEXCOORD1;
};

static matrix gmxTexture = { 0.5,0.0,0.0,0.0,
							0.0,-0.5,0.0,0.0,
							0.0,0.0,1.0,0.0,
							0.5,0.5,0.0,1.0 };

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;
	output.uv = input.uv;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) 
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
		float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
		cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	else 
		cAlbedoColor = gMaterial.m_cDiffuse;

	if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
		cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	else
		cSpecularColor = gMaterial.m_cSpecular;
	
	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);
	cColor = cAlbedoColor+ cSpecularColor;
	

	return(lerp(cColor, cIllumination, 0.75f));
}

////////////////////////////////////////////////////////////////
// BoundingBox Wire
struct VS_BOUNDINGBOX_INPUT
{
	float3 position : POSITION;
};

struct VS_BOUNDINGBOX_OUTPUT
{
	float4 positionH : SV_POSITION;
};

VS_BOUNDINGBOX_OUTPUT VSBoundingBox(VS_BOUNDINGBOX_INPUT input)
{
	VS_BOUNDINGBOX_OUTPUT output;
	output.positionH = mul(mul(float4(input.position, 1.0f), gmtxView), gmtxProjection);
	return(output);
}

float4 PSBoundingBox(VS_BOUNDINGBOX_OUTPUT input) : SV_TARGET
{
	return(float4(1.0f, 0.0f, 1.0f, 1.0f));
}