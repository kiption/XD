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
#include <limits>
#include <algorithm>
#include <unordered_set>
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
#include "../../MainServer/Server/CP_KEYS.h"
#include "../../MainServer/Server/RayCast.h"
#include "CheckPoint.h"

using namespace std;
using namespace chrono;

system_clock::time_point g_s_start_time;	// 서버 시작시간  (단위: ms)
milliseconds g_curr_servertime;
mutex servertime_lock;	// 서버시간 lock

enum NPCState { NPC_IDLE, NPC_CHASE, NPC_ST_ATK, NPC_DEATH };
enum Hit_target { g_none, g_body, g_profeller };
enum NPCType { NPC_HELICOPTER, NPC_ARMY };

bool ConnectingServer = false;
constexpr int HelicopterNum = 5;
constexpr int ArmyNum = 0;

struct Node
{
	int index; // 노드의 인덱스
	float heuristic; // 목표지점까지의 휴리스틱 값
	float cost; // 출발지점부터 현재 노드까지의 비용
	Node* parent; // 부모 노드

	Node(int idx, float h, float c, Node* p) : index(idx), heuristic(h), cost(c), parent(p) {}

	// 비용과 휴리스틱 값을 합친 우선순위 큐를 위한 연산자 오버로딩
	bool operator<(const Node& other) const {
		return (cost + heuristic) > (other.cost + other.heuristic);
	}
};

class NodeMesh {
private:
	int index_num;
	float Center_x;
	float Center_z;
	float Small_x;
	float Small_z;
	float Large_x;
	float Large_z;
	bool Move_x, Move_z;
	bool User_check;
public:
	NodeMesh() {
		index_num = -1;
		Center_x = 0.0f;
		Center_z = 0.0f;
		Small_x = 0.0f;
		Small_z = 0.0f;
		Large_x = 0.0f;
		Large_z = 0.0f;
		Move_x = false;
		Move_z = false;
		User_check = false;
	}
	~NodeMesh() {

	}

public:
	vector<int>neighbor_mesh;
	void SetInit(int idx, float cx, float cz, float bx, float bz)
	{
		index_num = idx;
		Center_x = cx;
		Center_z = cz;

		Small_x = cx - (bx / 2);
		Small_z = cz - (bz / 2);
		Large_x = cx + (bx / 2);
		Large_z = cz + (bz / 2);
	}
	void SetMoveingSpace(bool x, bool z)
	{
		Move_x = x;
		Move_z = z;
	}

	bool GetMoveingSpaceX() { return Move_x; }
	bool GetMoveingSpaceZ() { return Move_z; }
	int GetIndex() { return index_num; }
	float GetCenterX() { return Center_x; }
	float GetCenterZ() { return Center_z; }
	float GetLargeX() { return Large_x; }
	float GetLargeZ() { return Large_z; }
	float GetSmallX() { return Small_x; }
	float GetSmallZ() { return Small_z; }

public:
	bool otherIndexIntersection(NodeMesh other)
	{
		if (Small_x > other.Large_x || Large_x < other.Small_x || Small_z > other.Large_z || Large_z < other.Small_z) {
			// 겹치지 않음
			return false;
		}
		else {
			// 겹침
			return true;
		}
	}
};
vector<NodeMesh>MeshInfo;

//======================================================================
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

float HeuristicEstimate(int startNode, int targetNode)
{
	// 각 노드의 위치 정보를 활용하여 맨하탄 거리 계산
	float startX = MeshInfo[startNode].GetCenterX();
	float startZ = MeshInfo[startNode].GetCenterZ();
	float targetX = MeshInfo[targetNode].GetCenterX();
	float targetZ = MeshInfo[targetNode].GetCenterZ();

	float heuristic = abs(targetX - startX) + abs(targetZ - startZ);
	return heuristic;
}

float DistanceBetween(int nodeIndex1, int nodeIndex2)
{
	float x1 = MeshInfo[nodeIndex1].GetCenterX();
	float z1 = MeshInfo[nodeIndex1].GetCenterZ();
	float x2 = MeshInfo[nodeIndex2].GetCenterX();
	float z2 = MeshInfo[nodeIndex2].GetCenterZ();

	float distance = sqrt((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1));
	return distance;
}

//======================================================================
class OBJECT {
public:
	mutex obj_lock;

	int id;
	char name[NAME_SIZE];
	short type;

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
	XMFLOAT3 m_User_Pos[MAX_USER];
	XMFLOAT3 m_AttackVec;
	short m_Hit;
	short m_state;
	int m_attack;
	int m_defence;
	int m_ProfellerHP;
	int m_BodyHP;
	int m_chaseID;
	int m_currentNodeIndex;
	int m_targetNodeIndex;
	float m_Speed;
	float m_Distance[MAX_USER];
	float m_destinationRange;

public:
	bool m_DeathCheck = false;
	bool PrintRayCast = false;

	vector<int>path;
	BoundingFrustum m_frustum;
	XMFLOAT3 m_VectorMAX = { -9999.f, -9999.f, -9999.f };
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
	int GetState() { return m_state; }
	int GetChaseID() { return m_chaseID; }
	int GetNodeIndex() { return m_currentNodeIndex; }
	int GetTargetNodeIndex() { return m_targetNodeIndex; }
	float GetDistance(int id) { return m_Distance[id]; }
	float GetSpeed() { return m_Speed; }
	XMFLOAT3 GetPosition() { return pos; }
	XMFLOAT3 GetAttackVec() { return m_AttackVec; }

public:
	// Set
	void SetHp(int thp) { hp = thp; }
	void SetID(int tid) { id = tid; }
	void SetChaseID(int cid) { m_chaseID = cid; }
	void SetNodeIndex(int idx) { m_currentNodeIndex = idx; }
	void SetTargetNodeIndex(int idx) { m_targetNodeIndex = idx; }
	void SetSpeed(float spd) { m_Speed = spd; }
	void SetDestinationRange(float range) { m_destinationRange = range; }
	void SetRotate(float y, float p, float r) { yaw = y, pitch = p, roll = r; }
	void SetPosition(XMFLOAT3 tpos) { pos = tpos; }
	void SetUser_Pos(XMFLOAT3 pos, int cid) { m_User_Pos[cid] = pos; }
	void SetAttackVec(XMFLOAT3 vec) { m_AttackVec = vec; }

public:
	// Base
		// Rotate
	float CalculateYawToTarget(const XMFLOAT3& targetPosition) const;						// yaw 각도에 따른 회전
	float CalculatePitchToTarget(const XMFLOAT3& targetPosition) const;						// Pitch 각도에 따른 회전
	float CalculateRollToTarget(const XMFLOAT3& targetPosition) const;						// Roll 각도에 따른 회전
	XMFLOAT3 NPCcalcRightRotate();						// 전체 각에 따른 right 설정
	XMFLOAT3 NPCcalcUpRotate();							// 전체 각에 따른 up 설정
	XMFLOAT3 NPCcalcLookRotate();							// 전체 각에 따른 look 설정

