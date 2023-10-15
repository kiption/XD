

float Compute3x3ShadowFactor(float2 uv, float fDepth, uint nIndex)
{
	float fPercentLit = gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv, fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(-DELTA_X, 0.0f), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(+DELTA_X, 0.0f), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(0.0f, -DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(0.0f, +DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(-DELTA_X, -DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(-DELTA_X, +DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(+DELTA_X, -DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(+DELTA_X, +DELTA_Y), fDepth).r;

	return(fPercentLit / 9.0f);
}

float Compute5x5ShadowFactor(float2 uv, float fDepth, uint nIndex)
{
	float fPercentLit = 0.0f;

	return(fPercentLit / 25.0f);
}

float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = -gLights[nIndex].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	float fSpecularFactor = 0.0f;
	if (fDiffuseFactor > 0.0f)
	{
		if (gMaterial.m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
		}
	}

	return((gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) +
		(gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular));
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			}
		}
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));

		return(((gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) 
			+ (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 EmissiveLight(int nIndex)
{
	return gLights[nIndex].m_cEmissive;
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			}
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));

		return(((gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) + 
			(gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor * fSpotFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);
	// Emissive Color: �ؽ�ó�� ������ ��ü�� ���� ������ ������

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[unroll(MAX_LIGHTS)] for (int i = 0; i < gnLights; i++)
	{
		if (gLights[i].m_bEnable)
		{
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += DirectionalLight(i, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera);
			}
		}
	}

	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cDiffuse.a;
	return(cColor);
}
float4 Lightings(float3 vPosition, float3 vNormal, float4 vEmissive)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);
	// Emissive Color: �ؽ�ó�� ������ ��ü�� ���� ������ ������

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[unroll(MAX_LIGHTS)] for (int i = 0; i < gnLights; i++)
	{
		if (gLights[i].m_bEnable)
		{
			vEmissive = gLights[i].m_cEmissive;
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += DirectionalLight(i, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera);
			}
		}
	}
	vEmissive += gMaterial.m_cEmissive;
	cColor += vEmissive;
	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cDiffuse.a;

	return(cColor);
}

float4 Lighting(float3 vPosition, float3 vNormal, bool bShadow, float4 uvs[MAX_LIGHTS])
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gLights[i].m_bEnable)
		{
			float fShadowFactor = 1.0f;
#ifdef _WITH_PCF_FILTERING
			if (bShadow) fShadowFactor = Compute3x3ShadowFactor(uvs[i].xy / uvs[i].ww, uvs[i].z / uvs[i].w, i);
#else
			if (bShadow) fShadowFactor = gtxtDepthTextures[i].SampleCmpLevelZero(gssComparisonPCFShadow, uvs[i].xy / uvs[i].ww, uvs[i].z / uvs[i].w).r;
#endif

			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += DirectionalLight(i, vNormal, vToCamera) * fShadowFactor;
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera) * fShadowFactor;
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera) * fShadowFactor;
			}

			cColor += gLights[i].m_cAmbient * gMaterial.m_cAmbient;
		}
	}

	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cDiffuse.a;


	return cColor;
}


















float4 EmiPlusSpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			}
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) +
			(gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular))
			+ (gLights[nIndex].m_cEmissive * fDiffuseFactor * gMaterial.m_cEmissive) * fAttenuationFactor * fSpotFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 ParticleLighting(float3 vPosition, float3 vNormal, bool bShadow, float4 uvs[MAX_LIGHTS])
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

	float4 cColor = float4(0.3f, 0.3f, 0.3f, 1.0f);
	[unroll]
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (gLights[i].m_bEnable)
		{
			float fShadowFactor = 1.0f;
#ifdef _WITH_PCF_FILTERING
			if (bShadow) fShadowFactor = Compute3x3ShadowFactor(uvs[i].xy / uvs[i].ww, uvs[i].z / uvs[i].w, i);
#else
			if (bShadow) fShadowFactor = gtxtDepthTextures[i].SampleCmpLevelZero(gssComparisonPCFShadow, uvs[i].xy / uvs[i].ww, uvs[i].z / uvs[i].w).r;
#endif

			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += DirectionalLight(i, vNormal, vToCamera) * fShadowFactor;
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera) * fShadowFactor;
				cColor += EmissiveLight(i);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += EmiPlusSpotLight(i, vPosition, vNormal, vToCamera) * fShadowFactor;
				cColor += EmissiveLight(i);
			}
			cColor += gLights[i].m_cAmbient * gMaterial.m_cAmbient * gMaterial.m_cEmissive;;
		}
	}

	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cDiffuse.a;

	return(cColor);
}