#include "NPC.h"

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

float Calculation_Distance(XMFLOAT3 vec, vector<City_Info>const& v, int c_id) // vec-> Player's pos, v -> city's center pos 
{
	float dist = sqrtf(powf(vec.x - v[c_id].Centerx, 2) + powf(vec.z - v[c_id].Centerz, 2));
	return dist;
}

float Calculation_Distance(XMFLOAT3 vec, vector<City_Info>const& v, int c_id, int s_id)
{
	float cx = (v[c_id].SectionNum[s_id].lx + v[c_id].SectionNum[s_id].sx) / 2;
	float cz = (v[c_id].SectionNum[s_id].lz + v[c_id].SectionNum[s_id].sz) / 2;
	
	float dist = sqrtf(powf(vec.x - cx, 2) + powf(vec.z - cz, 2));
	return dist;
}
// ===========================================
// =============      BASE      ==============
// ===========================================

ST1_NPC::ST1_NPC()
{
	m_curr_coordinate.right = { 1.0f, 0.0f, 0.0f };
	m_curr_coordinate.up = { 0.0f, 1.0f, 0.0f };
	m_curr_coordinate.look = { 0.0f, 0.0f, 1.0f };

	for (int i{}; i < User_num; ++i) {
		m_Distance[i] = { 10000 };
	}

	m_state = NPC_IDLE;
	m_Hit = g_none;
	m_ProfellerHP = 50;
	m_BodyHP = 50;

	m_attack = 25;
	m_defence = 100;
	m_chaseID = -1;

	m_SectionMoveDir = true;
	m_xoobb_Pro = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_xoobb_Body = BoundingOrientedBox(XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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

void ST1_NPC::SetIdleCity(int num)
{
	m_IdleCity = num;
}

void ST1_NPC::SetIdleSection(int num)
{
	m_IdleSection = num;
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

void ST1_NPC::SetCurr_coordinate(Coordinate cor)
{
	m_curr_coordinate = cor;
}

void ST1_NPC::SetUser_Pos(XMFLOAT3 pos, int id)
{
	m_User_Pos[id] = pos;
}

void ST1_NPC::SetInitSection(vector<City_Info> const& v)
{
	m_Section = v;
}

void ST1_NPC::SetDistance(float dis)
{
	*m_Distance = dis;
}

void ST1_NPC::SetSpeed(float spd)
{
	m_Speed = spd;
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

int ST1_NPC::GetIdleCity()
{
	return m_IdleCity;
}

int ST1_NPC::GetIdleSection()
{
	return m_IdleSection;
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

Coordinate ST1_NPC::GetCurr_coordinate()
{
	return m_curr_coordinate;
}

float ST1_NPC::GetDistance(int id)
{
	return m_Distance[id];
}

float ST1_NPC::GetSpeed(float spd)
{
	return m_Speed;
}

vector<City_Info> ST1_NPC::GetCityInfo()
{
	return m_Section;
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
		MoveInSection();
		for (int i{}; i < User_num; ++i) {
			if (m_Distance[i] <= 400) {
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
	}
	break;
	case NPC_FLY:
	{
		// Fly 지속 or Fly -> chase or Fly -> Idle
		FlyOnNpc(m_User_Pos[m_chaseID], m_chaseID); // 대상 플레이어와의 y 좌표를 비슷하게 맞춤.
		bool State_check = false;

		for (int i{}; i < User_num; ++i) {
			if (m_Distance[i] <= 200) {  // 추적상태로 변환
				State_check = true;
				if (m_Distance[i] < m_Distance[m_chaseID]) {
					m_chaseID = i;
				}
				m_state = NPC_CHASE; // 날기 후 추적 상태로 돌입
				m_chaseID = i;
			}
			else if (i == User_num - 1 && !State_check) { // 자신의 상태 유지
				m_state = NPC_FLY;
			}
		}

		int Idle_Change = 0;
		if (m_state == NPC_FLY) {
			for (int i{}; i < User_num; ++i) {
				if (m_Distance[i] > 400) {
					Idle_Change++;
				}
			}
		}
		if (Idle_Change == User_num) {
			m_state = NPC_IDLE;
			m_chaseID = -1;
			// Idle로 돌아가니까, NPC 객체 하나당 갖고있던 플레이어들의 거리 배열을 초기화
			for (int i{}; i < User_num; ++i) {
				m_Distance[i] = 10000;
			}
			MoveChangeIdle(); // City, Section 지정
		}
	}
	break;
	case NPC_CHASE:
	{
		// chase -> chase or chase -> attack or chase -> fly	
		PlayerChasing();

		bool State_check = false;

		for (int i{}; i < User_num; ++i) {
			if (m_Distance[i] <= 150) {  // 공격상태로 변환
				State_check = true;
				if (m_Distance[i] < m_Distance[m_chaseID]) {
					m_chaseID = i;
				}
				//m_state = NPC_ATTACK; // 날기 후 공격 상태로 돌입
				m_chaseID = i;
			}
			else if (i == User_num - 1 && !State_check) { // 자신의 상태 유지
				m_state = NPC_CHASE;
			}
		}

		int Fly_Change = 0;
		if (m_state == NPC_CHASE) {
			for (int i{}; i < User_num; ++i) {
				if (m_Distance[i] > 200) {
					Fly_Change++;
				}
			}
		}
		if (Fly_Change == User_num) {
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
		if (m_Pos.y > 0.0f) {
			ST1_Death_motion();
		}
	}
	break;
	default:
		break;
	}
	setBB_Pro();
	setBB_Body();
}

void ST1_NPC::ST1_Death_motion()
{
	m_Pos.y -= 6.0f;

	// 빙글빙글 돌며 추락
	m_yaw += 3.0f;

	Coordinate base_coordinate;
	m_curr_coordinate.right = NPCcalcRotate(base_coordinate.right, m_pitch, m_yaw, m_roll);
	m_curr_coordinate.up = NPCcalcRotate(base_coordinate.up, m_pitch, m_yaw, m_roll);
	m_curr_coordinate.look = NPCcalcRotate(base_coordinate.look, m_pitch, m_yaw, m_roll);
}

void ST1_NPC::MoveInSection()
{
	if (m_SectionMoveDir) {
		switch (m_IdleSection)
		{
		case 0:
		{
			if (25.0f > abs(m_Section[m_IdleCity].SectionNum[m_IdleSection].sx - m_Pos.x)) {
				m_IdleSection++;
			}
			else {
				XMFLOAT3 sec_look = { m_Section[m_IdleCity].SectionNum[m_IdleSection].sx, m_Pos.y, m_Pos.z };
				XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&m_Pos)));
				XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

				// Right
				Coordinate base_coordinate;
				base_coordinate.up = { 0,1,0 };

				XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
				XMStoreFloat3(&m_curr_coordinate.right, righttemp);

				// up
				XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
				XMStoreFloat3(&m_curr_coordinate.up, uptemp);

				m_Pos.x += m_Speed * m_curr_coordinate.look.x;
			}
		}
		break;
		case 1:
		{
			if (25.0f > abs(m_Section[m_IdleCity].SectionNum[m_IdleSection].lz - m_Pos.z)) {
				m_IdleSection++;
			}
			else {
				XMFLOAT3 sec_look = { m_Pos.x, m_Pos.y, m_Section[m_IdleCity].SectionNum[m_IdleSection].lz };
				XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&m_Pos)));
				XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

				// Right
				Coordinate base_coordinate;
				base_coordinate.up = { 0,1,0 };

				XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
				XMStoreFloat3(&m_curr_coordinate.right, righttemp);

				// up
				XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
				XMStoreFloat3(&m_curr_coordinate.up, uptemp);

				m_Pos.z += m_Speed * m_curr_coordinate.look.z;
			}
		}
		break;
		case 2:
		{
			if (25.0f > abs(m_Section[m_IdleCity].SectionNum[m_IdleSection].lx - m_Pos.x)) {
				m_SectionMoveDir = false;
			}
			else {
				XMFLOAT3 sec_look = { m_Section[m_IdleCity].SectionNum[m_IdleSection].lx, m_Pos.y, m_Pos.z };
				XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&m_Pos)));
				XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

				// Right
				Coordinate base_coordinate;
				base_coordinate.up = { 0,1,0 };

				XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
				XMStoreFloat3(&m_curr_coordinate.right, righttemp);

				// up
				XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
				XMStoreFloat3(&m_curr_coordinate.up, uptemp);

				m_Pos.x += m_Speed * m_curr_coordinate.look.x;
			}
		}
		break;
		}
	}
	else {
		switch (m_IdleSection)
		{
		case 0:
		{
			if (25.0f > abs(m_Section[m_IdleCity].SectionNum[m_IdleSection].lx - m_Pos.x)) {
				m_SectionMoveDir = true;
			}
			else {
				XMFLOAT3 sec_look = { m_Section[m_IdleCity].SectionNum[m_IdleSection].lx, m_Pos.y, m_Pos.z };
				XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&m_Pos)));
				XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

				// Right
				Coordinate base_coordinate;
				base_coordinate.up = { 0,1,0 };

				XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
				XMStoreFloat3(&m_curr_coordinate.right, righttemp);

				// up
				XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
				XMStoreFloat3(&m_curr_coordinate.up, uptemp);

				m_Pos.x += m_Speed * m_curr_coordinate.look.x;
			}
		}
		break;
		case 1:
		{
			if (25.0f > abs(m_Section[m_IdleCity].SectionNum[m_IdleSection].sz - m_Pos.z)) {
				m_IdleSection--;
			}
			else {
				XMFLOAT3 sec_look = { m_Pos.x, m_Pos.y, m_Section[m_IdleCity].SectionNum[m_IdleSection].sz };
				XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&m_Pos)));
				XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

				// Right
				Coordinate base_coordinate;
				base_coordinate.up = { 0,1,0 };

				XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
				XMStoreFloat3(&m_curr_coordinate.right, righttemp);

				// up
				XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
				XMStoreFloat3(&m_curr_coordinate.up, uptemp);

				m_Pos.z += m_Speed * m_curr_coordinate.look.z;
			}
		}
		break;
		case 2:
		{
			if (25.0f > abs(m_Section[m_IdleCity].SectionNum[m_IdleSection].sx - m_Pos.x)) {
				m_IdleSection--;
			}
			else {
				XMFLOAT3 sec_look = { m_Section[m_IdleCity].SectionNum[m_IdleSection].sx, m_Pos.y, m_Pos.z };
				XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&sec_look), XMLoadFloat3(&m_Pos)));
				XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

				// Right
				Coordinate base_coordinate;
				base_coordinate.up = { 0,1,0 };

				XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
				XMStoreFloat3(&m_curr_coordinate.right, righttemp);

				// up
				XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
				XMStoreFloat3(&m_curr_coordinate.up, uptemp);

				m_Pos.x += m_Speed * m_curr_coordinate.look.x;
			}
		}
		break;
		}
	}
}

