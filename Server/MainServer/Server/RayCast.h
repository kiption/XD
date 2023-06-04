#pragma once
#include "SimpleMathWithDX.h"

// R(t) = (Px + t * nx, Py + t * ny, Pz + t * nz)  ( t >= 0 )

// P(s, t) = P0 + s * (P1 - P0) + t(P2 - P0) = P0 + su + tv 

// n ( a, b, c )
// ax + by + cz + -(ax0 + by0 + cz0) = 0  <= d = -(ax0 + by0 + cz0)
// 3 Points on Planes: P0, P1, P2
// n = vec_P0->P1 (cross) vec_P0->P2


// Raycast Intersect Solution
// Ray: R(t) = S + t * v, Plane: ax + by + cz + d = 0
// t = -{ d + (S dot n) } / (v dot n)
// => P = R(t) = S + [ - { d + (S dot n) } / (v dot n) ] * v

// ����� �븻����
XMFLOAT3 Get_NormalVec(XMFLOAT3 plane_center, XMFLOAT3 plane_p1, XMFLOAT3 plane_p2) {
	XMFLOAT3 vecC1 = XMF_Substract(plane_p1, plane_center);
	XMFLOAT3 vecC2 = XMF_Substract(plane_p2, plane_center);
	return XMF_CrossProduct(vecC1, vecC2);
}

// ��� �����Ŀ��� d��
float Get_ValueD_PlaneDescription(XMFLOAT3 plane_center, XMFLOAT3 plane_p1, XMFLOAT3 plane_p2) {
	XMFLOAT3 normal = Get_NormalVec(plane_center, plane_p1, plane_p2);
	float ax0 = normal.x * plane_center.x;
	float by0 = normal.y * plane_center.y;
	float cz0 = normal.z * plane_center.z;
	return -1.0f * (ax0 + by0 + cz0);
}

// ���� ������ �Ű����� t
float Get_ParamT_RayDescription(XMFLOAT3 start, XMFLOAT3 direction, XMFLOAT3 plane_center, XMFLOAT3 plane_p1, XMFLOAT3 plane_p2) {
	float d = Get_ValueD_PlaneDescription(plane_center, plane_p1, plane_p2);
	XMFLOAT3 normal = Get_NormalVec(plane_center, plane_p1, plane_p2);
	return (-1.0f * (d + XMF_DotProduct(start, normal))) / XMF_DotProduct(direction, normal);
}

// ����ĳ��Ʈ
XMFLOAT3 RayCast(XMFLOAT3 start, XMFLOAT3 direction, float range_limit, XMFLOAT3 plane_center, XMFLOAT3 plane_p1, XMFLOAT3 plane_p2) {
	XMFLOAT3 normal = Get_NormalVec(plane_center, plane_p1, plane_p2);
	if (XMF_DotProduct(direction, normal) == 0) return XMF_fault; // ������ ����� ���� -> �浹X
	
	XMFLOAT3 v{ 0,0,0 };
	bool infinite_ray = false;
	if (range_limit >= FLT_MAX) {
		v = direction;
		infinite_ray = true;
	}
	else {
		XMFLOAT3 end = XMF_Add(start, XMF_MultiplyScalar(direction, range_limit));
		v = XMF_Substract(end, start);
		infinite_ray = false;
	}

	float t = Get_ParamT_RayDescription(start, direction, plane_center, plane_p1, plane_p2);
	if (t < 0) return XMF_fault;	// �������� ��� �ڿ� ����. -> ������ ��� ���� �ݴ뿡 ����� �ֱ⶧���� �浹X
	if ((!infinite_ray) && (t > 0)) return XMF_fault;	// ������ ��� �տ� ����. -> �浹���� Ray�� ��Ÿ� ������ �ָ� �ִ� ���̹Ƿ� �浹�� ���� ����.
	
	return XMF_Add(start, XMF_MultiplyScalar(v, t));
}

// ���� ����(�簢��) �ȿ� �ִ��� �˻�
bool Check_PointInQuadrangle(XMFLOAT3 point, XMFLOAT3 quad_lefttop, XMFLOAT3 quad_rightbottom) {
	XMFLOAT3 dist_vec = XMF_Substract(quad_lefttop, quad_rightbottom);
	bool x_check = false;
	bool y_check = false;
	bool z_check = false;

	if (dist_vec.x > 0.0f) {	// lefttop x�� �� ū ���
		if (quad_rightbottom.x <= point.x && point.x <= quad_lefttop.x) x_check = true;
	}
	else {						// righttbottom x�� �� ū ���
		if (quad_lefttop.x <= point.x && point.x <= quad_rightbottom.x) x_check = true;
	}

	if (dist_vec.y > 0.0f) {	// lefttop y�� �� ū ���
		if (quad_rightbottom.y <= point.y && point.y <= quad_lefttop.y) x_check = true;
	}
	else {						// righttbottom y�� �� ū ���
		if (quad_lefttop.y <= point.y && point.y <= quad_rightbottom.y) x_check = true;
	}

	if (dist_vec.z > 0.0f) {	// lefttop z�� �� ū ���
		if (quad_rightbottom.z <= point.z && point.z <= quad_lefttop.z) x_check = true;
	}
	else {						// righttbottom z�� �� ū ���
		if (quad_lefttop.z <= point.z && point.z <= quad_rightbottom.z) x_check = true;
	}

	if (x_check && y_check && z_check)
		return true;
	else
		return false;
}

