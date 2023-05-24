//============================================================
//						  Standard
//============================================================
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <mutex>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <queue>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

//============================================================
//						 DirectX API
//============================================================
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

using namespace DirectX;
using namespace DirectX::PackedVector;

//============================================================
//			메인서버에서 사용한 것들을 그대로 사용
//============================================================
#include "../../MainServer/Server/Protocol.h"
#include "../../MainServer/Server/Constant.h"
#include "../../MainServer/Server/MathFuncs.h"
//#include "../../MainServer/Server/MapObjects.h"
#include "../../MainServer/Server/CP_KEYS.h"
#include "CheckPoint.h"

using namespace std;
using namespace chrono;

system_clock::time_point g_s_start_time;	// 서버 시작시간  (단위: ms)
milliseconds g_curr_servertime;
mutex servertime_lock;	// 서버시간 lock

enum NPCState { NPC_IDLE, NPC_FLY, NPC_CHASE, NPC_ATTACK, NPC_DEATH };
enum Hit_target { g_none, g_body, g_profeller };

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
Coordinate basic_coordinate;	// 기본(초기) 좌표계

//======================================================================
struct Section_Info {
	float sx, sz, lx, lz;
	int ID;
};
struct City_Info {
	Section_Info SectionNum[4];
	float Centerx, Centerz;
	int id;
};

vector<City_Info>Cities;
//======================================================================

float C_cx[4];
float C_cz[4];

float Calculation_Distance(XMFLOAT3 vec, int c_id) // vec-> Player's pos, v -> city's center pos 
{
	float dist = sqrtf(powf(vec.x - Cities[c_id].Centerx, 2) + powf(vec.z - Cities[c_id].Centerz, 2));
	return dist;
}

float Calculation_Distance(XMFLOAT3 vec, int c_id, int s_id)
{
	float cx = (Cities[c_id].SectionNum[s_id].lx + Cities[c_id].SectionNum[s_id].sx) / 2;
	float cz = (Cities[c_id].SectionNum[s_id].lz + Cities[c_id].SectionNum[s_id].sz) / 2;

	float dist = sqrtf(powf(vec.x - cx, 2) + powf(vec.z - cz, 2));
	return dist;
}

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

//======================================================================
class OBJECT {
public:
	mutex obj_lock;

	int id;
	char name[NAME_SIZE];

	short state;	// PLAYER_STATE
	int hp;
	int remain_bullet;

	XMFLOAT3 pos;									// Position (x, y, z)
	float pitch, yaw, roll;							// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// 현재 Look, Right, Up Vectors

	BoundingOrientedBox m_xoobb;	// Bounding Box

public:
	OBJECT()
	{
		id = -1;
		name[0] = 0;

		state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
	void memberClear()
	{
		id = -1;
		name[0] = 0;

		state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		setBB();
	}
};

//======================================================================
class PLAYER : public OBJECT {
public:
	PLAYER() : OBJECT()
	{
		hp = HELI_MAXHP;
	}
};

array<PLAYER, MAX_USER> playersInfo;

//======================================================================
class NPC : public OBJECT {
private:
	Coordinate m_coordinate;

	XMFLOAT3 m_User_Pos[MAX_USER];

	short m_Hit;
	short m_state;
	int m_attack;
	int m_defence;
	int m_ProfellerHP;
	int m_BodyHP;
	int m_chaseID;
	int m_IdleCity;
	int m_IdleSection;

	float m_Speed;
	float m_Distance[MAX_USER];

	bool m_SectionMoveDir;

public:
	bool m_DeathCheck = false;
	bool PrintRayCast = false;
	vector<City_Info>m_Section;
	BoundingFrustum m_frustum;
	MyVector3 m_VectorMAX = { -9999.f, -9999.f, -9999.f };
public:
	NPC() : OBJECT() {
		m_ProfellerHP = HELI_MAXHP;
		m_BodyHP = HELI_MAXHP;
		m_state = NPC_IDLE;
		m_chaseID = -1;
		for (int i{}; i < MAX_USER; ++i) {
			m_Distance[i] = 20000;
		}
	}

public:
	// Get
	int GetHp() { return hp; }
	int GetID() { return id; }
	int GetChaseID() { return m_chaseID; }
	int GetState() { return m_state; }
	int GetIdleCity() { return m_IdleCity; }
	int GetIdleSection() { return m_IdleSection; }
	XMFLOAT3 GetPosition() { return pos; }
	Coordinate GetCurr_coordinate() { return m_coordinate; }
	float GetDistance(int id) { return m_Distance[id]; }
	float GetSpeed() { return m_Speed; }

	//int GetType();
	//float GetRotate();
	//vector<City_Info>GetCityInfo();
	//XMFLOAT3 GetBuildingInfo(int id);

public:
	// Set
	void SetHp(int thp) { hp = thp; }
	void SetID(int tid) { id = tid; }
	void SetChaseID(int cid) { m_chaseID = cid; }
	void SetIdleCity(int num) { m_IdleCity = num; }
	void SetIdleSection(int num) { m_IdleSection = num; }
	void SetRotate(float y, float p, float r) { yaw = y, pitch = p, roll = r; }
	void SetPosition(XMFLOAT3 tpos) { pos = tpos; }
	void SetCurr_coordinate(Coordinate cor) { m_coordinate = cor; }
	void SetUser_Pos(XMFLOAT3 pos, int cid) { m_User_Pos[cid] = pos; }
	void SetSpeed(float spd) { m_Speed = spd; }
	void SetFrustum();

	//void SetNpcType(int type);
	//void SetPosition(float x, float y, float z);
	//void SetInitSection(vector<City_Info>const& v);	
	//void SetBuildingInfo(int id, XMFLOAT3 bPos, XMFLOAT3 bScale);
	//void SetBuildingNode();
public:
	// Normal
		// State
	void NPC_State_Manegement(int state); // 상태 관리
	void Caculation_Distance(XMFLOAT3 vec, int id); // 범위 내 플레이어 탐색


	void NPC_Death_motion(); // HP 0

	// Find Distance
	void SetTrackingIDbyDistance(float setDistance, int curState, int nextState);

	// Restore the state according to the distance to the previous
	bool SetTrackingPrevStatebyDistance(float setDistance, int curState, int prevState);

