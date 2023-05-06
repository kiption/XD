#pragma once
#include <array>
#include <vector>
#include "../../../Server/RelayServer/Protocol.h"

enum OBJECT_STATE { OBJ_ST_EMPTY, OBJ_ST_STANDBY, OBJ_ST_LOGOUT, OBJ_ST_RUNNING };
struct ObjectsInfo
{
	short m_id;
	char m_name[20];
	int m_hp;
	int m_bullet;
	XMFLOAT3 m_pos;
	XMFLOAT3 m_right_vec;
	XMFLOAT3 m_up_vec;
	XMFLOAT3 m_look_vec;
	int m_state;		// 세션 상태
	int m_ingame_state;	// 인게임내 상태

	bool m_damaged_effect_on;

	ObjectsInfo() {
		m_id = -1;
		m_name[0] = '\n';
		m_hp = 100;
		m_bullet = MAX_BULLET;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
		m_ingame_state = PL_ST_DEAD;
		m_damaged_effect_on = false;
	}

	void InfoClear() {
		m_id = -1;
		m_name[0] = '\n';
		m_hp = 100;
		m_bullet = MAX_BULLET;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
		m_ingame_state = PL_ST_DEAD;
		m_damaged_effect_on = false;
	}
};

ObjectsInfo my_info;
std::array<ObjectsInfo, MAX_USER> other_players;
std::array<ObjectsInfo, MAX_NPCS> npcs_info;
int left_npc;


struct MapObjectsInfo
{

	XMFLOAT3 m_pos;
	XMFLOAT3 m_scale;
	BoundingOrientedBox m_xoobb;

	MapObjectsInfo() {
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_scale = { 0.0f, 0.0f, 0.0f };;
		m_xoobb = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void InfoClear() {
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_scale = { 0.0f, 0.0f, 0.0f };
		m_xoobb = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void setBB() {
		m_xoobb = BoundingOrientedBox(XMFLOAT3(m_pos.x, m_pos.y, m_pos.z), XMFLOAT3(m_scale.x, m_scale.y, m_scale.z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}
};

std::vector<MapObjectsInfo> stage1_mapobj_info;

enum DeadObjType { D_OBJ_PLAYER, D_OBJ_NPC };
struct DeathInfo
{
	char obj_type;
	int  obj_id;

	DeathInfo() { obj_type = -1; obj_id = -1; }
	DeathInfo(char type, int id) { obj_type = type; obj_id = id; }
};
std::queue<DeathInfo> new_death_objs;