	// Base
		// Random
	float getRandomOffset(float min, float max);											// random 내장 함수

	// State
		// base
	bool CheckChaseState();																	// idle-chase 상태 변환 확인
	void NPC_State_Manegement(int state);													// 상태 관리
	void Caculation_Distance(XMFLOAT3 vec, int id);											// 범위 내 플레이어 탐색

	// 1. Helicopter
	// State
		// Idle
	void		H_MoveToNode();																		// 지정된 노드를 찾아가며 이동 - Idle
	void		H_UpdateCurrentNodeIndex();															// 노드 변환 시 다음 노드 지정
	void		H_MoveChangeIdle();																	// Idle 상태로 전환하면서 세팅

	// State
		// Chase
	bool		H_IsUserOnSafeZone(int user_City);													// User가 안전지대에 존재하는 지 확인
	int			H_GetUserCity();																	// User가 속한 도시 확인
	int			H_FindUserNode(int user_City);														// User가 속한 도시의 노드 확인
	void		H_IsPathMove();																		// Path가 있다면 해당 Path대로 이동
	void		H_PlayerChasing();																	// Chase 상태에서 동작하는 함수
	vector<int> H_AStarSearch(int startNode, int targetNode, int user_City);						// 길찾기 알고리즘

	// State
		// Attack
	bool		H_PlayerDetact();																	// 뷰 프러스텀 내의 플레이어 탐색
	void		H_SetFrustum();																		// 프러스텀 설정
	void		H_PlayerAttack();																	// Attack 상태에서 동작하는 함수

	// State
		// Death
	void		H_NPC_Damege_Calc(int id);															// 플레이어 ID 값 받아서 데미지 계산
	void		H_NPC_Check_HP();																	// HP 계산
	void		H_NPC_Death_motion();																// HP 0

	// 2. Army
	// State
		// Idle
	void		A_MoveToNode();																		// 지정된 노드를 찾아가며 이동 - Idle
	void		A_UpdateCurrentNodeIndex();															// 노드 변환 시 다음 노드 지정
	void		A_MoveChangeIdle();																	// Idle 상태로 전환하면서 세팅

	// State
		// Chase
	bool		A_IsUserOnSafeZone(int user_City);													// User가 안전지대에 존재하는 지 확인
	int			A_GetUserCity();																	// User가 속한 도시 확인
	int			A_FindUserNode(int user_City);														// User가 속한 도시의 노드 확인
	void		A_IsPathMove();																		// Path가 있다면 해당 Path대로 이동
	void		A_PlayerChasing();																	// Chase 상태에서 동작하는 함수
	vector<int> A_AStarSearch(int startNode, int targetNode, int user_City);						// 길찾기 알고리즘

	// State
		// Attack
	bool		A_PlayerDetact();																	// 뷰 프러스텀 내의 플레이어 탐색
	void		A_SetFrustum();																		// 프러스텀 설정
	void		A_PlayerAttack();																	// Attack 상태에서 동작하는 함수

	// State
		// Death
	void		A_NPC_Damege_Calc(int id);															// 플레이어 ID 값 받아서 데미지 계산
	void		A_NPC_Check_HP();																	// HP 계산
	void		A_NPC_Death_motion();																// HP 0

};

