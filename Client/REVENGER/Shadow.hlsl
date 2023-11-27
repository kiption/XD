#include "Shaders.hlsl"

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float2 uv : TEXCOORD;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};
struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};
struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;

	float2 uv : TEXCOORD;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float4 uvs[MAX_LIGHTS] : TEXCOORD1; 
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;

};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	if (!gbBoneShader)
	{
		output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
		output.positionW = (float3) mul(float4(input.position, 1.0f), gmtxGameObject);
		output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
		output.uv = input.uv;
		output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
		output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
	}
	else if (gbBoneShader)
	{
		float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
		for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
		{
			mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		}
		float4 positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld);
		output.positionW = positionW.xyz;
		output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
		output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
		output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
		output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
		output.uv = input.uv;
	}


	return(output);
}

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return(output);
}

static matrix gmxTexture = { 0.5,0.0,0.0,0.0,
							0.0,-0.5,0.0,0.0,
							0.0,0.0,1.0,0.0,
							0.5,0.5,0.0,1.0 };

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;
	float4 positionW = (float4) 0.0f;
	if (!gbBoneShader)
	{
		positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
		output.positionW = positionW.xyz;
		output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
		output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;
		output.uv = input.uv;
		output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
		output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
	}
	if (gbBoneShader)
	{
		float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
		for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
		{
			mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		}
		positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld);
		output.positionW = positionW.xyz;
		output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
		output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
		output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
		output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
		output.uv = input.uv;
	}

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}


float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
		cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	else
		cAlbedoColor = gMaterial.m_cDiffuse;

	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
		input.normalW = normalize(mul(vNormal, TBN));
	}
	

	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);
	float4 cColor = cAlbedoColor;
	return (lerp(cIllumination,cColor,0.6));


}