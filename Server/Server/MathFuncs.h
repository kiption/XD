#pragma once
#include <cmath>

double round_digit(double num, int d) {
	double t = pow(10, d - 1);
	return round(num * t) / t;
}

//=========================
//		   RayCast
//=========================
struct MyVector3
{
	float x, y, z;

	MyVector3 operator+(MyVector3 right) { float ret_x = x + right.x; float ret_y = y + right.y; float ret_z = z + right.z; return MyVector3{ ret_x, ret_y, ret_z }; }
	MyVector3 operator-(MyVector3 right) { float ret_x = x - right.x; float ret_y = y - right.y; float ret_z = z - right.z; return MyVector3{ ret_x, ret_y, ret_z }; }

	bool operator==(MyVector3 right) {
		if (x != right.x) return false;
		if (y != right.y) return false;
		if (z != right.z) return false;
		return true;
	}

	float getSize() { return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2)); }
};
MyVector3 defaultVec{ -9999.f, -9999.f, -9999.f };

MyVector3 calcCrossProduct(MyVector3 lval, MyVector3 rval) {
	float cp_x = lval.y * rval.z - lval.z * rval.y;
	float cp_y = lval.z * rval.x - lval.x * rval.z;
	float cp_z = lval.x * rval.y - lval.y * rval.x;
	return MyVector3{ cp_x, cp_y, cp_z };
}
MyVector3 getNormalVec(MyVector3 v1, MyVector3 v2, MyVector3 v3) {
	MyVector3 vec_21 = v1 - v2;
	MyVector3 vec_23 = v3 - v2;
	return (calcCrossProduct(vec_21, vec_23));
}

struct Rectangle
{
	MyVector3 p1, p2, p3, p4;
};
struct Cube
{
	MyVector3 center;				// 중심 좌표
	float width, height, length;	// 가로 세로 높이

	Cube() { center = { 0,0,0 }; width = height = length = 0; }
	Cube(MyVector3 c, float w, float h, float l) { center = c; width = w; height = h; length = l; }

