#pragma once
#include <array>
#include "../Server/Server/protocol.h"

enum OBJECT_STATE { ST_EMPTY, ST_RUNNING };
struct ObjectsInfo
{
	short m_id;
	float m_x, m_y, m_z;
	int m_state;

	ObjectsInfo() {
		m_id = -1;
		m_x = 0, m_y = 0, m_z = 0;
		m_state = ST_EMPTY;
	}
	ObjectsInfo(short id, float x, float y, float z) {
		m_id = id;
		m_x = x, m_y = y, m_z = z;
		m_state = ST_RUNNING;
	}
};

ObjectsInfo my_info;
std::array<ObjectsInfo, MAX_USER> other_players;