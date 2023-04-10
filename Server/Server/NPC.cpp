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

ST1_NPC::ST1_NPC()
{
	m_curr_coordinate.right = { 1.0f, 0.0f, 0.0f };
	m_curr_coordinate.up = { 0.0f, 1.0f, 0.0f };
	m_curr_coordinate.look = { 0.0f, 0.0f, 1.0f };

	for (int i{}; i < 3; ++i) {
		m_Distance[i] = { 10000 };
	}

	m_state = NPC_IDLE;
	m_Hit = g_none;
	m_ProfellerHP = 50;
	m_BodyHP = 50;

	m_attack = 25;
	m_defence = 100;
	m_Speed = 3.0f;
	m_chaseID = -1;
}

ST1_NPC::~ST1_NPC()
{

}

// ===========================================
// =============       SET      ==============
// ===========================================

void ST1_NPC::SetID(int id)
{
	m_ID = id;
}

void ST1_NPC::SetNpcType(int type)
{
	m_NpcType = type;
}

void ST1_NPC::SetChaseID(int id)
{
	m_chaseID = id;
}

void ST1_NPC::SetRotate(float y, float p, float r)
{
	m_yaw = y;
	m_pitch = p;
	m_roll = r;
}

void ST1_NPC::SetPosition(float x, float y, float z)
{
	m_Pos = { x,y,z };
}

void ST1_NPC::SetPosition(XMFLOAT3 pos)
{
	m_Pos = pos;
}

void ST1_NPC::SetOrgPosition(XMFLOAT3 pos)
{
	m_saveOrgPos = pos;
}

void ST1_NPC::SetCurr_coordinate(Coordinate cor)
{
	m_curr_coordinate = cor;
}

void ST1_NPC::SetUser_Pos(XMFLOAT3 pos, int id)
{
	m_User_Pos[id] = pos;
}

void ST1_NPC::SetTheta(float t)
{
	m_theta = t;
}

void ST1_NPC::SetRange(float r)
{
	m_range = r;
}

void ST1_NPC::SetAcc(float acc)
{
	m_Acc = acc;
}

void ST1_NPC::SetDistance(float dis)
{
	*m_Distance = dis;
}

// ===========================================
// =============       GET      ==============
// ===========================================

int ST1_NPC::GetID()
{
	return m_ID;
}

int ST1_NPC::GetChaseID()
{
	return m_chaseID;
}

int ST1_NPC::GetType()
{
	return m_NpcType;
}

int ST1_NPC::GetState()
{
	return m_state;
}

float ST1_NPC::GetRotate()
{
	return m_yaw, m_pitch, m_roll;
}

float ST1_NPC::MyGetPosition()
{
	return m_Pos.x, m_Pos.y, m_Pos.z;
}

XMFLOAT3 ST1_NPC::GetPosition()
{
	return m_Pos;
}

XMFLOAT3 ST1_NPC::GetOrgPosition()
{
	return m_saveOrgPos;
}

Coordinate ST1_NPC::GetCurr_coordinate()
{
	return m_curr_coordinate;
}

float ST1_NPC::GetTheta()
{
	return m_theta;
}

float ST1_NPC::GetRange()
{
	return m_range;
}

float ST1_NPC::GetAcc()
{
	return m_Acc;
}

float ST1_NPC::GetDistance(int id)
{
	return m_Distance[id];
}