	// Damege
	void NPC_Damege_Calc(int id);
	void NPC_Check_HP();

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

	//float Building_Caculation_Distance(XMFLOAT3 vec);
	/*void setBB_Pro() { m_xoobb_Pro = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X * 2, HELI_BBSIZE_Y * 2, HELI_BBSIZE_Z * 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
	void setBB_Body() { m_xoobb_Body = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X * 2, HELI_BBSIZE_Y * 2, HELI_BBSIZE_Z * 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }*/

	// Building Collide
	//void BuildingToNPC_Distance();
	//void NPCtoBuilding_collide();
	//bool isPathPossible(const NPC_Building& building1, const NPC_Building& building2);
};

void NPC::SetFrustum()
{
	// NPC의 위치와 Look 벡터를 가져온다.
	XMVECTOR position = XMLoadFloat3(&pos);
	XMVECTOR look = XMLoadFloat3(&m_coordinate.look);

	// Frustum의 사이드 범위와 상하 범위를 설정한다.
	float width = 50.0f;
	float height = 50.0f;

	// Frustum의 시작점을 설정한다.
	XMVECTOR startPoint = position;

	// Frustum의 끝점을 설정한다.
	XMVECTOR endPoint = position + (look * 200.0f);

	// Frustum의 Up 벡터를 설정한다.
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Frustum을 생성하고 설정한다.
	BoundingFrustum frustum;

	// Frustum의 Origin을 설정한다.
	XMStoreFloat3(&frustum.Origin, startPoint);

	// Frustum의 Orientation을 설정한다.
	XMMATRIX viewMatrix = XMMatrixLookAtRH(startPoint, endPoint, up);
	XMVECTOR quaternion = XMQuaternionRotationMatrix(viewMatrix);
	XMStoreFloat4(&frustum.Orientation, quaternion);

	frustum.RightSlope = width / 50.0f;
	frustum.LeftSlope = width / -50.0f;
	frustum.TopSlope = height / 50.0f;
	frustum.BottomSlope = height / -50.0f;
	frustum.Near = 0.1f;
	frustum.Far = 50.0f;

	m_frustum = frustum;
}
XMFLOAT3 NPC::NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll)
{
	float curr_pitch = XMConvertToRadians(pitch);
	float curr_yaw = XMConvertToRadians(yaw);
	float curr_roll = XMConvertToRadians(roll);

	// roll
	float x1, y1;
	x1 = vec.x * cos(curr_roll) - vec.y * sin(curr_roll);
	y1 = vec.x * sin(curr_roll) + vec.y * cos(curr_roll);

	// pitch
	float y2, z1;
	y2 = y1 * cos(curr_pitch) - vec.z * sin(curr_pitch);
	z1 = y1 * sin(curr_pitch) + vec.z * cos(curr_pitch);

	// yaw
	float x2, z2;
	z2 = z1 * cos(curr_yaw) - x1 * sin(curr_yaw);
	x2 = z1 * sin(curr_yaw) + x1 * cos(curr_yaw);

	// Update
	vec = { x2, y2, z2 };

	return NPCNormalize(vec);
}
void NPC::Caculation_Distance(XMFLOAT3 vec, int id) // 서버에서 따로 부를 것.
{
	m_Distance[id] = sqrtf(pow((vec.x - pos.x), 2) + pow((vec.y - pos.y), 2) + pow((vec.z - pos.z), 2));
}
void NPC::NPC_State_Manegement(int state)
{
	switch (m_state)
	{
	case NPC_IDLE: // 매복 혹은 특정 운동을 하는 중.
	{
		MoveInSection();

		SetTrackingIDbyDistance(500.0f, NPC_IDLE, NPC_FLY);
	}
	break;
	case NPC_FLY:
	{
		// Fly 지속 or Fly -> chase or Fly -> Idle
		FlyOnNpc(m_User_Pos[m_chaseID], m_chaseID); // 대상 플레이어와의 y 좌표를 비슷하게 맞춤.

		SetTrackingIDbyDistance(350.0f, NPC_FLY, NPC_CHASE);
		if (m_state == NPC_FLY) {
			if (!SetTrackingPrevStatebyDistance(350.0f, NPC_FLY, NPC_IDLE)) {
				MoveChangeIdle();
				m_chaseID = -1;
			}
		}
	}
	break;
	case NPC_CHASE:
	{
		// chase -> chase or chase -> attack or chase -> fly	
		PlayerChasing();
		SetTrackingIDbyDistance(300.0f, NPC_CHASE, NPC_ATTACK); // ID 탐색
		if (PlayerDetact()) { // Id 탐색은 이미 가장 가까운 대상으로 지정하기에 따로 탐색은 하지 않음.
			m_state = NPC_ATTACK; // 다음 상태
		}
		else {
			SetTrackingPrevStatebyDistance(300.0f, NPC_CHASE, NPC_FLY);
		}
	}
	break;
	case NPC_ATTACK:
	{
		// bullet 관리
		SetTrackingIDbyDistance(200.0f, NPC_ATTACK, NPC_ATTACK); // ID 탐색
		if (!PlayerDetact()) { // Id 탐색은 이미 가장 가까운 대상으로 지정하기에 따로 탐색은 하지 않음.
			m_state = NPC_CHASE; // 이전 상태
			PrintRayCast = false;
		}
		else {
			PlayerAttack();
		}
	}
	break;
	case NPC_DEATH:
	{
		if (pos.y > 0.0f) {
			NPC_Death_motion();
		}
	}
	break;
	default:
		break;
	}

	//BuildingToNPC_Distance();
	NPC_Check_HP();
	//setBB_Pro();
	//setBB_Body();
	SetFrustum();
}
void NPC::NPC_Death_motion()
{
	pos.y -= 6.0f;

	// 빙글빙글 돌며 추락
	yaw += 3.0f;

	Coordinate base_coordinate;
	m_coordinate.right = NPCcalcRotate(base_coordinate.right, pitch, yaw, roll);
	m_coordinate.up = NPCcalcRotate(base_coordinate.up, pitch, yaw, roll);
	m_coordinate.look = NPCcalcRotate(base_coordinate.look, pitch, yaw, roll);
}
void NPC::SetTrackingIDbyDistance(float setDistance, int curState, int nextState)
{
	bool State_check = false;
	float MinDis = 50000;
	for (int i{}; i < MAX_USER; ++i) {
		if (m_Distance[i] < setDistance) {  // 상태로 변환하기 위한 조건 및 추적 ID 갱신
			State_check = true;
			if (m_Distance[i] < MinDis) {
				m_chaseID = i;
				MinDis = m_Distance[i];
			}
			m_state = nextState; // 자신의 상태를 다음 상태로 변경
		}
		if (i == MAX_USER - 1 && !State_check) { // 자신의 상태 유지
			m_state = curState;
		}
	}
}
bool NPC::SetTrackingPrevStatebyDistance(float setDistance, int curState, int prevState)
{
	int change_cnt = 0;

	for (int i{}; i < MAX_USER; ++i) {
		if (m_Distance[i] >= setDistance) {  // 상태로 변환하기 위한 조건 및 추적 ID 갱신
			change_cnt++;
		}
	}

	if (change_cnt != MAX_USER) {
		m_state = curState;
		return true;
	}
	else {
		for (int i{}; i < MAX_USER; ++i) {
			m_Distance[i] = 10000;
		}
		m_state = prevState;
		return false;
	}
}
void NPC::MoveInSection()
{
	if (m_SectionMoveDir) {
		switch (m_IdleCity)
		{
		case 0:
			switch (m_IdleSection)
			{
			case 0:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 1:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 2:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 3:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 4:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 5:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_SectionMoveDir = false;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx , pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			}
		case 1:
			switch (m_IdleSection)
			{
			case 0:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 1:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 2:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 3:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 4:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 5:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_SectionMoveDir = false;
				}
				else {
					XMFLOAT3 sec_look = { pos.x , pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			}
		case 2:
			switch (m_IdleSection)
			{
			case 0:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 1:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 2:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 3:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 4:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection++;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 5:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_SectionMoveDir = false;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx , pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			}
		}
	}
	else {
		switch (m_IdleCity)
		{
		case 0:
			switch (m_IdleSection)
			{
			case 0:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_SectionMoveDir = true;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 1:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 2:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 3:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 4:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 5:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx , pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			}
		case 1:
			switch (m_IdleSection)
			{
			case 0:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_SectionMoveDir = true;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 1:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 2:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].sx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 3:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 4:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 5:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x , pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			}
		case 2:
			switch (m_IdleSection)
			{
			case 0:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_SectionMoveDir = true;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 1:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 2:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].sz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].sz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 3:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx, pos.y, pos.z };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			case 4:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lz - pos.z)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { pos.x, pos.y, Cities[m_IdleCity].SectionNum[m_IdleSection].lz };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.z += m_Speed * m_coordinate.look.z;
				}
			}
			break;
			case 5:
			{
				if (25.0f > abs(Cities[m_IdleCity].SectionNum[m_IdleSection].lx - pos.x)) {
					m_IdleSection--;
				}
				else {
					XMFLOAT3 sec_look = { Cities[m_IdleCity].SectionNum[m_IdleSection].lx , pos.y, pos.x };
					XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&pos)));
					XMStoreFloat3(&m_coordinate.look, Looktemp);

					// Right
					Coordinate base_coordinate;
					base_coordinate.up = { 0,1,0 };

					XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
					XMStoreFloat3(&m_coordinate.right, righttemp);

					// up
					XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
					XMStoreFloat3(&m_coordinate.up, uptemp);

					pos.x += m_Speed * m_coordinate.look.x;
				}
			}
			break;
			}
		}
	}
	//NPCtoBuilding_collide();
}
void NPC::MoveChangeIdle()
{
	float City_dis{};
	float Min_DisofCity = 100000;
	int id{};
	for (int i{}; i < 3; ++i) {
		City_dis = Calculation_Distance(pos, i);
		if (Min_DisofCity > City_dis) {
			Min_DisofCity = City_dis;
			id = i;
		}
	}

	if (Min_DisofCity > 600.0f) {
		// 계산한 값이 거리가 일정 구간 안이 아닐 경우	
		float dis{};
		int s_id{};
		float Min_DisofSec = 100000;
		for (int i{}; i < 3; ++i) {
			dis = Calculation_Distance(pos, id, i);
			if (Min_DisofSec > dis) {
				Min_DisofSec = dis;
				s_id = i;
			}
		}
		// 구역 지정
		m_IdleCity = id;
		m_IdleSection = s_id;
	}
}
void NPC::NPC_Damege_Calc(int id)
{
	if (m_Hit == g_body) {
		int distance_damege = 0;
		if (((int)(m_Distance[id])) > 2000) {
			distance_damege = (2000 / 100) * 5;
		}
		else {
			distance_damege = ((((int)(m_Distance[id])) / 100) * 5);
		}
		int damege = (20 * distance_damege) / m_defence;
		m_BodyHP -= damege;
		m_Hit = g_none;
	}
	else if (m_Hit == g_profeller) {
		int distance_damege = 0;
		if (((int)(m_Distance[id])) > 2000) {
			distance_damege = (2000 / 100) * 5;
		}
		else {
			distance_damege = ((((int)(m_Distance[id])) / 100) * 5);
		}
		int damege = (20 * distance_damege) / m_defence;
		m_ProfellerHP -= damege;
		m_Hit = g_none;
	}
}
void NPC::NPC_Check_HP()
{
	if ((m_BodyHP <= 0) || (m_ProfellerHP <= 0)) {
		m_state = NPC_DEATH;
	}
}
void NPC::FlyOnNpc(XMFLOAT3 vec, int id) // 추적대상 플레이어와 높이 맞추기
{
	if (pos.y < vec.y) {
		pos.y += 1.0f;
	}
	if (pos.y > vec.y) {
		pos.y -= 1.5f;
	}
}
void NPC::PlayerChasing()
{
	// Look
	XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&m_User_Pos[m_chaseID]), XMLoadFloat3(&pos)));
	Coordinate base_coordinate;
	base_coordinate.right = { 1,0,0 };
	float x = XMVectorGetX(XMVector3AngleBetweenVectors(Looktemp, XMLoadFloat3(&base_coordinate.right)));
	yaw = x;
	if (x < 1.0f && x > -1.0f) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&base_coordinate.right), XMConvertToRadians(yaw));
		XMStoreFloat3(&m_coordinate.look, Looktemp);
	}
	else {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&base_coordinate.right), XMConvertToRadians(0.0f));
		XMStoreFloat3(&m_coordinate.look, Looktemp);
	}
	// Right
	base_coordinate.up = { 0,1,0 };

	XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
	XMStoreFloat3(&m_coordinate.right, righttemp);

	// up
	XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
	XMStoreFloat3(&m_coordinate.up, uptemp);

	if (m_Distance[m_chaseID] < 50) {
		m_Speed = 0;
	}
	else {
		m_Speed = 1.5f;
	}

	// 위치 변환
	pos.x += m_Speed * m_coordinate.look.x;
	pos.y += m_Speed * m_coordinate.look.y;
	pos.z += m_Speed * m_coordinate.look.z;
	//NPCtoBuilding_collide();
}
bool NPC::PlayerDetact()
{
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	XMVECTOR FrustumOrigin = XMLoadFloat3(&m_frustum.Origin);
	XMVECTOR FrustumOrientation = XMLoadFloat4(&m_frustum.Orientation);

	// Frustum의 꼭짓점 8개를 구한다.
	XMFLOAT3 corners[8];
	m_frustum.GetCorners(corners);

	// Frustum과 Player의 bounding sphere와의 거리를 구한다.
	float distance = FLT_MAX;
	for (int i = 0; i < 8; i++)
	{
		float d = XMVectorGetX(XMVector3Length(PlayerPos - XMLoadFloat3(&corners[i])));
		if (d < distance)
		{
			distance = d;
		}
	}

	// 거리가 bounding sphere의 반지름보다 작으면 충돌했다고 판단한다.
	if (distance < 50.0f)
	{
		return true;
	}

	return false;
}
void NPC::PlayerAttack()
{
	// Look
	PlayerChasing();

	// Attack
	Cube ChasePlayer{ {m_User_Pos[m_chaseID].x, m_User_Pos[m_chaseID].y, m_User_Pos[m_chaseID].z },
		HELI_BOXSIZE_X, HELI_BOXSIZE_Y, HELI_BOXSIZE_Z };
	MyVector3 NPC_bullet_Pos = { pos.x, pos.y , pos.z };
	MyVector3 NPC_Look_Vec = { m_coordinate.look.x, m_coordinate.look.y , m_coordinate.look.z };
	MyVector3 NPC_result;
	NPC_result = MyRaycast_InfiniteRay(NPC_bullet_Pos, NPC_Look_Vec, ChasePlayer);
	if (NPC_result != m_VectorMAX) {
		PrintRayCast = false;
	}
	else {
		PrintRayCast = true;
	}

	//NPCtoBuilding_collide();
}