void ST1_NPC::MoveChangeIdle()
{
	float City_dis{};
	float Min_DisofCity = 100000;
	int id{};
	for (int i{}; i < 3; ++i) {
		City_dis = Calculation_Distance(m_Pos, m_Section, i);
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
			dis = Calculation_Distance(m_Pos, m_Section, id, i);
			if (Min_DisofSec > dis) {
				Min_DisofSec = dis;
				s_id = i;
			}
		}
		// 구역 지정
		m_IdleCity = id;
		m_IdleSection = s_id;

	}
	//else {
	//	// 일정 구간 안인 경우

	//	m_Section[m_IdleCity].SectionNum[m_IdleSection];
	//}
}


XMFLOAT3 ST1_NPC::NPCcalcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll)
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
	// Look
	XMVECTOR Looktemp = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&m_User_Pos[m_chaseID]), XMLoadFloat3(&m_Pos)));
	XMStoreFloat3(&m_curr_coordinate.look, Looktemp);

	// Right
	Coordinate base_coordinate;
	base_coordinate.up = { 0,1,0 };

	XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
	XMStoreFloat3(&m_curr_coordinate.right, righttemp);

	// up
	XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
	XMStoreFloat3(&m_curr_coordinate.up, uptemp);

	if (m_Distance[m_chaseID] < 30) {
		m_Speed = 0;
	}
	else {
		m_Speed = 1.5f;
	}

	// 위치 변환
	m_Pos.x += m_Speed * m_curr_coordinate.look.x;
	m_Pos.y += m_Speed * m_curr_coordinate.look.y;
	m_Pos.z += m_Speed * m_curr_coordinate.look.z;
}