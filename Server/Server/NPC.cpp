#include "NPC.h"

// ===========================================
// =============      BASE      ==============
// ===========================================

NPC::NPC()
{
	m_curr_coordinate.right = { 1.0f, 0.0f, 0.0f };
	m_curr_coordinate.up = { 0.0f, 1.0f, 0.0f };
	m_curr_coordinate.look = { 0.0f, 0.0f, 1.0f };


}

NPC::~NPC()
{

}

// ===========================================
// =============       SET      ==============
// ===========================================

void NPC::SetID(int id)
{
	m_ID = id;
}

void NPC::SetNpcType(int type)
{
	m_NpcType = type;
}

void NPC::SetRotate(float y, float p, float r)
{
	m_yaw = y;
	m_pitch = p;
	m_roll = r;
}

void NPC::SetPosition(float x, float y, float z)
{
	m_Pos = { x,y,z };
}

void NPC::SetPosition(XMFLOAT3 pos)
{
	m_Pos = pos;
}

void NPC::SetOrgPosition(XMFLOAT3 pos)
{
	m_saveOrgPos = pos;
}

void NPC::SetCurr_coordinate(Coordinate cor)
{
	m_curr_coordinate = cor;
}

void NPC::SetActive(bool act)
{
	m_Active = act;
}

void NPC::SetTheta(float t)
{
	m_theta = t;
}

void NPC::SetRange(float r)
{
	m_range = r;
}

void NPC::SetAcc(float acc)
{
	m_Acc = acc;
}

// ===========================================
// =============       GET      ==============
// ===========================================

int NPC::GetID()
{
	return m_ID;
}

int NPC::GetType()
{
	return m_NpcType;
}

float NPC::GetRotate()
{
	return m_yaw, m_pitch, m_roll;
}

float NPC::MyGetPosition()
{
	return m_Pos.x, m_Pos.y, m_Pos.z;
}

XMFLOAT3 NPC::GetPosition()
{
	return m_Pos;
}

XMFLOAT3 NPC::GetOrgPosition()
{
	return m_saveOrgPos;
}

Coordinate NPC::GetCurr_coordinate()
{
	return m_curr_coordinate;
}

bool NPC::GetActive()
{
	return m_Active;
}

float NPC::GetTheta()
{
	return m_theta;
}

float NPC::GetRange()
{
	return m_range;
}

float NPC::GetAcc()
{
	return m_Acc;
}

// ===========================================
// =============     NORMAL     ==============
// ===========================================

void NPC::Move()
{
	int sign = 1.0f;
	XMFLOAT3 vec1 = m_Pos;
	XMFLOAT3 move_dir{ 0,0,0 };
	move_dir.x = m_curr_coordinate.look.x * sign;
	move_dir.y = m_curr_coordinate.look.y * sign;
	move_dir.z = m_curr_coordinate.look.z * sign;

	float Fixed_Acc = 0.3f;

	vec1.x = vec1.x + move_dir.x * Fixed_Acc;
	vec1.y = vec1.y + move_dir.y * Fixed_Acc;
	vec1.z = vec1.z + move_dir.z * Fixed_Acc;
}

void NPC::MovetoRotate()
{
	m_roll += 1.0f * PI * m_theta / 360.0f;
	Coordinate base_coordinate;
	m_curr_coordinate.right = NPCcalcRotate(base_coordinate.right, m_roll, m_pitch, m_yaw);
	m_curr_coordinate.up = NPCcalcRotate(base_coordinate.up, m_roll, m_pitch, m_yaw);
	m_curr_coordinate.look = NPCcalcRotate(base_coordinate.look, m_roll, m_pitch, m_yaw);

	m_Acc += m_theta;
	m_Pos.x = m_range * cos(m_Acc * PI / 360) + m_saveOrgPos.x;
	//m_Pos.y = m_Pos.y;
	m_Pos.z = m_range * sin(m_Acc * PI / 360) + m_saveOrgPos.z;
}



XMFLOAT3 NPC::NPCcalcRotate(XMFLOAT3 vec, float roll, float pitch, float yaw)
{
	float x1, y1;
	x1 = vec.x * cos(roll) - vec.y * sin(roll);
	y1 = vec.x * sin(roll) + vec.y * cos(roll);

	// pitch
	float y2, z1;
	y2 = y1 * cos(pitch) - vec.z * sin(pitch);
	z1 = y1 * sin(pitch) + vec.z * cos(pitch);

	// yaw
	float x2, z2;
	z2 = z1 * cos(yaw) - x1 * sin(yaw);
	x2 = z1 * sin(yaw) + x1 * cos(yaw);

	// Update
	vec = { x2, y2, z2 };

	return vec;
}