array<NPC, MAX_NPCS> npcsInfo;

//======================================================================
enum PACKET_PROCESS_TYPE { OP_RECV, OP_SEND, OP_CONNECT };

class OVER_EX {
public:
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	char send_buf[BUF_SIZE];
	PACKET_PROCESS_TYPE process_type;

	OVER_EX()
	{
		wsabuf.len = BUF_SIZE;
		wsabuf.buf = send_buf;
		process_type = OP_RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	OVER_EX(char* packet)
	{
		wsabuf.len = packet[0];
		wsabuf.buf = send_buf;
		ZeroMemory(&overlapped, sizeof(overlapped));
		process_type = OP_SEND;
		memcpy(send_buf, packet, packet[0]);
	}
};

//======================================================================
class SERVER {
public:
	OVER_EX recv_over;
	int remain_size;
	int id;
	SOCKET sock;

public:
	SERVER() { remain_size = 0; id = -1; sock = 0; }

public:
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(sock, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void do_send(void* packet)
	{
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		int ret = WSASend(sock, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}

	void send_npc_init_packet(int npc_id);
	void send_npc_move_packet(int npc_id);
	void send_npc_rotate_packet(int npc_id);
	void send_npc_move_rotate_packet(int npc_id);
};

HANDLE h_iocp;											// IOCP 핸들
int a_lgcsvr_num;										// Active상태인 메인서버
array<SERVER, MAX_LOGIC_SERVER> g_logicservers;			// 로직서버 정보

void SERVER::send_npc_init_packet(int npc_id) {
	NPC_FULL_INFO_PACKET npc_init_packet;
	npc_init_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_init_packet.type = NPC_ROTATE;
	npc_init_packet.n_id = npc_id;
	npc_init_packet.hp = npcsInfo[npc_id].hp;
	npc_init_packet.x = npcsInfo[npc_id].pos.x;
	npc_init_packet.y = npcsInfo[npc_id].pos.y;
	npc_init_packet.z = npcsInfo[npc_id].pos.z;
	npc_init_packet.right_x = npcsInfo[npc_id].GetCurr_coordinate().right.x;
	npc_init_packet.right_y = npcsInfo[npc_id].GetCurr_coordinate().right.y;
	npc_init_packet.right_z = npcsInfo[npc_id].GetCurr_coordinate().right.z;
	npc_init_packet.up_x = npcsInfo[npc_id].GetCurr_coordinate().up.x;
	npc_init_packet.up_y = npcsInfo[npc_id].GetCurr_coordinate().up.y;
	npc_init_packet.up_z = npcsInfo[npc_id].GetCurr_coordinate().up.z;
	npc_init_packet.look_x = npcsInfo[npc_id].GetCurr_coordinate().look.x;
	npc_init_packet.look_y = npcsInfo[npc_id].GetCurr_coordinate().look.y;
	npc_init_packet.look_z = npcsInfo[npc_id].GetCurr_coordinate().look.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_init_packet);
}
void SERVER::send_npc_move_packet(int npc_id) {
	NPC_MOVE_PACKET npc_move_packet;
	npc_move_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_move_packet.type = NPC_MOVE;
	npc_move_packet.n_id = npc_id;
	npc_move_packet.x = npcsInfo[npc_id].pos.x;
	npc_move_packet.y = npcsInfo[npc_id].pos.y;
	npc_move_packet.z = npcsInfo[npc_id].pos.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_move_packet);
}
void SERVER::send_npc_rotate_packet(int npc_id) {
	NPC_ROTATE_PACKET npc_rotate_packet;
	npc_rotate_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_rotate_packet.type = NPC_ROTATE;
	npc_rotate_packet.n_id = npc_id;
	npc_rotate_packet.right_x = npcsInfo[npc_id].GetCurr_coordinate().right.x;
	npc_rotate_packet.right_y = npcsInfo[npc_id].GetCurr_coordinate().right.y;
	npc_rotate_packet.right_z = npcsInfo[npc_id].GetCurr_coordinate().right.z;
	npc_rotate_packet.up_x = npcsInfo[npc_id].GetCurr_coordinate().up.x;
	npc_rotate_packet.up_y = npcsInfo[npc_id].GetCurr_coordinate().up.y;
	npc_rotate_packet.up_z = npcsInfo[npc_id].GetCurr_coordinate().up.z;
	npc_rotate_packet.look_x = npcsInfo[npc_id].GetCurr_coordinate().look.x;
	npc_rotate_packet.look_y = npcsInfo[npc_id].GetCurr_coordinate().look.y;
	npc_rotate_packet.look_z = npcsInfo[npc_id].GetCurr_coordinate().look.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_rotate_packet);
}
void SERVER::send_npc_move_rotate_packet(int npc_id) {
	NPC_MOVE_PACKET npc_mv_packet;
	npc_mv_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_mv_packet.type = NPC_MOVE;
	npc_mv_packet.n_id = npc_id;
	npc_mv_packet.x = npcsInfo[npc_id].pos.x;
	npc_mv_packet.y = npcsInfo[npc_id].pos.y;
	npc_mv_packet.z = npcsInfo[npc_id].pos.z;
	g_logicservers[a_lgcsvr_num].do_send(&npc_mv_packet);

	NPC_ROTATE_PACKET npc_rt_packet;
	npc_rt_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_rt_packet.type = NPC_ROTATE;
	npc_rt_packet.n_id = npc_id;
	npc_rt_packet.right_x = npcsInfo[npc_id].GetCurr_coordinate().right.x;
	npc_rt_packet.right_y = npcsInfo[npc_id].GetCurr_coordinate().right.y;
	npc_rt_packet.right_z = npcsInfo[npc_id].GetCurr_coordinate().right.z;
	npc_rt_packet.up_x = npcsInfo[npc_id].GetCurr_coordinate().up.x;
	npc_rt_packet.up_y = npcsInfo[npc_id].GetCurr_coordinate().up.y;
	npc_rt_packet.up_z = npcsInfo[npc_id].GetCurr_coordinate().up.z;
	npc_rt_packet.look_x = npcsInfo[npc_id].GetCurr_coordinate().look.x;
	npc_rt_packet.look_y = npcsInfo[npc_id].GetCurr_coordinate().look.y;
	npc_rt_packet.look_z = npcsInfo[npc_id].GetCurr_coordinate().look.z;
	cout << npcsInfo[npc_id].GetCurr_coordinate().look.x << ', ' << npcsInfo[npc_id].GetCurr_coordinate().look.y << ', ' << npcsInfo[npc_id].GetCurr_coordinate().look.z << endl;
	g_logicservers[a_lgcsvr_num].do_send(&npc_rt_packet);
}


