#pragma once

struct RaycastResult {
	bool hit; // 충돌 여부
	float distance; // 레이의 시작점에서 충돌 지점까지의 거리
	XMFLOAT3 hitPoint; // 충돌 지점
};

RaycastResult Raycast(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, const BoundingOrientedBox& box) {
	RaycastResult result;
	result.hit = false;

	// 월드 공간에서 로컬 공간으로 레이의 시작점을 변환
	XMFLOAT3 localRayOrigin;
	XMStoreFloat3(&localRayOrigin, XMVector3InverseRotate(XMLoadFloat3(&rayOrigin) - XMLoadFloat3(&box.Center), XMLoadFloat4(&box.Orientation)));

	// 월드 공간에서 로컬 공간으로 레이의 방향을 변환
	XMFLOAT3 localRayDirection;
	XMStoreFloat3(&localRayDirection, XMVector3InverseRotate(XMLoadFloat3(&rayDirection), XMLoadFloat4(&box.Orientation)));

	XMFLOAT3 invDir(1.0f / localRayDirection.x, 1.0f / localRayDirection.y, 1.0f / localRayDirection.z);

	// 바운딩 박스의 최소 지점과 최대 지점
	const XMFLOAT3& boxExtents = box.Extents;
	XMFLOAT3 boxMin(-boxExtents.x, -boxExtents.y, -boxExtents.z);
	XMFLOAT3 boxMax(boxExtents.x, boxExtents.y, boxExtents.z);

	// 바운딩 박스와의 충돌 시간 계산
	float tmin = (boxMin.x - localRayOrigin.x) * invDir.x;
	float tmax = (boxMax.x - localRayOrigin.x) * invDir.x;

	float tymin = (boxMin.y - localRayOrigin.y) * invDir.y;
	float tymax = (boxMax.y - localRayOrigin.y) * invDir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return result;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (boxMin.z - localRayOrigin.z) * invDir.z;
	float tzmax = (boxMax.z - localRayOrigin.z) * invDir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return result;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	// 충돌 지점 계산
	result.hit = true;
	result.distance = tmin;
	result.hitPoint = XMFLOAT3(
		localRayOrigin.x + localRayDirection.x * tmin,
		localRayOrigin.y + localRayDirection.y * tmin,
		localRayOrigin.z + localRayDirection.z * tmin
	);

	// 로컬 공간에서 월드 공간으로 충돌 지점을 변환
	XMStoreFloat3(&result.hitPoint, XMLoadFloat3(&result.hitPoint) + XMLoadFloat3(&box.Center));

	return result;
}