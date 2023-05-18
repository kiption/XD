#pragma once
#include <math.h>
#include <vector>
#include <DirectXCollision.h>
#include "Constant.h"
#include "MyVectors.h"

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
	Section_Info SectionNum[6];
	float Centerx, Centerz;
	int id;
};

// 도시 위치 바뀌면 위치가 정리된 파일을 읽던가 이렇게 배열 형태로 해서 가져다 쓰던가로 해야함. --> 임시
const float SX_range[18] = {
	-780.0f, -780.0f, -380.0f, -380.0f, -75.0f, -75.0f,
	110.0f, 110.0f, 110.0f, 1090.0f, 660.0f, 660.0f,
	150.0f, -400.0f, -400.0f, -600.0f, -600.0f, -1550.0f };
const float LX_range[18] = { 
	-710.0f, -300.0f, -300.0f, 10.0f, 10.0f, 950.0f,
	690.0f, 200.0f, 1150.0f, 1150.0f, 1150.0f, 740.0f,
	230.0f, 230.0f, -370.0f, -370.0f, -520.0f, -520.0f };
const float SZ_range[18] = { 
	-900.0f, -430.0f, -1390.0f, -1390.0f, -1390.0f, -980.0f, 
	-690.0f, -690.0f, -260.0f, -260.0f, 30.0f, 30.0f,
	140.0f, 140.0f, 140.0f , 350.0f, -60.0f, -60.0f };
const float LZ_range[18] = {
	-350.0f, -350.0f, -350.0f, -1330.0f, -900.0f, -900.0f,
	-600.0f, -190.0f, -190.0f, 110.0f, 110.0f, 1060.0f,
	720.0f, 200.0f, 410.0f, 410.0f, 410.0f, 0.0f };
const float C_cx[3] = { -396.0f, 651.0f, -393.0f };
const float C_cz[3] = { -888.0f, -128.0f, 233.0f };

class NPC_MapObject
{
private:
	float pos_x, pos_y, pos_z;
	float scale_x, scale_y, scale_z;

public:
	NPC_MapObject() {
		pos_x = pos_y = pos_z = 0;
		scale_x = scale_y = scale_z = 0;
	}
	NPC_MapObject(float px, float py, float pz, float sx, float sy, float sz) {
		pos_x = px, pos_y = py, pos_z = pz;
		scale_x = sx, scale_y = sy, scale_z = sz;
	}

public:
	void setPos(float px, float py, float pz) {
		pos_x = px, pos_y = py, pos_z = pz;
	}
	void setScale(float sx, float sy, float sz) {
		scale_x = sx, scale_y = sy, scale_z = sz;
	}

public:
	float getPosX() { return pos_x; }
	float getPosY() { return pos_y; }
	float getPosZ() { return pos_z; }
	float getScaleX() { return scale_x; }
	float getScaleY() { return scale_y; }
	float getScaleZ() { return scale_z; }
};

class NPC_Building : public NPC_MapObject
{
public:
	NPC_Building() : NPC_MapObject() {}
	NPC_Building(float px, float py, float pz, float sx, float sy, float sz) : NPC_MapObject(px, py, pz, sx, sy, sz) {}

public:
	BoundingOrientedBox m_xoobb;

public:
	void setBB() {
		m_xoobb = BoundingOrientedBox(XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()),
			XMFLOAT3(this->getScaleX(), this->getScaleY(), this->getScaleZ()),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	XMFLOAT3 getPos() { return XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()); }
};

struct Node {
	int id;
	vector<int> neighbors;
	NPC_Building allow_Building_info; // 건물 정보를 저장할 멤버 변수
};

class Graph {
private:
	vector<Node> nodes;

public:
	void addNode(Node node, NPC_Building Building_info) {
		node.allow_Building_info = Building_info; // 건물 정보 설정
		nodes.push_back(node);
	}

	void addEdge(int node1, int node2) {
		nodes[node1].neighbors.push_back(node2);
		nodes[node2].neighbors.push_back(node1);
	}

	vector<Node> getNodes() const {
		return nodes;
	}
};

struct Npc_Vector3
{
	float x, y, z;

	Npc_Vector3 operator+(Npc_Vector3 right) { float ret_x = x + right.x; float ret_y = y + right.y; float ret_z = z + right.z; return Npc_Vector3{ ret_x, ret_y, ret_z }; }
	Npc_Vector3 operator-(Npc_Vector3 right) { float ret_x = x - right.x; float ret_y = y - right.y; float ret_z = z - right.z; return Npc_Vector3{ ret_x, ret_y, ret_z }; }

