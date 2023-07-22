#pragma once

class MapObject
{
private:
	int obj_type;
	float pos_x, pos_y, pos_z;
	float scale_x, scale_y, scale_z;

	XMFLOAT3 local_forward;
	XMFLOAT3 local_right;
	XMFLOAT3 local_rotate;

	float angle_aob;
	float angle_boc;

public:
	BoundingOrientedBox m_xoobb;
	int id;

public:
	MapObject() {
		obj_type = -1;

		pos_x = pos_y = pos_z = 0;
		scale_x = scale_y = scale_z = 0;

		local_forward = { 0,0,0 };
		local_right = { 0,0,0 };
		local_rotate = { 0,0,0 };

		angle_aob = 0.f;
		angle_boc = 0.f;
	}

public:
	void setPos(float px, float py, float pz) {
		pos_x = px, pos_y = py, pos_z = pz;
	}
	void setScale(float sx, float sy, float sz) {
		scale_x = sx, scale_y = sy, scale_z = sz;
	}

public:
	float getPosX() { return pos_x; }
	float getPosY() { return pos_y; }
	float getPosZ() { return pos_z; }
	float getScaleX() { return scale_x; }
	float getScaleY() { return scale_y; }
	float getScaleZ() { return scale_z; }

public:
	void setPos2(XMFLOAT3 p) { setPos(p.x, p.y, p.z); }
	void setScale2(XMFLOAT3 s) { setScale(s.x, s.y, s.z); }
	void setLocalForward(XMFLOAT3 localforward) { local_forward = localforward; }
	void setLocalRight(XMFLOAT3 localright) { local_right = localright; }
	void setLocalRotate(XMFLOAT3 localrotate) { local_rotate = localrotate; }
	void setAngleAOB(float angleaob) { angle_aob = angleaob; }
	void setAngleBOC(float angleboc) { angle_boc = angleboc; }

public:
	XMFLOAT3 getPos2() { return XMFLOAT3{ getPosX(), getPosY(), getPosZ() }; }
	XMFLOAT3 getScale2() { return XMFLOAT3{ getScaleX(), getScaleY(), getScaleZ() }; }
	XMFLOAT3 getLocalForward() { return local_forward; }
	XMFLOAT3 getLocalRight() { return local_right; }
	XMFLOAT3 getLocalRotate() { return local_rotate; }
	float getAngleAOB() { return angle_aob; }
	float getAngleBOC() { return angle_boc; }

public:
	void setBB() {
		XMFLOAT4 orientation(local_rotate.x, local_rotate.y, local_rotate.z, 1.f);

		m_xoobb = BoundingOrientedBox(XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()),
			XMFLOAT3(this->getScaleX(), this->getScaleY(), this->getScaleZ()),
			orientation);
	}
	XMFLOAT3 getPos() { return XMFLOAT3(this->getPosX(), this->getPosY(), this->getPosZ()); }
};