	MyVector3 getP1() { float x = center.x - width / 2; float y = center.y + height / 2; float z = center.z + length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP2() { float x = center.x + width / 2; float y = center.y + height / 2; float z = center.z + length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP4() { float x = center.x + width / 2; float y = center.y + height / 2; float z = center.z - length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP3() { float x = center.x - width / 2; float y = center.y + height / 2; float z = center.z - length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP5() { float x = center.x - width / 2; float y = center.y - height / 2; float z = center.z + length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP6() { float x = center.x + width / 2; float y = center.y - height / 2; float z = center.z + length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP7() { float x = center.x - width / 2; float y = center.y - height / 2; float z = center.z - length / 2; return MyVector3{ x, y, z }; };
	MyVector3 getP8() { float x = center.x + width / 2; float y = center.y - height / 2; float z = center.z - length / 2; return MyVector3{ x, y, z }; };
};

MyVector3 GetIntersection_Line2Plane(MyVector3 pos, MyVector3 look, MyVector3 plane_p1, MyVector3 plane_p2, MyVector3 plane_p3) {
	// 법선벡터 구하기
	MyVector3 normal = getNormalVec(plane_p1, plane_p2, plane_p3);
	float a = normal.x;
	float b = normal.y;
	float c = normal.z;
	float d = -(a * plane_p1.x + b * plane_p1.y + c * plane_p1.z);


	MyVector3 intersection;
	float t_denominator = a * look.x + b * look.y + c * look.z;	// 매개변수t의 분모
	if (t_denominator == 0) {
		intersection = pos;
	}
	else {
		float t = -1.0f * (a * pos.x + b * pos.y + c * pos.z + d) / (a * look.x + b * look.y + c * look.z);	// 매개변수 t
		if (t >= 0) {
			intersection = pos + MyVector3{ look.x * t, look.y * t, look.z * t };
		}
		else {
			return defaultVec;
		}
	}

	if (a * intersection.x + b * intersection.y + c * intersection.z + d != 0) {
		return defaultVec;
	}
	return intersection;

}

float calcDistance(MyVector3 v1, MyVector3 v2) {
	return sqrtf(powf((v1.x - v2.x), 2) + powf((v1.y - v2.y), 2) + powf(v1.z - v2.z, 2));
}

MyVector3 MyRaycast_InfiniteRay(MyVector3 p, MyVector3 lkvec, Cube bb) {
	MyVector3 Intersections[6] = { defaultVec, defaultVec, defaultVec, defaultVec, defaultVec, defaultVec };
	// ㅁp1p2p3p4 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_1234 = GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP2(), bb.getP3());
	Intersections[0] = Intersection_1234;

	// ㅁp1p3p5p7 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_1357 = GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP3(), bb.getP5());
	Intersections[1] = Intersection_1357;

	// ㅁp3p4p7p8 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_3478 = GetIntersection_Line2Plane(p, lkvec, bb.getP3(), bb.getP4(), bb.getP7());
	Intersections[2] = Intersection_3478;

	// ㅁp4p2p8p6 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_4286 = GetIntersection_Line2Plane(p, lkvec, bb.getP4(), bb.getP2(), bb.getP8());
	Intersections[3] = Intersection_4286;

	// ㅁp2p1p6p5 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_2165 = GetIntersection_Line2Plane(p, lkvec, bb.getP2(), bb.getP1(), bb.getP6());
	Intersections[4] = Intersection_2165;

	// ㅁp5p6p7p8 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_7856 = GetIntersection_Line2Plane(p, lkvec, bb.getP7(), bb.getP8(), bb.getP5());
	Intersections[5] = Intersection_7856;

	// 3. p와 교차하는 점들 사이의 거리를 계산해서 가장 가까운 점이 교점이다.
	float min_dist = INFINITY;
	int min_index = 0;
	for (int i = 0; i < 5; ++i) {
		if (Intersections[i] == defaultVec) continue;
		float cur_dist = calcDistance(p, Intersections[i]);
		if (cur_dist < min_dist) {
			min_dist = cur_dist;
			min_index = i;
		}
	}

	// 4. 그 충돌점이 큐브 안에 있는지 검사한다.
	if ((bb.getP3().x <= Intersections[min_index].x && Intersections[min_index].x <= bb.getP4().x)
		&& (bb.getP7().y <= Intersections[min_index].y && Intersections[min_index].y <= bb.getP3().y)
		&& (bb.getP3().z <= Intersections[min_index].z && Intersections[min_index].z <= bb.getP1().z)) {
		return Intersections[min_index];
	}

	// 5. 큐브 안에 없다면 충돌하지 않은 것이다.
	return defaultVec;
}

MyVector3 MyRaycast_LimitDistance(MyVector3 p, MyVector3 lkvec, Cube bb, float dist) {
	MyVector3 Intersections[6] = { defaultVec, defaultVec, defaultVec, defaultVec, defaultVec, defaultVec };
	// ㅁp1p2p3p4 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_1234 = GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP2(), bb.getP3());
	Intersections[0] = Intersection_1234;

	// ㅁp1p3p5p7 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_1357 = GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP3(), bb.getP5());
	Intersections[1] = Intersection_1357;

	// ㅁp3p4p7p8 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_3478 = GetIntersection_Line2Plane(p, lkvec, bb.getP3(), bb.getP4(), bb.getP7());
	Intersections[2] = Intersection_3478;

	// ㅁp4p2p8p6 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_4286 = GetIntersection_Line2Plane(p, lkvec, bb.getP4(), bb.getP2(), bb.getP8());
	Intersections[3] = Intersection_4286;

	// ㅁp2p1p6p5 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_2165 = GetIntersection_Line2Plane(p, lkvec, bb.getP2(), bb.getP1(), bb.getP6());
	Intersections[4] = Intersection_2165;

	// ㅁp5p6p7p8 평면방정식과 반직선의 충돌점 구하기
	MyVector3 Intersection_7856 = GetIntersection_Line2Plane(p, lkvec, bb.getP7(), bb.getP8(), bb.getP5());
	Intersections[5] = Intersection_7856;

	// 3. p와 교차하는 점들 사이의 거리를 계산해서 가장 가까운 점이 교점이다.
	float min_dist = INFINITY;
	int min_index = 0;
	for (int i = 0; i < 5; ++i) {
		if (Intersections[i] == defaultVec) continue;

		float cur_dist = calcDistance(p, Intersections[i]);
		if (cur_dist > dist) continue;	// 설정한 거리를 넘어가는 충돌점은 제외합니다.

		if (cur_dist < min_dist) {
			min_dist = cur_dist;
			min_index = i;
		}
	}

	// 4. 그 충돌점이 큐브 안에 있는지 검사한다.
	if ((bb.getP3().x <= Intersections[min_index].x && Intersections[min_index].x <= bb.getP4().x)
		&& (bb.getP7().y <= Intersections[min_index].y && Intersections[min_index].y <= bb.getP3().y)
		&& (bb.getP3().z <= Intersections[min_index].z && Intersections[min_index].z <= bb.getP1().z)) {
		return Intersections[min_index];
	}

	// 5. 큐브 안에 없다면 충돌하지 않은 것이다.
	return defaultVec;
}