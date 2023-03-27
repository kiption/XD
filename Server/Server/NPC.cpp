#include "NPC.h"

XMFLOAT3 NPCNormalize(XMFLOAT3 vec)
{
	float dist = sqrtf(powf(vec.x, 2) + powf(vec.y, 2) + powf(vec.z, 2));

	vec.x = vec.x / dist;
	vec.y = vec.y / dist;
	vec.z = vec.z / dist;

	return vec;
}


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

void NPC::SetDistance(float dis)
{
	*m_Distance = dis;
}

void NPC::SetFindRange(float f)
{
	m_FindRange = f;
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

float NPC::GetDistance()
{
	return *m_Distance;
}

float NPC::GetFindRange()
{
	return m_FindRange;
}

// ===========================================
// =============     NORMAL     ==============
// ===========================================

//void CSuperCobraObject::Ai(float fTimeElapsed)
//{
//	if (CheckPlayer())
//	{
//		XMFLOAT3 xmf3Dir = Vector3::Normalize(Vector3::Subtract(m_PlayerPosition, GetPosition()));
//		bool checkR = RotateXZPlayer(GetLook(), xmf3Dir);
//		bool checkM = MoveYPlayer(m_PlayerPosition);
//		if (checkR && checkM)
//		{
//			if (m_fShotDelay == 0.f)
//				m_bShotMissile = true;
//		}
//		m_fShotDelay += fTimeElapsed * 5.0f;
//		if (m_fShotDelay >= 10.f)
//		{
//			m_fShotDelay = 0.f;
//		}
//	}
//	else
//	{
//		switch (m_behavior)
//		{
//		case ENEMY_PLAY::ENEMY_MOVECHECK:
//		{
//			m_iRoteDest = RandRotate();
//			Rotate(0.f, m_iRoteDest, 0.f);
//			MoveForward(100.f);
//			XMFLOAT3 pos = GetPosition();
//			if (pos.x > 4400.f || pos.x < 200.f || pos.z > 4400.f || pos.z < 200.f);
//			else
//				m_behavior = ENEMY_PLAY::ENEMY_ROTATE;
//			Rotate(0.f, 180.f, 0.f);
//			MoveForward(100.f);
//			Rotate(0.f, 180.f, 0.f);
//			Rotate(0.f, -m_iRoteDest, 0.f);
//		}
//		break;
//		case ENEMY_PLAY::ENEMY_ROTATE:
//			if (m_iRoteDest < 0)
//			{
//				Rotate(0.f, -1.f, 0.f);
//				--m_iRotateCnt;
//				if (m_iRotateCnt <= m_iRoteDest)
//				{
//					m_behavior = ENEMY_PLAY::ENEMY_MOVE;
//					m_iRotateCnt = 0;
//				}
//			}
//			else
//			{
//				Rotate(0.f, 1.f, 0.f);
//				++m_iRotateCnt;
//				if (m_iRotateCnt >= m_iRoteDest)
//				{
//					m_behavior = ENEMY_PLAY::ENEMY_MOVE;
//					m_iRotateCnt = 0;
//				}
//			}
//			break;
//		case ENEMY_PLAY::ENEMY_MOVE:
//			MoveForward(0.5f);
//			++m_iMoveCnt;
//			if (m_iMoveCnt >= 100)
//			{
//				m_behavior = ENEMY_PLAY::ENEMY_MOVECHECK;
//				m_iMoveCnt = 0;
//			}
//
//			break;
//		default:
//			break;
//		}
//	}
//}
//


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
	m_yaw += 0.01f * m_theta;
	Coordinate base_coordinate;
	m_curr_coordinate.right = NPCcalcRotate(base_coordinate.right, m_pitch, m_yaw, m_roll);
	m_curr_coordinate.up = NPCcalcRotate(base_coordinate.up, m_pitch, m_yaw, m_roll);
	m_curr_coordinate.look = NPCcalcRotate(base_coordinate.look, m_pitch, m_yaw, m_roll);

	m_Acc += m_theta;
	m_Pos.x = m_range * cos(m_Acc * PI / 360) + m_saveOrgPos.x;
	//m_Pos.y = m_Pos.y;
	m_Pos.z = m_range * sin(m_Acc * PI / 360) + m_saveOrgPos.z;
}


XMFLOAT3 NPC::NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll)
{
	float curr_pitch = XMConvertToRadians(pitch);
	float curr_yaw = XMConvertToRadians(yaw);
	float curr_roll = XMConvertToRadians(roll);


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

void NPC::Caculation_Distance(XMFLOAT3 vec, int id)
{
	m_Distance[id] = sqrtf(pow((vec.x - m_Pos.x), 2) + pow((vec.y - m_Pos.y), 2) + pow((vec.z - m_Pos.z), 2));
}

void NPC::FindTarget(XMFLOAT3 vec)
{
	float Dis = 20000;
	int chaseID = -1;
	int SubID[3] = { -1 };
	int cnt{};
	for (int i{}; i < 3; ++i) {
		if (m_Distance[i] <  Dis) {
			Dis = m_Distance[i];
			chaseID = i;
			*SubID = { -1 };
		}
		if (m_Distance[i] == Dis) {
			SubID[i] = i;
			cnt++;
		}
	}

	if (cnt != 0) {
		std::uniform_int_distribution<int>uid(chaseID, chaseID + cnt);
		chaseID = uid(dre);
		cnt = 0;
	}

	if (Dis < 500.0f) {
		m_Active = true;
		FlyOnNpc(vec, chaseID);
	}
	else {
		m_Active = false;
	}

}

void NPC::FlyOnNpc(XMFLOAT3 vec, int id)
{
	if (m_Active) {
		if (m_Pos.y < vec.y) {
			m_Pos.y += 12.0f;
		}
		if (m_Pos.y > vec.y) {
			m_Pos.y -= 6.0f;
		}	
	}
	else {

	}
}

// 阿 按眉 喊 利 包府

Stage1Enemy::Stage1Enemy()
{

}

Stage1Enemy::~Stage1Enemy()
{

}

void Stage1Enemy::Caculation_Distance(XMFLOAT3 vec)
{

}

void Stage1Enemy::ChaseToPlayer()
{

}

Stage2Enemy::Stage2Enemy()
{

}

Stage2Enemy::~Stage2Enemy()
{

}

void Stage2Enemy::Caculation_Distance(XMFLOAT3 vec)
{

}

void Stage2Enemy::ChaseToPlayer()
{

}
