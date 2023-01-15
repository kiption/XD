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

XMFLOAT3 calcRotate(XMFLOAT3 vec, float roll, float pitch, float yaw)
{
	// roll
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
	return myNormalize(vec);
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
