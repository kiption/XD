#pragma once
#include <math.h>
#include "MyVectors.h"

// ===========================================
//			   Object 관련 함수들
// ===========================================
XMFLOAT3 myNormalize(XMFLOAT3 vec);

XMFLOAT3 calcMove(XMFLOAT3 vec1, XMFLOAT3 vec2, float scalar);
XMFLOAT3 calcRotate(XMFLOAT3 vec, float roll, float pitch, float yaw);


// ===========================================
//				Objects 클래스
// ===========================================
class Objects
{
private:
	int m_id;
	XMFLOAT3 m_pos;								// Position (x, y, z)
	float m_pitch, m_yaw, m_roll;				// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;	// 현재 Look, Right, Up Vectors

public:
	Objects() {
		m_id = -1;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_pitch = m_yaw = m_roll = 0.0f;
		m_rightvec = { 0.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 0.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 0.0f };
	}
	Objects(int id, XMFLOAT3 pos, float pitch, float yaw, float roll, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 look) {
		m_id = id;
		m_pos = pos;
		m_pitch = pitch;
		m_yaw = yaw;
		m_roll = roll;
		m_rightvec = right;
		m_upvec = up;
		m_lookvec = look;
	}
	~Objects() {
	}

public:
	// 1. Accessor Function
	//  1-1. Set
	void setId(int id) { m_id = id; }
	void setPos(XMFLOAT3 pos) { m_pos = pos; }
	void setPitch(float pitch) { m_pitch = pitch; }
	void setYaw(float yaw) { m_yaw = yaw; }
	void setRoll(float roll) { m_roll = roll; }
	void setRightvector(XMFLOAT3 right) { m_rightvec = right; }
	void setUpvector(XMFLOAT3 up) { m_upvec = up; }
	void setLookvector(XMFLOAT3 look) { m_lookvec = look; }
	//  1-2. Get
	int			getId() { return m_id; }
	XMFLOAT3	getPos() { return m_pos; }
	float		getPitch() { return m_pitch; }
	float		getYaw() { return m_yaw; }
	float		getRoll() { return m_roll; }
	XMFLOAT3	getRightvector() { return m_rightvec; }
	XMFLOAT3	getUpvector() { return m_upvec; }
	XMFLOAT3	getLookvector() { return m_lookvec; }

public:
	void moveObj(XMFLOAT3 direction, float scalar);
	void rotateObj(float roll, float pitch, float yaw);

};
