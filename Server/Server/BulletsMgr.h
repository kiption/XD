#pragma once
#include "ObjectsMgr.h"

class Bullets : public Objects
{
private:
	int m_owner_id;
	XMFLOAT3 m_initial_pos;

public:
	Bullets() : Objects() {
		m_owner_id = -1;
		m_initial_pos = this->getPos();
	}
	Bullets(int id, XMFLOAT3 pos, float pitch, float yaw, float roll, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 look, int ownerID)
		: Objects(id, pos, pitch, yaw, roll, right, up, look)  {
		m_owner_id = ownerID;
		m_initial_pos = this->getPos();
	}

public:
	void setOwner(int ownerID) { m_owner_id = ownerID; }
	void setInitialPos(XMFLOAT3 initial_pos) { m_initial_pos = initial_pos; }

	int			getOwner() { return m_owner_id; }
	XMFLOAT3	getInitialPos() { return m_initial_pos; }
};
