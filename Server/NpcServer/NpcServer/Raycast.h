#pragma once

struct RaycastResult {
	bool hit; // �浹 ����
	float distance; // ������ ���������� �浹 ���������� �Ÿ�
	XMFLOAT3 hitPoint; // �浹 ����
};

RaycastResult Raycast(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, const BoundingOrientedBox& box) {
	RaycastResult result;
	result.hit = false;

	// ���� �������� ���� �������� ������ �������� ��ȯ
	XMFLOAT3 localRayOrigin;
	XMStoreFloat3(&localRayOrigin, XMVector3InverseRotate(XMLoadFloat3(&rayOrigin) - XMLoadFloat3(&box.Center), XMLoadFloat4(&box.Orientation)));

	// ���� �������� ���� �������� ������ ������ ��ȯ
	XMFLOAT3 localRayDirection;
	XMStoreFloat3(&localRayDirection, XMVector3InverseRotate(XMLoadFloat3(&rayDirection), XMLoadFloat4(&box.Orientation)));

	XMFLOAT3 invDir(1.0f / localRayDirection.x, 1.0f / localRayDirection.y, 1.0f / localRayDirection.z);

	// �ٿ�� �ڽ��� �ּ� ������ �ִ� ����
	const XMFLOAT3& boxExtents = box.Extents;
	XMFLOAT3 boxMin(-boxExtents.x, -boxExtents.y, -boxExtents.z);
	XMFLOAT3 boxMax(boxExtents.x, boxExtents.y, boxExtents.z);

	// �ٿ�� �ڽ����� �浹 �ð� ���
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

	// �浹 ���� ���
	result.hit = true;
	result.distance = tmin;
	result.hitPoint = XMFLOAT3(
		localRayOrigin.x + localRayDirection.x * tmin,
		localRayOrigin.y + localRayDirection.y * tmin,
		localRayOrigin.z + localRayDirection.z * tmin
	);

	// ���� �������� ���� �������� �浹 ������ ��ȯ
	XMStoreFloat3(&result.hitPoint, XMLoadFloat3(&result.hitPoint) + XMLoadFloat3(&box.Center));

	return result;
}