//======================================================================
class CheckPoint : public MapObject
{
public:
	CheckPoint() : MapObject() {}
	CheckPoint(float px, float py, float pz, float sx, float sy, float sz) : MapObject(px, py, pz, sx, sy, sz) {}

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
vector<CheckPoint> CP;

//======================================================================
void process_packet(char* packet)
{
	switch (packet[1]) {
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* login_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet);

		int client_id = login_packet->id;

		playersInfo[client_id].id = login_packet->id;
		strcpy_s(playersInfo[client_id].name, login_packet->name);

		playersInfo[client_id].pos.x = login_packet->x;
		playersInfo[client_id].pos.y = login_packet->y;
		playersInfo[client_id].pos.z = login_packet->z;

		playersInfo[client_id].m_rightvec.x = login_packet->right_x;
		playersInfo[client_id].m_rightvec.y = login_packet->right_y;
		playersInfo[client_id].m_rightvec.z = login_packet->right_z;

		playersInfo[client_id].m_upvec.x = login_packet->up_x;
		playersInfo[client_id].m_upvec.y = login_packet->up_y;
		playersInfo[client_id].m_upvec.z = login_packet->up_z;

		playersInfo[client_id].m_lookvec.x = login_packet->look_x;
		playersInfo[client_id].m_lookvec.y = login_packet->look_y;
		playersInfo[client_id].m_lookvec.z = login_packet->look_z;

		/*cout << "[Add New Player] Player[ID:" << client_id << ", Name:" << playersInfo[client_id].name << "]의 정보를 받았습니다." << endl;
		cout << "POS: (" << playersInfo[client_id].pos.x << ", " << playersInfo[client_id].pos.y << ", " << playersInfo[client_id].pos.z << "), ";
		cout << "Look: (" << playersInfo[client_id].m_lookvec.x << ", " << playersInfo[client_id].m_lookvec.y << ", " << playersInfo[client_id].m_lookvec.z << ")\n" << endl;*/

		break;
	}// SC_ADD_OBJECT end
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* move_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(packet);

