cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	matrix		gmtxInverseView : packoffset(c8);
	float3		gvCameraPosition : packoffset(c12);
};

struct MATERIAL
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular; //a = power
	float4				m_cEmissive;

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

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0); //16
	MATERIAL	gMaterial : packoffset(c4); // 16
	uint		gnTexturesMask : packoffset(c8); // 1
};

cbuffer cbFrameworkInfo : register(b5)
{
	float		gfCurrentTime : packoffset(c0.x);
	float		gfElapsedTime : packoffset(c0.y);
	float		gfSecondsPerFirework : packoffset(c0.z);
	int			gnFlareParticlesToEmit : packoffset(c0.w);;
	float3		gf3Gravity : packoffset(c1.x);
	int			gnMaxFlareType2Particles : packoffset(c1.w);;
};
cbuffer cbStreamGameObjectInfo : register(b6)
{
	matrix		gmtxWorld : packoffset(c0);

};

cbuffer CDynamicObjectInfo : register(b8)
{
	matrix		gDynamicmtxWorld : packoffset(c0); //16
	MATERIAL	gDynamicMaterial : packoffset(c8); // 16

};

cbuffer cbStreamGameObjectInfo : register(b9)
{
	matrix		gmtxSpriteWorld : packoffset(c0);
	EXPLOSIONMATERIAL	gSpriteActionMaterial : packoffset(c4); // 16
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

#define _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
Texture2D gtxtAlbedoTexture : register(t0);
Texture2D gtxtSpecularTexture : register(t1);
Texture2D gtxtNormalTexture : register(t2);
Texture2D gtxtMetallicTexture : register(t3);
Texture2D gtxtEmissionTexture : register(t4);
Texture2D gtxtDetailAlbedoTexture : register(t5);
Texture2D gtxtDetailNormalTexture : register(t6);
#else
Texture2D gtxtStandardTextures[7] : register(t0);
#endif

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);
SamplerState gssMirror : register(s2);
SamplerState gssPoint : register(s3);

TextureCube gtxtSkyCubeTexture : register(t13);
Texture2D gtxtTexture : register(t19);
Texture2D<float4> gtxtParticleTexture : register(t23);
Buffer<float4> gRandomBuffer : register(t24);
Buffer<float4> gRandomSphereBuffer : register(t25);


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
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor = lerp(cColor, cIllumination, 0.3f);
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
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxSpriteWorld), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}
float4 PSTextured(VS_TEXTURED_OUTPUT input, uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	return(cColor);
}
VS_TEXTURED_OUTPUT VSSpriteAnimation(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxSpriteWorld), gmtxView), gmtxProjection);
	output.uv = mul(float3(input.uv, 1.0f), (float3x3)(gSpriteActionMaterial.gmtxTexture)).xy;

	return(output);
}
float4 PSSpriteAnimation(VS_TEXTURED_OUTPUT input, uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	//cColor.a = Alpha;

	return (cColor);
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
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	//output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{

	input.normalW = normalize(input.normalW);
	float4 cBaseTexColor = gtxtTerrainTexture.Sample(gssWrap, input.uv0 * 1.0f);
	float fAlpha = gtxtAlphaTexture.Sample(gssWrap, input.uv0).w;
	float4 cIllumination = float4(1.0, 1.0, 1.0, 1.0);

	float4 cDetailTexColors[4];
	cDetailTexColors[0] = gtxtDetailTexture[0].Sample(gssWrap, input.uv1 * 1.0f);
	cDetailTexColors[1] = gtxtDetailTexture[1].Sample(gssWrap, input.uv1 * 1.6f);
	cDetailTexColors[2] = gtxtDetailTexture[2].Sample(gssWrap, input.uv1 * 1.7f);
	cDetailTexColors[3] = gtxtDetailTexture[3].Sample(gssWrap, input.uv1 * 1.2f);

	cIllumination = Lighting(input.positionW, input.normalW);
	float4 cColor = (cBaseTexColor * cDetailTexColors[0]);
	cColor += lerp(cDetailTexColors[1] * 0.55f, (cDetailTexColors[2]) * 0.5f, cDetailTexColors[3] * 0.5);

	cColor += lerp(cColor, cIllumination, 0.2f);

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

	cColor += lerp(cColor, cIllumination, 0.5);

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
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return (output);

}
float4 PSBillBoardTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{

	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
	return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

VS_TEXTURED_OUTPUT VSBillboard(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSBillboard(VS_TEXTURED_OUTPUT input) : SV_TARGET
{

		float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

		return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
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

static float3 gf3Positions[4] = { float3(-1.0f, +1.0f, 0.5f), float3(+1.0f, +1.0f, 0.5f), float3(-1.0f, -1.0f, 0.5f), float3(+1.0f, -1.0f, 0.5f) };
static float2 gf2QuadUVs[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };


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

TextureCube gtxtCubeMap : register(t26);

///////////////////////////////////////////////////////////////////////
struct VS_LIGHTING_INPUT
{
	float3	position    : POSITION;
	float3	normal		: NORMAL;

};

struct VS_LIGHTING_OUTPUT
{
	float4	position    : SV_POSITION;
	float3	positionW   : POSITION;
	float3	normalW		: NORMAL;

};
VS_LIGHTING_OUTPUT VSCubeMapping(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gDynamicmtxWorld).xyz;
	//	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.normalW = mul(float4(input.normal, 0.0f), gDynamicmtxWorld).xyz;
	//	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}

float4 PSCubeMapping(VS_LIGHTING_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);

	float4 cIllumination = DynamicLighting(input.positionW, input.normalW);

	float3 vFromCamera = normalize(input.positionW - gvCameraPosition.xyz);
	float3 vReflected = normalize(reflect(vFromCamera, input.normalW));
	float4 cCubeTextureColor = gtxtCubeMap.Sample(gssMirror, vReflected);

	//	return(float4(vReflected * 0.5f + 0.5f, 1.0f));
		return(cCubeTextureColor);
		//	return(cIllumination * cCubeTextureColor);
}


VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	float3 normalW = mul(input.normal, (float3x3)gmtxGameObject);
#ifdef _WITH_VERTEX_LIGHTING
	output.color = Lighting(output.positionW, normalize(normalW));
#else
	output.normalW = normalW;
#endif

	return(output);
}

float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
	float3 normalW = normalize(input.normalW);
	float4 cColor = Lighting(input.positionW, normalW);
	return(cColor);
#endif
}

