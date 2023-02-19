Texture2D texDiffuse;
SamplerState sampLinear;
#define MAX_LIGHTS			16 
#define MAX_MATERIALS		16

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

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
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
};

cbuffer cbPerFrame : register(b0)
{
    float4x4 gViewProjMatrix;
    float4x4 gViewMatrix;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorldMatrix;
};

cbuffer cbLights : register(b4)
{
    LIGHT					gLights[MAX_LIGHTS];
    float4					gcGlobalAmbientLight;
    int						gnLights;
};

float4 PS(PS_INPUT input) : SV_TARGET
{
    float3 vToCamera = normalize(input.pos.xyz - gViewMatrix[3].xyz);
    float3 vNormal = normalize(input.normal);
    float4 cDiffuse = texDiffuse.Sample(sampLinear, input.texcoord);

    // Emissive Color: �ؽ�ó�� ������ ��ü�� ���� ������ ������
    float4 cEmissive = texDiffuse.Sample(sampLinear, input.texcoord);

    float4 cFinalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            float4 cLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
            if (gLights[i].m_nType == POINT_LIGHT)
            {
                cLightColor = PointLight(i, input.pos.xyz, vNormal, vToCamera);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cLightColor = SpotLight(i, input.pos.xyz, vNormal, vToCamera);
            }
            else if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                cLightColor = DirectionalLight(i, vNormal, vToCamera);
            }

            cFinalColor += cLightColor;
        }
    }

    // Emissive Color�� �����Ͽ� ��ü�� ������ ȿ���� ����
    cFinalColor += cEmissive;

    // ����Ʈ�� ������ ������ �κп� ������ ����
    cFinalColor += (gcGlobalAmbientLight * cDiffuse);

    return cFinalColor;
}
