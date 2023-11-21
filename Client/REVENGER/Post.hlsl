#include "Shaders.hlsl"


Texture2D<float> gtxtDepthTexture : register(t28);

float4 PSPostbyTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);

	return(cColor);
}

struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};
struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 color : SV_TARGET0;

	float4 cTexture : SV_TARGET1;
	float4 cIllumination : SV_TARGET2;
	float4 normal : SV_TARGET3;
	float zDepth : SV_TARGET4;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.cTexture = gtxtTextureArray.Sample(gssWrap, uvw);

	input.normalW = normalize(input.normalW);
	output.cIllumination = Lighting(input.positionW, input.normalW);

	output.color = output.cIllumination * output.cTexture;

	output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);

	output.zDepth = input.position.z;

	return(output);
}

float4 VSPostProcessing(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0)	return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1)	return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2)	return(float4(+1.0f, -1.0f, 0.0f, 1.0f));

	if (nVertexID == 3)	return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4)	return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5)	return(float4(-1.0f, -1.0f, 0.0f, 1.0f));

	return(float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION) : SV_Target
{
	return(float4(0.0f, 0.0f, 0.0f, 1.0f));
}

struct VS_SCREEN_RECT_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SCREEN_RECT_TEXTURED_OUTPUT VSScreenRectSamplingTextured(uint nVertexID : SV_VertexID)
{
	VS_SCREEN_RECT_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }

	else if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 PSScreenRectSamplingTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float fDepth = gtxtDepthTexture.Load(uint3((uint)input.position.x, (uint)input.position.y, 0));

	//float4 cColor2 = gtxtTextureTexture.Sample(gssWrap, input.uv);
	if (MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);

	cColor = lerp(fDepth, cAlbedoColor,0.4);
		
	return(cColor);
}