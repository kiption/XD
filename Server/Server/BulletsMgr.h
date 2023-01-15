#pragma once
#include "ObjectsMgr.h"

class Bullets : public Objects
{
private:
	int m_owner_id;

public:
	Bullets() : Objects() {
		m_owner_id = -1;
	}
	Bullets(int id, XMFLOAT3 pos, float pitch, float yaw, float roll, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 look, int ownerID)
		: Objects(id, pos, pitch, yaw, roll, right, up, look)  {
		m_owner_id = ownerID;
	}

public:
	void setOwner(int ownerID) { m_owner_id = ownerID; }

	int getOwner() { return m_owner_id; }
};