		int client_id = move_packet->id;

		playersInfo[client_id].pos.x = move_packet->x;
		playersInfo[client_id].pos.y = move_packet->y;
		playersInfo[client_id].pos.z = move_packet->z;

		/*cout << "[Move Player] Player[ID:" << client_id << "]가 이동하였습니다." << endl;
		cout << "New POS: (" << playersInfo[client_id].pos.x << ", " << playersInfo[client_id].pos.y << ", " << playersInfo[client_id].pos.z << ")\n" << endl;*/

		break;
	}// SC_MOVE_OBJECT end
	case SC_ROTATE_OBJECT:
	{
		SC_ROTATE_OBJECT_PACKET* rotate_packet = reinterpret_cast<SC_ROTATE_OBJECT_PACKET*>(packet);

		int client_id = rotate_packet->id;

		playersInfo[client_id].m_rightvec.x = rotate_packet->right_x;
		playersInfo[client_id].m_rightvec.y = rotate_packet->right_y;
		playersInfo[client_id].m_rightvec.z = rotate_packet->right_z;

		playersInfo[client_id].m_upvec.x = rotate_packet->up_x;
		playersInfo[client_id].m_upvec.y = rotate_packet->up_y;
		playersInfo[client_id].m_upvec.z = rotate_packet->up_z;

		playersInfo[client_id].m_lookvec.x = rotate_packet->look_x;
		playersInfo[client_id].m_lookvec.y = rotate_packet->look_y;
		playersInfo[client_id].m_lookvec.z = rotate_packet->look_z;

		/*cout << "[Rotate Player] Player[ID:" << client_id << "]가 회전하였습니다." << endl;
		cout << "New Look: (" << playersInfo[client_id].m_lookvec.x << ", " << playersInfo[client_id].m_lookvec.y << ", " << playersInfo[client_id].m_lookvec.z << ")\n" << endl;*/

		break;
	}// SC_ROTATE_OBJECT end
	case SC_MOVE_ROTATE_OBJECT:
	{
		SC_MOVE_ROTATE_OBJECT_PACKET* mvrt_packet = reinterpret_cast<SC_MOVE_ROTATE_OBJECT_PACKET*>(packet);

		int client_id = mvrt_packet->id;

		playersInfo[client_id].pos.x = mvrt_packet->x;
		playersInfo[client_id].pos.y = mvrt_packet->y;
		playersInfo[client_id].pos.z = mvrt_packet->z;

		playersInfo[client_id].m_rightvec.x = mvrt_packet->right_x;
		playersInfo[client_id].m_rightvec.y = mvrt_packet->right_y;
		playersInfo[client_id].m_rightvec.z = mvrt_packet->right_z;

		playersInfo[client_id].m_upvec.x = mvrt_packet->up_x;
		playersInfo[client_id].m_upvec.y = mvrt_packet->up_y;
		playersInfo[client_id].m_upvec.z = mvrt_packet->up_z;

		playersInfo[client_id].m_lookvec.x = mvrt_packet->look_x;
		playersInfo[client_id].m_lookvec.y = mvrt_packet->look_y;
		playersInfo[client_id].m_lookvec.z = mvrt_packet->look_z;

		break;
	}// SC_MOVE_ROTATE_OBJECT end
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* remove_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(packet);
		int client_id = remove_packet->id;
		playersInfo[client_id].memberClear();

		//cout << "[Remove Player] Player[ID:" << client_id << "]가 접속을 종료하였습니다.\n" << endl;

		break;
	}// SC_REMOVE_OBJECT end
	case SC_DAMAGED:
	{
		SC_DAMAGED_PACKET* damaged_packet = reinterpret_cast<SC_DAMAGED_PACKET*>(packet);
		int obj_id = damaged_packet->id;
		if (damaged_packet->target == TARGET_PLAYER) {
			playersInfo[obj_id].hp -= damaged_packet->damage;
		}
		else if (damaged_packet->target == TARGET_NPC) {

		}

		break;
	}// SC_DAMAGED end
	case SC_OBJECT_STATE:
	{
		SC_OBJECT_STATE_PACKET* chgstate_packet = reinterpret_cast<SC_OBJECT_STATE_PACKET*>(packet);
		int client_id = chgstate_packet->id;
		playersInfo[client_id].state = chgstate_packet->state;

		break;
	}// SC_OBJECT_STATE end
	}
}

