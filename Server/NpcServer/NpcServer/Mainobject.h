#pragma once

class OBJECT {
public:
	mutex obj_lock;

	int id;
	char name[NAME_SIZE];
	short type;

	short state;	// PLAYER_STATE
	int hp;
	int remain_bullet;

	XMFLOAT3 pos;									// Position (x, y, z)
	float pitch, yaw, roll;							// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// ÇöÀç Look, Right, Up Vectors

	BoundingOrientedBox m_xoobb;	// Bounding Box

public:
	OBJECT()
	{
		id = -1;
		name[0] = 0;

		state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void setBB() {

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	void memberClear()
	{
		id = -1;
		name[0] = 0;

		state = PL_ST_IDLE;
		hp = 1000;
		remain_bullet = MAX_BULLET;

		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };

		setBB();
	}
};