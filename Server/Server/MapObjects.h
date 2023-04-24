#pragma once
#include <filesystem>
#include <fstream>
#include <string>

float string2data(std::string str);

class MapObject
{
private:
	float pos_x, pos_y, pos_z;
	float scale_x, scale_y, scale_z;

public:
	MapObject() {
		pos_x = pos_y = pos_z = 0;
		scale_x = scale_y = scale_z = 0;
	}
	MapObject(float px, float py, float pz, float sx, float sy, float sz) {
		pos_x = px, pos_y = py, pos_z = pz;
		scale_x = sz, scale_y = sy, scale_z = sz;
	}

public:
	void setPos(float px, float py, float pz) {
		pos_x = px, pos_y = py, pos_z = pz;
	}
	void setScale(float sx, float sy, float sz) {
		scale_x = sz, scale_y = sy, scale_z = sz;
	}

	float getPosX() { return pos_x; }
	float getPosY() { return pos_y; }
	float getPosZ() { return pos_z; }
	float getScaleX() { return scale_x; }
	float getScaleY() { return scale_y; }
	float getScaleZ() { return scale_z; }
};