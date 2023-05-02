#pragma once
#include <math.h>
#include "Constant.h"
#include "MyVectors.h"
#include <vector>
//#include <random>
enum NpcType { NPC_Helicopter, NPC_Bunker, NPC_Terret };
enum St1_NPCState { NPC_IDLE, NPC_FLY, NPC_CHASE, NPC_ATTACK, NPC_DEATH };
enum Hit_target { g_none, g_body, g_profeller };
enum IDLE_Section { Sectoin_1, Sectoin_2, Sectoin_3 };

const int User_num = 5;

struct Section_Info {
	float sx, sz, lx, lz;
	int ID;
};
struct City_Info {
	Section_Info SectionNum[3];
	float Centerx, Centerz;
	int id;
};

// 도시 위치 바뀌면 위치가 정리된 파일을 읽던가 이렇게 배열 형태로 해서 가져다 쓰던가로 해야함. --> 임시
const float SX_range[9] = { 520.0f, 520.0f, 520.0f, -980.0f,-980.0f, -980.0f, -385.0f, -180.0f, -180.0f };
const float LX_range[9] = { 1000.0f, 580.0f, 1550.0f, -300.0f,-900.0f, 50.0f, -100.0f, -100.0f, 845.0f };
const float SZ_range[9] = { 350.0f, 350.0f, 770.0f, 240.0f,240.0f, 660.0f, -535.0f, -535.0f, -110.0f };
const float LZ_range[9] = { 410.0f, 835.0f, 840.0f,  295.0f,720.0f,720.0f, -455.0f, -30.0f, -30.0f };
const float C_cx[3] = { 782.0f, -680.0f, -17.0f };
const float C_cz[3] = { 592.0f, 477.5f, -282.0f };
//
//struct NpcVector3
//{
//	float x, y, z;
//
//	NpcVector3 operator+(NpcVector3 right) { float ret_x = x + right.x; float ret_y = y + right.y; float ret_z = z + right.z; return NpcVector3{ ret_x, ret_y, ret_z }; }
//	NpcVector3 operator-(NpcVector3 right) { float ret_x = x - right.x; float ret_y = y - right.y; float ret_z = z - right.z; return NpcVector3{ ret_x, ret_y, ret_z }; }
//
//	bool operator==(NpcVector3 right) {
//		if (x != right.x) return false;
//		if (y != right.y) return false;
//		if (z != right.z) return false;
//		return true;
//	}
//
//	float getSize() { return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2)); }
//};
//
//NpcVector3 NpcdefaultVec{ -9999.f, -9999.f, -9999.f };
//
//NpcVector3 NPC_calcCrossProduct(NpcVector3 lval, NpcVector3 rval) {
//	float cp_x = lval.y * rval.z - lval.z * rval.y;
//	float cp_y = lval.z * rval.x - lval.x * rval.z;
//	float cp_z = lval.x * rval.y - lval.y * rval.x;
//	return NpcVector3{ cp_x, cp_y, cp_z };
//}
//
//NpcVector3 NPC_getNormalVec(NpcVector3 v1, NpcVector3 v2, NpcVector3 v3) {
//	NpcVector3 vec_21 = v1 - v2;
//	NpcVector3 vec_23 = v3 - v2;
//	return (NPC_calcCrossProduct(vec_21, vec_23));
//}
//
//struct NPC_Rectangle
//{
//	NpcVector3 p1, p2, p3, p4;
//};
//
//struct NPCCube
//{
//	NpcVector3 center;				// 중심 좌표
//	float width, height, length;	// 가로 세로 높이
//
//	NPCCube() { center = { 0,0,0 }; width = height = length = 0; }
//	NPCCube(NpcVector3 c, float w, float h, float l) { center = c; width = w; height = h; length = l; }
//
//	NpcVector3 getP1() { float x = center.x - width / 2; float y = center.y + height / 2; float z = center.z + length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP2() { float x = center.x + width / 2; float y = center.y + height / 2; float z = center.z + length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP4() { float x = center.x + width / 2; float y = center.y + height / 2; float z = center.z - length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP3() { float x = center.x - width / 2; float y = center.y + height / 2; float z = center.z - length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP5() { float x = center.x - width / 2; float y = center.y - height / 2; float z = center.z + length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP6() { float x = center.x + width / 2; float y = center.y - height / 2; float z = center.z + length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP7() { float x = center.x - width / 2; float y = center.y - height / 2; float z = center.z - length / 2; return NpcVector3{ x, y, z }; };
//	NpcVector3 getP8() { float x = center.x + width / 2; float y = center.y - height / 2; float z = center.z - length / 2; return NpcVector3{ x, y, z }; };
//};
//
//NpcVector3 NPC_GetIntersection_Line2Plane(NpcVector3 pos, NpcVector3 look, NpcVector3 plane_p1, NpcVector3 plane_p2, NpcVector3 plane_p3) {
//	// 법선벡터 구하기
//	NpcVector3 normal = NPC_getNormalVec(plane_p1, plane_p2, plane_p3);
//	float a = normal.x;
//	float b = normal.y;
//	float c = normal.z;
//	float d = -(a * plane_p1.x + b * plane_p1.y + c * plane_p1.z);
//
//
//	NpcVector3 intersection;
//	float t_denominator = a * look.x + b * look.y + c * look.z;	// 매개변수t의 분모
//	if (t_denominator == 0) {
//		intersection = pos;
//	}
//	else {
//		float t = -1.0f * (a * pos.x + b * pos.y + c * pos.z + d) / (a * look.x + b * look.y + c * look.z);	// 매개변수 t
//		if (t >= 0) {
//			intersection = pos + NpcVector3{ look.x * t, look.y * t, look.z * t };
//		}
//		else {
//			return NpcdefaultVec;
//		}
//	}
//
//	if (a * intersection.x + b * intersection.y + c * intersection.z + d != 0) {
//		return NpcdefaultVec;
//	}
//	return intersection;
//
//}
//
//float calcDistance(NpcVector3 v1, NpcVector3 v2) {
//	return sqrtf(powf((v1.x - v2.x), 2) + powf((v1.y - v2.y), 2) + powf(v1.z - v2.z, 2));
//}
//
//NpcVector3 GetInterSection_Line2Cube(NpcVector3 p, NpcVector3 lkvec, NPCCube bb) {
//	NpcVector3 Intersections[6] = { NpcdefaultVec, NpcdefaultVec, NpcdefaultVec, NpcdefaultVec, NpcdefaultVec, NpcdefaultVec };
//	// ㅁp1p2p3p4 평면방정식과 반직선의 충돌점 구하기
//	NpcVector3 Intersection_1234 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP2(), bb.getP3());
//	Intersections[0] = Intersection_1234;
//
//	// ㅁp1p3p5p7 평면방정식과 반직선의 충돌점 구하기
//	NpcVector3 Intersection_1357 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP3(), bb.getP5());
//	Intersections[1] = Intersection_1357;
//
//	// ㅁp3p4p7p8 평면방정식과 반직선의 충돌점 구하기
//	NpcVector3 Intersection_3478 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP3(), bb.getP4(), bb.getP7());
//	Intersections[2] = Intersection_3478;
//
//	// ㅁp4p2p8p6 평면방정식과 반직선의 충돌점 구하기
//	NpcVector3 Intersection_4286 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP4(), bb.getP2(), bb.getP8());
//	Intersections[3] = Intersection_4286;
//
//	// ㅁp2p1p6p5 평면방정식과 반직선의 충돌점 구하기
//	NpcVector3 Intersection_2165 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP2(), bb.getP1(), bb.getP6());
//	Intersections[4] = Intersection_2165;
//
//	// ㅁp5p6p7p8 평면방정식과 반직선의 충돌점 구하기
//	NpcVector3 Intersection_7856 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP7(), bb.getP8(), bb.getP5());
//	Intersections[5] = Intersection_7856;
//
//	// 3. p와 교차하는 점들 사이의 거리를 계산해서 가장 가까운 점이 교점이다.
//	float min_dist = INFINITY;
//	int min_index = 0;
//	for (int i = 0; i < 5; ++i) {
//		if (Intersections[i] == NpcdefaultVec) continue;
//		float cur_dist = calcDistance(p, Intersections[i]);
//		if (cur_dist < min_dist) {
//			min_dist = cur_dist;
//			min_index = i;
//		}
//	}
//
//	// 4. 그 충돌점이 큐브 안에 있는지 검사한다.
//	if ((bb.getP3().x <= Intersections[min_index].x && Intersections[min_index].x <= bb.getP4().x)
//		&& (bb.getP7().y <= Intersections[min_index].y && Intersections[min_index].y <= bb.getP3().y)
//		&& (bb.getP3().z <= Intersections[min_index].z && Intersections[min_index].z <= bb.getP1().z)) {
//		return Intersections[min_index];
//	}
//
//	// 5. 큐브 안에 없다면 충돌하지 않은 것이다.
//	return NpcdefaultVec;
//}