	bool operator==(Npc_Vector3 right) {
		if (x != right.x) return false;
		if (y != right.y) return false;
		if (z != right.z) return false;
		return true;
	}

	float getSize() { return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2)); }
};

struct NPC_Rectangle
{
	Npc_Vector3 p1, p2, p3, p4;
};

struct NPCCube
{
	Npc_Vector3 center;				// 중심 좌표
	float width, height, length;	// 가로 세로 높이

	NPCCube() { center = { 0,0,0 }; width = height = length = 0; }
	NPCCube(Npc_Vector3 c, float w, float h, float l) { center = c; width = w; height = h; length = l; }

	Npc_Vector3 getP1() { float x = center.x - width / 2; float y = center.y + height / 2; float z = center.z + length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP2() { float x = center.x + width / 2; float y = center.y + height / 2; float z = center.z + length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP4() { float x = center.x + width / 2; float y = center.y + height / 2; float z = center.z - length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP3() { float x = center.x - width / 2; float y = center.y + height / 2; float z = center.z - length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP5() { float x = center.x - width / 2; float y = center.y - height / 2; float z = center.z + length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP6() { float x = center.x + width / 2; float y = center.y - height / 2; float z = center.z + length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP7() { float x = center.x - width / 2; float y = center.y - height / 2; float z = center.z - length / 2; return Npc_Vector3{ x, y, z }; };
	Npc_Vector3 getP8() { float x = center.x + width / 2; float y = center.y - height / 2; float z = center.z - length / 2; return Npc_Vector3{ x, y, z }; };
};

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
	//float* m_BtoN_distance = NULL;
	bool m_SectionMoveDir;
	vector<City_Info>m_Section;
	float NPCtoBuilding_Dis[600];
public:
	bool PrintRayCast = false;
	bool m_DeathCheck = false;
	BoundingOrientedBox m_xoobb_Pro;
	BoundingOrientedBox m_xoobb_Body;
	vector<BoundingOrientedBox>m_mapxmoobb;
	BoundingFrustum m_frustum;

	Npc_Vector3 Npc_defaultVec{ -9999.f, -9999.f, -9999.f };
	vector<NPC_Building> npc_buildings_info;

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
	void SetBuildingInfo(int id, XMFLOAT3 bPos, XMFLOAT3 bScale);
	void SetBuildingNode();
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
	XMFLOAT3 GetBuildingInfo(int id);
public:
	// ===========================================
	// =============     NORMAL     ==============
	// ===========================================
	// State
	void ST1_State_Manegement(int state); // 상태 관리

	void Caculation_Distance(XMFLOAT3 vec, int id); // 범위 내 플레이어 탐색
	float Building_Caculation_Distance(XMFLOAT3 vec);

	void ST1_Death_motion(); // HP 0

	// Find Distance
	void SetTrackingIDbyDistance(float setDistance, int curState, int nextState);

	// Restore the state according to the distance to the previous
	bool SetTrackingPrevStatebyDistance(float setDistance, int curState, int prevState);

	// Damege
	void ST1_Damege_Calc(int id);
	void ST1_CheckNPC_HP();

	// Idle
	void MoveInSection();
	void MoveChangeIdle();

	// Fly
	void FlyOnNpc(XMFLOAT3 vec, int id);

	// Chase
	void PlayerChasing();
	bool PlayerDetact();

	// Attack
	void PlayerAttack();

	// Rotate
	XMFLOAT3 NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll);

	void setBB_Pro() { m_xoobb_Pro = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X * 2, HELI_BBSIZE_Y * 2, HELI_BBSIZE_Z * 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
	void setBB_Body() { m_xoobb_Body = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X * 2, HELI_BBSIZE_Y * 2, HELI_BBSIZE_Z * 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }

	// Building Collide
	void BuildingToNPC_Distance();
	void NPCtoBuilding_collide();
	bool isPathPossible(const NPC_Building& building1, const NPC_Building& building2);

	// Ray Cast
	Npc_Vector3 NPC_calcCrossProduct(Npc_Vector3 lval, Npc_Vector3 rval);
	Npc_Vector3 NPC_getNormalVec(Npc_Vector3 v1, Npc_Vector3 v2, Npc_Vector3 v3);
	Npc_Vector3 NPC_GetIntersection_Line2Plane(Npc_Vector3 pos, Npc_Vector3 look, Npc_Vector3 plane_p1, Npc_Vector3 plane_p2, Npc_Vector3 plane_p3);
	float NPC_calcDistance(Npc_Vector3 v1, Npc_Vector3 v2);
	Npc_Vector3 GetInterSection_Line2Cube(Npc_Vector3 p, Npc_Vector3 lkvec, NPCCube bb);
};
