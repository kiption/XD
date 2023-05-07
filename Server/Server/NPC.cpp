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
	m_hp = 100;
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
void ST1_NPC::SetHp(int hp)
{
	m_hp = hp;
}

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

void ST1_NPC::SetFrustum()
{
	// NPC의 위치와 Look 벡터를 가져온다.
	XMVECTOR position = XMLoadFloat3(&m_Pos);
	XMVECTOR look = XMLoadFloat3(&m_curr_coordinate.look);

	// Frustum의 사이드 범위와 상하 범위를 설정한다.
	float width = 50.0f;
	float height = 50.0f;

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

void ST1_NPC::SetBuildingInfo(XMFLOAT3 bPos, XMFLOAT3 bScale)
{
	BoundingOrientedBox temp = BoundingOrientedBox(bPos, XMFLOAT3(bScale.x / 2.5f, bScale.y / 2.0f, bScale.z / 2.5f), XMFLOAT4(0, 0, 0, 1));
	m_mapxmoobb.emplace_back(temp);
}

// ===========================================
// =============       GET      ==============
// ===========================================
int ST1_NPC::GetHp()
{
	return m_hp;
}

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

XMFLOAT3 ST1_NPC::GetBuildingInfo(int id)
{
	return m_mapxmoobb[id].Center;
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

		SetTrackingIDbyDistance(500.0f, NPC_IDLE, NPC_FLY);
	}
	break;
	case NPC_FLY:
	{
		// Fly 지속 or Fly -> chase or Fly -> Idle
		FlyOnNpc(m_User_Pos[m_chaseID], m_chaseID); // 대상 플레이어와의 y 좌표를 비슷하게 맞춤.

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
		SetTrackingIDbyDistance(300.0f, NPC_CHASE, NPC_ATTACK); // ID 탐색
		if (PlayerDetact()) { // Id 탐색은 이미 가장 가까운 대상으로 지정하기에 따로 탐색은 하지 않음.
			m_state = NPC_ATTACK; // 다음 상태
		}
		else {
			SetTrackingPrevStatebyDistance(300.0f, NPC_CHASE, NPC_FLY);
		}
	}
	break;
	case NPC_ATTACK:
	{
		// bullet 관리
		SetTrackingIDbyDistance(200.0f, NPC_ATTACK, NPC_ATTACK); // ID 탐색
		if (!PlayerDetact()) { // Id 탐색은 이미 가장 가까운 대상으로 지정하기에 따로 탐색은 하지 않음.
			m_state = NPC_CHASE; // 이전 상태
			PrintRayCast = false;
		}
		else {
			PlayerAttack();
		}
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

	BuildingToNPC_Distance();
	ST1_CheckNPC_HP();
	setBB_Pro();
	setBB_Body();
	SetFrustum();
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

void ST1_NPC::SetTrackingIDbyDistance(float setDistance, int curState, int nextState)
{
	bool State_check = false;
	float MinDis = 50000;
	for (int i{}; i < User_num; ++i) {
		if (m_Distance[i] < setDistance) {  // 상태로 변환하기 위한 조건 및 추적 ID 갱신
			State_check = true;
			if (m_Distance[i] < MinDis) {
				m_chaseID = i;
				MinDis = m_Distance[i];
			}
			m_state = nextState; // 자신의 상태를 다음 상태로 변경
		}
		if (i == User_num - 1 && !State_check) { // 자신의 상태 유지
			m_state = curState;
		}
	}
}

bool ST1_NPC::SetTrackingPrevStatebyDistance(float setDistance, int curState, int prevState)
{
	int change_cnt = 0;

	for (int i{}; i < User_num; ++i) {
		if (m_Distance[i] >= setDistance) {  // 상태로 변환하기 위한 조건 및 추적 ID 갱신
			change_cnt++;
		}
	}

	if (change_cnt != User_num) {
		m_state = curState;
		return true;
	}
	else {
		for (int i{}; i < User_num; ++i) {
			m_Distance[i] = 10000;
		}
		m_state = prevState;
		return false;
	}
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
	NPCtoBuilding_collide();
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

float ST1_NPC::Building_Caculation_Distance(XMFLOAT3 vec)
{
	float dis = sqrtf(pow((vec.x - m_Pos.x), 2) + pow((vec.y - m_Pos.y), 2) + pow((vec.z - m_Pos.z), 2));
	return dis;
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
}

void ST1_NPC::ST1_CheckNPC_HP()
{
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
	Coordinate base_coordinate;
	base_coordinate.right = { 1,0,0 };
	float x = XMVectorGetX(XMVector3AngleBetweenVectors(Looktemp, XMLoadFloat3(&base_coordinate.right)));
	m_yaw = x;
	if (x < 1.0f && x > -1.0f) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&base_coordinate.right), XMConvertToRadians(m_yaw));
		XMStoreFloat3(&m_curr_coordinate.look, Looktemp);
	}
	else {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&base_coordinate.right), XMConvertToRadians(0.0f));
		XMStoreFloat3(&m_curr_coordinate.look, Looktemp);
	}
	// Right
	base_coordinate.up = { 0,1,0 };

	XMVECTOR righttemp = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&base_coordinate.up), Looktemp));
	XMStoreFloat3(&m_curr_coordinate.right, righttemp);

	// up
	XMVECTOR uptemp = XMVector3Normalize(XMVector3Cross(Looktemp, righttemp));
	XMStoreFloat3(&m_curr_coordinate.up, uptemp);

	if (m_Distance[m_chaseID] < 50) {
		m_Speed = 0;
	}
	else {
		m_Speed = 1.5f;
	}

	// 위치 변환
	m_Pos.x += m_Speed * m_curr_coordinate.look.x;
	m_Pos.y += m_Speed * m_curr_coordinate.look.y;
	m_Pos.z += m_Speed * m_curr_coordinate.look.z;
	NPCtoBuilding_collide();
}