//======================================================================
void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EX* ex_over = reinterpret_cast<OVER_EX*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_CONNECT) {
				// 서버번호를 바꿔가면서 비동기Connect를 재시도합니다.
				if (a_lgcsvr_num == 0)		a_lgcsvr_num = 1;
				else if (a_lgcsvr_num == 1)	a_lgcsvr_num = 0;
				int new_portnum = a_lgcsvr_num + PORTNUM_LGCNPC_0;
				cout << "[ConnectEX Failed] ";
				cout << "Logic Server[" << a_lgcsvr_num << "] (PORTNUM:" << new_portnum << ")로 다시 연결합니다. \n" << endl;

				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN logic_server_addr;
				ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
				logic_server_addr.sin_family = AF_INET;
				g_logicservers[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<LPSOCKADDR>(&logic_server_addr), sizeof(logic_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, new_portnum, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
				logic_server_addr.sin_family = AF_INET;
				logic_server_addr.sin_port = htons(new_portnum);
				inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);
				// 1. 메인서버와 NPC서버가 루프백에서 동작할 때
				//inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

				// 2. 메인서버와 NPC서버가 다른 PC에서 동작할 떄
				if (a_lgcsvr_num == 0) {
					inet_pton(AF_INET, IPADDR_LOGIC0, &logic_server_addr.sin_addr);
				}
				else if (a_lgcsvr_num == 1) {
					inet_pton(AF_INET, IPADDR_LOGIC1, &logic_server_addr.sin_addr);
				}

				BOOL bret = connectExFP(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<sockaddr*>(&logic_server_addr), sizeof(SOCKADDR_IN),
					nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no) {
						//cout << "Server Connect 재시도 중...\n" << endl;
					}
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
			else {
				//cout << "GQCS Error ( client[" << key << "] )" << endl;

				//disconnect(static_cast<int>(key - CP_KEY_LOGIC2CLIENT));
				if (ex_over->process_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->process_type) {
		case OP_RECV: {
			int remain_data = num_bytes + g_logicservers[a_lgcsvr_num].remain_size;
			char* p = ex_over->send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			g_logicservers[a_lgcsvr_num].remain_size = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}

			g_logicservers[a_lgcsvr_num].do_recv();

			break;
		}//OP_RECV end
		case OP_SEND: {
			//if (0 == num_bytes) disconnect(key - CP_KEY_LOGIC2CLIENT, SESSION_CLIENT);
			delete ex_over;
			break;
		}//OP_SEND end
		case OP_CONNECT: {
			if (FALSE != ret) {
				int server_id = key - PORTNUM_LGCNPC_0;
				std::cout << "성공적으로 Logic Server[" << server_id << "]에 연결되었습니다.\n" << endl;
				g_logicservers[a_lgcsvr_num].remain_size = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, NULL, 0);
				delete ex_over;
				g_logicservers[a_lgcsvr_num].do_recv();
			}

		}//OP_CONN end
		}
	}
}

