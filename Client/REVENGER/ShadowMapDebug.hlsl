Texture2D<float> g_texDepth : register(t0);
SamplerState g_sampler : register(s0);

struct VS_DEBUG_SHADOW_OUTPUT { float4 position : SV_POSITION; float2 uv : TEXCOORD; };

VS_DEBUG_SHADOW_OUTPUT VSDebugShadowMap(uint nVertexID : SV_VertexID)
{
	VS_DEBUG_SHADOW_OUTPUT output;
	if (nVertexID == 0) { output.position = float4(0.0f, 0.4f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 1) { output.position = float4(0.4f, 0.4f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (nVertexID == 2) { output.position = float4(0.4f, 0.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 3) { output.position = float4(0.0f, 0.4f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 4) { output.position = float4(0.4f, 0.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else { output.position = float4(0.0f, 0.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }
	return output;
}

float4 PSDebugShadowMap(VS_DEBUG_SHADOW_OUTPUT input) : SV_Target
{
	float d = g_texDepth.Sample(g_sampler, input.uv);
	if (d < 0.0001f) return float4(1.0f, 0.0f, 0.0f, 1.0f);
	float g = saturate(d * 0.9f + 0.1f);
	return float4(g, g, g, 1.0f);
}