// �ڽ��� ������ 8��
XMFLOAT3 Make_BBPoint(XMFLOAT3 box_center, float box_width, float box_height, float box_length, int point_num) {
	// Point_num
	// ����    // 0: x + , y + , z -	// 1: x + , y + , z +	// 2: x - , y + , z -	// 3: x - , y + , z +
	// �Ʒ���  // 4: x + , y - , z -	// 5: x + , y - , z +	// 6: x - , y - , z -	// 7: x - , y - , z +
	if (point_num < 1 || point_num > 8) return XMF_fault;

	XMFLOAT3 point;
	if (point_num == 0 || point_num == 1 || point_num == 4 || point_num == 5)
		point.x = box_center.x + box_width / 2.0f;
	else
		point.x = box_center.x - box_width / 2.0f;

	if (point_num == 0 || point_num == 1 || point_num == 2 || point_num == 3)
		point.y = box_center.y + box_height / 2.0f;
	else
		point.y = box_center.y - box_height / 2.0f;

	if (point_num == 1 || point_num == 3 || point_num == 5 || point_num == 7)
		point.z = box_center.z + box_length / 2.0f;
	else
		point.z = box_center.z - box_length / 2.0f;

	return point;
}

// ����-�ڽ� �浹�˻�
XMFLOAT3 Intersect_Ray_Box(XMFLOAT3 start, XMFLOAT3 direction, float range_limit, XMFLOAT3 box_center, float box_width, float box_height, float box_length) {
	// �浹 ����� �ִ� ��Ÿ����� �ָ� �ִ� ���
	if (XMF_Distance(start, box_center) > range_limit) return XMF_fault;

	// �浹�ڽ� �����
	XMFLOAT3 p[8];
	for (int i = 0; i < 8; ++i) {
		p[i] = Make_BBPoint(box_center, box_width, box_height, box_length, i);
	}

	XMFLOAT3 tmp_result[6];
	// ��0123: Center = 0, p1 = 1, p2 = 2
	tmp_result[0] = RayCast(start, direction, range_limit, p[0], p[1], p[2]);
	if (tmp_result[0] != XMF_fault) {
		if (!Check_PointInQuadrangle(tmp_result[0], p[0], p[3])) tmp_result[0] = XMF_fault;
	}
	// ��1054: Center = 1, p1 = 0, p2 = 5
	tmp_result[1] = RayCast(start, direction, range_limit, p[1], p[0], p[5]);
	if (tmp_result[1] != XMF_fault) {
		if (!Check_PointInQuadrangle(tmp_result[1], p[1], p[4])) tmp_result[1] = XMF_fault;
	}
	// ��2064: Center = 2, p1 = 6, p2 = 0
	tmp_result[2] = RayCast(start, direction, range_limit, p[2], p[6], p[0]);
	if (tmp_result[2] != XMF_fault) {
		if (!Check_PointInQuadrangle(tmp_result[2], p[2], p[4])) tmp_result[2] = XMF_fault;
	}
	// ��3276: Center = 3, p1 = 7, p2 = 2
	tmp_result[3] = RayCast(start, direction, range_limit, p[3], p[7], p[2]);
	if (tmp_result[3] != XMF_fault) {
		if (!Check_PointInQuadrangle(tmp_result[3], p[3], p[6])) tmp_result[3] = XMF_fault;
	}
	// ��5713: Center = 5, p1 = 7, p2 = 1
	tmp_result[4] = RayCast(start, direction, range_limit, p[5], p[7], p[1]);
	if (tmp_result[4] != XMF_fault) {
		if (!Check_PointInQuadrangle(tmp_result[4], p[5], p[3])) tmp_result[4] = XMF_fault;
	}
	// ��6745: Center = 6, p1 = 7, p2 = 4
	tmp_result[5] = RayCast(start, direction, range_limit, p[6], p[7], p[4]);
	if (tmp_result[5] != XMF_fault) {
		if (!Check_PointInQuadrangle(tmp_result[5], p[6], p[5])) tmp_result[5] = XMF_fault;
	}

	XMFLOAT3 final_result = XMF_fault;
	float min_dist = FLT_MAX;
	for (int i = 0; i < 6; ++i) {
		if (tmp_result[i] == XMF_fault) continue;
		float cur_dist = XMF_Distance(start, tmp_result[i]);
		if (cur_dist < min_dist) {
			min_dist = cur_dist;
			final_result = tmp_result[i];
		}
	}

	return final_result;
}