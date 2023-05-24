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
//			���μ������� ����� �͵��� �״�� ���
//============================================================
#include "../../MainServer/Server/Protocol.h"
#include "../../MainServer/Server/Constant.h"
#include "../../MainServer/Server/MathFuncs.h"
//#include "../../MainServer/Server/MapObjects.h"
#include "../../MainServer/Server/CP_KEYS.h"
#include "CheckPoint.h"

using namespace std;
using namespace chrono;

system_clock::time_point g_s_start_time;	// ���� ���۽ð�  (����: ms)
milliseconds g_curr_servertime;
mutex servertime_lock;	// �����ð� lock

enum NPCState { NPC_IDLE, NPC_FLY, NPC_CHASE, NPC_ATTACK, NPC_DEATH };
enum Hit_target { g_none, g_body, g_profeller };

bool ConnectingServer = false;

class CheckPoint : public MapObject
{
private:
	vector<int> neighbors;

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

	float distanceTo(CheckPoint& other) {
		float dx = getPosX() - other.getPosX();
		float dy = getPosY() - other.getPosY();
		float dz = getPosZ() - other.getPosZ();

		return sqrt(dx * dx + dy * dy + dz * dz);
	}

	void addNeighbors(const std::vector<int>& indices) {
		neighbors.insert(neighbors.end(), indices.begin(), indices.end());
	}

	void addNeighbor(int index) {
		neighbors.push_back(index);
	}
	const vector<int>& getNeighbors() const {
		return neighbors;
	}


	XMFLOAT3 getPos() { return XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()); }
};
vector<CheckPoint> CP;

//======================================================================
struct Edge {
	int from;
	int to;
	float value;

	Edge(int from, int to, float value) : from(from), to(to), value(value) {}
};
vector<Edge> edges;

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
Coordinate basic_coordinate;	// �⺻(�ʱ�) ��ǥ��

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
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// ���� Look, Right, Up Vectors

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
	int m_currentNodeIndex;
	float m_Speed;
	float m_Distance[MAX_USER];

	bool m_SectionMoveDir;

public:
	bool m_DeathCheck = false;
	bool PrintRayCast = false;
	vector<CheckPoint> graph;
	vector<int>path;
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
	int GetNodeIndex() { return m_currentNodeIndex; }
	XMFLOAT3 GetPosition() { return pos; }
	Coordinate GetCurr_coordinate() { return m_coordinate; }
	float GetDistance(int id) { return m_Distance[id]; }
	float GetSpeed() { return m_Speed; }

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
	