//======================================================================
void initNpc() {
	for (int i{}; i < 4; ++i) {
		C_cx[i] = (CP[i * 4 + 2].getPosX() + CP[i * 4 + 3].getPosX() + CP[i * 4 + 4].getPosX() + CP[i * 4 + 5].getPosX()) / 4;
		C_cz[i] = (CP[i * 4 + 2].getPosZ() + CP[i * 4 + 3].getPosZ() + CP[i * 4 + 4].getPosZ() + CP[i * 4 + 5].getPosZ()) / 4;
	}


	for (int i{}; i < 4; ++i) {
		City_Info temp;
		temp.id = i;
		temp.Centerx = C_cx[i];
		temp.Centerz = C_cz[i];

		for (int j{}; j < 4; ++j) {
			temp.SectionNum[j].ID = j;
			temp.SectionNum[j].lx = CP[4 * i + 2 + j].getPosX() + (CP[4 * i + 2 + j].getScaleX()) / 2;
			temp.SectionNum[j].lz = CP[4 * i + 2 + j].getPosZ() + (CP[4 * i + 2 + j].getScaleZ()) / 2;
			temp.SectionNum[j].sx = CP[4 * i + 2 + j].getPosX() - (CP[4 * i + 2 + j].getScaleX()) / 2;
			temp.SectionNum[j].sz = CP[4 * i + 2 + j].getPosZ() - (CP[4 * i + 2 + j].getScaleZ()) / 2;
		}
		Cities.emplace_back(temp);
	}

	/*for (int i{}; i < Cities.size(); ++i) {
		Cities[i].print();
	}*/

	for (int i{}; i < MAX_NPCS; i++) {
		int npc_id = i;
		npcsInfo[i].SetID(npc_id);
		//npcs[i].SetNpcType(NPC_Helicopter);
		npcsInfo[i].SetRotate(0.0f, 0.0f, 0.0f);
		//npcs[i].SetActive(false);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float>AirHigh(50, 270);

		uniform_int_distribution<int>Ci_num(0, 2);
		uniform_int_distribution<int>Sec_num(0, 5);

		int city_num = Ci_num(dre);
		int section_num = Sec_num(dre);

		float lx, lz, sx, sz = 0;

		sx = Cities[city_num].SectionNum[section_num].sx;
		lx = Cities[city_num].SectionNum[section_num].lx;
		sz = Cities[city_num].SectionNum[section_num].sz;
		lz = Cities[city_num].SectionNum[section_num].lz;

		npcsInfo[i].SetIdleCity(city_num);
		npcsInfo[i].SetIdleSection(section_num);

		uniform_real_distribution<float>AirXPos(sx, lx);
		uniform_real_distribution<float>AirZPos(sz, lz);
		XMFLOAT3 setPos{ AirXPos(dre), AirHigh(dre), AirZPos(dre) };
		npcsInfo[i].SetPosition(setPos);

		uniform_real_distribution<float>SpdSet(3.5f, 5.2f);
		float speed = SpdSet(dre);
		npcsInfo[i].SetSpeed(speed);
		npcsInfo[i].SetChaseID(-1);
		//npcsInfo[i].SetInitSection(Cities);

		/*for (int j{}; j < buildings_info.size(); ++j) {
			XMFLOAT3 Build_pos = { buildings_info[j].getPos() };
			XMFLOAT3 Build_scale = { buildings_info[j].getScaleX(),buildings_info[j].getScaleY() ,buildings_info[j].getScaleZ() };
			npcsInfo[i].SetBuildingInfo(j, Build_pos, Build_scale);
		}
		npcsInfo[i].SetBuildingNode();*/
	}
}