class ST1_NPC {
private:
	int m_ID;
	int m_NpcType;
	float m_pitch, m_yaw, m_roll;					// Rotated Degree

	XMFLOAT3 m_Pos;
	Coordinate m_curr_coordinate;				// 현재 Look, Right, Up Vectors

	int m_hp;

	XMFLOAT3 m_User_Pos[5];
	int m_state;

	int m_Hit;
	int m_ProfellerHP;
	int m_BodyHP;
	int m_attack;
	int m_defence;
	int m_chaseID;
	int m_IdleCity;
	int m_IdleSection;

	float m_Speed;

	float m_Distance[5];

	bool m_SectionMoveDir;
	vector<City_Info>m_Section;
public:
	BoundingOrientedBox m_xoobb_Pro;
	BoundingOrientedBox m_xoobb_Body;

	BoundingFrustum m_frustum;

	/*MyVector3 exc_XMFtoMyVec(XMFLOAT3 xmf3) { return MyVector3{ xmf3.x, xmf3.y, xmf3.z }; }
	XMFLOAT3 exc_MyVectoXMF(MyVector3 mv3) { return XMFLOAT3{ mv3.x, mv3.y, mv3.z }; }*/
public:
	// ===========================================
	// =============      BASE      ==============
	// ===========================================
	ST1_NPC();
	~ST1_NPC();

