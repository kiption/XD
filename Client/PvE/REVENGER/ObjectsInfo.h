#pragma once
#include <array>
#include <queue>
#include "../../../Server/RelayServer/Protocol.h"

enum OBJECT_STATE { OBJ_ST_EMPTY, OBJ_ST_STANDBY, OBJ_ST_LOGOUT, OBJ_ST_RUNNING };
struct ObjectsInfo
{
	short m_id;
	int m_hp;
	int m_bullet;
	XMFLOAT3 m_pos;
	XMFLOAT3 m_right_vec;
	XMFLOAT3 m_up_vec;
	XMFLOAT3 m_look_vec;
	int m_state;

	bool m_damaged_effect_on;

	ObjectsInfo() {
		m_id = -1;
		m_hp = 100;
		m_bullet = 100;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
		m_damaged_effect_on = false;
	}

	void returnToInitialState() {
		m_id = -1;
		m_hp = 100;
		m_bullet = 100;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
		m_damaged_effect_on = false;
	}
};

ObjectsInfo my_info;
std::array<ObjectsInfo, MAX_USER> other_players;
std::array<ObjectsInfo, MAX_NPCS> npcs_info;
int left_npc;

std::array<ObjectsInfo, MAX_BULLET> bullets_info;
