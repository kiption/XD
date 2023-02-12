#include "ObjectsMgr.h"

// ===========================================
//			   Object 관련 함수들
// ===========================================
XMFLOAT3 myNormalize(XMFLOAT3 vec)
{
	float dist = sqrtf(powf(vec.x, 2) + powf(vec.y, 2) + powf(vec.z, 2));

	vec.x = vec.x / dist;
	vec.y = vec.y / dist;
	vec.z = vec.z / dist;

	return vec;
}

XMFLOAT3 calcMove(XMFLOAT3 vec1, XMFLOAT3 vec2, float scalar)
{
	vec1.x = vec1.x + vec2.x * scalar;
	vec1.y = vec1.y + vec2.y * scalar;
	vec1.z = vec1.z + vec2.z * scalar;

	return vec1;
}

XMFLOAT3 calcRotate(XMFLOAT3 vec, float pitch, float yaw, float roll)
{
	float cur_pitch = XMConvertToRadians(pitch);
	float cur_yaw = XMConvertToRadians(yaw);
	float cur_roll = XMConvertToRadians(roll);

	// roll
	float x1, y1;
	x1 = vec.x * cos(cur_roll) - vec.y * sin(cur_roll);
	y1 = vec.x * sin(cur_roll) + vec.y * cos(cur_roll);

	// pitch
	float y2, z1;
	y2 = y1 * cos(cur_pitch) - vec.z * sin(cur_pitch);
	z1 = y1 * sin(cur_pitch) + vec.z * cos(cur_pitch);

	// yaw
	float x2, z2;
	z2 = z1 * cos(cur_yaw) - x1 * sin(cur_yaw);
	x2 = z1 * sin(cur_yaw) + x1 * cos(cur_yaw);

	// Update
	vec = { x2, y2, z2 };
	return myNormalize(vec);
}


void Objects::clear()
{
	m_id = -1;
	m_pos = { 0.0f, 0.0f, 0.0f };
	m_pitch = m_yaw = m_roll = 0.0f;
	m_rightvec = { 0.0f, 0.0f, 0.0f };
	m_upvec = { 0.0f, 0.0f, 0.0f };
	m_lookvec = { 0.0f, 0.0f, 0.0f };
}

// ===========================================
//				Objects 클래스
// ===========================================
void Objects::moveObj(XMFLOAT3 direction, float scalar)
{
	XMFLOAT3 moveResult = calcMove(Objects::getPos(), direction, scalar);
	setPos(moveResult);
}

void Objects::rotateObj(float roll, float pitch, float yaw)
{
	Coordinate basic_coordinate;	// 기본(초기) 좌표계
	setRightvector(calcRotate(basic_coordinate.right, m_roll, m_pitch, m_yaw));
	setUpvector(calcRotate(basic_coordinate.up, m_roll, m_pitch, m_yaw));
	setLookvector(calcRotate(basic_coordinate.look, m_roll, m_pitch, m_yaw));
}

float Objects::calcDistance(XMFLOAT3 others_pos)
{
	float dist = 0;
	
	float x_difference = pow(others_pos.x - m_pos.x, 2);
	float y_difference = pow(others_pos.y - m_pos.y, 2);
	float z_difference = pow(others_pos.z - m_pos.z, 2);
	dist = sqrtf(x_difference + y_difference + z_difference);

	return dist;
}

bool Objects::intersectsCheck(BoundingOrientedBox other_bb)
{
	if (m_xoobb.Intersects(other_bb)) {
		return true;
	}

	return false;
}
