#pragma once
#include <math.h>
#include "Constant.h"
#include "MyVectors.h"

enum NpcType{NPC_Helicopter, NPC_Bunker, NPC_Terret};

class NPC{
private:
	int m_ID;									// ID�� 7001~���� ����
	int m_NpcType;
	float m_pitch, m_yaw, m_roll;					// Rotated Degree
	
	XMFLOAT3 m_Pos;
	XMFLOAT3 m_saveOrgPos;
	Coordinate m_curr_coordinate;				// ���� Look, Right, Up Vectors

	bool m_Active;
	float m_theta;
	float m_range;					// �ӽ� ���� �� ���۽� ����� ���
	float m_Acc;

public:
	// ===========================================
	// =============      BASE      ==============
	// ===========================================
	NPC();
	~NPC();


	// ===========================================
	// =============       SET      ==============
	// ===========================================
	void SetID(int id);
	void SetNpcType(int type);
	void SetRotate(float y, float p, float r);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 pos);
	void SetOrgPosition(XMFLOAT3 pos);
	void SetCurr_coordinate(Coordinate cor);

	void SetActive(bool act);
	void SetTheta(float t);
	void SetRange(float r);
	void SetAcc(float acc);
	// ===========================================
	// =============       GET      ==============
	// ===========================================
	int GetID();
	int GetType();
	float GetRotate();
	float MyGetPosition();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetOrgPosition();
	Coordinate GetCurr_coordinate();

	bool GetActive();
	float GetTheta();
	float GetRange();
	float GetAcc();

public:
	// ===========================================
	// =============     NORMAL     ==============
	// ===========================================
	void Move();					// ���Ŀ� A* �˰��� �߰��� ���� ����� Ư�� ��� ����
	void MovetoRotate();
	XMFLOAT3 NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll);


};

