#pragma once
#include <math.h>
#include "Constant.h"
#include "MyVectors.h"
//#include <random>
enum NpcType{NPC_Helicopter, NPC_Bunker, NPC_Terret};
enum St1_NPCState{NPC_IDLE, NPC_FLY, NPC_CHASE, NPC_ATTACK, NPC_DEATH};
enum Hit_target{g_none, g_body, g_profeller};
//std::random_device rd;
//std::default_random_engine dre(rd());

class ST1_NPC{
private:
	int m_ID;
	int m_NpcType;
	float m_pitch, m_yaw, m_roll;					// Rotated Degree
	
	XMFLOAT3 m_Pos;
	XMFLOAT3 m_saveOrgPos;
	Coordinate m_curr_coordinate;				// 현재 Look, Right, Up Vectors

	XMFLOAT3 m_User_Pos[5];
	int m_state;
	
	int m_Hit;
	int m_ProfellerHP;
	int m_BodyHP;
	int m_attack;
	int m_defence;
	int m_chaseID;

	float m_Speed;
	float m_theta;
	float m_range;					// 임시 변수 재 제작시 사라질 운명
	float m_Acc;

	float m_Distance[3];

public:
	// ===========================================
	// =============      BASE      ==============
	// ===========================================
	ST1_NPC();
	~ST1_NPC();

	// ===========================================
	// =============       SET      ==============
	// ===========================================
	void SetID(int id);
	void SetNpcType(int type);
	void SetChaseID(int id);
	void SetRotate(float y, float p, float r);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 pos);
	void SetOrgPosition(XMFLOAT3 pos);
	void SetCurr_coordinate(Coordinate cor);
	void SetUser_Pos(XMFLOAT3 pos, int id);

	void SetTheta(float t);
	void SetRange(float r);
	void SetAcc(float acc);
	void SetDistance(float dis);

	// ===========================================
	// =============       GET      ==============
	// ===========================================
	int GetID();
	int GetChaseID();
	int GetType();
	int GetState();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetOrgPosition();
	Coordinate GetCurr_coordinate();

	float MyGetPosition();
	float GetRotate();
	float GetTheta();
	float GetRange();
	float GetAcc();
	float GetDistance(int id);

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
	void MovetoRotate();
	
	// Fly
	void FlyOnNpc(XMFLOAT3 vec, int id);
	
	// Chase
	void PlayerChasing();

	// Rotate
	XMFLOAT3 NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll);
};