bool ST1_NPC::PlayerDetact()
{
	XMVECTOR PlayerPos = XMLoadFloat3(&m_User_Pos[m_chaseID]);

	XMVECTOR FrustumOrigin = XMLoadFloat3(&m_frustum.Origin);
	XMVECTOR FrustumOrientation = XMLoadFloat4(&m_frustum.Orientation);

	// Frustum의 꼭짓점 8개를 구한다.
	XMFLOAT3 corners[8];
	m_frustum.GetCorners(corners);

	// Frustum과 Player의 bounding sphere와의 거리를 구한다.
	float distance = FLT_MAX;
	for (int i = 0; i < 8; i++)
	{
		float d = XMVectorGetX(XMVector3Length(PlayerPos - XMLoadFloat3(&corners[i])));
		if (d < distance)
		{
			distance = d;
		}
	}

	// 거리가 bounding sphere의 반지름보다 작으면 충돌했다고 판단한다.
	if (distance < 50.0f)
	{
		return true;
	}

	return false;
}

void ST1_NPC::PlayerAttack()
{
	// Look
	PlayerChasing();

	// Attack
	NPCCube ChasePlayer{ {m_User_Pos[m_chaseID].x, m_User_Pos[m_chaseID].y, m_User_Pos[m_chaseID].z },
		HELI_BOXSIZE_X, HELI_BOXSIZE_Y, HELI_BOXSIZE_Z };
	Npc_Vector3 NPC_bullet_Pos = { m_Pos.x, m_Pos.y , m_Pos.z };
	Npc_Vector3 NPC_Look_Vec = { m_curr_coordinate.look.x, m_curr_coordinate.look.y , m_curr_coordinate.look.z };
	Npc_Vector3 NPC_result;
	NPC_result = GetInterSection_Line2Cube(NPC_bullet_Pos, NPC_Look_Vec, ChasePlayer);
	if (NPC_result != Npc_defaultVec) {
		PrintRayCast = false;
	}
	else {
		PrintRayCast = true;
	}

	NPCtoBuilding_collide();
}

void ST1_NPC::BuildingToNPC_Distance()
{
	int array_size = m_mapxmoobb.size();

	for (int i{}; i < m_mapxmoobb.size(); ++i) {
		float d = Building_Caculation_Distance(m_mapxmoobb[i].Center);
		NPCtoBuilding_Dis[i] = d;
	}
}

void ST1_NPC::NPCtoBuilding_collide()
{
	for (int i{}; i < m_mapxmoobb.size(); ++i) {
		if (NPCtoBuilding_Dis[i] >= 200) {
			continue;
		}
		if (m_xoobb_Body.Intersects(m_mapxmoobb[i]))
		{
			m_Pos.y += 3.0f;
			m_BodyHP -= 10.0f;
		}
	}
}

// Ray Casting
Npc_Vector3 ST1_NPC::NPC_calcCrossProduct(Npc_Vector3 lval, Npc_Vector3 rval)
{
	float cp_x = lval.y * rval.z - lval.z * rval.y;
	float cp_y = lval.z * rval.x - lval.x * rval.z;
	float cp_z = lval.x * rval.y - lval.y * rval.x;
	return Npc_Vector3{ cp_x, cp_y, cp_z };
}

