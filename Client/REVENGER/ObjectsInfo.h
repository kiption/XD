#pragma once
#include <array>
#include <vector>

//#include "../../../Server/MainServer/Server/Protocol.h"
///#include "C:\Users\재성\Desktop\졸업작품\XD\Server\MainServer\Server\Protocol.h"
#include "../REVENGER/Protocol.h"

enum OBJECT_STATE { OBJ_ST_EMPTY, OBJ_ST_STANDBY, OBJ_ST_LOGOUT, OBJ_ST_RUNNING };
struct ObjectsInfo
{
	short m_id;
	char m_name[20];
	int m_role;
	int m_hp;
	int m_bullet;
	XMFLOAT3 m_pos;
	XMFLOAT3 m_right_vec;
	XMFLOAT3 m_up_vec;
	XMFLOAT3 m_look_vec;
	XMFLOAT3 m_attack_dir;
	int m_state;		// 세션 상태
	volatile int curr_scene;		// 몇번째 씬인지
	int m_ingame_state;	// 인게임내 상태

	bool m_new_state_update; // 상태가 바뀌었는지
	bool m_attack_on;	// 공격을 했는 지
	bool m_damaged_effect_on;
	bool m_near_death_hp;	// HP 30%미만

	ObjectsInfo() {
		m_id = -1;
		strcpy_s(m_name, "\0");
		m_role = ROLE_NOTCHOOSE;
		m_hp = 100;
		m_bullet = MAX_BULLET;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_attack_dir = { 0.0f, 0.0f, 1.0f };
		m_state = OBJ_ST_EMPTY;
		curr_scene = 0;
		m_ingame_state = PL_ST_DEAD;
		m_new_state_update = false;
		m_damaged_effect_on = false;
		m_attack_on = false;
		m_near_death_hp = false;
	}

	void InfoClear() {
		m_id = -1;
		strcpy_s(m_name, "\0");
		m_role = ROLE_NOTCHOOSE;
		m_hp = 100;
		m_bullet = MAX_BULLET;
		m_pos = { 0.0f, 0.0f, 0.0f };
		m_right_vec = { 1.0f, 0.0f, 0.0f };
		m_up_vec = { 0.0f, 1.0f, 0.0f };
		m_look_vec = { 0.0f, 0.0f, 1.0f };
		m_attack_dir = { 0.0f, 0.0f, 1.0f };
		curr_scene = 0;
		m_state = OBJ_ST_EMPTY;
		m_ingame_state = PL_ST_DEAD;
		m_new_state_update = false;
		m_damaged_effect_on = false;
		m_attack_on = false;
		m_near_death_hp = false;
	}
};

std::array<ObjectsInfo, MAX_USER> players_info;
std::array<ObjectsInfo, MAX_NPCS> npcs_info;
int left_npc = 0;

//==================================================
struct MapObjectsInfo
{
	XMFLOAT3 m_pos;
	XMFLOAT3 m_scale;
	XMFLOAT3 m_local_forward;
	XMFLOAT3 m_local_right;
	XMFLOAT3 m_local_rotate;
	float m_angle_aob;
	float m_angle_boc;

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
		XMFLOAT4 oriented(m_local_rotate.x, m_local_rotate.y, m_local_rotate.z, 1.0f);
		m_xoobb = BoundingOrientedBox(XMFLOAT3(m_pos.x, m_pos.y, m_pos.z), XMFLOAT3(m_scale.x, m_scale.y, m_scale.z), oriented);
	}
};

std::vector<MapObjectsInfo> stage1_mapobj_info;

//==================================================
enum DeadObjType { D_OBJ_PLAYER, D_OBJ_NPC };
struct DeathInfo
{
	char obj_type;
	int  obj_id;

	DeathInfo() { obj_type = -1; obj_id = -1; }
	DeathInfo(char type, int id) { obj_type = type; obj_id = id; }
};
std::queue<DeathInfo> new_death_objs;

//==================================================
struct RoomInfo
{
	int room_id;
	char room_name[ROOM_NAME_SIZE];
	int room_state;
	int user_count;
	int user_state[MAX_USER];

	RoomInfo() {
		room_id = -1;
		strcpy_s(room_name, "\0");
		room_state = R_ST_WAIT;
		user_count = 0;
		for (int i = 0; i < MAX_USER;++i)
			user_state[i] = RM_ST_EMPTY;
	}
};
std::vector<RoomInfo> lobby_rooms;
RoomInfo curr_room;
int curr_room_id = -1;
bool b_room_manager = false;
int my_room_index = -1;
bool b_room_PlzChooseRole = false;
volatile bool b_room_AnyOneNotReady = false;
float b_roomPlzRoleTime = 0.0f;
float b_roomNotReadyTime = 0.0f;
bool b_startTime = false;

void CurrRoomInfoClear() {	// Room에서 나와 Lobby로 이동할 때 그동안 있었던 방에 대한 정보를 초기화하는 함수입니다.
	curr_room.room_id = -1;
	strcpy_s(curr_room.room_name, "\0");
	curr_room.room_state = R_ST_WAIT;
	curr_room.user_count = 0;
	for (int i = 0; i < MAX_USER; ++i) {
		curr_room.user_state[i] = RM_ST_EMPTY;
	}

	curr_room_id = -1;
	b_room_manager = false;
	my_room_index = -1;
}

int removeRoom(int target_room_id) {
	lobby_rooms.erase(remove_if(lobby_rooms.begin(), lobby_rooms.end(), [target_room_id](RoomInfo room){return room.room_id == target_room_id;}), lobby_rooms.end());
	cout << "Room[" << target_room_id << "]가 제거되었습니다." << endl;
	return 0;
}

//==================================================
queue<XMFLOAT3> q_bullet_hit_pos_mapobj;
queue<XMFLOAT3> q_bullet_hit_pos_ground;