// ===========================================
// =============     NORMAL     ==============
// ===========================================
void ST1_NPC::ST1_State_Manegement(int state)
{
	switch (m_state)
	{
	case NPC_IDLE: // 매복 혹은 특정 운동을 하는 중.
	{
		//this->MovetoRotate(); // 원운동 중
		MovetoRotate();
		for (int i{}; i < 3; ++i) {
			if (m_Distance[i] < 400) {
				if (m_chaseID != -1) {
					if (m_Distance[i] < m_Distance[m_chaseID]) {
						m_chaseID = i;
						m_state = NPC_FLY; // 수시로 탐색 후 날기 상태로 돌입
					}
					else {
						break;
					}
				}
				else {
					m_state = NPC_FLY; // 수시로 탐색 후 날기 상태로 돌입
					m_chaseID = i;

				}
			}
		}
		//std::cout << m_ID << "번째 NPC Status - " << m_state << std::endl;
	}
	break;
	case NPC_FLY:
	{
		// Fly 지속 or Fly -> chase or Fly -> Idle
		FlyOnNpc(m_User_Pos[m_chaseID], m_chaseID); // 대상 플레이어와의 y 좌표를 비슷하게 맞춤.
		bool State_check = false;

		for (int i{}; i < 3; ++i) {
			if (m_Distance[i] < 200) {  // 추적상태로 변환
				State_check = true;
				if (m_Distance[i] < m_Distance[m_chaseID]) {
					m_chaseID = i;
				}
				m_state = NPC_CHASE; // 날기 후 추적 상태로 돌입
				m_chaseID = i;
			}
			else if (i == 2 && !State_check) { // 자신의 상태 유지
				m_state = NPC_FLY;
			}
		}

		int Idle_Change = 0;
		if (m_state == NPC_FLY) {
			for (int i{}; i < 3; ++i) {
				if (m_Distance[i] > 700) {
					Idle_Change++;
				}
			}
		}
		if (Idle_Change == 3) {
			m_state = NPC_IDLE;
			m_chaseID = -1;
		}
	}
	break;
	case NPC_CHASE:
	{
		// chase -> chase or chase -> attack or chase -> fly	
		PlayerChasing();

		bool State_check = false;

		for (int i{}; i < 3; ++i) {
			if (m_Distance[i] < 150) {  // 공격상태로 변환
				State_check = true;
				if (m_Distance[i] < m_Distance[m_chaseID]) {
					m_chaseID = i;
				}
				//m_state = NPC_ATTACK; // 날기 후 공격 상태로 돌입
				m_chaseID = i;
			}
			else if (i == 2 && !State_check) { // 자신의 상태 유지
				m_state = NPC_CHASE;
			}
		}

		int Fly_Change = 0;
		if (m_state == NPC_CHASE) {
			for (int i{}; i < 3; ++i) {
				if (m_Distance[i] > 400) {
					Fly_Change++;
				}
			}
		}
		if (Fly_Change == 3) {
			m_state = NPC_FLY;
			m_chaseID = -1;
		}

	}
	break;
	case NPC_ATTACK:
	{
		// bullet 관리



	}
	break;
	case NPC_DEATH:
	{
		if (m_Pos.y >= 0) {
			ST1_Death_motion();
		}
	}
	break;
	default:
		break;
	}
}

void ST1_NPC::ST1_Death_motion()
{
	m_Pos.y -= 6.0f;
}

void ST1_NPC::MovetoRotate()
{
	m_yaw += 1.0f * m_theta;
	Coordinate base_coordinate;
	m_curr_coordinate.right = NPCcalcRotate(base_coordinate.right, m_pitch, m_yaw, m_roll);
	m_curr_coordinate.up = NPCcalcRotate(base_coordinate.up, m_pitch, m_yaw, m_roll);
	m_curr_coordinate.look = NPCcalcRotate(base_coordinate.look, m_pitch, m_yaw, m_roll);

	m_Acc += m_theta;
	m_Pos.x = m_range * cos(m_Acc * PI / 360) + m_saveOrgPos.x;

	//m_Pos.y = m_Pos.y;
	m_Pos.z = m_range * sin(m_Acc * PI / 360) + m_saveOrgPos.z;
}


XMFLOAT3 ST1_NPC::NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll)
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

void ST1_NPC::Caculation_Distance(XMFLOAT3 vec, int id) // 서버에서 따로 부를 것.
{
	m_Distance[id] = sqrtf(pow((vec.x - m_Pos.x), 2) + pow((vec.y - m_Pos.y), 2) + pow((vec.z - m_Pos.z), 2));

}

void ST1_NPC::ST1_Damege_Calc(int id)
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
		int damege = (20 * ((((int)(m_Distance[id])) / 100) * 5)) / m_defence;
		m_ProfellerHP -= damege;
		m_Hit = g_none;
	}

	if ((m_BodyHP <= 0) || (m_ProfellerHP <= 0)) {
		m_state = NPC_DEATH;
	}
}

void ST1_NPC::FlyOnNpc(XMFLOAT3 vec, int id) // 추적대상 플레이어와 높이 맞추기
{
	if (m_Pos.y < vec.y) {
		m_Pos.y += 1.0f;
	}
	if (m_Pos.y > vec.y) {
		m_Pos.y -= 1.5f;
	}
}

void ST1_NPC::PlayerChasing()
{
	// Look 방향 변환 --> 조금씩 회전

	//XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&m_User_Pos[m_chaseID]), XMLoadFloat3(&m_Pos)));
	//XMStoreFloat3(&m_curr_coordinate.look, Looktemp);
	//
	//// Right 방향 변환
	//Coordinate base_coordinate;
	//base_coordinate.up = { 0,1,0 };

	//XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
	//XMStoreFloat3(&m_curr_coordinate.right, righttemp);

	//// Up 방향 변환
	//XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
	//XMStoreFloat3(&m_curr_coordinate.up, uptemp);

	// 위치 변환
	m_Pos.x += m_Speed * m_curr_coordinate.look.x;
	m_Pos.z += m_Speed * m_curr_coordinate.look.z;
}