Npc_Vector3 ST1_NPC::NPC_getNormalVec(Npc_Vector3 v1, Npc_Vector3 v2, Npc_Vector3 v3)
{
	Npc_Vector3 vec_21 = v1 - v2;
	Npc_Vector3 vec_23 = v3 - v2;
	return (NPC_calcCrossProduct(vec_21, vec_23));
}

Npc_Vector3 ST1_NPC::NPC_GetIntersection_Line2Plane(Npc_Vector3 pos, Npc_Vector3 look, Npc_Vector3 plane_p1, Npc_Vector3 plane_p2, Npc_Vector3 plane_p3)
{
	// 법선벡터 구하기
	Npc_Vector3 normal = NPC_getNormalVec(plane_p1, plane_p2, plane_p3);
	float a = normal.x;
	float b = normal.y;
	float c = normal.z;
	float d = -(a * plane_p1.x + b * plane_p1.y + c * plane_p1.z);


	Npc_Vector3 intersection;
	float t_denominator = a * look.x + b * look.y + c * look.z;	// 매개변수t의 분모
	if (t_denominator == 0) {
		intersection = pos;
	}
	else {
		float t = -1.0f * (a * pos.x + b * pos.y + c * pos.z + d) / (a * look.x + b * look.y + c * look.z);	// 매개변수 t
		if (t >= 0) {
			intersection = pos + Npc_Vector3{ look.x * t, look.y * t, look.z * t };
		}
		else {
			return Npc_defaultVec;
		}
	}

	if (a * intersection.x + b * intersection.y + c * intersection.z + d != 0) {
		return Npc_defaultVec;
	}
	return intersection;

}

float ST1_NPC::NPC_calcDistance(Npc_Vector3 v1, Npc_Vector3 v2)
{
	return sqrtf(powf((v1.x - v2.x), 2) + powf((v1.y - v2.y), 2) + powf(v1.z - v2.z, 2));
}

Npc_Vector3 ST1_NPC::GetInterSection_Line2Cube(Npc_Vector3 p, Npc_Vector3 lkvec, NPCCube bb)
{
	Npc_Vector3 Intersections[6] = { Npc_defaultVec, Npc_defaultVec, Npc_defaultVec, Npc_defaultVec, Npc_defaultVec, Npc_defaultVec };
	// ㅁp1p2p3p4 평면방정식과 반직선의 충돌점 구하기
	Npc_Vector3 Intersection_1234 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP2(), bb.getP3());
	Intersections[0] = Intersection_1234;

	// ㅁp1p3p5p7 평면방정식과 반직선의 충돌점 구하기
	Npc_Vector3 Intersection_1357 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP1(), bb.getP3(), bb.getP5());
	Intersections[1] = Intersection_1357;

	// ㅁp3p4p7p8 평면방정식과 반직선의 충돌점 구하기
	Npc_Vector3 Intersection_3478 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP3(), bb.getP4(), bb.getP7());
	Intersections[2] = Intersection_3478;

	// ㅁp4p2p8p6 평면방정식과 반직선의 충돌점 구하기
	Npc_Vector3 Intersection_4286 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP4(), bb.getP2(), bb.getP8());
	Intersections[3] = Intersection_4286;

	// ㅁp2p1p6p5 평면방정식과 반직선의 충돌점 구하기
	Npc_Vector3 Intersection_2165 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP2(), bb.getP1(), bb.getP6());
	Intersections[4] = Intersection_2165;

	// ㅁp5p6p7p8 평면방정식과 반직선의 충돌점 구하기
	Npc_Vector3 Intersection_7856 = NPC_GetIntersection_Line2Plane(p, lkvec, bb.getP7(), bb.getP8(), bb.getP5());
	Intersections[5] = Intersection_7856;

	// 3. p와 교차하는 점들 사이의 거리를 계산해서 가장 가까운 점이 교점이다.
	float min_dist = INFINITY;
	int min_index = 0;
	for (int i = 0; i < 5; ++i) {
		if (Intersections[i] == Npc_defaultVec) continue;
		float cur_dist = NPC_calcDistance(p, Intersections[i]);
		if (cur_dist < min_dist) {
			min_dist = cur_dist;
			min_index = i;
		}
	}

	// 4. 그 충돌점이 큐브 안에 있는지 검사한다.
	if ((bb.getP3().x <= Intersections[min_index].x && Intersections[min_index].x <= bb.getP4().x)
		&& (bb.getP7().y <= Intersections[min_index].y && Intersections[min_index].y <= bb.getP3().y)
		&& (bb.getP3().z <= Intersections[min_index].z && Intersections[min_index].z <= bb.getP1().z)) {
		return Intersections[min_index];
	}

	// 5. 큐브 안에 없다면 충돌하지 않은 것이다.
	return Npc_defaultVec;
}