public:
// Normal
	// Rotate
	XMFLOAT3 NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll);

	bool SetTrackingPrevStatebyDistance(float setDistance, int curState, int prevState);
	bool PlayerDetact();

	void SetFrustum();
	void SetIndexNode(int idx);
	// State
	void NPC_State_Manegement(int state); // ���� ����
	void Caculation_Distance(XMFLOAT3 vec, int id); // ���� �� �÷��̾� Ž��
	void MoveInSection();
	void MoveChangeIdle();
	void MoveToNode();
	void FlyOnNpc(XMFLOAT3 vec, int id);
	void PlayerChasing();
	void PlayerAttack();
	void NPC_Death_motion(); // HP 0
	void NPC_Damege_Calc(int id);
	void NPC_Check_HP();
	void SetTrackingIDbyDistance(float setDistance, int curState, int nextState);

	float getRandomOffset();
	float CalculateYawToTarget(const XMFLOAT3& targetPosition) const;
	float CalculatePitchToTarget(const XMFLOAT3& targetPosition) const;
	float CalculateRollToTarget(const XMFLOAT3& targetPosition) const;
};

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
bool NPC::SetTrackingPrevStatebyDistance(float setDistance, int curState, int prevState)
{
	int change_cnt = 0;

	for (int i{}; i < MAX_USER; ++i) {
		if (m_Distance[i] >= setDistance) {  // ���·� ��ȯ�ϱ� ���� ���� �� ���� ID ����
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
bool NPC::PlayerDetact()
{
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	XMVECTOR FrustumOrigin = XMLoadFloat3(&m_frustum.Origin);
	XMVECTOR FrustumOrientation = XMLoadFloat4(&m_frustum.Orientation);

	// Frustum�� ������ 8���� ���Ѵ�.
	XMFLOAT3 corners[8];
	m_frustum.GetCorners(corners);

	// Frustum�� Player�� bounding sphere���� �Ÿ��� ���Ѵ�.
	float distance = FLT_MAX;
	for (int i = 0; i < 8; i++)
	{
		float d = XMVectorGetX(XMVector3Length(PlayerPos - XMLoadFloat3(&corners[i])));
		if (d < distance)
		{
			distance = d;
		}
	}

	// �Ÿ��� bounding sphere�� ���������� ������ �浹�ߴٰ� �Ǵ��Ѵ�.
	if (distance < 50.0f)
	{
		return true;
	}

	return false;
}
void NPC::SetFrustum()
{
	// NPC�� ��ġ�� Look ���͸� �����´�.
	XMVECTOR position = XMLoadFloat3(&pos);
	XMVECTOR look = XMLoadFloat3(&m_coordinate.look);

	// Frustum�� ���̵� ������ ���� ������ �����Ѵ�.
	float width = 50.0f;
	float height = 50.0f;

	// Frustum�� �������� �����Ѵ�.
	XMVECTOR startPoint = position;

	// Frustum�� ������ �����Ѵ�.
	XMVECTOR endPoint = position + (look * 200.0f);

	// Frustum�� Up ���͸� �����Ѵ�.
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Frustum�� �����ϰ� �����Ѵ�.
	BoundingFrustum frustum;

	// Frustum�� Origin�� �����Ѵ�.
	XMStoreFloat3(&frustum.Origin, startPoint);

	// Frustum�� Orientation�� �����Ѵ�.
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
void NPC::SetIndexNode(int idx) 
{
	m_currentNodeIndex = idx;
	CheckPoint cp = graph[idx]; // CP���� �ش� �ε����� CheckPoint ��������
	XMFLOAT3 cpPos = cp.getPos(); // CheckPoint�� ��ġ ���� ��������

	// NPC�� �������� CP�� ��ġ �ֺ����� ����
	// ���⿡�� �ʿ信 ���� ���� ������ ��ġ ������ �� �� �ֽ��ϴ�.
	XMFLOAT3 setPos = {
		cpPos.x + getRandomOffset(), // X ��ǥ�� ������ �������� ���� ��ġ ����
		cpPos.y + getRandomOffset(), // Y ��ǥ�� ������ �������� ���� ��ġ ����
		cpPos.z + getRandomOffset()  // Z ��ǥ�� ������ �������� ���� ��ġ ����
	};

	SetPosition(setPos);
}

void NPC::NPC_State_Manegement(int state)
{
	switch (m_state)
	{
	case NPC_IDLE: // �ź� Ȥ�� Ư�� ��� �ϴ� ��.
	{
		//MoveInSection();
		MoveToNode();
		SetTrackingIDbyDistance(500.0f, NPC_IDLE, NPC_FLY);
	}
	break;
	case NPC_FLY:
	{
		// Fly ���� or Fly -> chase or Fly -> Idle
		FlyOnNpc(m_User_Pos[m_chaseID], m_chaseID); // ��� �÷��̾���� y ��ǥ�� ����ϰ� ����.

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
		SetTrackingIDbyDistance(300.0f, NPC_CHASE, NPC_ATTACK); // ID Ž��
		if (PlayerDetact()) { // Id Ž���� �̹� ���� ����� ������� �����ϱ⿡ ���� Ž���� ���� ����.
			m_state = NPC_ATTACK; // ���� ����
		}
		else {
			SetTrackingPrevStatebyDistance(300.0f, NPC_CHASE, NPC_FLY);
		}
	}
	break;
	case NPC_ATTACK:
	{
		// bullet ����
		SetTrackingIDbyDistance(200.0f, NPC_ATTACK, NPC_ATTACK); // ID Ž��
		if (!PlayerDetact()) { // Id Ž���� �̹� ���� ����� ������� �����ϱ⿡ ���� Ž���� ���� ����.
			m_state = NPC_CHASE; // ���� ����
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
void NPC::Caculation_Distance(XMFLOAT3 vec, int id) // �������� ���� �θ� ��.
{
	m_Distance[id] = sqrtf(pow((vec.x - pos.x), 2) + pow((vec.y - pos.y), 2) + pow((vec.z - pos.z), 2));
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
		// ����� ���� �Ÿ��� ���� ���� ���� �ƴ� ���	
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
		// ���� ����
		m_IdleCity = id;
		m_IdleSection = s_id;
	}
}
void NPC::MoveToNode()
{
	XMFLOAT3 targetPosition = graph[m_currentNodeIndex].getPos();

	// NPC�� ���� ��ġ�� ��ǥ ��ġ ���� ���͸� ����մϴ�.
	XMFLOAT3 direction = {
		targetPosition.x - pos.x,
		targetPosition.y - pos.y,
		targetPosition.z - pos.z
	};
	XMFLOAT3 temp = NPCNormalize(direction);

	// �̵� �ӵ��� �����մϴ�.
	float moveSpeed = m_Speed * 0.0016f; // deltaTime�� ������ ���� �ð� �����Դϴ�.

	// NPC�� ��ġ�� ��ǥ ��ġ�� �����Ͽ� �̵���ŵ�ϴ�.
	pos.x += temp.x * moveSpeed;
	pos.y += temp.y * moveSpeed;
	pos.z += temp.z * moveSpeed;

	float distanceToTarget = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

	// �Ÿ��� 50 ������ ��� ���� ���� �����մϴ�.
	if (distanceToTarget <= 50.0f)
	{
		// ���� ���� �̵��ϱ� ���� currentNodeIndex�� ������Ʈ�մϴ�.
		if (m_currentNodeIndex >= 2 && m_currentNodeIndex <= 5) {
			m_currentNodeIndex = (m_currentNodeIndex + 1) % 6 + 2; // 2->3->4->5->2
		}
		else if (m_currentNodeIndex >= 6 && m_currentNodeIndex <= 9) {
			m_currentNodeIndex = (m_currentNodeIndex + 1) % 4 + 6; // 6->7->8->9->6
		}
		else if (m_currentNodeIndex >= 10 && m_currentNodeIndex <= 13) {
			m_currentNodeIndex = (m_currentNodeIndex + 1) % 4 + 10; // 10->11->12->13->10
		}
		else if (m_currentNodeIndex >= 14 && m_currentNodeIndex <= 17) {
			m_currentNodeIndex = (m_currentNodeIndex + 1) % 4 + 14; // 14->15->16->17->14
		}
	}

	// NPC�� ȸ���� �����մϴ�.
	// ���÷� yaw, pitch, roll ���� ����Ͽ� ȸ���� �����մϴ�.
	// �� �κ��� NPC�� �̵� ������ �°� �����ؾ� �մϴ�.
	yaw = CalculateYawToTarget(targetPosition);
	pitch = CalculatePitchToTarget(targetPosition);
	roll = CalculateRollToTarget(targetPosition);

	// yaw, pitch, roll ���� ����Ͽ� Look, Right, Up ���͸� ����մϴ�.
	XMVECTOR forwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Yaw ȸ��
	XMMATRIX yawRotationMatrix = XMMatrixRotationAxis(upVector, yaw);
	forwardVector = XMVector3TransformCoord(forwardVector, yawRotationMatrix);

	// Pitch ȸ��
	XMMATRIX pitchRotationMatrix = XMMatrixRotationAxis(XMLoadFloat3(&m_lookvec), pitch);
	forwardVector = XMVector3TransformCoord(forwardVector, pitchRotationMatrix);
	upVector = XMVector3TransformCoord(upVector, pitchRotationMatrix);

	// Roll ȸ��
	XMMATRIX rollRotationMatrix = XMMatrixRotationAxis(forwardVector, roll);
	//m_rightvec = XMVector3TransformCoord(XMLoadFloat3(&m_rightvec), rollRotationMatrix);

	XMStoreFloat3(&m_rightvec, XMVector3TransformCoord(XMLoadFloat3(&m_rightvec), rollRotationMatrix));
	upVector = XMVector3TransformCoord(upVector, rollRotationMatrix);

	// XMVECTOR�� XMFLOAT3�� ��ȯ�Ͽ� �����մϴ�.
	XMStoreFloat3(&m_lookvec, forwardVector);
	XMStoreFloat3(&m_upvec, upVector);

}
void NPC::FlyOnNpc(XMFLOAT3 vec, int id) // ������� �÷��̾�� ���� ���߱�
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

	// ��ġ ��ȯ
	pos.x += m_Speed * m_coordinate.look.x;
	pos.y += m_Speed * m_coordinate.look.y;
	pos.z += m_Speed * m_coordinate.look.z;
	//NPCtoBuilding_collide();
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
void NPC::NPC_Death_motion()
{
	pos.y -= 6.0f;

	// ���ۺ��� ���� �߶�
	yaw += 3.0f;

	Coordinate base_coordinate;
	m_coordinate.right = NPCcalcRotate(base_coordinate.right, pitch, yaw, roll);
	m_coordinate.up = NPCcalcRotate(base_coordinate.up, pitch, yaw, roll);
	m_coordinate.look = NPCcalcRotate(base_coordinate.look, pitch, yaw, roll);
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
void NPC::SetTrackingIDbyDistance(float setDistance, int curState, int nextState)
{
	bool State_check = false;
	float MinDis = 50000;
	for (int i{}; i < MAX_USER; ++i) {
		if (m_Distance[i] < setDistance) {  // ���·� ��ȯ�ϱ� ���� ���� �� ���� ID ����
			State_check = true;
			if (m_Distance[i] < MinDis) {
				m_chaseID = i;
				MinDis = m_Distance[i];
			}
			m_state = nextState; // �ڽ��� ���¸� ���� ���·� ����
		}
		if (i == MAX_USER - 1 && !State_check) { // �ڽ��� ���� ����
			m_state = curState;
		}
	}
}
float NPC::getRandomOffset()
{
	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float> offset(-10.0f, 10.0f); // ���ϴ� ������ ���� ����

	return offset(dre);
}
float NPC::CalculateYawToTarget(const XMFLOAT3& targetPosition) const {
	// NPC�� ���� ��ġ�� ��ǥ ��ġ ���� ���͸� ����մϴ�.
	XMFLOAT3 direction = {
		targetPosition.x - pos.x,
		targetPosition.y - pos.y,
		targetPosition.z - pos.z
	};

	// NPC�� Yaw ���� ����մϴ�.
	// atan2 �Լ��� ����Ͽ� ���� ������ X, Z ������ �̿��մϴ�.
	float yaw = atan2(direction.x, direction.z);

	// ���� ���� ������ ��ȯ�մϴ�.
	yaw = XMConvertToDegrees(yaw);

	// ������ 0~360 ������ �����մϴ�.
	if (yaw < 0)
		yaw += 360.0f;

	return yaw;
}
float NPC::CalculatePitchToTarget(const XMFLOAT3& targetPosition) const {
	// NPC�� ���� ��ġ�� ��ǥ ��ġ ���� ���͸� ����մϴ�.
	XMFLOAT3 direction = {
		targetPosition.x - pos.x,
		targetPosition.y - pos.y,
		targetPosition.z - pos.z
	};

	// NPC�� Pitch ���� ����մϴ�.
	// atan2 �Լ��� ����Ͽ� ���� ������ Y, XZ ��鿡���� ���̸� �̿��մϴ�.
	float pitch = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z));

	// ���� ���� ������ ��ȯ�մϴ�.
	pitch = XMConvertToDegrees(pitch);

	return pitch;
}
float NPC::CalculateRollToTarget(const XMFLOAT3& targetPosition) const {
	// NPC�� Roll ���� �Ϲ������� ����ϴ� ������ �����ؾ� �մϴ�.
	// Roll�� �Ϲ������� NPC�� �̵� ��ο� ������ �ִ� ��쿡 ���˴ϴ�.
	// NPC�� �̵� ��ο� ��ǥ ��ġ ���� ���迡 ���� ������ Roll ���� ����ؾ� �մϴ�.
	// ��ü���� NPC�� ���� �� �̵� ��Ŀ� ���� Roll ���� �����ϰ� ����ؾ� �մϴ�.
	// �� ���ÿ����� NPC�� Roll �� ����� �����ϰ� 0�� ��ȯ�մϴ�.
	return 0.0f;
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

HANDLE h_iocp;											// IOCP �ڵ�
int a_lgcsvr_num;										// Active������ ���μ���
array<SERVER, MAX_LOGIC_SERVER> g_logicservers;			// �������� ����

void SERVER::send_npc_init_packet(int npc_id) {
	NPC_FULL_INFO_PACKET npc_init_packet;
	npc_init_packet.size = sizeof(NPC_MOVE_PACKET);
	npc_init_packet.type = NPC_ROTATE;
	npc_init_packet.n_id = npc_id;
	npc_init_packet.hp = npcsInfo[npc_id].hp;
	npc_init_packet.x = npcsInfo[npc_id].pos.x;
	npc_init_packet.y = npcsInfo[npc_id].pos.y;
	npc_init_packet.z = npcsInfo[npc_id].pos.z;
	npc_init_packet.right_x = npcsInfo[npc_id].m_rightvec.x;
	npc_init_packet.right_y = npcsInfo[npc_id].m_rightvec.y;
	npc_init_packet.right_z = npcsInfo[npc_id].m_rightvec.z;
	npc_init_packet.up_x = npcsInfo[npc_id].m_upvec.x;
	npc_init_packet.up_y = npcsInfo[npc_id].m_upvec.y;
	npc_init_packet.up_z = npcsInfo[npc_id].m_upvec.z;
	npc_init_packet.look_x = npcsInfo[npc_id].m_lookvec.x;
	npc_init_packet.look_y = npcsInfo[npc_id].m_lookvec.y;
	npc_init_packet.look_z = npcsInfo[npc_id].m_lookvec.z;
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
	npc_rotate_packet.right_x = npcsInfo[npc_id].m_rightvec.x;
	npc_rotate_packet.right_y = npcsInfo[npc_id].m_rightvec.y;
	npc_rotate_packet.right_z = npcsInfo[npc_id].m_rightvec.z;
	npc_rotate_packet.up_x = npcsInfo[npc_id].m_upvec.x;
	npc_rotate_packet.up_y = npcsInfo[npc_id].m_upvec.y;
	npc_rotate_packet.up_z = npcsInfo[npc_id].m_upvec.z;
	npc_rotate_packet.look_x = npcsInfo[npc_id].m_lookvec.x;
	npc_rotate_packet.look_y = npcsInfo[npc_id].m_lookvec.y;
	npc_rotate_packet.look_z = npcsInfo[npc_id].m_lookvec.z;
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
	npc_rt_packet.right_x = npcsInfo[npc_id].m_rightvec.x;
	npc_rt_packet.right_y = npcsInfo[npc_id].m_rightvec.y;
	npc_rt_packet.right_z = npcsInfo[npc_id].m_rightvec.z;
	npc_rt_packet.up_x = npcsInfo[npc_id].m_upvec.x;
	npc_rt_packet.up_y = npcsInfo[npc_id].m_upvec.y;
	npc_rt_packet.up_z = npcsInfo[npc_id].m_upvec.z;
	npc_rt_packet.look_x = npcsInfo[npc_id].m_lookvec.x;
	npc_rt_packet.look_y = npcsInfo[npc_id].m_lookvec.y;
	npc_rt_packet.look_z = npcsInfo[npc_id].m_lookvec.z;
	cout << npcsInfo[npc_id].m_lookvec.x << ", " << npcsInfo[npc_id].m_lookvec.y << ", " << npcsInfo[npc_id].m_lookvec.z << endl;
	g_logicservers[a_lgcsvr_num].do_send(&npc_rt_packet);
}

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

		/*cout << "[Add New Player] Player[ID:" << client_id << ", Name:" << playersInfo[client_id].name << "]�� ������ �޾ҽ��ϴ�." << endl;
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

		/*cout << "[Move Player] Player[ID:" << client_id << "]�� �̵��Ͽ����ϴ�." << endl;
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

		/*cout << "[Rotate Player] Player[ID:" << client_id << "]�� ȸ���Ͽ����ϴ�." << endl;
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

		//cout << "[Remove Player] Player[ID:" << client_id << "]�� ������ �����Ͽ����ϴ�.\n" << endl;

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
				// ������ȣ�� �ٲ㰡�鼭 �񵿱�Connect�� ��õ��մϴ�.
				if (a_lgcsvr_num == 0)		a_lgcsvr_num = 1;
				else if (a_lgcsvr_num == 1)	a_lgcsvr_num = 0;
				int new_portnum = a_lgcsvr_num + PORTNUM_LGCNPC_0;
				cout << "[ConnectEX Failed] ";
				cout << "Logic Server[" << a_lgcsvr_num << "] (PORTNUM:" << new_portnum << ")�� �ٽ� �����մϴ�. \n" << endl;

				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN logic_server_addr;
				ZeroMemory(&logic_server_addr, sizeof(logic_server_addr));
				logic_server_addr.sin_family = AF_INET;
				g_logicservers[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
				// 1. ���μ����� NPC������ �����鿡�� ������ ��
				//inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

				// 2. ���μ����� NPC������ �ٸ� PC���� ������ ��
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
						//cout << "Server Connect ��õ� ��...\n" << endl;
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
				std::cout << "���������� Logic Server[" << server_id << "]�� ����Ǿ����ϴ�.\n" << endl;
				g_logicservers[a_lgcsvr_num].remain_size = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, NULL, 0);
				delete ex_over;
				g_logicservers[a_lgcsvr_num].do_recv();
				ConnectingServer = true;
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

	for (int i{}; i < MAX_NPCS; i++) {
		int npc_id = i;
		npcsInfo[i].SetID(npc_id);
		random_device rd;
		default_random_engine dre(rd());

		uniform_int_distribution<int>idx(2, 17);
		npcsInfo[i].graph = CP;
		npcsInfo[i].SetIndexNode(idx(dre));

		npcsInfo[i].SetRotate(0.0f, 0.0f, 0.0f);
		
		uniform_int_distribution<int>Ci_num(0, 3);
		uniform_int_distribution<int>Sec_num(0, 3);

		int city_num = Ci_num(dre);
		int section_num = Sec_num(dre);

		npcsInfo[i].SetIdleCity(city_num);
		npcsInfo[i].SetIdleSection(section_num);

		uniform_real_distribution<float>SpdSet(3.5f, 5.2f);
		float speed = SpdSet(dre);
		npcsInfo[i].SetSpeed(speed);
		npcsInfo[i].SetChaseID(-1);
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
		if (ConnectingServer) {


			for (int i = 0; i < MAX_NPCS; ++i) {
				// Ŭ���̾�Ʈ��� NPC ������ �Ÿ� ���

				if (npcsInfo[i].GetState() == NPC_DEATH && npcsInfo[i].GetPosition().y < 0) {
					NPC_REMOVE_PACKET npc_remove_packet;

					npc_remove_packet.size = sizeof(NPC_REMOVE_PACKET);
					npc_remove_packet.type = SC_REMOVE_OBJECT;
					npc_remove_packet.n_id = npcsInfo[i].GetID();

					npcsInfo[i].m_DeathCheck = true;

					//for (auto& send_target : playersInfo) {
					//	if (send_target.curr_stage != 1) continue;// ��������2 ��������ȭ �������� ����ϴ� �ӽ��ڵ�.
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
					//cout << i << "��° Status - " << npcs[i].GetState() << endl;				
					npcsInfo[i].NPC_State_Manegement(npcsInfo[i].GetState());
					// NPC�� �����Ϸ��� ���̵� �ִ������� Ȯ��, ������ ���� ��� �÷��̾� ��ǥ�� �ӽ� ����
					if (npcsInfo[i].GetChaseID() != -1) {
						npcsInfo[i].SetUser_Pos(playersInfo[npcsInfo[i].GetChaseID()].pos, npcsInfo[i].GetChaseID());
					}

					// npc pos Ȯ��
					cout << "=============" << endl;
					cout << i << "��° NPC�� ���� ID: " << npcsInfo[i].GetIdleCity() << ", NPC�� ���� ID: " << npcsInfo[i].GetIdleSection() << endl;
					cout << i << "��° NPC�� NodeIndex: " << npcsInfo[i].GetNodeIndex() << endl;
					cout << i << "��° NPC�� Pos: " << npcsInfo[i].GetPosition().x << ',' << npcsInfo[i].GetPosition().y << ',' << npcsInfo[i].GetPosition().z << endl;
					cout << i << "��° NPC�� ����: " << npcsInfo[i].GetState() << endl;

					/*if (npcs[i].PrintRayCast) {
						cout << i << "��° NPC�� �� �Ѿ˿� ����" << npcs[i].GetChaseID() << "�� ID�� ���� �÷��̾ �ǰݵǾ����ϴ�." << endl;
					}*/

					// ���¸��� �ٸ� �������� �ϴ� �Ŵ�����Ʈ

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
}


//======================================================================
int main(int argc, char* argv[])
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	//					���������� �񵿱� Connect ��û
	//======================================================================
	int lgvsvr_port = PORTNUM_LGCNPC_0 + a_lgcsvr_num;

	cout << "���� ����(Server[" << a_lgcsvr_num << "] (PORT: " << lgvsvr_port << ")�� �񵿱�Connect�� ��û�մϴ�." << endl;

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
	g_logicservers[a_lgcsvr_num].sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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

	// 1. ���μ����� NPC������ �����鿡�� ������ ��
	//inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

	// 2. ���μ����� NPC������ �ٸ� PC���� ������ ��
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
			cout << "Server Connect �õ� ��...\n" << endl;
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
			if (path_name.find(".txt") != string::npos) {	// .txt �� �� ���ϸ� �����մϴ�. (���丮 �̸��� �ִ� path ����)
				readTargets.push_back(path_name);
			}
			itr++;
		}
	}
	else {
		cout << "[Directory Search Error] Unknown Directory." << endl;
	}

	// 2. ���� �б�
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

		float tmp_pos[3] = { 0.f, 0.f, 0.f }; // ���� ��ǥ������ �ӽ� ������ ����, 3�� ������ ���Ϳ� �־��ְ� �������.
		float tmp_scale[3] = { 0.f, 0.f, 0.f }; // ���� ũ�������� �ӽ� ������ ����, 3�� ������ ���Ϳ� �־��ְ� �������.
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

	vector<vector<int>> neighborIndices = {
		{8, 14},      // 0��° �ε����� �̿� ���
		{3, 11},      // 1��° �ε����� �̿� ���
		{3, 5},       // 2��° �ε����� �̿� ���
		{1, 2, 4},    // 3��° �ε����� �̿� ���
		{3, 5},       // 4��° �ε����� �̿� ���
		{2, 4, 6},    // 5��° �ε����� �̿� ���
		{5, 7, 9},    // 6��° �ε����� �̿� ���
		{6, 8},       // 7��° �ε����� �̿� ���
		{0, 7, 9},    // 8��° �ε����� �̿� ���
		{6, 8},       // 9��° �ε����� �̿� ���
		{11, 13},     // 10��° �ε����� �̿� ���
		{1, 10, 12},  // 11��° �ε����� �̿� ���
		{11, 13},     // 12��° �ε����� �̿� ���
		{10, 12, 16}, // 13��° �ε����� �̿� ���
		{0, 15, 17},  // 14��° �ε����� �̿� ���
		{14, 16},     // 15��° �ε����� �̿� ���
		{13, 15, 17}, // 16��° �ε����� �̿� ���
		{14, 16},     // 17��° �ε����� �̿� ���
	};
	int numCP = CP.size();

	for (int i = 0; i < numCP; ++i) {
		vector<int> neighbors = neighborIndices[i];
		CP[i].addNeighbors(neighbors);
	}

	for (int i = 0; i < numCP; ++i) {
		CheckPoint tempCP = CP[i];
		const std::vector<int>& neighbors = tempCP.getNeighbors();

		for (int neighborIndex : neighbors) {
			float distance = tempCP.distanceTo(CP[neighborIndex]);
			edges.emplace_back(i, neighborIndex, distance);
		}
	}

	/*for (const auto& edge : edges) {
		std::cout << "From: " << edge.from << ", To: " << edge.to << ", Value: " << edge.value << std::endl;
	}*/

	initNpc();


	//======================================================================
	//						  Threads Initialize
	//======================================================================
	vector <thread> worker_threads;
	for (int i = 0; i < 5; ++i)
		worker_threads.emplace_back(do_worker);			// ���μ���-npc���� ��ſ� Worker������

	vector<thread> timer_threads;
	//timer_threads.emplace_back(timerFunc);				// npc ���� Ÿ�̸ӽ�����
	timer_threads.emplace_back(MoveNPC);

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();


	//closesocket(g_sc_listensock);
	WSACleanup();
}
