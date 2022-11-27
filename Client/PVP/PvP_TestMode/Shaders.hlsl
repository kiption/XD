cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

struct MATERIAL
{

	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
	

};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0); //16
	MATERIAL	gMaterial : packoffset(c4); // 16
	uint		gnTexturesMask : packoffset(c8); // 1
};

cbuffer cbFrameTimekInfo : register(b3)
{
	float 		gDeltaTime;
};

cbuffer cbWaterMatrixInfo : register(b4)
{
	matrix		gf4x4TextureAnimation : packoffset(c0);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
///#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

//#define _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS

#define _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);
#else
Texture2D gtxtStandardTextures[7] : register(t0);
#endif

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
	float2 uv : TEXCOORD;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};

Texture2D gtxtAlphaTexture : register(t18);
VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return(output);
}
float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0);

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
#else
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtStandardTextures[0].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtStandardTextures[1].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtStandardTextures[2].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtStandardTextures[3].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtStandardTextures[4].Sample(gssWrap, input.uv);
#endif

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float fAlpha = gtxtAlbedoTexture.Sample(gssWrap, input.uv).w + gtxtSpecularTexture.Sample(gssWrap, input.uv).w
		+ gtxtEmissionTexture.Sample(gssWrap, input.uv).w+ gtxtMetallicTexture.Sample(gssWrap, input.uv).w;
	cMetallicColor.x=0.0;
	cMetallicColor.y=0.0;
	cMetallicColor.z=0.0;
	fAlpha = 0.8;

	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor+ cMetallicColor+ fAlpha;

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		
		float3 normalW = input.normalW;
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor -= lerp(cColor, cIllumination, 0.9- fAlpha);
	}

	return(cColor);
}
float4 PSObjStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
#else
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtStandardTextures[0].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtStandardTextures[1].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtStandardTextures[2].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtStandardTextures[3].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtStandardTextures[4].Sample(gssWrap, input.uv);
#endif

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normalW = input.normalW;
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor += lerp(cColor, cIllumination, 0.5f);
	}

	return(cColor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
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
Texture2D gtxtTexture : register(t19);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	return(cColor);
}

Texture2D gtxtTerrainTexture : register(t14);
Texture2D gtxtDetailTexture[3]:register(t15);


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

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	//output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	
	input.normalW = normalize(input.normalW);
	float4 cBaseTexColor = gtxtTerrainTexture.Sample(gssWrap, input.uv0 *2.0f);
	float fAlpha = gtxtAlphaTexture.Sample(gssWrap, input.uv0).w;
	float4 cIllumination = float4(0.4f, 0.8f, 0.4f, 1.0f);
	
	float4 cDetailTexColors[4];
	cDetailTexColors[0] = gtxtDetailTexture[0].Sample(gssWrap, input.uv1 * 1.0f);
	cDetailTexColors[1] = gtxtDetailTexture[1].Sample(gssWrap, input.uv1 * 1.6f);
	cDetailTexColors[2] = gtxtDetailTexture[2].Sample(gssWrap, input.uv1 * 0.7f);
	cDetailTexColors[3] = gtxtDetailTexture[3].Sample(gssWrap, input.uv1 * 1.2f);

	cIllumination = Lighting(input.positionW, input.normalW);
	float4 cColor = (cBaseTexColor * cDetailTexColors[0]);
	//float4 cColor = cBaseTexColor * cDetailTexColors[0];
	cColor += lerp(cDetailTexColors[1] * 0.35f, (cDetailTexColors[2] * cDetailTexColors[3])*0.55f, 0.5f - fAlpha);

	cColor = lerp(cColor, cIllumination, 0.05f);

	return(cColor);
}

float4 PSWater(VS_TERRAIN_OUTPUT input) : SV_TARGET
{

	input.normalW = normalize(input.normalW);
	float4 cBaseTexColor = gtxtTerrainTexture.Sample(gssWrap, input.uv0 * 1.0f);
	float fAlpha = gtxtAlphaTexture.Sample(gssWrap, input.uv0).w;
	float4 cIllumination = float4(0.4f, 0.8f, 0.4f, 1.0f);

	float4 cDetailTexColors[1];
	cDetailTexColors[0] = gtxtDetailTexture[0].Sample(gssWrap, input.uv1 * 0.1f);


	cIllumination = Lighting(input.positionW, input.normalW);

	float4 cColor = (cBaseTexColor * cDetailTexColors[0]);

	cColor += lerp(cColor, cIllumination, 0.05);

	return(cColor);
}
///////////////////////////////////////////////////////////////////
struct VS_WATER_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_WATER_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D<float4> gtxtWaterTexture[3] : register(t20); //20~22
static matrix<float, 3, 3> Af3x3TerrainWaterAnimation = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

VS_WATER_OUTPUT VSTerrainWater(VS_WATER_INPUT input)
{
	VS_WATER_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTerrainWater(VS_WATER_OUTPUT input) : SV_TARGET
{
	float2 uv = input.uv;
	uv = mul(float3(input.uv, 1.0f), (float3x3)gf4x4TextureAnimation).xy;
	uv.y += gDeltaTime * 0.00125f;
	float4 cBaseTexColor = gtxtWaterTexture[0].Sample(gssWrap, input.uv,0);
	float4 cDetail0TexColor = gtxtWaterTexture[1].Sample(gssWrap, input.uv * 20.0f);
	float4 cDetail1TexColor = gtxtWaterTexture[2].Sample(gssWrap, input.uv * 20.0f);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	cColor = lerp(cBaseTexColor * cDetail0TexColor, cDetail1TexColor.r * 0.5f, 0.35f);

	return(cColor);
}


VS_WATER_OUTPUT VSTerrainMoveWater(VS_WATER_INPUT input)
{
	VS_WATER_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTerrainMoveWater(VS_WATER_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainTexture.Sample(gssWrap, input.uv * 1.0f);
	float fAlpha = gtxtAlphaTexture.Sample(gssWrap, input.uv).w;

	float4 cDetailTexColors[3];
	cDetailTexColors[0] = gtxtWaterTexture[0].Sample(gssWrap, input.uv * 1.0f);
	cDetailTexColors[1] = gtxtWaterTexture[1].Sample(gssWrap, input.uv * 0.125f);
	cDetailTexColors[2] = gtxtWaterTexture[2].Sample(gssWrap, input.uv * 1.5f);

	float4 cColor = cBaseTexColor * cDetailTexColors[0];
	cColor += lerp(cDetailTexColors[1] * 0.45f, cDetailTexColors[2], 1.0f - fAlpha);
	return(cColor);
}

VS_TEXTURED_OUTPUT VSBillBoardTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position,1.0f), gmtxGameObject), gmtxView), gmtxProjection); 
	output.uv = input.uv;
	return (output);

}
float4 PSBillBoardTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{

	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
	return (cColor);
}