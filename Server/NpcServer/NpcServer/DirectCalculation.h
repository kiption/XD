#pragma once

XMFLOAT3 NPCNormalize(XMFLOAT3 vec)
{
	float dist = sqrtf(powf(vec.x, 2) + powf(vec.y, 2) + powf(vec.z, 2));

	if (dist != 0.0f) {
		vec.x = vec.x / dist;
		vec.y = vec.y / dist;
		vec.z = vec.z / dist;
	}

	return vec;
}

XMFLOAT3 ProjectToXZPlane(const XMFLOAT3& vector)
{
	return XMFLOAT3(vector.x, 0.0f, vector.z);
}

// 벡터를 정규화하고 xz 평면으로 투영하는 함수
XMFLOAT3 ProjectToXZPlaneNormalized(const XMFLOAT3& vector)
{
	XMFLOAT3 projectedVector = ProjectToXZPlane(vector);
	XMVECTOR normalizedVector = XMVector3Normalize(XMLoadFloat3(&projectedVector));
	XMFLOAT3 normalizedResult;
	XMStoreFloat3(&normalizedResult, normalizedVector);
	return normalizedResult;
}

float Length(XMFLOAT3& xmf3Vector)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
	return(xmf3Result.x);
}

float DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
	return(xmf3Result.x);
}

XMFLOAT3 Lerp(XMFLOAT3 start, XMFLOAT3 end, float t)
{
	XMVECTOR startVector = XMLoadFloat3(&start);
	XMVECTOR endVector = XMLoadFloat3(&end);

	XMVECTOR interpolatedVector = XMVectorLerp(startVector, endVector, t);

	XMFLOAT3 interpolatedFloat3;
	XMStoreFloat3(&interpolatedFloat3, interpolatedVector);

	return interpolatedFloat3;
}

XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));
	return(xmf3Result);
}

XMFLOAT3 Normalize(XMFLOAT3& xmf3Vector)
{
	XMFLOAT3 m_xmf3Normal;
	XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
	return(m_xmf3Normal);
}
