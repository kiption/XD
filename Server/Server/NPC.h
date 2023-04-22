#pragma once
#include <math.h>
#include "Constant.h"
#include "MyVectors.h"
//#include <random>
enum NpcType{NPC_Helicopter, NPC_Bunker, NPC_Terret};
enum St1_NPCState{NPC_IDLE, NPC_FLY, NPC_CHASE, NPC_ATTACK, NPC_DEATH};
enum Hit_target{g_none, g_body, g_profeller};
enum IDLE_Section {Sectoin_1, Sectoin_2, Sectoin_3};

struct Section_Info {
	float sx, sz, lx, lz;
	int ID;
};
struct City_Info {
	Section_Info SectionNum[3];
	float Centerx, Centerz;
	int id;
};



// ���� ��ġ �ٲ�� ��ġ�� ������ ������ �д��� �̷��� �迭 ���·� �ؼ� ������ �������� �ؾ���. --> �ӽ�
const float SX_range[9] = { 520.0f, 520.0f, 520.0f, -900.0f,-980.0f, -1000.0f, -385.0f, -180.0f, -180.0f };
const float LX_range[9] = { 1000.0f, 580.0f, 1550.0f, 50.0f,-900.0f, -300.0f, -100.0f, -100.0f, 845.0f };
const float SZ_range[9] = { 350.0f, 350.0f, 770.0f, 660.0f,240.0f, 240.0f, -535.0f, -535.0f, -110.0f };
const float LZ_range[9] = { 410.0f, 835.0f, 840.0f, 720.0f,720.0f, 295.0f, -455.0f, -30.0f, -30.0f };
const float C_cx[3] = { 782.0f, -680.0f, -17.0f };
const float C_cz[3] = { 592.0f, 477.5f, -282.0f };


class ST1_NPC{
private:
	int m_ID;
	int m_NpcType;
	float m_pitch, m_yaw, m_roll;					// Rotated Degree
	
	XMFLOAT3 m_Pos;
	XMFLOAT3 m_saveOrgPos;
	Coordinate m_curr_coordinate;				// ���� Look, Right, Up Vectors

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

	float m_turnSpeed;
	float m_Speed;
	float m_theta;
	float m_range;					// �ӽ� ���� �� ���۽� ����� ���
	float m_Acc;

	float m_Distance[3];

public:
	BoundingOrientedBox m_xoobb_Pro;
	BoundingOrientedBox m_xoobb_Body;
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
	void SetIdleCity(int num);
	void SetIdleSection(int num);
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
	int GetIdleCity();
	int GetIdleSection();
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
	void ST1_State_Manegement(int state); // ���� ����
	
	void Caculation_Distance(XMFLOAT3 vec, int id); // ���� �� �÷��̾� Ž��
	void ST1_Death_motion(); // HP 0

	// Damege
	void ST1_Damege_Calc(int id);
	
	// Idle
	void MovetoRotate();
	void MoveInSection();

	// Fly
	void FlyOnNpc(XMFLOAT3 vec, int id);
	
	// Chase
	void PlayerChasing();

	// Rotate
	XMFLOAT3 NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll);

	void setBB_Pro() { m_xoobb_Pro = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
	void setBB_Body() { m_xoobb_Body = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }
};