//#define _WITH_SCALING
//#define _WITH_NORMAL_DISPLACEMENT
#define _WITH_PROJECTION_SPACE

#define FRAME_BUFFER_WIDTH		1480
#define FRAME_BUFFER_HEIGHT		960

VS_LIGHTING_OUTPUT VSOutline(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

#ifdef _WITH_SCALING
	//Scaling
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	float fScale = 1.00175f + length(gvCameraPosition - output.positionW) * 0.00035f;
	output.positionW = (float3)mul(float4(input.position * fScale, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
#endif

#ifdef _WITH_NORMAL_DISPLACEMENT
	//Normal Displacement
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	float3 normalW = normalize(mul(input.normal, (float3x3)gmtxGameObject));
	float fScale = length(gvCameraPosition - output.positionW) * 0.01f;
	output.positionW += normalW * fScale;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
#endif

#ifdef _WITH_PROJECTION_SPACE
	//Projection Space
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	float3 normal = mul(mul(input.normal, (float3x3)gmtxGameObject), (float3x3)mul(gmtxView, gmtxProjection));
	float2 f2Offset = normalize(normal.xy) / float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT) * output.position.w * 4.0f;
	output.position.xy += f2Offset;
	output.position.z += 0.5f;
#endif

	return(output);
}

float4 PSOutline(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
	return(float4(1.0f, 0.2f, 0.2f, 0.0f));
}















struct CB_TOOBJECTSPACE
{
	matrix		mtxToTexture;
	float4		f4Position;
};

cbuffer cbToLightSpace : register(b10)
{
	CB_TOOBJECTSPACE gcbToLightSpaces[MAX_LIGHTS];
	CB_TOOBJECTSPACE gcbToProjectorSpaces[MAX_LIGHTS];
};

struct VS_SDWLIGHTING_INPUT
{
	float3	position    : POSITION;
	float3	normal		: NORMAL;
	float2  uv			: TEXCOORD;
};

struct VS_SDWLIGHTING_OUTPUT
{
	float4	position    : SV_POSITION;
	float3	positionW   : POSITION;
	float3	normalW		: NORMAL;
	float2  uv			: TEXCOORD;
};
VS_SDWLIGHTING_OUTPUT VSShadowLighting(VS_SDWLIGHTING_INPUT input)
{
	VS_SDWLIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	return(output);
}

float4 PSShadowLighting(VS_SDWLIGHTING_OUTPUT input) : SV_TARGET
{
	input.normalW = normalize(input.normalW);
	float4 uvs[MAX_LIGHTS];
	float4 cColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cIllumination = Lighting(input.positionW, input.normalW, false, uvs);
	//	return(cIllumination);

	//return(float4(input.normalW * 0.5f + 0.5f, 1.0f));
	return(lerp(cColor, cIllumination, 0.5));
}

////////////////////////////////////////////////////////////////
struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float4 uvs[MAX_LIGHTS] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_SDWLIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;
	//output.uvs[MAX_LIGHTS].xy = input.uv.xy;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f) output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cIllumination = Lighting(input.positionW, normalize(input.normalW), true, input.uvs);

	//	cIllumination = saturate(gtxtDepthTextures[3].SampleLevel(gssProjector, f3uvw.xy, 0).r);
//	cIllumination = float4(0, 0, 0, 1);
	return(cIllumination);
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

struct VS_PLANAR_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_PLANAR_SHADOW_OUTPUT VSPlanarShadow(VS_SDWLIGHTING_INPUT input)
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

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_SDWLIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return(output);
}