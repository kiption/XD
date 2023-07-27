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
#include "CP_KEY.h"
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

//============================================================
//						 DirectX API
//============================================================
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
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
#include "CheckPoint.h"
#include "Raycast.h"
#include "Node.h"
#include "CollideMapData.h"
#include "DirectCalculation.h"

using namespace std;
using namespace chrono;

//============================================================
system_clock::time_point last_send_checkpos_time;	// 마지막으로 check_pos패킷 보낸 시간
mutex time_lock;	// 서버시간 lock

enum NPCState { NPC_IDLE, NPC_CHASE, NPC_ST_ATK, NPC_DEATH, NPC_BACK };
enum Hit_target { g_none, g_body, g_profeller };
enum NPCType { NPC_HELICOPTER, NPC_ARMY };
enum MAP_OBJ_TYPE { M_OBJ_BUILDING, M_OBJ_TREE, M_OBJ_OTHERS };

bool ConnectingServer = false;
bool ClientConnected = false;
constexpr int HelicopterNum = 5;
constexpr int ArmyNum = 20;

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
// Map Objects CollideBox
vector<MapObject> mapobjects_info;

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
		hp = 100;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void setBB() {

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}
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
	int role;
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
	XMFLOAT3 m_OriginPos;
	XMFLOAT3 m_otherNPCPos;
	short m_Hit;
	short m_state;
	int m_chaseID;
	int m_currentNodeIndex;
	int m_targetNodeIndex;
	int m_OriginNodeIndex;
	float m_Speed;
	float m_Distance[MAX_USER];
	float m_destinationRange;
	float m_PrevYaw;

public:
	bool m_DeathCheck = false;
	bool m_shooton = false;
	bool PrintRayCast = false;
	bool TurnBack = false;
	bool m_UpdateTurn = false;
	int m_attackcount = 0;
	vector<int>path;
	unordered_set<int>m_objectlist;
	BoundingFrustum m_frustum;
	vector<MapObject> m_collideBox;
	XMFLOAT3 m_VectorMAX = { -9999.f, -9999.f, -9999.f };
	vector<int>m_mapcollideData;

	chrono::system_clock::time_point PrevTime;
	chrono::system_clock::time_point CurrTime;

	chrono::system_clock::time_point ChaseTime;
	chrono::system_clock::time_point AttackTime;
public:
	NPC() : OBJECT() {
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
	int GetOriginNodeIndex() { return m_OriginNodeIndex; }
	int GetTargetNodeIndex() { return m_targetNodeIndex; }
	float GetDistance(int id) { return m_Distance[id]; }
	float GetSpeed() { return m_Speed; }
	XMFLOAT3 GetPosition() { return pos; }
	XMFLOAT3 GetAttackVec() { return m_AttackVec; }
	XMFLOAT3 GetOriginPos() { return m_OriginPos; }

public:
	// Set
	void SetHp(int thp) { hp = thp; }
	void SetID(int tid) { id = tid; }
	void SetState(int type) { m_state = type; }
	void SetChaseID(int cid) { m_chaseID = cid; }
	void SetNodeIndex(int idx) { m_currentNodeIndex = idx; }
	void SetTargetNodeIndex(int idx) { m_targetNodeIndex = idx; }
	void SetOriginNodeIndex(int idx) { m_OriginNodeIndex = idx; }
	void SetSpeed(float spd) { m_Speed = spd; }
	void SetDestinationRange(float range) { m_destinationRange = range; }
	void SetRotate(float y, float p, float r) { yaw = y, pitch = p, roll = r; }
	void SetPosition(XMFLOAT3 tpos) { pos = tpos; }
	void SetUser_Pos(XMFLOAT3 pos, int cid) { m_User_Pos[cid] = pos; }
	void SetAttackVec(XMFLOAT3 vec) { m_AttackVec = vec; }
	void SetOriginPos(XMFLOAT3 opos) { m_OriginPos = opos; }

public:
	// Base
		// Rotate
	XMFLOAT3 NPCcalcRightRotate();						// 전체 각에 따른 right 설정
	XMFLOAT3 NPCcalcUpRotate();							// 전체 각에 따른 up 설정
	XMFLOAT3 NPCcalcLookRotate();							// 전체 각에 따른 look 설정

	// Base
		// Random
	float getRandomOffset(float min, float max);											// random 내장 함수

	// State
		// base
	void NPC_State_Manegement(int state);													// 상태 관리
	void Caculation_Distance(XMFLOAT3 vec, int id);											// 범위 내 플레이어 탐색

	// 1. Helicopter
	// State
		// Idle
	void		H_MoveToNode();																		// 지정된 노드를 찾아가며 이동 - Idle
	void		H_UpdateCurrentNodeIndex();															// 노드 변환 시 다음 노드 지정

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
	bool		H_SetFrustum(XMVECTOR startPoint, XMVECTOR lookVector, XMVECTOR playerPosition);	// 프러스텀 설정
	void		H_PlayerAttack();																	// Attack 상태에서 동작하는 함수

	// 2. Army
	// State
		// Idle
	void		A_MoveToNode();																		// 지정된 노드를 찾아가며 이동 - Idle
	void		A_UpdateCurrentNodeIndex();															// 노드 변환 시 다음 노드 지정

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
	bool		A_SetFrustum(XMVECTOR startPoint, XMVECTOR lookVector, XMVECTOR playerPosition);	// 프러스텀 설정
	void		A_PlayerAttack();																	// Attack 상태에서 동작하는 함수

	// 3. Common
	bool		NPC_CollideByMap();																	// 맵이랑 충돌인지 판단
	bool		NPC_CollideByOtherNPC();															// NPC 간의 충돌
	bool		NPC_BulletRaycast();																// NPC가 쏜 총알 충돌 여부 판단
	void		NPC_CalculationCollide();															// 충돌된 경우 위치 및 회전 정보 재설정
	void		NPC_CalculationOtherCollide();														// NPC 간의 충돌된 경우 위치 및 회전 정보 재설정
	void		NPC_BackOwnsPos();																	// NPC Back 상황
	void		NPC_SetObjectList();																// 충돌 판정을 위한 오브젝트 뷰 리스트 담기
	void		H_setBB() {
		XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(0.0f, yaw, 0.0f);

		XMFLOAT4 oriented;
		XMStoreFloat4(&oriented, rotation);
		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), oriented);
	}
	void		A_setBB() {
		XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(0.0f, yaw, 0.0f);

		XMFLOAT4 oriented;
		XMStoreFloat4(&oriented, rotation);
		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HUMAN_BBSIZE_X, HUMAN_BBSIZE_Y, HUMAN_BBSIZE_Z), oriented);
	}

};
array<NPC, MAX_NPCS> npcsInfo;

//======================================================================
enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
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
	void send_npc_attack_packet(int npc_id);
};

HANDLE h_iocp;											// IOCP 핸들
int a_lgcsvr_num;										// Active상태인 메인서버
array<SERVER, MAX_LOGIC_SERVER> g_logicservers;			// 로직서버 정보