//======================================================================
void timerFunc() {
	while (true) {
		auto start_t = system_clock::now();
		//======================================================================



		//======================================================================
		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}

//======================================================================
void MoveNPC()
{
	while (true) {
		auto start_t = system_clock::now();
		//======================================================================

		for (int i = 0; i < MAX_NPCS; ++i) {
			// 클라이언트들과 NPC 사이의 거리 계산

			if (npcsInfo[i].GetState() == NPC_DEATH && npcsInfo[i].GetPosition().y < 0) {
				NPC_REMOVE_PACKET npc_remove_packet;

				npc_remove_packet.size = sizeof(NPC_REMOVE_PACKET);
				npc_remove_packet.type = SC_REMOVE_OBJECT;
				npc_remove_packet.n_id = npcsInfo[i].GetID();

				npcsInfo[i].m_DeathCheck = true;

				//for (auto& send_target : playersInfo) {
				//	if (send_target.curr_stage != 1) continue;// 스테이지2 서버동기화 이전까지 사용하는 임시코드.
				//	if (send_target.s_state != ST_INGAME) continue;

				//	lock_guard<mutex> lg{ send_target.s_lock };
				//	send_target.do_send(&npc_remove_packet);
				//}
			}
			if (npcsInfo[i].GetPosition().y > 0)
			{
				for (auto& cl : playersInfo) {
					if (cl.id != -1) {
						npcsInfo[i].Caculation_Distance(cl.pos, cl.id);
					}
				}
				//cout << i << "번째 Status - " << npcs[i].GetState() << endl;				
				npcsInfo[i].NPC_State_Manegement(npcsInfo[i].GetState());
				// NPC가 추적하려는 아이디가 있는지부터 확인, 있으면 추적 대상 플레이어 좌표를 임시 저장
				if (npcsInfo[i].GetChaseID() != -1) {
					npcsInfo[i].SetUser_Pos(playersInfo[npcsInfo[i].GetChaseID()].pos, npcsInfo[i].GetChaseID());
				}

				// npc pos 확인
				cout << "=============" << endl;
				cout << i << "번째 NPC의 도시 ID: " << npcsInfo[i].GetIdleCity() << ", NPC의 섹션 ID: " << npcsInfo[i].GetIdleSection() << endl;
				cout << i << "번째 NPC의 Pos: " << npcsInfo[i].GetPosition().x << ',' << npcsInfo[i].GetPosition().y << ',' << npcsInfo[i].GetPosition().z << endl;
				cout << i << "번째 NPC의 상태: " << npcsInfo[i].GetState() << endl;

				/*if (npcs[i].PrintRayCast) {
					cout << i << "번째 NPC가 쏜 총알에 대해" << npcs[i].GetChaseID() << "의 ID를 가진 플레이어가 피격되었습니다." << endl;
				}*/

				// 상태마다 다른 움직임을 하는 매니지먼트

				//SERVER temp;
				g_logicservers[a_lgcsvr_num].send_npc_move_rotate_packet(npcsInfo[i].GetID());

			}
		}

		//======================================================================
		auto curr_t = system_clock::now();
		if (curr_t - start_t < 500ms)
			this_thread::sleep_for(500ms - (curr_t - start_t));
	}
}


//======================================================================
int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	//					로직서버로 비동기 Connect 요청
	//======================================================================
	int lgvsvr_port = PORTNUM_LGCNPC_0 + a_lgcsvr_num;

	cout << "로직 서버(Server[" << a_lgcsvr_num << "] (PORT: " << lgvsvr_port << ")에 비동기Connect를 요청합니다." << endl;

	// ConnectEx
	SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	GUID guid = WSAID_CONNECTEX;
	DWORD bytes = 0;
	LPFN_CONNECTEX connectExFP;
	::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
	closesocket(temp_s);

	SOCKADDR_IN logic_server_addr;
	ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
	logic_server_addr.sin_family = AF_INET;
	g_logicservers[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
	int ret = ::bind(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<LPSOCKADDR>(&logic_server_addr), sizeof(logic_server_addr));
	if (ret != 0) {
		cout << "Bind Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	OVER_EX* con_over = new OVER_EX;
	con_over->process_type = OP_CONNECT;
	HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, lgvsvr_port, 0);
	if (NULL == hret) {
		cout << "CreateIoCompletoinPort Error - " << ret << endl;
		cout << WSAGetLastError() << endl;
		exit(-1);
	}

	ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
	logic_server_addr.sin_family = AF_INET;
	logic_server_addr.sin_port = htons(lgvsvr_port);

	// 1. 메인서버와 NPC서버가 루프백에서 동작할 때
	//inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

	// 2. 메인서버와 NPC서버가 다른 PC에서 동작할 떄
	if (a_lgcsvr_num == 0) {
		inet_pton(AF_INET, IPADDR_LOGIC0, &logic_server_addr.sin_addr);
	}
	else if (a_lgcsvr_num == 1) {
		inet_pton(AF_INET, IPADDR_LOGIC1, &logic_server_addr.sin_addr);
	}

	BOOL bret = connectExFP(g_logicservers[a_lgcsvr_num].sock, reinterpret_cast<sockaddr*>(&logic_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
	if (FALSE == bret) {
		int err_no = GetLastError();
		if (ERROR_IO_PENDING == err_no)
			cout << "Server Connect 시도 중...\n" << endl;
		else {
			cout << "ConnectEX Error - " << err_no << endl;
			cout << WSAGetLastError() << endl;
		}
	}

	//======================================================================
	//							NPC Initialize
	//======================================================================
	string filename;
	vector<string> readTargets;

	filesystem::path CP_path(".\\checkpoint");
	if (filesystem::exists(CP_path)) {
		filesystem::recursive_directory_iterator itr(CP_path);
		while (itr != filesystem::end(itr)) {
			const filesystem::directory_entry& entry = *itr;
			//cout << entry.path().string() << endl;
			string path_name = entry.path().string();
			if (path_name.find(".txt") != string::npos) {	// .txt 가 들어간 파일만 저장합니다. (디렉토리 이름만 있는 path 배제)
				readTargets.push_back(path_name);
			}
			itr++;
		}
	}
	else {
		cout << "[Directory Search Error] Unknown Directory." << endl;
	}

	// 2. 파일 읽기
	for (auto& fname : readTargets) {
		cout << "[Map Loading...] " << fname;
		//string fname = readTargets[0];
		ifstream txtfile(fname);

		string line;

		int line_cnt = 0;

		char b_pos = 0;
		int pos_count = 0;

		char b_scale = 0;
		int scale_count = 0;

		float tmp_pos[3] = { 0.f, 0.f, 0.f }; // 뽑은 좌표정보를 임시 저장할 공간, 3개 꽉차면 벡터에 넣어주고 비워두자.
		float tmp_scale[3] = { 0.f, 0.f, 0.f }; // 뽑은 크기정보를 임시 저장할 공간, 3개 꽉차면 벡터에 넣어주고 비워두자.
		if (txtfile.is_open()) {
			while (txtfile >> line) {
				if (line == "Position:") {
					b_pos = 1;
					pos_count = 0;
				}
				else if (line == "Size:") {
					b_scale = 1;
					scale_count = 0;
				}
				else {
					if (b_pos == 1) {
						tmp_pos[pos_count] = string2data(line);

						if (pos_count == 2) {
							tmp_pos[pos_count] = string2data(line);

							b_pos = 0;
							pos_count = 0;
						}
						else {
							pos_count += 1;
						}
					}
					else if (b_scale == 1) {
						tmp_scale[scale_count] = string2data(line);

						if (scale_count == 2) {
							tmp_scale[scale_count] = string2data(line);
							b_scale = 0;
							scale_count = 0;

							CheckPoint tmp_mapobj(tmp_pos[0], tmp_pos[1], tmp_pos[2], tmp_scale[0], tmp_scale[1], tmp_scale[2]);
							tmp_mapobj.setBB();
							CP.push_back(tmp_mapobj);
							memset(tmp_pos, 0, sizeof(tmp_pos));
							memset(tmp_scale, 0, sizeof(tmp_scale));
						}
						else {
							scale_count += 1;
						}
					}
				}
				line_cnt++;
			}
			cout << " ---- OK." << endl;
		}
		else {
			cout << "[Error] Unknown File." << endl;
		}
		txtfile.close();
	}
	cout << "\n";

	for (int i{}; i < CP.size(); ++i) {
		cout << i << " Pos --> x: " << CP[i].getPosX() << ", y: " << CP[i].getPosY() << ", z: " << CP[i].getPosZ() << endl;
		cout << i << " Scale --> x: " << CP[i].getScaleX() << ", y: " << CP[i].getScaleY() << ", z: " << CP[i].getScaleZ() << endl;
		cout << "=============================" << endl;
	}

	initNpc();


	//======================================================================
	//						  Threads Initialize
	//======================================================================
	vector <thread> worker_threads;
	for (int i = 0; i < 5; ++i)
		worker_threads.emplace_back(do_worker);			// 메인서버-npc서버 통신용 Worker스레드

	vector<thread> timer_threads;
	//timer_threads.emplace_back(timerFunc);				// npc 로직 타이머스레드
	timer_threads.emplace_back(MoveNPC);

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();


	//closesocket(g_sc_listensock);
	WSACleanup();
}
