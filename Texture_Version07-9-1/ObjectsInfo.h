#pragma once
#include <array>
#include "../Server/Server/protocol.h"

struct MyFloat3 {
	float x, y, z;
};

enum OBJECT_STATE { OBJ_ST_EMPTY, OBJ_ST_LOGOUT, OBJ_ST_RUNNING };
struct ObjectsInfo
{
	short m_id;
	float m_x, m_y, m_z;
	MyFloat3 m_right_vec;
	MyFloat3 m_up_vec;
	MyFloat3 m_look_vec;
	int m_state;

	ObjectsInfo() {
		m_id = -1;
		m_x = 0, m_y = 0, m_z = 0;
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
	}
};

ObjectsInfo my_info;
std::array<ObjectsInfo, MAX_USER> other_players;
std::array<ObjectsInfo, MAX_NPCS> npcs_info;