void SERVER::send_npc_init_packet(int npc_id)
{
	NPC_FULL_INFO_PACKET npc_init_packet;
	npc_init_packet.size = sizeof(NPC_FULL_INFO_PACKET);
	npc_init_packet.type = NPC_FULL_INFO;
	npc_init_packet.n_id = npc_id;
	npc_init_packet.speed = npcsInfo[npc_id].GetSpeed();
	npc_init_packet.ishuman = npcsInfo[npc_id].type;
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
void SERVER::send_npc_move_packet(int npc_id)
{
	if (npcsInfo[npc_id].state == PL_ST_DEAD) return;

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
void SERVER::send_npc_rotate_packet(int npc_id)
{
	if (npcsInfo[npc_id].state == PL_ST_DEAD) return;

	NPC_ROTATE_PACKET npc_rotate_packet;
	npc_rotate_packet.size = sizeof(NPC_ROTATE_PACKET);
	npc_rotate_packet.type = NPC_ROTATE;
	npc_rotate_packet.n_id = npc_id;
	npc_rotate_packet.x = npcsInfo[npc_id].pos.x;
	npc_rotate_packet.y = npcsInfo[npc_id].pos.y;
	npc_rotate_packet.z = npcsInfo[npc_id].pos.z;
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
void SERVER::send_npc_attack_packet(int npc_id)
{
	if (npcsInfo[npc_id].state == PL_ST_DEAD) return;

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

//============================================================
// [HA]
//======================================================================
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };
enum SESSION_TYPE { SESSION_LOGIC, SESSION_NPC };
void disconnect(int target_id, int target);
class HA_SERVER {
	OVER_EX recv_over;

public:
	mutex s_lock;
	SESSION_STATE s_state;
	int id;
	SOCKET socket;
	int remain_size;
	chrono::system_clock::time_point heartbeat_recv_time;
	chrono::system_clock::time_point heartbeat_send_time;

public:
	HA_SERVER() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		heartbeat_recv_time = chrono::system_clock::now();
		heartbeat_send_time = chrono::system_clock::now();
	}
	~HA_SERVER() {}

	void do_recv() {
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
	void do_send(void* packet) {
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
};
array<HA_SERVER, MAX_LOGIC_SERVER> extended_servers;	// HA구현을 위해 수평확장된 서버들

int find_empty_extended_server() {	// ex_servers의 비어있는 칸을 찾아서 새로운 Server_ex의 아이디를 할당해주는 함수
	for (int i = 0; i < MAX_USER; ++i) {
		extended_servers[i].s_lock.lock();
		if (extended_servers[i].s_state == ST_FREE) {
			extended_servers[i].s_state = ST_ACCEPTED;
			extended_servers[i].heartbeat_recv_time = chrono::system_clock::now();
			extended_servers[i].heartbeat_send_time = chrono::system_clock::now();
			extended_servers[i].s_lock.unlock();
			return i;
		}
		extended_servers[i].s_lock.unlock();
	}
	return -1;
}

SOCKET g_ss_listensock;		// 수평확장 서버 간 통신 listen 소켓

SOCKET left_ex_server_sock;								// 이전 번호의 서버
SOCKET right_ex_server_sock;							// 다음 번호의 서버

int my_server_id;										// 내 서버 식별번호
bool b_active_server;									// Active 서버인가?

//======================================================================
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

void NPC::NPC_State_Manegement(int state)
{
	m_PrevYaw = yaw;
	switch (type)
	{
	case NPC_HELICOPTER:
	{
		switch (m_state)
		{
		case NPC_IDLE:
		{
			H_MoveToNode();
			if (((H_PlayerDetact()) || m_Distance[m_chaseID] < 200.0f) && m_chaseID != -1) {
				m_state = NPC_ATTACK;
				ChaseTime = system_clock::now();
				AttackTime = ChaseTime;
			}
			break;
		}
		case NPC_ATTACK:
		{
			if (m_chaseID == -1 || ChaseTime - AttackTime > 5000ms || m_Distance[m_chaseID] >= 200.0f) {
				//cout << "NPC ID: " << id << "원래의 상태로 돌아갑니다." << endl;
				m_state = NPC_IDLE;
				if (!path.empty()) path.clear();
				break;
			}
			else {
				if (H_PlayerDetact() || (m_Distance[m_chaseID] < 200.0f)) {
					AttackTime = system_clock::now();
				}
				ChaseTime = system_clock::now();
				H_PlayerAttack();
			}
			break;
		}
		case NPC_DEATH:
		{
			break;
		}
		default:
			break;
		}
		H_setBB();
		break;
	}
	case NPC_ARMY:
	{
		switch (m_state)
		{
		case NPC_IDLE:
		{
			A_MoveToNode();
			if ((A_PlayerDetact() || m_Distance[m_chaseID] < 200.0f) && m_chaseID != -1) {
				m_state = NPC_ATTACK;
				ChaseTime = system_clock::now();
				AttackTime = ChaseTime;
			}
			break;
		}
		case NPC_ATTACK:
		{
			ChaseTime = system_clock::now();
			if (m_chaseID == -1 || ChaseTime - AttackTime > 5000ms) {
				m_state = NPC_BACK;
				m_targetNodeIndex = m_OriginNodeIndex;
				if (!path.empty()) path.clear();
				int myCity = m_OriginNodeIndex / 4;

				path = A_AStarSearch(m_currentNodeIndex, m_targetNodeIndex, myCity);
				break;
			}
			else {
				if (A_PlayerDetact() || (m_Distance[m_chaseID] < 300)) {
					AttackTime = system_clock::now();
				}
				A_PlayerAttack();
			}
			break;
		}
		case NPC_DEATH:
		{
			break;
		}
		case NPC_BACK:
		{
			NPC_BackOwnsPos();
			break;
		}
		default:
			break;
		}

		A_setBB();
		break;
		break;
	}
	}
	NPC_SetObjectList();
}

void NPC::Caculation_Distance(XMFLOAT3 vec, int id) // 서버에서 따로 부를 것.
{
	m_Distance[id] = sqrtf(pow((vec.x - pos.x), 2) + pow((vec.y - pos.y), 2) + pow((vec.z - pos.z), 2));
}

// Helicopter
void NPC::H_MoveToNode()
{
	NodeMesh currentNode = MeshInfo[m_currentNodeIndex];
	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const bool isMovingForward = m_currentNodeIndex % 4 < 2;
	const float speed = m_Speed;

	XMFLOAT3 prevPos = pos;

	if (isMovingInZ) {
		const float destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;

		if (isMovingForward) prevPos.z += speed;
		else prevPos.z -= speed;

		if ((isMovingForward && prevPos.z >= destination) || (!isMovingForward && prevPos.z <= destination)) H_UpdateCurrentNodeIndex();
	}
	else {
		const float destination = isMovingForward ? currentNode.GetSmallX() + m_destinationRange : currentNode.GetLargeX() - m_destinationRange;

		if (isMovingForward) prevPos.x -= speed;
		else prevPos.x += speed;

		if ((isMovingForward && prevPos.x <= destination) || (!isMovingForward && prevPos.x >= destination)) H_UpdateCurrentNodeIndex();
	}
	float t = 0.3f; // 보간 시간 (조정 가능)
	XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);

	if (NPC_CollideByOtherNPC()) {
		NPC_CalculationOtherCollide();
	}
	else if (NPC_CollideByMap()) {
		NPC_CalculationCollide();
	}
	else {
		pos = interpolatedPos;
		if (m_UpdateTurn) {
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

			if (ConnectingServer) {
				g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
			}
			m_UpdateTurn = false;
		}
	}
}
void NPC::H_UpdateCurrentNodeIndex()
{
	const int cityNum = m_currentNodeIndex / 4;
	const int nextSection = m_currentNodeIndex + 1;
	m_currentNodeIndex = (cityNum * 4) + (nextSection % 4);
	m_UpdateTurn = true;
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

	float Curr_yaw = yaw;

	NodeMesh currentNode = MeshInfo[path[0]];
	NodeMesh nextNode = MeshInfo[path[1]];

	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const bool isMovingForward = nextNode.GetIndex() % 4 < 2;
	const float speed = m_Speed;
	XMFLOAT3 prevPos = pos;

	float destination;
	if (isMovingInZ) {
		destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;
		prevPos.z += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 0.0f : 180.0f;

		if (Curr_yaw != yaw) m_UpdateTurn = true;

		if ((isMovingForward && prevPos.z >= destination) || (!isMovingForward && prevPos.z <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}
	else {
		destination = isMovingForward ? currentNode.GetLargeX() - m_destinationRange : currentNode.GetSmallX() + m_destinationRange;
		prevPos.x += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 90.0f : 270.0f;

		if (Curr_yaw != yaw) m_UpdateTurn = true;

		if ((isMovingForward && prevPos.z >= destination) || (!isMovingForward && prevPos.z <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}
	float t = 0.3f; // 보간 시간 (조정 가능)
	XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);

	if (NPC_CollideByOtherNPC()) {
		NPC_CalculationOtherCollide();
	}
	else if (NPC_CollideByMap()) {
		NPC_CalculationCollide();
	}
	else {
		pos = interpolatedPos;
		if (m_UpdateTurn) {
			m_rightvec = NPCcalcRightRotate();
			m_lookvec = NPCcalcLookRotate();

			m_UpdateTurn = false;

			if (ConnectingServer) {
				g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
			}
		}
	}
}
void NPC::H_PlayerChasing()
{
	int user_City = H_GetUserCity();
	if (user_City == -1 || H_IsUserOnSafeZone(user_City)) {
		m_UpdateTurn = true;
		H_MoveToNode();
		m_state = NPC_IDLE;
		AttackTime = system_clock::now();
		ChaseTime = AttackTime;
		return;
	}

	int user_node = H_FindUserNode(user_City);
	if (user_node == -1) {
		m_UpdateTurn = true;
		H_MoveToNode();
		m_state = NPC_IDLE;
		AttackTime = system_clock::now();
		ChaseTime = AttackTime;
		return;
	}

	if (user_node == m_currentNodeIndex) {
		XMFLOAT3 prevPos = pos;
		XMFLOAT3 positionToUser = { m_User_Pos[m_chaseID].x - prevPos.x, m_User_Pos[m_chaseID].y - prevPos.y, m_User_Pos[m_chaseID].z - prevPos.z };
		XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

		XMFLOAT3 projectedPositionToUser = { positionToUser.x, 0.0f, positionToUser.z };
		NPCNormalize(projectedPositionToUser);

		float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(projectedPositionToUser.z, projectedPositionToUser.x);
		float angleDegree = XMConvertToDegrees(angleRadian);

		yaw = angleDegree;

		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		m_lookvec = NPCcalcLookRotate();
		m_rightvec = NPCcalcRightRotate();
		if (ConnectingServer) {
			g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
		}

		if (m_Distance[m_chaseID] >= 150.0f) {
			if (NPC_CollideByOtherNPC()) {
				NPC_CalculationOtherCollide();
			}
			else if (NPC_CollideByMap()) {
				NPC_CalculationCollide();
			}
			else {
				prevPos.x += m_lookvec.x * m_Speed;
				prevPos.z += m_lookvec.z * m_Speed;

				float t = 0.3f; // 보간 시간 (조정 가능)
				XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);

				pos = interpolatedPos;
			}
		}
		m_shooton = true;
	}
	else {
		path.clear();

		path = H_AStarSearch(m_currentNodeIndex, user_node, user_City);
		if (!path.empty()) {
			m_targetNodeIndex = path[path.size() - 1];
			m_currentNodeIndex = path[0];
			H_IsPathMove();
		}
		else {
			H_MoveToNode();
		}
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
	XMVECTOR NPCPos = XMLoadFloat3(&pos);
	XMVECTOR NPCLook = XMLoadFloat3(&m_lookvec);
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	if (H_SetFrustum(NPCPos, NPCLook, PlayerPos))
	{
		return true;
	}

	return false;
}

bool NPC::H_SetFrustum(XMVECTOR startPoint, XMVECTOR lookVector, XMVECTOR playerPosition)
{
	XMVECTOR endPoint = startPoint + (lookVector * 200.0f);

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	BoundingFrustum frustum;
	XMStoreFloat3(&frustum.Origin, startPoint);
	XMMATRIX viewMatrix = XMMatrixLookAtRH(startPoint, endPoint, up);
	XMVECTOR quaternion = XMQuaternionRotationMatrix(viewMatrix);
	XMStoreFloat4(&frustum.Orientation, quaternion);
	frustum.RightSlope = 0.5f;
	frustum.LeftSlope = -0.5f;
	frustum.TopSlope = 0.5f;
	frustum.BottomSlope = -0.5f;
	frustum.Near = 0.1f;
	frustum.Far = 200.0f;
	m_frustum = frustum;

	if (m_frustum.Contains(playerPosition) != ContainmentType::DISJOINT)
	{
		return true;
	}
	return false;
}
void NPC::H_PlayerAttack()
{
	H_PlayerChasing();

	if (m_shooton) {
		XMFLOAT3 NPCtoPlayerLook = { m_User_Pos[m_chaseID].x - pos.x,m_User_Pos[m_chaseID].y - pos.y, m_User_Pos[m_chaseID].z - pos.z };
		float distance = sqrtf(pow((m_User_Pos[m_chaseID].x - pos.x), 2) + pow((m_User_Pos[m_chaseID].y - pos.y), 2) + pow((m_User_Pos[m_chaseID].z - pos.z), 2));

		XMVECTOR NPCtoPlayerLookNormal = XMVector3Normalize(XMLoadFloat3(&NPCtoPlayerLook));
		XMStoreFloat3(&NPCtoPlayerLook, NPCtoPlayerLookNormal);

		XMFLOAT3 NPCtoPlayerUpTemp = { 0.0f, 1.0f, 0.0f };

		XMVECTOR NPCtoPlayerLookMat = XMLoadFloat3(&NPCtoPlayerLook);
		XMVECTOR NPCtoPlayerUpTempMat = XMLoadFloat3(&NPCtoPlayerUpTemp);

		XMVECTOR NPCtoPlayerRightMat = XMVector3Normalize(XMVector3Cross(NPCtoPlayerUpTempMat, NPCtoPlayerLookMat));

		XMFLOAT3 NPCtoPlayerRight;
		XMStoreFloat3(&NPCtoPlayerRight, NPCtoPlayerRightMat);

		XMVECTOR NPCtoPlayerUpMat = XMVector3Normalize(XMVector3Cross(NPCtoPlayerLookMat, NPCtoPlayerRightMat));

		XMFLOAT3 NPCtoPlayerUp;
		XMStoreFloat3(&NPCtoPlayerUp, NPCtoPlayerUpMat);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float> ShackingAttackRange(-4, 4);

		float UpShaking = ShackingAttackRange(dre);
		float UpshakingDevide = UpShaking / distance;

		float RightShaking = ShackingAttackRange(dre);
		float RightshakingDevide = UpShaking / distance;

		XMVECTOR ShakeUPMatrix = XMVectorScale(XMLoadFloat3(&NPCtoPlayerUp), UpshakingDevide);
		XMVECTOR ShakeRightMatrix = XMVectorScale(XMLoadFloat3(&NPCtoPlayerRight), RightshakingDevide);

		XMFLOAT3 ShakeUpVec;
		XMStoreFloat3(&ShakeUpVec, ShakeUPMatrix);

		XMFLOAT3 ShakeRightVec;
		XMStoreFloat3(&ShakeRightVec, ShakeRightMatrix);

		XMVECTOR ShakeUpRightVec = XMVectorAdd(XMLoadFloat3(&ShakeUpVec), XMLoadFloat3(&ShakeRightVec));
		XMVECTOR ShakeMat = XMVectorAdd(ShakeUpRightVec, NPCtoPlayerLookNormal);
		XMStoreFloat3(&m_AttackVec, ShakeMat);

		m_AttackVec = NPCNormalize(m_AttackVec);
	}
}

// Army
void NPC::A_MoveToNode()
{
	NodeMesh currentNode = MeshInfo[m_currentNodeIndex];
	const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
	const float speed = m_Speed;
	XMFLOAT3 prevPos = pos;

	if (isMovingInZ) {
		const float destination = TurnBack ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;

		if (TurnBack) prevPos.z += speed;
		else prevPos.z -= speed;

		if (((TurnBack == true) && prevPos.z >= destination) || ((TurnBack == false) && prevPos.z <= destination))
		{
			TurnBack = !TurnBack;
			m_UpdateTurn = true;
		}
	}
	else {
		const float destination = TurnBack ? currentNode.GetSmallX() + m_destinationRange : currentNode.GetLargeX() - m_destinationRange;

		if (TurnBack) prevPos.x -= speed;
		else prevPos.x += speed;

		if (((TurnBack == true) && prevPos.x <= destination) || ((TurnBack == false) && prevPos.x >= destination))
		{
			TurnBack = !TurnBack;
			m_UpdateTurn = true;
		}
	}
	float t = 0.3f; // 보간 시간 (조정 가능)
	XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);

	if (NPC_CollideByOtherNPC()) {
		NPC_CalculationOtherCollide();
	}
	else if (NPC_CollideByMap()) {
		NPC_CalculationCollide();
	}
	else {
		pos = interpolatedPos;
		if (m_UpdateTurn) {
			int section = m_currentNodeIndex % 2;

			switch (section)
			{
			case 0:
			{
				if (TurnBack) {
					yaw = 0.0f;
				}
				else {
					yaw = 180.0f;
				}
				break;
			}
			case 1:
				if (TurnBack) {
					yaw = 270.0f;
				}
				else {
					yaw = 90.0f;
				}
				break;
			}
			m_rightvec = NPCcalcRightRotate();
			m_lookvec = NPCcalcLookRotate();

			if (ConnectingServer) {
				g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
			}

			m_UpdateTurn = false;
		}
	}
}
void NPC::A_UpdateCurrentNodeIndex()
{
	const int cityNum = m_currentNodeIndex / 4;
	const int nextSection = m_currentNodeIndex + 1;
	m_currentNodeIndex = (cityNum * 4) + (nextSection % 4);
	m_UpdateTurn = true;
}

bool NPC::A_IsUserOnSafeZone(int user_City)
{
	return (-1 < user_City < 5) ? false : true;
}
int NPC::A_GetUserCity()
{
	for (int i = 0; i < 5; ++i) {
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

	XMFLOAT3 prevPos = pos;
	m_PrevYaw = yaw;
	float destination;
	if (isMovingInZ) {
		destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;
		prevPos.z += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 0.0f : 180.0f;
		if (m_PrevYaw != yaw) m_UpdateTurn = true;

		if ((isMovingForward && prevPos.z >= destination) || (!isMovingForward && prevPos.z <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}
	else {
		destination = isMovingForward ? currentNode.GetLargeX() - m_destinationRange : currentNode.GetSmallX() + m_destinationRange;
		prevPos.x += isMovingForward ? speed : -speed;
		yaw = isMovingForward ? 90.0f : 270.0f;
		if (m_PrevYaw != yaw) m_UpdateTurn = true;

		if ((isMovingForward && prevPos.x >= destination) || (!isMovingForward && prevPos.x <= destination)) m_currentNodeIndex = nextNode.GetIndex();
	}
	float t = 0.3f; // 보간 시간 (조정 가능)
	XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);

	if (NPC_CollideByOtherNPC()) {
		NPC_CalculationOtherCollide();
	}
	else if (NPC_CollideByMap()) {
		NPC_CalculationCollide();
	}
	else {
		pos = interpolatedPos;
		if (m_UpdateTurn) {
			m_rightvec = NPCcalcRightRotate();
			m_lookvec = NPCcalcLookRotate();

			m_UpdateTurn = false;
			if (ConnectingServer) {
				g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
			}
		}
	}
}
void NPC::A_PlayerChasing()
{
	int user_City = A_GetUserCity();
	if (user_City == -1 || A_IsUserOnSafeZone(user_City)) {
		m_UpdateTurn = true;
		A_MoveToNode();
		m_state = NPC_IDLE;
		AttackTime = system_clock::now();
		ChaseTime = AttackTime;
		return;
	}

	int user_node = A_FindUserNode(user_City);
	if (user_node == -1) {
		m_UpdateTurn = true;
		A_MoveToNode();
		m_state = NPC_IDLE;
		AttackTime = system_clock::now();
		ChaseTime = AttackTime;
		return;
	}

	if (user_node == m_currentNodeIndex) {
		XMFLOAT3 prevPos = pos;
		XMFLOAT3 positionToUser = { m_User_Pos[m_chaseID].x - prevPos.x, m_User_Pos[m_chaseID].y - prevPos.y, m_User_Pos[m_chaseID].z - prevPos.z };
		XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

		XMFLOAT3 projectedPositionToUser = { positionToUser.x, 0.0f, positionToUser.z };
		NPCNormalize(projectedPositionToUser);

		float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(projectedPositionToUser.z, projectedPositionToUser.x);
		float angleDegree = XMConvertToDegrees(angleRadian);

		yaw = angleDegree;

		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		m_lookvec = NPCcalcLookRotate();
		m_rightvec = NPCcalcRightRotate();

		if (ConnectingServer) {
			g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
		}

		if (m_Distance[m_chaseID] >= 100.0f) {
			if (NPC_CollideByOtherNPC()) {
				NPC_CalculationOtherCollide();
			}
			else if (NPC_CollideByMap()) {
				NPC_CalculationCollide();
			}
			else {
				prevPos.x += m_lookvec.x * m_Speed;
				prevPos.z += m_lookvec.z * m_Speed;

				float t = 0.5f; // 보간 시간 (조정 가능)
				XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);
				pos = interpolatedPos;
			}
		}
		m_shooton = true;
	}
	else {
		path.clear();

		path = A_AStarSearch(m_currentNodeIndex, user_node, user_City);
		if (path.empty()) {
			A_MoveToNode();
		}
		else {
			m_targetNodeIndex = path[path.size() - 1];
			m_currentNodeIndex = path[0];
			A_IsPathMove();
		}
	}
}
vector<int> NPC::A_AStarSearch(int startNode, int targetNode, int user_City)
{
	const int m_NumTotalNode = MeshInfo.size();
	vector<float> gScore(m_NumTotalNode, numeric_limits<float>::infinity());
	vector<float> fScore(m_NumTotalNode, numeric_limits<float>::infinity());
	vector<Node*> cameFrom(m_NumTotalNode, nullptr);

	gScore[startNode] = 0.0f;
	fScore[startNode] = HeuristicEstimate(startNode, targetNode);

	priority_queue<Node> openSet;
	openSet.emplace(startNode, fScore[startNode], gScore[startNode], nullptr);

	vector<int> temppath;

	while (!openSet.empty()) {
		Node current = openSet.top();
		openSet.pop();

		if (current.index == targetNode) {
			Node* node = &current;
			while (node != nullptr) {
				temppath.emplace_back(node->index);
				node = node->parent;
			}
			reverse(temppath.begin(), temppath.end());
			for (Node* node : cameFrom) {
				delete node;
			}
			return temppath;
		}

		for (int neighborIndex : MeshInfo[current.index].neighbor_mesh) {
			float tentative_gScore = gScore[current.index] + DistanceBetween(current.index, neighborIndex);

			if (tentative_gScore < gScore[neighborIndex]) {
				delete cameFrom[neighborIndex];
				cameFrom[neighborIndex] = new Node(current);

				gScore[neighborIndex] = tentative_gScore;
				fScore[neighborIndex] = gScore[neighborIndex] + HeuristicEstimate(neighborIndex, targetNode);

				openSet.emplace(neighborIndex, fScore[neighborIndex], gScore[neighborIndex], cameFrom[neighborIndex]);
			}
		}
	}

	for (Node* node : cameFrom) {
		delete node;
	}
	return vector<int>();
}

bool NPC::A_PlayerDetact()
{
	XMVECTOR NPCPos = XMLoadFloat3(&pos);
	XMVECTOR NPCLook = XMLoadFloat3(&m_lookvec);
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	if (A_SetFrustum(NPCPos, NPCLook, PlayerPos))
	{
		return true;
	}

	return false;
}

bool NPC::A_SetFrustum(XMVECTOR startPoint, XMVECTOR lookVector, XMVECTOR playerPosition)
{
	XMVECTOR endPoint = startPoint + (lookVector * 200.0f);

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	BoundingFrustum frustum;
	XMStoreFloat3(&frustum.Origin, startPoint);
	XMMATRIX viewMatrix = XMMatrixLookAtLH(startPoint, endPoint, up);
	XMVECTOR PrintviewMat;
	PrintviewMat = viewMatrix.r[2];
	XMFLOAT3 Printview;
	XMStoreFloat3(&Printview, PrintviewMat);


	XMVECTOR quaternion = XMQuaternionRotationMatrix(viewMatrix);
	XMStoreFloat4(&frustum.Orientation, quaternion);
	frustum.RightSlope = 0.6f;
	frustum.LeftSlope = -0.6f;
	frustum.TopSlope = 0.2f;
	frustum.BottomSlope = -0.2f;
	frustum.Near = 0.1f;
	frustum.Far = 200.0f;

	if (frustum.Contains(playerPosition) != ContainmentType::DISJOINT)
	{
		return true;
	}

	return false;
}
void NPC::A_PlayerAttack()
{
	A_PlayerChasing();
	if (m_shooton) {
		XMFLOAT3 NPCtoPlayerLook = { m_lookvec.x, 0.0f, m_lookvec.z };
		float distance = m_Distance[m_chaseID];

		XMVECTOR NPCtoPlayerLookNormal = XMVector3Normalize(XMLoadFloat3(&NPCtoPlayerLook));
		XMStoreFloat3(&NPCtoPlayerLook, NPCtoPlayerLookNormal);

		XMFLOAT3 NPCtoPlayerUpTemp = { 0.0f, 1.0f, 0.0f };

		XMVECTOR NPCtoPlayerLookMat = XMLoadFloat3(&NPCtoPlayerLook);
		XMVECTOR NPCtoPlayerUpTempMat = XMLoadFloat3(&NPCtoPlayerUpTemp);

		XMVECTOR NPCtoPlayerRightMat = XMVector3Normalize(XMVector3Cross(NPCtoPlayerUpTempMat, NPCtoPlayerLookMat));

		XMFLOAT3 NPCtoPlayerRight;
		XMStoreFloat3(&NPCtoPlayerRight, NPCtoPlayerRightMat);

		XMVECTOR NPCtoPlayerUpMat = XMVector3Normalize(XMVector3Cross(NPCtoPlayerLookMat, NPCtoPlayerRightMat));

		XMFLOAT3 NPCtoPlayerUp;
		XMStoreFloat3(&NPCtoPlayerUp, NPCtoPlayerUpMat);

		random_device rd;
		default_random_engine dre(rd());
		uniform_real_distribution<float> ShackingAttackRange(-5, 5);

		float UpShaking = ShackingAttackRange(dre);
		float UpshakingDevide = UpShaking / distance;

		float RightShaking = ShackingAttackRange(dre);
		float RightshakingDevide = UpShaking / distance;

		XMVECTOR ShakeUPMatrix = XMVectorScale(XMLoadFloat3(&NPCtoPlayerUp), UpshakingDevide);
		XMVECTOR ShakeRightMatrix = XMVectorScale(XMLoadFloat3(&NPCtoPlayerRight), RightshakingDevide);

		XMFLOAT3 ShakeUpVec;
		XMStoreFloat3(&ShakeUpVec, ShakeUPMatrix);

		XMFLOAT3 ShakeRightVec;
		XMStoreFloat3(&ShakeRightVec, ShakeRightMatrix);

		XMVECTOR ShakeUpRightVec = XMVectorAdd(XMLoadFloat3(&ShakeUpVec), XMLoadFloat3(&ShakeRightVec));
		XMVECTOR ShakeMat = XMVectorAdd(ShakeUpRightVec, NPCtoPlayerLookNormal);
		XMStoreFloat3(&m_AttackVec, ShakeMat);

		m_AttackVec = NPCNormalize(m_AttackVec);
		//cout << id << "번째 NPC의 m_AttackVec x: " << m_AttackVec.x << ", y: " << m_AttackVec.y << ", z: " << m_AttackVec.z << endl;
	}
}

bool NPC::NPC_CollideByMap()
{
	bool collide = false;
	m_collideBox.clear();

	for (int i{}; i < mapobjects_info.size(); ++i) {
		float dist_x = pos.x - mapobjects_info[i].getPos().x;
		float dist_y = pos.y - mapobjects_info[i].getPos().y;
		float dist_z = pos.z - mapobjects_info[i].getPos().z;
		float dist = sqrtf(powf(dist_x, 2) + powf(dist_y, 2) + powf(dist_z, 2));

		if (dist <= 300.f) {	// 먼거리는 충돌X
			if (mapobjects_info[i].m_xoobb.Intersects(m_xoobb)) {
				m_collideBox.emplace_back(mapobjects_info[i]);
				collide = true;
			}
		}
	}
	return collide;
}

bool NPC::NPC_CollideByOtherNPC()
{
	bool collide = false;
	for (int i{}; i < MAX_NPCS; ++i) {
		if (i == id) continue;
		if (npcsInfo[i].m_xoobb.Intersects(m_xoobb)) {
			collide = true;
			m_otherNPCPos = npcsInfo[i].GetPosition();
			break;
		}
	}
	return collide;
}

bool NPC::NPC_BulletRaycast()
{
	//cout << id << "번째 NPC의 공격 횟수: " << m_attackcount << endl;
	m_attackcount++;
	for (auto collideCheck : m_objectlist) {
		RaycastResult result = Raycast(mapobjects_info[collideCheck].getPos(), m_AttackVec, mapobjects_info[collideCheck].m_xoobb);
		if (result.hit) {
			//cout << "충돌 위치, x: " << result.hitPoint.x << ", y: " << result.hitPoint.y << ", z: " << result.hitPoint.z << endl;
			return true;
		}
	}
	return false;
}

void NPC::NPC_CalculationCollide()
{
	if (m_collideBox.size() >= 2) {
		XMFLOAT3 totalVector = { 0.0f, 0.0f, 0.0f };

		XMFLOAT3 prevPos = pos;
		for (int i{}; i < m_collideBox.size(); ++i) {
			XMFLOAT3 localforwark = m_collideBox[i].getLocalForward();
			XMFLOAT3 localright = m_collideBox[i].getLocalRight();

			XMFLOAT3 normalizedLocalForward;
			XMVECTOR localForwardNormalized = XMVector3Normalize(XMLoadFloat3(&localforwark));
			XMStoreFloat3(&normalizedLocalForward, localForwardNormalized);

			XMFLOAT3 normalizedLocalRight;
			XMVECTOR localRightNormalized = XMVector3Normalize(XMLoadFloat3(&localright));
			XMStoreFloat3(&normalizedLocalRight, localRightNormalized);


			XMFLOAT3 Center2NPCPos = Subtract(prevPos, m_collideBox[i].m_xoobb.Center);//벡터
			Center2NPCPos = Normalize(Center2NPCPos);

			float forwardDotResult = DotProduct(Center2NPCPos, localforwark); //객체의 center와 플레이어와 normal간의 cos값   
			float rightDotResult = DotProduct(Center2NPCPos, localright);

			float forwardDotResultAbs = abs(forwardDotResult);
			float rightDotResultAbs = abs(rightDotResult);

			float angle_a = m_collideBox[i].getAngleAOB();
			float radian = XMConvertToRadians(angle_a / 2);

			XMFLOAT3 NPCMoveDir;

			if (abs(cos(radian)) < forwardDotResultAbs) {
				if (forwardDotResult < 0) {
					XMVECTOR reversedLocalForward = XMVectorNegate(XMLoadFloat3(&normalizedLocalForward));
					XMStoreFloat3(&normalizedLocalForward, reversedLocalForward);

					NPCMoveDir = Normalize(normalizedLocalForward);
					
				}
				else {
					NPCMoveDir = Normalize(normalizedLocalForward);
				}
			}
			else {
				if (rightDotResult < 0) {
					XMVECTOR reversedLocalRight = XMVectorNegate(XMLoadFloat3(&normalizedLocalRight));
					XMStoreFloat3(&normalizedLocalRight, reversedLocalRight);
					NPCMoveDir = Normalize(normalizedLocalRight);
				}
				else {
					NPCMoveDir = Normalize(normalizedLocalRight);
				}
			}
			
			XMStoreFloat3(&totalVector, XMVectorAdd(XMLoadFloat3(&totalVector), XMLoadFloat3(&NPCMoveDir)));
		}
		totalVector = Normalize(totalVector);
		XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

		float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(totalVector.z, totalVector.x);
		float angleDegree = XMConvertToDegrees(angleRadian);
		yaw = angleDegree;

		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		m_lookvec = NPCcalcLookRotate();
		m_rightvec = NPCcalcRightRotate();

		prevPos.x += m_lookvec.x * m_Speed;
		prevPos.z += m_lookvec.z * m_Speed;

		float t = 0.8f; // 보간 시간 (조정 가능)
		XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);
		if (ConnectingServer) {
			g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
		}

		pos = interpolatedPos;

		m_UpdateTurn = true;
		m_collideBox.clear();
	}
	else {
		XMFLOAT3 prevPos = pos;

		XMFLOAT3 Center2NPCPos = Subtract(prevPos, m_collideBox[0].m_xoobb.Center);//벡터
		Center2NPCPos = Normalize(Center2NPCPos);

		
		XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

		float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(Center2NPCPos.z, Center2NPCPos.x);
		float angleDegree = XMConvertToDegrees(angleRadian);
		yaw = angleDegree;

		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		m_lookvec = NPCcalcLookRotate();
		m_rightvec = NPCcalcRightRotate();

		prevPos.x += m_lookvec.x * m_Speed;
		prevPos.z += m_lookvec.z * m_Speed;

		float t = 0.8f; // 보간 시간 (조정 가능)
		XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);
		if (ConnectingServer) {
			g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
		}

		pos = interpolatedPos;

		m_UpdateTurn = true;
		m_collideBox.clear();
	}
}

void NPC::NPC_CalculationOtherCollide()
{
	XMFLOAT3 prevPos = pos;

	XMFLOAT3 positionToOtherNPC = { m_otherNPCPos.x - prevPos.x, m_otherNPCPos.y - prevPos.y, m_otherNPCPos.z - prevPos.z };
	XMFLOAT3 DefaultTemp = { 0.0f, 0.0f, 1.0f };

	XMVECTOR PositionToOtherNPCnormal = XMVector3Normalize(XMLoadFloat3(&positionToOtherNPC));
	XMVECTOR reversedToOtherNPC = XMVectorNegate(PositionToOtherNPCnormal);
	XMVECTOR ProjCollideMat = XMVectorAdd(reversedToOtherNPC, XMLoadFloat3(&m_lookvec));
	ProjCollideMat = XMVector3Normalize(ProjCollideMat);
	XMFLOAT3 ProjCollideNormal;
	XMStoreFloat3(&ProjCollideNormal, ProjCollideMat);

	float angleRadian = atan2(DefaultTemp.z, DefaultTemp.x) - atan2(ProjCollideNormal.z, ProjCollideNormal.x);
	float angleDegree = XMConvertToDegrees(angleRadian);
	yaw = angleDegree;

	if (yaw > 360.0f)
		yaw -= 360.0f;
	if (yaw < 0.0f)
		yaw += 360.0f;

	m_lookvec = NPCcalcLookRotate();
	m_rightvec = NPCcalcRightRotate();

	prevPos.x += m_lookvec.x * m_Speed;
	prevPos.z += m_lookvec.z * m_Speed;

	float t = 0.5f; // 보간 시간 (조정 가능)
	XMFLOAT3 interpolatedPos = Lerp(pos, prevPos, t);

	if (ConnectingServer) {
		g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
	}

	pos = interpolatedPos;

	m_UpdateTurn = true;
}

void NPC::NPC_BackOwnsPos()
{
	if (path[0] == m_OriginNodeIndex || m_currentNodeIndex == m_OriginNodeIndex) {
		m_state = NPC_IDLE;
	}
	else {
		NodeMesh currentNode = MeshInfo[path[0]];
		NodeMesh nextNode = MeshInfo[path[1]];
		const bool isMovingInZ = currentNode.GetMoveingSpaceZ();
		bool isMovingForward = nextNode.GetIndex() % 4 < 2;

		if (currentNode.GetIndex() / 4 > 3) {
			if (isMovingInZ) {
				isMovingForward = nextNode.GetIndex() % 8 < 4;
			}
			else {
				isMovingForward = nextNode.GetIndex() < 8;
			}
		}
		const float speed = m_Speed;

		XMFLOAT3 prevPos = pos;
		m_PrevYaw = yaw;
		float destination;
		if (isMovingInZ) {
			destination = isMovingForward ? currentNode.GetLargeZ() - m_destinationRange : currentNode.GetSmallZ() + m_destinationRange;
			pos.z += isMovingForward ? speed : -speed;
			yaw = isMovingForward ? 0.0f : 180.0f;
			if (m_PrevYaw != yaw) m_UpdateTurn = true;

			if ((isMovingForward && pos.z >= destination) || (!isMovingForward && pos.z <= destination)) {
				m_currentNodeIndex = nextNode.GetIndex();
				path.erase(path.begin());
			}
		}
		else {
			destination = isMovingForward ? currentNode.GetLargeX() - m_destinationRange : currentNode.GetSmallX() + m_destinationRange;
			pos.x += isMovingForward ? speed : -speed;
			yaw = isMovingForward ? 90.0f : 270.0f;
			if (m_PrevYaw != yaw) m_UpdateTurn = true;

			if ((isMovingForward && pos.x >= destination) || (!isMovingForward && pos.x <= destination)) {
				m_currentNodeIndex = nextNode.GetIndex();
				path.erase(path.begin());
			}
		}
		float t = 0.3f; // 보간 시간 (조정 가능)
		XMFLOAT3 interpolatedPos = Lerp(prevPos, pos, t);

		if (m_UpdateTurn) {
			m_rightvec = NPCcalcRightRotate();
			m_lookvec = NPCcalcLookRotate();

			m_UpdateTurn = false;
			if (ConnectingServer) {
				g_logicservers[a_lgcsvr_num].send_npc_rotate_packet(this->id);
			}
		}
		pos = interpolatedPos;
	}
}

void NPC::NPC_SetObjectList()
{
	for (int i{}; i < mapobjects_info.size(); ++i) {
		XMFLOAT3 O_pos = mapobjects_info[i].getPos();
		XMFLOAT3 NtoO_vec = Subtract(pos, O_pos);
		float NtoODistance = Length(NtoO_vec);
		if (NtoODistance < 200.0f) {
			if (!m_objectlist.count(i)) {
				m_objectlist.insert(i);
			}
		}
		else {
			if (m_objectlist.count(i)) {
				m_objectlist.erase(i);
			}
		}
	}
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
		playersInfo[client_id].role = login_packet->role;

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

		if (!ClientConnected) ClientConnected = true;
		/*
		cout << "[Add New Player] Player[ID:" << client_id << ", Name:" << playersInfo[client_id].name << ", Role: " << playersInfo[client_id].role <<"]의 정보를 받았습니다." << endl;
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
		int recv_id = remove_packet->id;
		short recv_target = remove_packet->target;
		if (recv_target == TARGET_PLAYER) {
			playersInfo[recv_id].memberClear();
		}
		else if (recv_target == TARGET_NPC) {
			npcsInfo[recv_id].state = /*NPC_DEATH*/PL_ST_DEAD;
		}

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
			if (playersInfo[obj_id].hp < 0) playersInfo[obj_id].hp = 0;
			if (playersInfo[obj_id].hp > 100) playersInfo[obj_id].hp = 100;
			playersInfo[obj_id].obj_lock.unlock();
		}
		else if (damaged_packet->target == TARGET_NPC) {
			npcsInfo[obj_id].obj_lock.lock();
			npcsInfo[obj_id].hp -= damaged_packet->damage;
			if (npcsInfo[obj_id].hp < 0)
			{
				npcsInfo[obj_id].hp = 0;
				npcsInfo[obj_id].SetState(NPC_DEATH);
			}
			npcsInfo[obj_id].obj_lock.unlock();
		}

		break;
	}// SC_DAMAGED end
	case SC_RESPAWN:
	{
		SC_RESPAWN_PACKET* respawn_packet = reinterpret_cast<SC_RESPAWN_PACKET*>(packet);

		int obj_id = respawn_packet->id;
		playersInfo[obj_id].obj_lock.lock();

		playersInfo[obj_id].hp = 100;

		playersInfo[obj_id].pos.x = respawn_packet->x;
		playersInfo[obj_id].pos.y = respawn_packet->y;
		playersInfo[obj_id].pos.z = respawn_packet->z;

		playersInfo[obj_id].m_rightvec.x = respawn_packet->right_x;
		playersInfo[obj_id].m_rightvec.y = respawn_packet->right_y;
		playersInfo[obj_id].m_rightvec.z = respawn_packet->right_z;

		playersInfo[obj_id].m_upvec.x = respawn_packet->up_x;
		playersInfo[obj_id].m_upvec.y = respawn_packet->up_y;
		playersInfo[obj_id].m_upvec.z = respawn_packet->up_z;

		playersInfo[obj_id].m_lookvec.x = respawn_packet->look_x;
		playersInfo[obj_id].m_lookvec.y = respawn_packet->look_y;
		playersInfo[obj_id].m_lookvec.z = respawn_packet->look_z;

		playersInfo[obj_id].obj_lock.unlock();

		break;
	}// SC_RESPAWN end
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

		break;
	}// SC_OBJECT_STATE end
	case SC_MAP_OBJINFO:
	{
		SC_MAP_OBJINFO_PACKET* mapobj_packet = reinterpret_cast<SC_MAP_OBJINFO_PACKET*>(packet);

		MapObject temp;
		temp.setPos(mapobj_packet->center_x, mapobj_packet->center_y, mapobj_packet->center_z);
		temp.setScale(mapobj_packet->scale_x, mapobj_packet->scale_y, mapobj_packet->scale_z);
		temp.setLocalForward(XMFLOAT3{ mapobj_packet->forward_x, mapobj_packet->forward_y, mapobj_packet->forward_z });
		temp.setLocalRight(XMFLOAT3{ mapobj_packet->right_x, mapobj_packet->right_y, mapobj_packet->right_z });
		temp.setLocalRotate(XMFLOAT3{ mapobj_packet->rotate_x, mapobj_packet->rotate_y, mapobj_packet->rotate_z });
		temp.setAngleAOB(mapobj_packet->aob);
		temp.setAngleBOC(mapobj_packet->boc);
		temp.setBB();
		mapobjects_info.push_back(temp);
	}
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
				// NPC서버는 비동기 Connect를 보내는 곳이 두군데이다! (로직서버, 수평확장NPC서버)
				// 1. Logic Server Error
				if (PORTNUM_LGCNPC_0 <= key && key <= PORTNUM_LGCNPC_1) {
					// 서버번호를 바꿔가면서 비동기Connect를 재시도합니다.
					if (a_lgcsvr_num == 0)		a_lgcsvr_num = 1;
					else if (a_lgcsvr_num == 1)	a_lgcsvr_num = 0;
					int new_portnum = a_lgcsvr_num + PORTNUM_LGCNPC_0;
					//cout << "[ConnectEX Failed] ";
					//cout << "Logic Server[" << a_lgcsvr_num << "] (PORTNUM:" << new_portnum << ")로 다시 연결합니다. \n" << endl;

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
					// 1. 메인서버와 NPC서버가 루프백에서 동작할 때
					inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

					// 2. 메인서버와 NPC서버가 다른 PC에서 동작할 떄
					/*
					if (a_lgcsvr_num == 0) {
						inet_pton(AF_INET, IPADDR_LOGIC0, &logic_server_addr.sin_addr);
					}
					else if (a_lgcsvr_num == 1) {
						inet_pton(AF_INET, IPADDR_LOGIC1, &logic_server_addr.sin_addr);
					}
					*/

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
				// 2. Ex_Server Error
				else if (CP_KEY_EX_NPC <= key && key <= CP_KEY_EX_NPC_LISTEN) {
					cout << "EX SERVER ERROR" << endl;

					// 비동기Conn를 다시 시도합니다.
					SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
					GUID guid = WSAID_CONNECTEX;
					DWORD bytes = 0;
					LPFN_CONNECTEX connectExFP;
					::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
					closesocket(temp_s);

					SOCKADDR_IN ha_server_addr;
					ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
					ha_server_addr.sin_family = AF_INET;
					right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
					int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
					if (ret != 0) {
						cout << "Bind Error - " << ret << endl;
						cout << WSAGetLastError() << endl;
						exit(-1);
					}

					OVER_EX* con_over = new OVER_EX;
					con_over->process_type = OP_CONNECT;
					HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
					if (NULL == hret) {
						cout << "CreateIoCompletoinPort Error - " << ret << endl;
						cout << WSAGetLastError() << endl;
						exit(-1);
					}

					int target_portnum = key - CP_KEY_EX_NPC + HA_PORTNUM_NPC0;
					ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
					ha_server_addr.sin_family = AF_INET;
					ha_server_addr.sin_port = htons(target_portnum);
					inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

					BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
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

			}
			else {
				// 1. Logic Server
				if (PORTNUM_LGCNPC_0 <= key && key <= PORTNUM_LGCNPC_1) {
					disconnect(static_cast<int>(key - PORTNUM_LGCNPC_0), SESSION_LOGIC);
					if (ex_over->process_type == OP_SEND) delete ex_over;
				}
				// 2. EX_Server
				else if (CP_KEY_EX_NPC <= key && key < CP_KEY_EX_NPC_LISTEN) {
					disconnect(static_cast<int>(key), SESSION_LOGIC);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			if (key == CP_KEY_EX_NPC_LISTEN) {
				SOCKET extended_server_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				left_ex_server_sock = extended_server_socket;
				int new_id = find_empty_extended_server();
				if (new_id != -1) {
					cout << "NPC Sever[" << new_id << "]의 연결요청을 받았습니다.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_EX_NPC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_iocp, new_key, 0);
					extended_servers[new_id].do_recv();
					extended_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "다른 Sever의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(extended_server_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_ss_listensock, extended_server_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
		}
		case OP_RECV: {
			// 1. Logic Server
			if (PORTNUM_LGCNPC_0 <= key && key <= PORTNUM_LGCNPC_1) {
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
			}
			// 2. EX_Server
			else if (CP_KEY_EX_NPC <= key && key < CP_KEY_EX_NPC_LISTEN) {
				if (0 == num_bytes) disconnect(key, SESSION_NPC);
				int server_id = key - CP_KEY_EX_NPC;

				int remain_data = num_bytes + extended_servers[server_id].remain_size;
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
				extended_servers[server_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				extended_servers[server_id].do_recv();
				break;
			}

			break;
		}//OP_RECV end
		case OP_SEND: {
			//if (0 == num_bytes) disconnect(key - CP_KEY_LOGIC2CLIENT, SESSION_CLIENT);
			delete ex_over;
			break;
		}//OP_SEND end
		case OP_CONNECT: {
			if (FALSE != ret) {
				// 1. Logic Server
				if (PORTNUM_LGCNPC_0 <= key && key <= PORTNUM_LGCNPC_1) {
					int server_id = key - PORTNUM_LGCNPC_0;
					std::cout << "성공적으로 Logic Server[" << server_id << "]에 연결되었습니다.\n" << endl;
					g_logicservers[a_lgcsvr_num].remain_size = 0;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_logicservers[a_lgcsvr_num].sock), h_iocp, NULL, 0);
					delete ex_over;
					g_logicservers[a_lgcsvr_num].do_recv();
					initNpc();
					ConnectingServer = true;
				}
				// 2. EX_Server
				else if (CP_KEY_EX_NPC <= key && key < CP_KEY_EX_NPC_LISTEN) {
					int server_id = key - CP_KEY_EX_NPC;
					std::cout << "성공적으로 Server[" << server_id << "]에 연결되었습니다.\n" << endl;
					extended_servers[server_id].id = server_id;
					extended_servers[server_id].remain_size = 0;
					extended_servers[server_id].socket = right_ex_server_sock;
					extended_servers[server_id].s_state = ST_ACCEPTED;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, NULL, 0);
					delete ex_over;
					extended_servers[server_id].do_recv();
				}				
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
		npcsInfo[i].SetOriginNodeIndex(npcsInfo[i].GetNodeIndex());
		npcsInfo[i].path.clear();
		npcsInfo[i].SetTargetNodeIndex(-1);
		npcsInfo[i].SetHp(300);
		npcsInfo[i].CurrTime = chrono::system_clock::now();
		npcsInfo[i].PrevTime = npcsInfo[i].CurrTime;
		g_logicservers[a_lgcsvr_num].send_npc_init_packet(npc_id);
	}

	if (HelicopterNum != MAX_NPCS) {
		for (int i = HelicopterNum; i < MAX_NPCS; i++) {
			int npc_id = i;
			npcsInfo[i].SetID(npc_id);
			npcsInfo[i].type = NPC_ARMY;
			random_device rd;
			default_random_engine dre(rd());

			uniform_int_distribution<int>idx(0, 15);
			npcsInfo[i].SetNodeIndex(idx(dre));

			float x = npcsInfo[i].getRandomOffset(MeshInfo[npcsInfo[i].GetNodeIndex()].GetSmallX(),
				MeshInfo[npcsInfo[i].GetNodeIndex()].GetLargeX());
			float y = 6.0f;
			float z = npcsInfo[i].getRandomOffset(MeshInfo[npcsInfo[i].GetNodeIndex()].GetSmallZ(),
				MeshInfo[npcsInfo[i].GetNodeIndex()].GetLargeZ());

			XMFLOAT3 tpos = { x,y,z };
			npcsInfo[i].SetPosition(tpos);

			npcsInfo[i].SetRotate(0.0f, 0.0f, 0.0f);

			uniform_int_distribution<int>drange(0.0f, 20.0f);
			npcsInfo[i].SetDestinationRange(drange(dre));

			uniform_real_distribution<float>SpdSet(1.2f, 2.5f);
			float speed = SpdSet(dre);
			npcsInfo[i].SetSpeed(speed);
			npcsInfo[i].SetChaseID(-1);
			npcsInfo[i].SetOriginNodeIndex(npcsInfo[i].GetNodeIndex());
			npcsInfo[i].path.clear();
			npcsInfo[i].SetTargetNodeIndex(-1);
			npcsInfo[i].SetHp(100);

			npcsInfo[i].CurrTime = chrono::system_clock::now();
			npcsInfo[i].PrevTime = npcsInfo[i].CurrTime;
			g_logicservers[a_lgcsvr_num].send_npc_init_packet(npc_id);
		}
	}
}

//======================================================================
void MoveNPC()
{
	while (true) {
		auto start_t = system_clock::now();
		//======================================================================
		if (ConnectingServer && ClientConnected) {
			for (int i = 0; i < MAX_NPCS; ++i) {
				// 클라이언트들과 NPC 사이의 거리 계산
				if (npcsInfo[i].GetState() != NPC_DEATH)
				{
					float temp_min = numeric_limits<float>::infinity();
					int temp_id = -1;

					for (auto& cl : playersInfo) {
						if (cl.id != -1 && cl.hp > 0) {
							if (cl.role == ROLE_RIFLE && npcsInfo[i].type == NPC_ARMY) {
								npcsInfo[i].Caculation_Distance(cl.pos, cl.id);
								// 가장 가까운 거리를 갖고있는 아이를 chase_id로 지정
								float distance = npcsInfo[i].GetDistance(cl.id);
								if (temp_min > distance && distance < 200.0f) {
									temp_min = distance;
									temp_id = cl.id;
								}
							}
							else if (npcsInfo[i].type == NPC_HELICOPTER) {
								npcsInfo[i].Caculation_Distance(cl.pos, cl.id);
								// 가장 가까운 거리를 갖고있는 아이를 chase_id로 지정
								float distance = npcsInfo[i].GetDistance(cl.id);
								if (temp_min > distance && distance < 200.0f) {
									temp_min = distance;
									temp_id = cl.id;
								}
							}
						}
					}

					npcsInfo[i].SetChaseID(temp_id);

					npcsInfo[i].NPC_State_Manegement(npcsInfo[i].GetState());
					// NPC가 추적하려는 아이디가 있는지부터 확인, 있으면 추적 대상 플레이어 좌표를 임시 저장
					if (npcsInfo[i].GetChaseID() != -1) {
						npcsInfo[i].SetUser_Pos(playersInfo[npcsInfo[i].GetChaseID()].pos, npcsInfo[i].GetChaseID());
					}

					// 메인서버로 변경된 NPC좌표 전달
					g_logicservers[a_lgcsvr_num].send_npc_move_packet(npcsInfo[i].GetID());

					if (npcsInfo[i].GetState() == NPC_ATTACK && npcsInfo[i].m_shooton) {
						if (npcsInfo[i].type == NPC_HELICOPTER) {
							npcsInfo[i].CurrTime = system_clock::now();
							npcsInfo[i].m_shooton = false;
							if (npcsInfo[i].CurrTime - npcsInfo[i].PrevTime > 300ms) {
								if (!npcsInfo[i].NPC_BulletRaycast()) {
									g_logicservers[a_lgcsvr_num].send_npc_attack_packet(npcsInfo[i].GetID());
									npcsInfo[i].PrevTime = npcsInfo[i].CurrTime;
									/*	cout << i << "번째 NPC가" << npcsInfo[i].GetChaseID() << " 플레이어에게 공격하였습니다." << endl;
										cout << i << "번째 NPC Pos x: " << npcsInfo[i].GetPosition().x << ", y: " << npcsInfo[i].GetPosition().y << ", z: " << npcsInfo[i].GetPosition().z << endl;
										cout << i << "번쨰 NPC Node: " << npcsInfo[i].GetNodeIndex() << endl;
									*/
								}
							}
						}
						else {
							npcsInfo[i].CurrTime = system_clock::now();
							npcsInfo[i].m_shooton = false;
							if (npcsInfo[i].CurrTime - npcsInfo[i].PrevTime > 700ms) {
								if (!npcsInfo[i].NPC_BulletRaycast()) {
									g_logicservers[a_lgcsvr_num].send_npc_attack_packet(npcsInfo[i].GetID());
									npcsInfo[i].PrevTime = npcsInfo[i].CurrTime;
									//cout << i << "번째 NPC Attack Vec x: " << npcsInfo[i].GetAttackVec().x <<  ", y: " << npcsInfo[i].GetAttackVec().y << ", z: " << npcsInfo[i].GetAttackVec().z << endl;
								/*	cout << i << "번째 NPC가" << npcsInfo[i].GetChaseID() << " 플레이어에게 공격하였습니다." << endl;
									cout << i << "번째 NPC Pos x: " << npcsInfo[i].GetPosition().x << ", y: " << npcsInfo[i].GetPosition().y << ", z: " << npcsInfo[i].GetPosition().z << endl;
									cout << i << "번쨰 NPC Node: " << npcsInfo[i].GetNodeIndex() << endl;
								*/
								}
							}
						}
					}
				}
			}
			//======================================================================
			auto curr_t = system_clock::now();
			if (curr_t - start_t < 16ms)
				this_thread::sleep_for(16ms - (curr_t - start_t));
		}
	}
}
//======================================================================
int main(int argc, char* argv[])
{
	last_send_checkpos_time = system_clock::now();	// 서버 시작시간

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================	
	// [ HA - 서버 ID, 포트번호 지정 ]
	// 어떤 서버를 가동할 것인지를 명령행 인수로 입력받아서 그 서버에 포트 번호를 부여합니다.
	my_server_id = 0;		// 서버 구분을 위한 지정번호
	int ss_portnum = -1;	// 서버 간 통신용 포트번호
	if (argc > 1) {			// 입력된 명령행 인수가 있을 때 (주로 서버다운으로 인한 서버 재실행때 사용됨)
		// Serve ID지정
		my_server_id = atoi(argv[1]) % 10;

		// Active 여부 결정
		int is_active = atoi(argv[1]) / 10;	// 십의자리가 1: Standby, 2: Active
		if (is_active == 0) {	// 서버의 첫 실행은 ID에 따라 구분
			if (my_server_id == 0) {
				b_active_server = false;
			}
			else if (my_server_id == 1) {
				b_active_server = true;
			}
		}
		else if (is_active == 1) {	// 강제 Standby모드 실행
			b_active_server = false;
		}
		else if (is_active == 2) {	// 강제 Active모드 실행
			b_active_server = true;
		}
		else {
			cout << "[Server ID Error] Unknown ID.\n" << endl;
			return -1;
		}
	}
	else {				// 만약 입력된 명령행 인수가 없다면 입력을 받습니다.
		cout << "실행할 서버: ";
		cin >> my_server_id;

		// Active 여부 결정
		switch (my_server_id) {
		case 0:	// 0번 서버
			b_active_server = false;
			break;
		case 1:	// 1번 서버
			b_active_server = true;
			break;
		case 10:	// 0번 서버 (강제 Standby)
		case 11:	// 1번 서버 (강제 Standby)
			b_active_server = false;
			break;
		case 20:	// 0번 서버 (강제 Active)
		case 21:	// 1번 서버 (강제 Active)
			b_active_server = true;
			break;
		default:
			cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
			return 0;
		}
	}

	// 서버번호에 따라 포트번호를 지정해줍니다.
	switch (my_server_id % 10) {
	case 0:	// 0번 서버
		ss_portnum = HA_PORTNUM_NPC0;
		break;
	case 1:	// 1번 서버
		ss_portnum = HA_PORTNUM_NPC1;
		break;
	default:
		cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
		return 0;
	}
	cout << "NPC Server[" << my_server_id << "] 가 가동되었습니다. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else				 cout << "Stand-By";
	cout << " / S - S PORT : " << ss_portnum << " ]" << endl;

	//======================================================================
	// [ HA - 서버 수평확장 ]
	// HA Listen Socket (서버 간 통신을 위한 Listen소켓)
	g_ss_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN ha_server_addr;
	memset(&ha_server_addr, 0, sizeof(ha_server_addr));
	ha_server_addr.sin_family = AF_INET;
	ha_server_addr.sin_port = htons(ss_portnum);
	ha_server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_ss_listensock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(ha_server_addr));
	listen(g_ss_listensock, SOMAXCONN);
	SOCKADDR_IN ha_addr;
	int ha_addr_size = sizeof(ha_addr);

	// HA Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ss_listensock), h_iocp, CP_KEY_EX_NPC_LISTEN, 0);
	right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX ha_over;
	ha_over.process_type = OP_ACCEPT;
	ha_over.wsabuf.buf = reinterpret_cast<CHAR*>(right_ex_server_sock);
	AcceptEx(g_ss_listensock, right_ex_server_sock, ha_over.send_buf, 0, ha_addr_size + 16, ha_addr_size + 16, 0, &ha_over.overlapped);

	// 수평확장된 서버의 마지막 구성원이 아니라면, 오른쪽에 있는 서버에 비동기connect 요청을 보냅니다.
	if (my_server_id < MAX_NPC_SERVER - 1) {
		int right_servernum = my_server_id + 1;
		int right_portnum = ss_portnum + 1;
		cout << "다른 이중화 서버(Server[" << right_servernum << "] (S-S PORT: " << right_portnum << ")에 비동기Connect를 요청합니다." << endl;

		// ConnectEx
		SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		GUID guid = WSAID_CONNECTEX;
		DWORD bytes = 0;
		LPFN_CONNECTEX connectExFP;
		::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
		closesocket(temp_s);

		SOCKADDR_IN ha_server_addr;
		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
		int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
		if (ret != 0) {
			cout << "Bind Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		OVER_EX* con_over = new OVER_EX;
		con_over->process_type = OP_CONNECT;
		int key_num = CP_KEY_EX_NPC + right_servernum;
		HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key_num, 0);
		if (NULL == hret) {
			cout << "CreateIoCompletoinPort Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		ha_server_addr.sin_port = htons(right_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버

		// 루프백
		inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

		BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
		if (FALSE == bret) {
			int err_no = GetLastError();
			if (ERROR_IO_PENDING == err_no)
				cout << "Server Connect 시도 중...\n" << endl;
			else {
				cout << "ConnectEX Error - " << err_no << endl;
				cout << WSAGetLastError() << endl;
			}
		}
	}
	else {
		cout << "마지막 서버구성원이므로 Connect를 수행하지않습니다.\n" << endl;
	}
	extended_servers[my_server_id].id = my_server_id;
	extended_servers[my_server_id].s_state = ST_ACCEPTED;

	//======================================================================
	// [ Main - 로직서버로 비동기 Connect 요청 ]
	ConnectingServer = false;
	if (b_active_server) {
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
		inet_pton(AF_INET, IPADDR_LOOPBACK, &logic_server_addr.sin_addr);

		// 2. 메인서버와 NPC서버가 다른 PC에서 동작할 떄
		/*
		if (a_lgcsvr_num == 0) {
			inet_pton(AF_INET, IPADDR_LOGIC0, &logic_server_addr.sin_addr);
		}
		else if (a_lgcsvr_num == 1) {
			inet_pton(AF_INET, IPADDR_LOGIC1, &logic_server_addr.sin_addr);
		}*/
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
		if (i % 2 == 0) {
			MeshInfo[i].SetMoveingSpace(false, true);
		}
		else {
			MeshInfo[i].SetMoveingSpace(true, false);
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
	/*initNpc();*/

	//======================================================================
	//						  Threads Initialize
	//======================================================================
	vector<thread> worker_threads;
	for (int i = 0; i < 5; ++i)
		worker_threads.emplace_back(do_worker);			// 메인서버-npc서버 통신용 Worker스레드

	vector<thread> timer_threads;
	timer_threads.emplace_back(MoveNPC);


	for (auto& th : worker_threads)
		th.join();

	for (auto& th : timer_threads)
		th.join();

	//closesocket(g_sc_listensock);
	WSACleanup();
}

void disconnect(int target_id, int target)
{
	switch (target) {
	case SESSION_LOGIC:
	{

	}
	case SESSION_NPC:
	{
		int session_id = target_id - CP_KEY_EX_NPC;
		extended_servers[session_id].s_lock.lock();
		if (extended_servers[session_id].s_state == ST_FREE) {
			extended_servers[session_id].s_lock.unlock();
			return;
		}
		closesocket(extended_servers[session_id].socket);
		extended_servers[session_id].s_state = ST_FREE;
		extended_servers[session_id].s_lock.unlock();

		cout << "NPC Server[" << extended_servers[session_id].id << "]의 다운이 감지되었습니다." << endl;	// server message

		// 서버 재실행
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + session_id);	// 십의자리: Actvie여부(S: 1, A: 2), 일의자리: 서버ID

		// XD폴더 내에서 동작할 때(내부 테스트)와 외부에서 실행할 때를 구분해줍니다.
		string XDFolderKeyword = "XD";
		if (filesystem::current_path().string().find(XDFolderKeyword) != string::npos) {
			ShellExecute(NULL, L"open", L"NpcServer.exe", wchar_buf, L"./x64/Release", SW_SHOW);	// 내부 테스트용
		}
		else {
			ShellExecute(NULL, L"open", L"NpcServer.exe", wchar_buf, L".", SW_SHOW);					// 외부 수출용 (exe로 실행될때)
		}

		// 클라이언트에게 Active서버가 다운되었다고 알려줌.
		if (!b_active_server) {	// 내가 Active가 아니면 상대가 Active임. (서버가 2개밖에 없기 때문)
			b_active_server = true;
			cout << "현재 NPC Server[" << my_server_id << "] 가 Active 서버로 승격되었습니다. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// 만약 자신의 오른쪽 서버가 다운되었는데, 그 서버가 서버군의 마지막 서버인 경우 재실행된 서버에게 ConnectEx 요청을 보냅니다.
		if (session_id == MAX_LOBBY_SERVER - 1) {
			if (my_server_id < session_id) {
				// ConnectEx
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				int key = CP_KEY_EX_NPC + MAX_LOBBY_SERVER - 1;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_EX_NPC + HA_PORTNUM_NPC0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버
				inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect 재시도 중...\n" << endl;
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
		}
		break;
	}
	}
}