float NPC::CalculateYawToTarget(const XMFLOAT3& targetPosition) const
{
	// NPC의 현재 위치와 목표 위치 간의 벡터를 계산합니다.
	XMFLOAT3 direction = {
		targetPosition.x - pos.x,
		targetPosition.y - pos.y,
		targetPosition.z - pos.z
	};

	// NPC의 Yaw 값을 계산합니다.
	// atan2 함수를 사용하여 방향 벡터의 X, Z 성분을 이용합니다.
	float yaw = atan2(direction.x, direction.z);

	// 라디안 값을 각도로 변환합니다.
	yaw = XMConvertToDegrees(yaw);

	// 각도를 0~360 범위로 조정합니다.
	if (yaw < 0)
		yaw += 360.0f;

	return yaw;
}
float NPC::CalculatePitchToTarget(const XMFLOAT3& targetPosition) const
{
	// NPC의 현재 위치와 목표 위치 간의 벡터를 계산합니다.
	XMFLOAT3 direction = {
		targetPosition.x - pos.x,
		targetPosition.y - pos.y,
		targetPosition.z - pos.z
	};

	// NPC의 Pitch 값을 계산합니다.
	// atan2 함수를 사용하여 방향 벡터의 Y, XZ 평면에서의 길이를 이용합니다.
	float pitch = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z));

	// 라디안 값을 각도로 변환합니다.
	pitch = XMConvertToDegrees(pitch);

	return pitch;
}
float NPC::CalculateRollToTarget(const XMFLOAT3& targetPosition) const
{
	// NPC의 Roll 값을 일반적으로 계산하는 로직을 구현해야 합니다.
	// Roll은 일반적으로 NPC의 이동 경로와 관련이 있는 경우에 사용됩니다.
	// NPC의 이동 경로와 목표 위치 간의 관계에 따라 적절한 Roll 값을 계산해야 합니다.
	// 구체적인 NPC의 동작 및 이동 방식에 따라 Roll 값을 정의하고 계산해야 합니다.
	// 이 예시에서는 NPC의 Roll 값 계산을 생략하고 0을 반환합니다.
	return 0.0f;
}
XMFLOAT3 NPC::NPCcalcRightRotate()
{
	float curr_pitch = XMConvertToRadians(pitch);
	float curr_yaw = XMConvertToRadians(yaw);
	float curr_roll = XMConvertToRadians(roll);

	XMFLOAT3 vec = { 1.0f, 0.0f, 0.0f };

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
XMFLOAT3 NPC::NPCcalcUpRotate()
{
	float curr_pitch = XMConvertToRadians(pitch);
	float curr_yaw = XMConvertToRadians(yaw);
	float curr_roll = XMConvertToRadians(roll);

	XMFLOAT3 vec = { 0.0f, 1.0f, 0.0f };

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
XMFLOAT3 NPC::NPCcalcLookRotate()
{
	float curr_pitch = XMConvertToRadians(pitch);
	float curr_yaw = XMConvertToRadians(yaw);
	float curr_roll = XMConvertToRadians(roll);

	XMFLOAT3 vec = { 0.0f, 0.0f, 1.0f };

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

float NPC::getRandomOffset(float min, float max)
{
	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float> offset(min, max); // 원하는 오프셋 범위 설정

	return offset(dre);
}

bool NPC::CheckChaseState()
{
	if (m_chaseID != -1) {
		int userCity = H_GetUserCity();
		int npcCity = m_currentNodeIndex / 4;
		if (npcCity == userCity)
		{
			return true;
		}
	}
	if (m_state == NPC_IDLE) return false;

	m_chaseID = -1;
	for (int i = 0; i < MAX_USER; ++i) {
		m_Distance[i] = 20000;
	}
	return false;
}
void NPC::NPC_State_Manegement(int state)
{
	switch (type)
	{
	case NPC_HELICOPTER:
	{
		switch (m_state)
		{
		case NPC_IDLE:
			H_MoveToNode();
			if (CheckChaseState()) {
				m_state = NPC_CHASE;
			}
			break;
		case NPC_CHASE:
			if (!CheckChaseState()) {
				m_state = NPC_IDLE;
				break;
			}
			H_PlayerChasing();
			if (H_PlayerDetact())
				m_state = NPC_ATTACK;
			break;
		case NPC_ATTACK:
			if (!H_PlayerDetact()) {
				m_state = NPC_CHASE;
				PrintRayCast = false;
			}
			else {
				H_PlayerAttack();
			}
			break;
		case NPC_DEATH:
			if (pos.y > 0.0f)
				H_NPC_Death_motion();
			break;
		default:
			break;
		}

		H_NPC_Check_HP();
		H_SetFrustum();
		break;
	}
	case NPC_ARMY:
	{

		break;
	}
	}
}
void NPC::Caculation_Distance(XMFLOAT3 vec, int id) // 서버에서 따로 부를 것.
{
	m_Distance[id] = sqrtf(pow((vec.x - pos.x), 2) + pow((vec.z - pos.z), 2));
}

// Helicopter
void NPC::H_MoveToNode()
{
	NodeMesh currentNode = MeshInfo[m_currentNodeIndex];
	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const bool isMovingForward = m_currentNodeIndex % 4 < 2;
	const float speed = m_Speed;

	if (isMovingInZ) {
		const float destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;

		if (isMovingForward) pos.z += speed;
		else pos.z -= speed;

		if ((isMovingForward && pos.z >= destination) || (!isMovingForward && pos.z <= destination)) H_UpdateCurrentNodeIndex();
	}
	else {
		const float destination = isMovingForward ? currentNode.GetSmallX() + m_destinationRange : currentNode.GetLargeX() - m_destinationRange;

		if (isMovingForward) pos.x -= speed;
		else pos.x += speed;

		if ((isMovingForward && pos.x <= destination) || (!isMovingForward && pos.x >= destination)) H_UpdateCurrentNodeIndex();
	}

	int section = m_currentNodeIndex % 4;

	switch (section)
	{
	case 0:
		yaw = 0.0f;
		break;
	case 1:
		yaw = 270.0f;
		break;
	case 2:
		yaw = 180.0f;
		break;
	case 3:
		yaw = 90.0f;
		break;
	}
	m_rightvec = NPCcalcRightRotate();
	m_lookvec = NPCcalcLookRotate();
}
void NPC::H_UpdateCurrentNodeIndex()
{
	const int cityNum = m_currentNodeIndex / 4;
	const int nextSection = m_currentNodeIndex + 1;
	m_currentNodeIndex = (cityNum * 4) + (nextSection % 4);
}
void NPC::H_MoveChangeIdle()
{

}

bool NPC::H_IsUserOnSafeZone(int user_City)
{
	return (-1 < user_City < 4) ? false : true;
}
int NPC::H_GetUserCity()
{
	for (int i = 0; i < 4; ++i) {
		if (m_User_Pos[m_chaseID].z < MeshInfo[i * 4 + 1].GetLargeZ() &&
			m_User_Pos[m_chaseID].z > MeshInfo[i * 4 + 3].GetSmallZ() &&
			m_User_Pos[m_chaseID].x < MeshInfo[i * 4].GetLargeX() &&
			m_User_Pos[m_chaseID].x > MeshInfo[i * 4 + 2].GetSmallX()) {
			return i;
		}
	}
	return -1;
}
int NPC::H_FindUserNode(int user_City)
{
	for (int i = 0; i < 4; ++i) {
		if (m_User_Pos[m_chaseID].x > MeshInfo[4 * user_City + i].GetLargeX() ||
			m_User_Pos[m_chaseID].x < MeshInfo[4 * user_City + i].GetSmallX() ||
			m_User_Pos[m_chaseID].z > MeshInfo[4 * user_City + i].GetLargeZ() ||
			m_User_Pos[m_chaseID].z < MeshInfo[4 * user_City + i].GetSmallZ()) {
			continue;
		}
		else {
			return 4 * user_City + i;
		}
	}
	return -1;
}
void NPC::H_IsPathMove()
{
	if (path.empty() || path.size() < 2)
		return;

	NodeMesh currentNode = MeshInfo[path[0]];
	NodeMesh nextNode = MeshInfo[path[1]];

	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const bool isMovingForward = nextNode.GetIndex() % 4 < 2;
	const float speed = m_Speed;

	float destination;
	if (isMovingInZ) {
		destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;
		pos.z += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 0.0f : 180.0f;

		if ((isMovingForward && pos.z >= destination) || (!isMovingForward && pos.z <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}
	else {
		destination = isMovingForward ? currentNode.GetLargeX() - m_destinationRange : currentNode.GetSmallX() + m_destinationRange;
		pos.x += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 90.0f : 270.0f;

		if ((isMovingForward && pos.x >= destination) || (!isMovingForward && pos.x <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}

	m_rightvec = NPCcalcRightRotate();
	m_lookvec = NPCcalcLookRotate();
}
void NPC::H_PlayerChasing()
{
	// 같은 도시에 있는 지 판별
	int user_City = H_GetUserCity();
	if (user_City == -1 || H_IsUserOnSafeZone(user_City)) return;

	// 있다면 노드 검색
	int user_node = H_FindUserNode(user_City);
	if (user_node == -1) return;

	// 노드 검색 후 같은 섹션에 있는 지 판별
	if (user_node == m_currentNodeIndex) {
		if (m_Distance[m_chaseID] >= 150.0f) {
			// 내 포지션과 유저의 포지션이 이루는 벡터를 구함
			XMFLOAT3 positionToUser = { m_User_Pos[m_chaseID].x - pos.x, m_User_Pos[m_chaseID].y - pos.y, m_User_Pos[m_chaseID].z - pos.z };
			XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

			// xz 평면으로 투영
			XMFLOAT3 projectedPositionToUser = { positionToUser.x, 0.0f, positionToUser.z };
			NPCNormalize(projectedPositionToUser);

			// DefalutTemp, 가야하는 방향 사이의 각도 구하기
			float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(projectedPositionToUser.z, projectedPositionToUser.x);
			float angleDegree = XMConvertToDegrees(angleRadian);

			yaw = angleDegree;

			if (yaw > 360.0f)
				yaw -= 360.0f;
			if (yaw < 0.0f)
				yaw += 360.0f;

			// 회전한 look 벡터를 통해 이동 (look은 계산하기 전에 노멀라이즈)
			m_lookvec = NPCcalcLookRotate();
			m_rightvec = NPCcalcRightRotate();

			pos.x += m_lookvec.x * m_Speed;
			pos.z += m_lookvec.z * m_Speed;
		}
	}
	else {
		// path 데이터가 있다면 제거
		path.clear();

		path = H_AStarSearch(m_currentNodeIndex, user_node, user_City);
		m_targetNodeIndex = path[path.size() - 1];
		m_currentNodeIndex = path[0];
		H_IsPathMove();
	}
}
vector<int> NPC::H_AStarSearch(int startNode, int targetNode, int user_City)
{
	const int m_NumTotalNode = MeshInfo.size();
	vector<bool> visited(m_NumTotalNode, false);
	vector<float> gScore(m_NumTotalNode, numeric_limits<float>::infinity());
	vector<float> fScore(m_NumTotalNode, numeric_limits<float>::infinity());
	vector<Node*> cameFrom(m_NumTotalNode, nullptr);

	gScore[startNode] = 0.0f;
	fScore[startNode] = HeuristicEstimate(startNode, targetNode);

	priority_queue<Node> openSet;
	openSet.emplace(startNode, fScore[startNode], gScore[startNode], nullptr);
	visited[startNode] = true;

	vector<int> temppath;
	while (!openSet.empty()) {
		if (cameFrom[targetNode] != nullptr) {
			Node* node = cameFrom[targetNode];
			while (node != nullptr) {
				temppath.push_back(node->index);
				node = node->parent;
			}
			reverse(temppath.begin(), temppath.end());
			temppath.push_back(targetNode);
			break;
		}

		Node current = openSet.top();
		openSet.pop();

		visited[current.index] = true;

		for (int neighborIndex : MeshInfo[current.index].neighbor_mesh) {
			if (user_City != neighborIndex / 4)
				continue;

			if (!visited[neighborIndex]) {
				float tentative_gScore = gScore[current.index] + DistanceBetween(current.index, neighborIndex);

				if (tentative_gScore < gScore[neighborIndex]) {
					cameFrom[neighborIndex] = new Node(current);  // 복사본을 만들어서 cameFrom에 저장
					gScore[neighborIndex] = tentative_gScore;
					fScore[neighborIndex] = gScore[neighborIndex] + HeuristicEstimate(neighborIndex, targetNode);

					openSet.emplace(neighborIndex, fScore[neighborIndex], gScore[neighborIndex], cameFrom[neighborIndex]);
				}
			}
		}
	}

	return temppath;
}

bool NPC::H_PlayerDetact()
{
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	XMVECTOR NPCPos = XMLoadFloat3(&pos);
	XMVECTOR NPCLook = XMLoadFloat3(&m_lookvec);
	XMVECTOR NPCToPlayer = XMVectorSubtract(PlayerPos, NPCPos);

	// 플레이어의 위치가 NPC가 바라보는 방향에 있어야만 충돌로 간주한다.
	if (XMVectorGetX(XMVector3Dot(NPCToPlayer, NPCLook)) > 0)
	{
		// 프러스텀과 Player의 bounding sphere와의 거리를 구한다.
		float distance = XMVectorGetX(XMVector3Length(NPCToPlayer));

		// 거리가 bounding sphere의 반지름보다 작으면 충돌했다고 판단한다.
		if (distance < 200.0f)
		{
			return true;
		}
	}

	return false;
}

void NPC::H_SetFrustum()
{
	// NPC의 위치와 Look 벡터를 가져온다.
	XMVECTOR position = XMLoadFloat3(&pos);
	XMVECTOR look = XMLoadFloat3(&m_lookvec);

	// Frustum의 사이드 범위와 상하 범위를 설정한다.
	float width = 100.0f;
	float height = 100.0f;

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
void NPC::H_PlayerAttack()
{
	// Look
	H_PlayerChasing();

	// Attack
	/* ChasePlayer{ {m_User_Pos[m_chaseID].x, m_User_Pos[m_chaseID].y, m_User_Pos[m_chaseID].z },
		HELI_BBSIZE_X, HELI_BBSIZE_X, HELI_BBSIZE_X };
	XMFLOAT3 NPC_bullet_Pos = { pos.x, pos.y , pos.z };
	XMFLOAT3 NPC_Look_Vec = m_lookvec;
	XMFLOAT3 NPC_result;
	NPC_result = MyRaycast_InfiniteRay(NPC_bullet_Pos, NPC_Look_Vec, ChasePlayer);
	if (NPC_result != m_VectorMAX) {
		PrintRayCast = false;
	}
	else {
		PrintRayCast = true;
	}*/

	XMFLOAT3 AttackVec = { m_User_Pos[m_chaseID].x - pos.x,m_User_Pos[m_chaseID].y - pos.y, m_User_Pos[m_chaseID].z - pos.z };
	m_AttackVec = NPCNormalize(AttackVec);

	//NPCtoBuilding_collide();
}
void NPC::H_NPC_Damege_Calc(int id)
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
void NPC::H_NPC_Check_HP()
{
	if ((m_BodyHP <= 0) || (m_ProfellerHP <= 0)) {
		m_state = NPC_DEATH;
	}
}
void NPC::H_NPC_Death_motion()
{
	pos.y -= 6.0f;

	// 빙글빙글 돌며 추락
	pitch += 3.0f;

	m_rightvec = NPCcalcRightRotate();
	m_lookvec = NPCcalcLookRotate();
}

// Army
void NPC::A_MoveToNode()
{
	NodeMesh currentNode = MeshInfo[m_currentNodeIndex];
	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const bool isMovingForward = m_currentNodeIndex % 4 < 2;
	const float speed = m_Speed;

	if (isMovingInZ) {
		const float destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;

		if (isMovingForward) pos.z += speed;
		else pos.z -= speed;

		if ((isMovingForward && pos.z >= destination) || (!isMovingForward && pos.z <= destination)) H_UpdateCurrentNodeIndex();
	}
	else {
		const float destination = isMovingForward ? currentNode.GetSmallX() + m_destinationRange : currentNode.GetLargeX() - m_destinationRange;

		if (isMovingForward) pos.x -= speed;
		else pos.x += speed;

		if ((isMovingForward && pos.x <= destination) || (!isMovingForward && pos.x >= destination)) H_UpdateCurrentNodeIndex();
	}

	int section = m_currentNodeIndex % 4;

	switch (section)
	{
	case 0:
		yaw = 0.0f;
		break;
	case 1:
		yaw = 270.0f;
		break;
	case 2:
		yaw = 180.0f;
		break;
	case 3:
		yaw = 90.0f;
		break;
	}
	m_rightvec = NPCcalcRightRotate();
	m_lookvec = NPCcalcLookRotate();
}
void NPC::A_UpdateCurrentNodeIndex()
{
	const int cityNum = m_currentNodeIndex / 4;
	const int nextSection = m_currentNodeIndex + 1;
	m_currentNodeIndex = (cityNum * 4) + (nextSection % 4);
}
void NPC::A_MoveChangeIdle()
{

}

bool NPC::A_IsUserOnSafeZone(int user_City)
{
	return (-1 < user_City < 4) ? false : true;
}
int NPC::A_GetUserCity()
{
	for (int i = 0; i < 4; ++i) {
		if (m_User_Pos[m_chaseID].z < MeshInfo[i * 4 + 1].GetLargeZ() &&
			m_User_Pos[m_chaseID].z > MeshInfo[i * 4 + 3].GetSmallZ() &&
			m_User_Pos[m_chaseID].x < MeshInfo[i * 4].GetLargeX() &&
			m_User_Pos[m_chaseID].x > MeshInfo[i * 4 + 2].GetSmallX()) {
			return i;
		}
	}
	return -1;
}
int NPC::A_FindUserNode(int user_City)
{
	for (int i = 0; i < 4; ++i) {
		if (m_User_Pos[m_chaseID].x > MeshInfo[4 * user_City + i].GetLargeX() ||
			m_User_Pos[m_chaseID].x < MeshInfo[4 * user_City + i].GetSmallX() ||
			m_User_Pos[m_chaseID].z > MeshInfo[4 * user_City + i].GetLargeZ() ||
			m_User_Pos[m_chaseID].z < MeshInfo[4 * user_City + i].GetSmallZ()) {
			continue;
		}
		else {
			return 4 * user_City + i;
		}
	}
	return -1;
}
void NPC::A_IsPathMove()
{
	if (path.empty() || path.size() < 2)
		return;

	NodeMesh currentNode = MeshInfo[path[0]];
	NodeMesh nextNode = MeshInfo[path[1]];

	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const bool isMovingForward = nextNode.GetIndex() % 4 < 2;
	const float speed = m_Speed;

	float destination;
	if (isMovingInZ) {
		destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;
		pos.z += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 0.0f : 180.0f;

		if ((isMovingForward && pos.z >= destination) || (!isMovingForward && pos.z <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}
	else {
		destination = isMovingForward ? currentNode.GetLargeX() - m_destinationRange : currentNode.GetSmallX() + m_destinationRange;
		pos.x += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 90.0f : 270.0f;

		if ((isMovingForward && pos.x >= destination) || (!isMovingForward && pos.x <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}

	m_rightvec = NPCcalcRightRotate();
	m_lookvec = NPCcalcLookRotate();
}
void NPC::A_PlayerChasing()
{
	// 같은 도시에 있는 지 판별
	int user_City = A_GetUserCity();
	if (user_City == -1 || A_IsUserOnSafeZone(user_City)) return;

	// 있다면 노드 검색
	int user_node = A_FindUserNode(user_City);
	if (user_node == -1) return;

	// 노드 검색 후 같은 섹션에 있는 지 판별
	if (user_node == m_currentNodeIndex) {
		if (m_Distance[m_chaseID] >= 150.0f) {
			// 내 포지션과 유저의 포지션이 이루는 벡터를 구함
			XMFLOAT3 positionToUser = { m_User_Pos[m_chaseID].x - pos.x, m_User_Pos[m_chaseID].y - pos.y, m_User_Pos[m_chaseID].z - pos.z };
			XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

			// xz 평면으로 투영
			XMFLOAT3 projectedPositionToUser = { positionToUser.x, 0.0f, positionToUser.z };
			NPCNormalize(projectedPositionToUser);

			// DefalutTemp, 가야하는 방향 사이의 각도 구하기
			float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(projectedPositionToUser.z, projectedPositionToUser.x);
			float angleDegree = XMConvertToDegrees(angleRadian);

			yaw = angleDegree;

			if (yaw > 360.0f)
				yaw -= 360.0f;
			if (yaw < 0.0f)
				yaw += 360.0f;

			// 회전한 look 벡터를 통해 이동 (look은 계산하기 전에 노멀라이즈)
			m_lookvec = NPCcalcLookRotate();
			m_rightvec = NPCcalcRightRotate();

			pos.x += m_lookvec.x * m_Speed;
			pos.z += m_lookvec.z * m_Speed;
		}
	}
	else {
		// path 데이터가 있다면 제거
		path.clear();

		path = A_AStarSearch(m_currentNodeIndex, user_node, user_City);
		m_targetNodeIndex = path[path.size() - 1];
		m_currentNodeIndex = path[0];
		A_IsPathMove();
	}
}
vector<int> NPC::A_AStarSearch(int startNode, int targetNode, int user_City)
{
	const int m_NumTotalNode = MeshInfo.size();
	vector<bool> visited(m_NumTotalNode, false);
	vector<float> gScore(m_NumTotalNode, numeric_limits<float>::infinity());
	vector<float> fScore(m_NumTotalNode, numeric_limits<float>::infinity());
	vector<Node*> cameFrom(m_NumTotalNode, nullptr);

	gScore[startNode] = 0.0f;
	fScore[startNode] = HeuristicEstimate(startNode, targetNode);

	priority_queue<Node> openSet;
	openSet.emplace(startNode, fScore[startNode], gScore[startNode], nullptr);
	visited[startNode] = true;

	vector<int> temppath;
	while (!openSet.empty()) {
		if (cameFrom[targetNode] != nullptr) {
			Node* node = cameFrom[targetNode];
			while (node != nullptr) {
				temppath.push_back(node->index);
				node = node->parent;
			}
			reverse(temppath.begin(), temppath.end());
			temppath.push_back(targetNode);
			break;
		}

		Node current = openSet.top();
		openSet.pop();

		visited[current.index] = true;

		for (int neighborIndex : MeshInfo[current.index].neighbor_mesh) {
			if (user_City != neighborIndex / 4)
				continue;

			if (!visited[neighborIndex]) {
				float tentative_gScore = gScore[current.index] + DistanceBetween(current.index, neighborIndex);

				if (tentative_gScore < gScore[neighborIndex]) {
					cameFrom[neighborIndex] = new Node(current);  // 복사본을 만들어서 cameFrom에 저장
					gScore[neighborIndex] = tentative_gScore;
					fScore[neighborIndex] = gScore[neighborIndex] + HeuristicEstimate(neighborIndex, targetNode);

					openSet.emplace(neighborIndex, fScore[neighborIndex], gScore[neighborIndex], cameFrom[neighborIndex]);
				}
			}
		}
	}

	return temppath;
}

bool NPC::A_PlayerDetact()
{
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	XMVECTOR NPCPos = XMLoadFloat3(&pos);
	XMVECTOR NPCLook = XMLoadFloat3(&m_lookvec);
	XMVECTOR NPCToPlayer = XMVectorSubtract(PlayerPos, NPCPos);

	// 플레이어의 위치가 NPC가 바라보는 방향에 있어야만 충돌로 간주한다.
	if (XMVectorGetX(XMVector3Dot(NPCToPlayer, NPCLook)) > 0)
	{
		// 프러스텀과 Player의 bounding sphere와의 거리를 구한다.
		float distance = XMVectorGetX(XMVector3Length(NPCToPlayer));

		// 거리가 bounding sphere의 반지름보다 작으면 충돌했다고 판단한다.
		if (distance < 200.0f)
		{
			return true;
		}
	}

	return false;
}

void NPC::A_SetFrustum()
{
	// NPC의 위치와 Look 벡터를 가져온다.
	XMVECTOR position = XMLoadFloat3(&pos);
	XMVECTOR look = XMLoadFloat3(&m_lookvec);

	// Frustum의 사이드 범위와 상하 범위를 설정한다.
	float width = 100.0f;
	float height = 100.0f;

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
void NPC::A_PlayerAttack()
{
	// Look
	A_PlayerChasing();

	// Attack
	/* ChasePlayer{ {m_User_Pos[m_chaseID].x, m_User_Pos[m_chaseID].y, m_User_Pos[m_chaseID].z },
		HELI_BBSIZE_X, HELI_BBSIZE_X, HELI_BBSIZE_X };
	XMFLOAT3 NPC_bullet_Pos = { pos.x, pos.y , pos.z };
	XMFLOAT3 NPC_Look_Vec = m_lookvec;
	XMFLOAT3 NPC_result;
	NPC_result = MyRaycast_InfiniteRay(NPC_bullet_Pos, NPC_Look_Vec, ChasePlayer);
	if (NPC_result != m_VectorMAX) {
		PrintRayCast = false;
	}
	else {
		PrintRayCast = true;
	}*/

	XMFLOAT3 AttackVec = { m_User_Pos[m_chaseID].x - pos.x,m_User_Pos[m_chaseID].y - pos.y, m_User_Pos[m_chaseID].z - pos.z };
	m_AttackVec = NPCNormalize(AttackVec);

	//NPCtoBuilding_collide();
}
void NPC::A_NPC_Damege_Calc(int id)
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
void NPC::A_NPC_Check_HP()
{
	if ((m_BodyHP <= 0) || (m_ProfellerHP <= 0)) {
		m_state = NPC_DEATH;
	}
}
void NPC::A_NPC_Death_motion()
{
	pos.y -= 6.0f;

	// 빙글빙글 돌며 추락
	pitch += 3.0f;

	m_rightvec = NPCcalcRightRotate();
	m_lookvec = NPCcalcLookRotate();
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
	mutex s_lock;

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
	void send_npc_attack_packet(int npc_id);
};

HANDLE h_iocp;											// IOCP 핸들
int a_lgcsvr_num;										// Active상태인 메인서버
array<SERVER, MAX_LOGIC_SERVER> g_logicservers;			// 로직서버 정보
bool b_lgcserver_conn;

void SERVER::send_npc_init_packet(int npc_id) {
	NPC_FULL_INFO_PACKET npc_init_packet;
	npc_init_packet.size = sizeof(NPC_FULL_INFO);
	npc_init_packet.type = NPC_FULL_INFO;
	npc_init_packet.n_id = npc_id;
	strcpy_s(npc_init_packet.name, npcsInfo[npc_id].name);
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

	lock_guard<mutex> lg{ g_logicservers[a_lgcsvr_num].s_lock };
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

	lock_guard<mutex> lg{ g_logicservers[a_lgcsvr_num].s_lock };
	g_logicservers[a_lgcsvr_num].do_send(&npc_move_packet);
}
void SERVER::send_npc_rotate_packet(int npc_id) {
	NPC_ROTATE_PACKET npc_rotate_packet;
	npc_rotate_packet.size = sizeof(NPC_ROTATE_PACKET);
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

	lock_guard<mutex> lg{ g_logicservers[a_lgcsvr_num].s_lock };
	g_logicservers[a_lgcsvr_num].do_send(&npc_rotate_packet);
}
void SERVER::send_npc_move_rotate_packet(int npc_id) {
	send_npc_move_packet(npc_id);
	send_npc_rotate_packet(npc_id);
}

void SERVER::send_npc_attack_packet(int npc_id)
{
	NPC_ATTACK_PACKET npc_attack_packet;
	npc_attack_packet.size = sizeof(NPC_ATTACK_PACKET);
	npc_attack_packet.type = NPC_ATTACK;
	npc_attack_packet.n_id = npc_id;
	npc_attack_packet.atklook_x = npcsInfo[npc_id].GetAttackVec().x;
	npc_attack_packet.atklook_y = npcsInfo[npc_id].GetAttackVec().y;
	npc_attack_packet.atklook_z = npcsInfo[npc_id].GetAttackVec().z;

	lock_guard<mutex> lg{ g_logicservers[a_lgcsvr_num].s_lock };
	g_logicservers[a_lgcsvr_num].do_send(&npc_attack_packet);
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
			playersInfo[obj_id].obj_lock.lock();
			playersInfo[obj_id].hp -= damaged_packet->damage;
			playersInfo[obj_id].obj_lock.unlock();
		}
		else if (damaged_packet->target == TARGET_NPC) {
			npcsInfo[obj_id].obj_lock.lock();
			npcsInfo[obj_id].hp -= damaged_packet->damage;
			npcsInfo[obj_id].obj_lock.unlock();
		}

		break;
	}// SC_DAMAGED end
	case SC_OBJECT_STATE:
	{
		SC_OBJECT_STATE_PACKET* chgstate_packet = reinterpret_cast<SC_OBJECT_STATE_PACKET*>(packet);
		int obj_id = chgstate_packet->id;
		short changed_state = chgstate_packet->state;

		// 1. 우선 상태를 바꿔준다.
		if (chgstate_packet->target == TARGET_PLAYER) {
			playersInfo[obj_id].obj_lock.lock();
			playersInfo[obj_id].state = chgstate_packet->state;
			playersInfo[obj_id].obj_lock.unlock();
		}
		else if (chgstate_packet->target == TARGET_NPC) {
			npcsInfo[obj_id].obj_lock.lock();
			npcsInfo[obj_id].state = chgstate_packet->state;
			npcsInfo[obj_id].obj_lock.unlock();
		}

		// 2. 상태별 추가 작업이 필요하면 여기서 처리한다.
		switch (changed_state) {
		case PL_ST_IDLE:
			if (chgstate_packet->target == TARGET_PLAYER) {

			}
			else if (chgstate_packet->target == TARGET_NPC) {

			}

			break;

		case PL_ST_MOVE_FRONT:
		case PL_ST_MOVE_BACK:
		case PL_ST_MOVE_SIDE:
			if (chgstate_packet->target == TARGET_PLAYER) {

			}
			else if (chgstate_packet->target == TARGET_NPC) {

			}

			break;

		case PL_ST_FLY:
			if (chgstate_packet->target == TARGET_PLAYER) {

			}
			else if (chgstate_packet->target == TARGET_NPC) {

			}

			break;

		case PL_ST_CHASE:
			if (chgstate_packet->target == TARGET_PLAYER) {

			}
			else if (chgstate_packet->target == TARGET_NPC) {

			}

			break;

		case PL_ST_ATTACK:
			if (chgstate_packet->target == TARGET_PLAYER) {

			}
			else if (chgstate_packet->target == TARGET_NPC) {

			}

			break;

		case PL_ST_DEAD:
			if (chgstate_packet->target == TARGET_PLAYER) {

			}
			else if (chgstate_packet->target == TARGET_NPC) {

			}

			break;

		default:
			cout << "[OBJECT_STATE Error] Unknown State Type.\n" << endl;
		}

		break;
	}// SC_OBJECT_STATE end
	}
}

//======================================================================
void initNpc();
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
				ConnectingServer = true;
				initNpc();
			}

		}//OP_CONN end
		}
	}
}

//======================================================================
void initNpc() {
	for (int i{}; i < HelicopterNum; i++) {
		int npc_id = i;
		npcsInfo[i].SetID(npc_id);
		npcsInfo[i].type = NPC_HELICOPTER;
		random_device rd;
		default_random_engine dre(rd());

		uniform_int_distribution<int>idx(0, 15);
		//npcsInfo[i].graph = CP;
		npcsInfo[i].SetNodeIndex(idx(dre));

		float x = npcsInfo[i].getRandomOffset(MeshInfo[npcsInfo[i].GetNodeIndex()].GetSmallX(),
			MeshInfo[npcsInfo[i].GetNodeIndex()].GetLargeX());
		float y = npcsInfo[i].getRandomOffset(30.0f, 60.0f);
		float z = npcsInfo[i].getRandomOffset(MeshInfo[npcsInfo[i].GetNodeIndex()].GetSmallZ(),
			MeshInfo[npcsInfo[i].GetNodeIndex()].GetLargeZ());

		XMFLOAT3 tpos = { x,y,z };
		npcsInfo[i].SetPosition(tpos);

		npcsInfo[i].SetRotate(0.0f, 0.0f, 0.0f);

		uniform_int_distribution<int>drange(0.0f, 20.0f);
		npcsInfo[i].SetDestinationRange(drange(dre));

		uniform_real_distribution<float>SpdSet(10.0f, 20.0f);
		float speed = SpdSet(dre);
		npcsInfo[i].SetSpeed(speed);
		npcsInfo[i].SetChaseID(-1);
		npcsInfo[i].path.clear();
		npcsInfo[i].SetTargetNodeIndex(-1);
		g_logicservers[a_lgcsvr_num].send_npc_init_packet(npc_id);
	}

	if (MAX_NPCS != HelicopterNum) {
		for (int i = HelicopterNum; i < MAX_NPCS; i++) {
			int npc_id = i;
			npcsInfo[i].SetID(npc_id);
			npcsInfo[i].type = NPC_ARMY;
			random_device rd;
			default_random_engine dre(rd());

			uniform_int_distribution<int>idx(0, 15);
			//npcsInfo[i].graph = CP;
			npcsInfo[i].SetNodeIndex(idx(dre));

			float x = npcsInfo[i].getRandomOffset(MeshInfo[npcsInfo[i].GetNodeIndex()].GetSmallX(),
				MeshInfo[npcsInfo[i].GetNodeIndex()].GetLargeX());
			float y = 11.0f;
			float z = npcsInfo[i].getRandomOffset(MeshInfo[npcsInfo[i].GetNodeIndex()].GetSmallZ(),
				MeshInfo[npcsInfo[i].GetNodeIndex()].GetLargeZ());

			XMFLOAT3 tpos = { x,y,z };
			npcsInfo[i].SetPosition(tpos);

			npcsInfo[i].SetRotate(0.0f, 0.0f, 0.0f);

			uniform_int_distribution<int>drange(0.0f, 20.0f);
			npcsInfo[i].SetDestinationRange(drange(dre));

			uniform_real_distribution<float>SpdSet(10.0f, 20.0f);
			float speed = SpdSet(dre);
			npcsInfo[i].SetSpeed(speed);
			npcsInfo[i].SetChaseID(-1);
			npcsInfo[i].path.clear();
			npcsInfo[i].SetTargetNodeIndex(-1);
			g_logicservers[a_lgcsvr_num].send_npc_init_packet(npc_id);
		}
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
				// 클라이언트들과 NPC 사이의 거리 계산

				if (npcsInfo[i].GetState() == NPC_DEATH && npcsInfo[i].GetPosition().y < 0) {
					NPC_REMOVE_PACKET npc_remove_packet;

					npc_remove_packet.size = sizeof(NPC_REMOVE_PACKET);
					npc_remove_packet.type = SC_REMOVE_OBJECT;
					npc_remove_packet.n_id = npcsInfo[i].GetID();

					npcsInfo[i].m_DeathCheck = true;
				}
				if (npcsInfo[i].GetPosition().y > 0)
				{
					float temp_min = numeric_limits<float>::infinity();
					int temp_id = -1;

					for (auto& cl : playersInfo) {
						if (cl.id != -1) {
							npcsInfo[i].Caculation_Distance(cl.pos, cl.id);
							// 가장 가까운 거리를 갖고있는 아이를 chase_id로 지정
							float distance = npcsInfo[i].GetDistance(cl.id);
							if (temp_min > distance) {
								temp_min = distance;
								temp_id = cl.id;
							}
						}
					}

					npcsInfo[i].SetChaseID(temp_id);

					npcsInfo[i].NPC_State_Manegement(npcsInfo[i].GetState());
					// NPC가 추적하려는 아이디가 있는지부터 확인, 있으면 추적 대상 플레이어 좌표를 임시 저장
					if (npcsInfo[i].GetChaseID() != -1) {
						npcsInfo[i].SetUser_Pos(playersInfo[npcsInfo[i].GetChaseID()].pos, npcsInfo[i].GetChaseID());
					}

					// npc pos 확인

					if (i > 8 && i < 12) {

						cout << "=============" << endl;

						cout << i << "번째 NPC의 NodeIndex: " << npcsInfo[i].GetNodeIndex() << endl;
						cout << i << "번째 NPC의 Type: " << npcsInfo[i].type << endl;
						cout << i << "번째 NPC의 Pos: " << npcsInfo[i].GetPosition().x << ',' << npcsInfo[i].GetPosition().y << ',' << npcsInfo[i].GetPosition().z << endl;
						cout << i << "번째 NPC의 Look: " << npcsInfo[i].m_lookvec.x << ", " << npcsInfo[i].m_lookvec.y << ", " << npcsInfo[i].m_lookvec.z << endl;
						cout << i << "번째 NPC의 상태: " << npcsInfo[i].GetState() << endl;

					}

					//if (npcs[i].PrintRayCast) {
					//	cout << i << "번째 NPC가 쏜 총알에 대해" << npcs[i].GetChaseID() << "의 ID를 가진 플레이어가 피격되었습니다." << endl;
					//}

					g_logicservers[a_lgcsvr_num].send_npc_move_rotate_packet(npcsInfo[i].GetID());

					if (npcsInfo[i].GetState() == NPC_ATTACK) {
						g_logicservers[a_lgcsvr_num].send_npc_attack_packet(npcsInfo[i].GetID());
					}

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
	//					로직서버로 비동기 Connect 요청
	//======================================================================
	ConnectingServer = false;
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

							NodeMesh temp_mesh;
							int idx = line_cnt / 10;
							temp_mesh.SetInit(idx, tmp_pos[0], tmp_pos[2], tmp_scale[0], tmp_scale[2]);
							cout << "index_num: " << idx << ", CPos: " << tmp_pos[0] << ", " << tmp_pos[2] << ", Bsize: " << tmp_scale[0] << ", " << tmp_scale[2] << endl;

							MeshInfo.push_back(temp_mesh);

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

	for (int i{}; i < MeshInfo.size(); ++i) {
		if (i < 16) {
			if (i % 2 == 0) {
				MeshInfo[i].SetMoveingSpace(false, true);
			}
			else {
				MeshInfo[i].SetMoveingSpace(true, false);
			}
		}

		for (int j{}; j < MeshInfo.size(); ++j) {
			if (i == j) {
				continue;
			}
			bool intersection = MeshInfo[i].otherIndexIntersection(MeshInfo[j]);
			if (intersection) {
				MeshInfo[i].neighbor_mesh.emplace_back(j);
			}
		}
	}

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