	// ===========================================
	// =============       SET      ==============
	// ===========================================
	void SetHp(int hp);
	void SetID(int id);
	void SetNpcType(int type);
	void SetChaseID(int id);
	void SetIdleCity(int num);
	void SetIdleSection(int num);
	void SetRotate(float y, float p, float r);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 pos);
	void SetCurr_coordinate(Coordinate cor);
	void SetUser_Pos(XMFLOAT3 pos, int id);
	void SetInitSection(vector<City_Info>const& v);

	void SetDistance(float dis);
	void SetSpeed(float spd);
	void SetFrustum();
	// ===========================================
	// =============       GET      ==============
	// ===========================================
	int GetHp();
	int GetID();
	int GetChaseID();
	int GetType();
	int GetState();
	int GetIdleCity();
	int GetIdleSection();
	XMFLOAT3 GetPosition();
	Coordinate GetCurr_coordinate();

	float MyGetPosition();
	float GetRotate();
	float GetDistance(int id);
	float GetSpeed(float spd);

	vector<City_Info>GetCityInfo();
public:

	// ===========================================
	// =============     NORMAL     ==============
	// ===========================================

	// State
	void ST1_State_Manegement(int state); // 상태 관리

	void Caculation_Distance(XMFLOAT3 vec, int id); // 범위 내 플레이어 탐색
	void ST1_Death_motion(); // HP 0

	// Damege
	void ST1_Damege_Calc(int id);

	// Idle
	void MoveInSection();
	void MoveChangeIdle();

	// Fly
	void FlyOnNpc(XMFLOAT3 vec, int id);

	// Chase
	void PlayerChasing();
	bool PlayerDetact();

	// Rotate
	XMFLOAT3 NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll);

	void setBB_Pro() { m_xoobb_Pro = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X * 2 , HELI_BBSIZE_Y * 2, HELI_BBSIZE_Z * 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
	void setBB_Body() { m_xoobb_Body = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X * 2, HELI_BBSIZE_Y * 2, HELI_BBSIZE_Z * 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }


	// Ray Cast


};
