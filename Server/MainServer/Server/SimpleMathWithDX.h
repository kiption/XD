#pragma once
#include <cmath>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <DirectXCollision.inl>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#define PI 3.14159

struct Coordinate {
	XMFLOAT3 right;
	XMFLOAT3 up;
	XMFLOAT3 look;

	Coordinate() {
		right = { 1.0f, 0.0f, 0.0f };
		up = { 0.0f, 1.0f, 0.0f };
		look = { 0.0f, 0.0f, 1.0f };
	}
};
Coordinate basic_coordinate;	// ±‚∫ª(√ ±‚) ¡¬«•∞Ë

bool operator==(XMFLOAT3 lval, XMFLOAT3 rval) {
	if (lval.x != rval.x) return false;
	if (lval.y != rval.y) return false;
	if (lval.z != rval.z) return false;
	return true;
}

// ∞ËªÍ Ω«∆–
XMFLOAT3 XMF_fault{ -9999.f, -9999.f, -9999.f };

// ∫§≈Õ¿« µ°º¿
XMFLOAT3 XMF_Add(XMFLOAT3 left, XMFLOAT3 right) {
	XMFLOAT3 result{ 0,0,0 };
	result.x = left.x + right.x;
	result.y = left.y + right.y;
	result.z = left.z + right.z;
	return result;
}

// ∫§≈Õ¿« Ω∫ƒÆ∂Û∞ˆ
XMFLOAT3 XMF_MultiplyScalar(XMFLOAT3 vec, float scalar) {
	XMFLOAT3 result{ 0,0,0 };
	result.x = vec.x * scalar;
	result.y = vec.y * scalar;
	result.z = vec.z * scalar;
	return result;
}

// ∫§≈Õ¿« ª¨º¿
XMFLOAT3 XMF_Substract(XMFLOAT3 left, XMFLOAT3 right) {
	XMFLOAT3 minus_right = XMF_MultiplyScalar(right, -1.0f);
	return XMF_Add(left, minus_right);
}

// ∫§≈Õ¿« ≈©±‚
float XMF_SizeOfVector(XMFLOAT3 vec) {
	return sqrtf(powf(vec.x, 2) + powf(vec.y, 2) + powf(vec.z, 2));
}

// µŒ ¡° ªÁ¿Ã ∞≈∏Æ ¿Á±‚
float XMF_Distance(XMFLOAT3 start, XMFLOAT3 end) {
	XMFLOAT3 vec_se = XMF_Substract(start, end);
	return XMF_SizeOfVector(vec_se);
}

// ¡§±‘»≠
XMFLOAT3 XMF_Normalize(XMFLOAT3 vec) {
	float reciprocal_vecsize = 1.0f / XMF_SizeOfVector(vec);
	return XMF_MultiplyScalar(vec, reciprocal_vecsize);
}

// ≥ª¿˚
float XMF_DotProduct(XMFLOAT3 left, XMFLOAT3 right) {
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

// ø‹¿˚
XMFLOAT3 XMF_CrossProduct(XMFLOAT3 left, XMFLOAT3 right) {
	XMFLOAT3 result{ 0,0,0 };
	result.x = left.y * right.z - left.z * right.y;
	result.y = left.z * right.x - left.x * right.z;
	result.z = left.x * right.y - left.y * right.x;
	return result;
}

// µŒ ∫§≈Õ¿« ∞¢µµ ¿Á±‚
float XMF_RadianTwoVectors(XMFLOAT3 vec1, XMFLOAT3 vec2) {
	float dot_result = XMF_DotProduct(vec1, vec2);
	float vec1_size = XMF_SizeOfVector(vec1);
	float vec2_size = XMF_SizeOfVector(vec2);
	float cos_result = dot_result / (vec1_size * vec2_size);
	return acos(cos_result);
}
float XMF_DegreeTwoVectors(XMFLOAT3 vec1, XMFLOAT3 vec2) {
	return XMF_RadianTwoVectors(vec1, vec2) * 180.0f / PI;
}

// Ω∫ƒÆ∂Û ªÔ¡ﬂ¿˚
float XMF_ScalarTripleProduct(XMFLOAT3 u, XMFLOAT3 v, XMFLOAT3 w) {
	XMFLOAT3 result_cross = XMF_CrossProduct(v, w);
	return XMF_DotProduct(u, result_cross);
}
