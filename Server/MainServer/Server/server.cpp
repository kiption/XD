#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <random>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

//======================================================================
#include "RayCast.h"
#include "Protocol.h"
#include "Timer.h"
#include "Constant.h"
#include "CP_KEYS.h"
#include "MapObjects.h"

//======================================================================
enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_CONNECT };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_RUNNING_SERVER, ST_DOWN_SERVER };
enum SESSION_TYPE { SESSION_CLIENT, SESSION_EXTENDED_SERVER, SESSION_NPC };

SOCKET manager_socket;

//======================================================================
chrono::system_clock::time_point g_s_start_time;	// 서버 시작시간  (단위: ms)
milliseconds g_curr_servertime;
bool b_isfirstplayer;	// 첫 player입장인지. (첫 클라 접속부터 서버시간이 흐르도록 하기 위함)
mutex servertime_lock;	// 서버시간 lock

//======================================================================
class Mission
{
public:
	short type;
	float goal;
	float curr;
	int start;				// 미션 시작 시간

public:
	Mission() {
		type = MISSION_KILL;
		goal = 0.0f;
		curr = 0.0f;
	}
};
mutex mission_lock;			// 미션 lock
array<int, TOTAL_STAGE + 1> curr_mission_stage;
array<Mission, ST1_MISSION_NUM> stage1_missions;

Mission setMission(short type, float goal, float curr) {
	Mission new_mission;
	new_mission.type = type;
	new_mission.goal = goal;
	new_mission.curr = curr;
	return new_mission;
}
void setMissions() {
	cout << "[Init Missions...]";
	// 스테이지1 미션
	stage1_missions[0] = setMission(MISSION_KILL, STAGE1_MISSION1_GOAL, 0.0f);
	stage1_missions[1] = setMission(MISSION_OCCUPY, STAGE1_MISSION2_GOAL * 1'000.f, 0.0f);

	cout << " ---- OK." << endl;
}

//======================================================================
array<MapObject, TOTAL_STAGE + 1> occupy_areas;	// 스테이지별 점령지역
void setOccupyAreas() {
	cout << "[Init Occupy Areas...]";
	// 스테이지0 에는 점령지역이 없다.
	occupy_areas[0] = MapObject{ -9999.f, -9999.f, -9999.f, 0.f, 0.f, 0.f };

	// 스테이지1 점령지역
	occupy_areas[1] = MapObject{ ST1_OCCUPY_AREA_POS_X, 0, ST1_OCCUPY_AREA_POS_Z, ST1_OCCUPY_AREA_SIZE_X, 0, ST1_OCCUPY_AREA_SIZE_Z };

	//
	cout << " ---- OK." << endl;
}

//======================================================================
class Building : public MapObject
{
private:
	XMFLOAT3 local_forward;
	XMFLOAT3 local_right;
	XMFLOAT3 local_rotate;
	float angle_aob;
	float angle_boc;

public:
	BoundingOrientedBox m_xoobb;

public:
	Building() : MapObject() {}

public:
	void setPos2(XMFLOAT3 p) { setPos(p.x, p.y, p.z); }
	void setScale2(XMFLOAT3 s) { setScale(s.x, s.y, s.z); }
	void setLocalForward(XMFLOAT3 localforward) { local_forward = localforward; }
	void setLocalRight(XMFLOAT3 localright) { local_right = localright; }
	void setLocalRotate(XMFLOAT3 localrotate) { local_rotate = localrotate; }
	void setAngleAOB(float angleaob) { angle_aob = angleaob; }
	void setAngleBOC(float angleboc) { angle_boc = angleboc; }

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
vector<Building> mapobj_info;	// Map Buildings CollideBox

//======================================================================
class OVER_EX {
public:
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	char send_buf[BUF_SIZE];
	PACKET_PROCESS_TYPE process_type;

	OVER_EX()
	{
		wsabuf.len = BUF_SIZE;
		wsabuf.buf = send_buf;
		process_type = OP_RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	OVER_EX(char* packet)
	{
		wsabuf.len = packet[0];
		wsabuf.buf = send_buf;
		ZeroMemory(&overlapped, sizeof(overlapped));
		process_type = OP_SEND;
		memcpy(send_buf, packet, packet[0]);
	}
};

//======================================================================
class SESSION {
	OVER_EX recv_over;

public:
	mutex s_lock;
	SESSION_STATE s_state;
	int inserver_index;

	int id;
	SOCKET socket;
	int remain_size;
	char name[NAME_SIZE];
	char role;

	short pl_state;
	int hp;
	int remain_bullet;
	XMFLOAT3 pos;									// Position (x, y, z)
	float pitch, yaw, roll;							// Rotated Degree
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// 현재 Look, Right, Up Vectors
	XMFLOAT3 m_cam_lookvec;							// 카메라 룩벡터 (조준을 여기에 하고 있음)

	bool b_occupying;	// 점령 중인지
	bool height_alert;	// 헬기는 일정고도 아래로 내려가면 경보음을 울린다.

	chrono::system_clock::time_point shoot_time;	// 총을 발사한 시간
	chrono::system_clock::time_point reload_time;	// 총알 장전 시작시간

	short curr_stage;

	bool in_spawn_area;		// 스폰지역에 있으면 조금씩 회복된다.

	BoundingOrientedBox m_xoobb;	// Bounding Box

	unordered_set<int> view_list;	// 시야처리 (id값을 넣어주게 된다.) => 그럼 무엇을 해야하냐: 플레이어, npc들의 ID를 완전 다른값으로 설정해줘야한다.

	bool oneshot_onekill_cheat;		//[치트키] 원샷 원킬
	bool immortal_cheat;			//[치트키] 무적

public:
	SESSION()
	{
		s_state = ST_FREE;
		id = -1;
		inserver_index = -1;
		socket = 0;
		remain_size = 0;
		name[0] = 0;

		pl_state = PL_ST_IDLE;
		hp = 0;
		remain_bullet = 0;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };
		height_alert = false;
		m_cam_lookvec = m_lookvec;
		curr_stage = 0;

		b_occupying = false;
		in_spawn_area = false;

		m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		oneshot_onekill_cheat = false;
		immortal_cheat = false;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "[Line: 224] WSARecv Error - " << GetLastError() << endl;
		}
	}

	void do_send(void* packet)
	{
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "[Line: 236] WSASend Error - " << GetLastError() << endl;
		}
	}

	void send_login_packet();
	void send_add_obj_packet(int obj_id, short obj_type);
	void send_move_packet(int obj_id, short obj_type, short dir);
	void send_rotate_packet(int obj_id, short obj_type);
	void send_move_rotate_packet(int obj_id, short obj_type, short dir);
	void send_mission_packet(short curr_stage);
	void send_remove_packet(int obj_id, short obj_type);

	void setBB() { m_xoobb = BoundingOrientedBox(XMFLOAT3(pos.x, pos.y, pos.z), XMFLOAT3(HUMAN_BBSIZE_X, HUMAN_BBSIZE_Y, HUMAN_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)); }

	void update_viewlist();

	void sessionClear() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		name[0] = 0;

		pl_state = PL_ST_IDLE;
		hp = 0;
		remain_bullet = 0;
		pos = { 0.0f, 0.0f, 0.0f };
		pitch = yaw = roll = 0.0f;
		m_rightvec = { 1.0f, 0.0f, 0.0f };
		m_upvec = { 0.0f, 1.0f, 0.0f };
		m_lookvec = { 0.0f, 0.0f, 1.0f };
		height_alert = false;
		m_cam_lookvec = m_lookvec;
		curr_stage = 0;

		b_occupying = false;
		in_spawn_area = false;

		setBB();

		oneshot_onekill_cheat = false;	// 원샷원킬 치트키
		immortal_cheat = false;	// 무적 치트키
	}
};

array<SESSION, MAX_USER> clients;
SESSION npc_server;
array<SESSION, MAX_NPCS> npcs;

void SESSION::send_login_packet() {
	SC_LOGIN_INFO_PACKET login_info_packet;
	login_info_packet.size = sizeof(SC_LOGIN_INFO_PACKET);
	login_info_packet.type = SC_LOGIN_INFO;

	strcpy_s(login_info_packet.name, name);
	login_info_packet.x = pos.x;
	login_info_packet.y = pos.y;
	login_info_packet.z = pos.z;

	login_info_packet.right_x = m_rightvec.x;
	login_info_packet.right_y = m_rightvec.y;
	login_info_packet.right_z = m_rightvec.z;

	login_info_packet.up_x = m_upvec.x;
	login_info_packet.up_y = m_upvec.y;
	login_info_packet.up_z = m_upvec.z;

	login_info_packet.look_x = m_lookvec.x;
	login_info_packet.look_y = m_lookvec.y;
	login_info_packet.look_z = m_lookvec.z;

	login_info_packet.hp = hp;
	login_info_packet.remain_bullet = remain_bullet;

	do_send(&login_info_packet);
}

void SESSION::send_add_obj_packet(int obj_id, short obj_type)
{
	switch (obj_type) {
	case TARGET_PLAYER:
		SC_ADD_OBJECT_PACKET add_player_packet;
		add_player_packet.size = sizeof(SC_ADD_OBJECT_PACKET);
		add_player_packet.type = SC_ADD_OBJECT;

		add_player_packet.target = TARGET_PLAYER;
		add_player_packet.id = clients[obj_id].id;
		strcpy_s(add_player_packet.name, clients[obj_id].name);
		add_player_packet.obj_state = clients[obj_id].pl_state;
		add_player_packet.role = clients[obj_id].role;

		add_player_packet.x = clients[obj_id].pos.x;
		add_player_packet.y = clients[obj_id].pos.y;
		add_player_packet.z = clients[obj_id].pos.z;

		add_player_packet.right_x = clients[obj_id].m_rightvec.x;
		add_player_packet.right_y = clients[obj_id].m_rightvec.y;
		add_player_packet.right_z = clients[obj_id].m_rightvec.z;

		add_player_packet.up_x = clients[obj_id].m_upvec.x;
		add_player_packet.up_y = clients[obj_id].m_upvec.y;
		add_player_packet.up_z = clients[obj_id].m_upvec.z;

		add_player_packet.look_x = clients[obj_id].m_lookvec.x;
		add_player_packet.look_y = clients[obj_id].m_lookvec.y;
		add_player_packet.look_z = clients[obj_id].m_lookvec.z;

		do_send(&add_player_packet);

		break;

	case TARGET_NPC:
		SC_ADD_OBJECT_PACKET add_npc_packet;
		add_npc_packet.size = sizeof(SC_ADD_OBJECT_PACKET);
		add_npc_packet.type = SC_ADD_OBJECT;

		add_npc_packet.target = TARGET_NPC;
		add_npc_packet.id = obj_id;
		strcpy_s(add_npc_packet.name, name);
		add_npc_packet.obj_state = npcs[obj_id].pl_state;

		add_npc_packet.x = npcs[obj_id].pos.x;
		add_npc_packet.y = npcs[obj_id].pos.y;
		add_npc_packet.z = npcs[obj_id].pos.z;

		add_npc_packet.right_x = npcs[obj_id].m_rightvec.x;
		add_npc_packet.right_y = npcs[obj_id].m_rightvec.y;
		add_npc_packet.right_z = npcs[obj_id].m_rightvec.z;

		add_npc_packet.up_x = npcs[obj_id].m_upvec.x;
		add_npc_packet.up_y = npcs[obj_id].m_upvec.y;
		add_npc_packet.up_z = npcs[obj_id].m_upvec.z;

		add_npc_packet.look_x = npcs[obj_id].m_lookvec.x;
		add_npc_packet.look_y = npcs[obj_id].m_lookvec.y;
		add_npc_packet.look_z = npcs[obj_id].m_lookvec.z;

		do_send(&add_npc_packet);

		break;
	}
}
void SESSION::send_move_packet(int obj_id, short obj_type, short dir)
{
	SC_MOVE_OBJECT_PACKET move_pl_packet;
	move_pl_packet.size = sizeof(SC_MOVE_OBJECT_PACKET);
	move_pl_packet.type = SC_MOVE_OBJECT;
	move_pl_packet.target = obj_type;
	
	move_pl_packet.direction = dir;

	switch (obj_type) {
	case TARGET_PLAYER:
		move_pl_packet.id = clients[obj_id].id;

		move_pl_packet.x = clients[obj_id].pos.x;
		move_pl_packet.y = clients[obj_id].pos.y;
		move_pl_packet.z = clients[obj_id].pos.z;
		break;

	case TARGET_NPC:
		move_pl_packet.id = obj_id;

		move_pl_packet.x = npcs[obj_id].pos.x;
		move_pl_packet.y = npcs[obj_id].pos.y;
		move_pl_packet.z = npcs[obj_id].pos.z;
		break;
	}

	do_send(&move_pl_packet);
}
void SESSION::send_rotate_packet(int obj_id, short obj_type)
{
	SC_ROTATE_OBJECT_PACKET rotate_pl_packet;
	rotate_pl_packet.size = sizeof(SC_ROTATE_OBJECT_PACKET);
	rotate_pl_packet.type = SC_ROTATE_OBJECT;
	rotate_pl_packet.target = obj_type;
	
	switch (obj_type) {
	case TARGET_PLAYER:
		rotate_pl_packet.id = clients[obj_id].id;

		rotate_pl_packet.right_x = clients[obj_id].m_rightvec.x;
		rotate_pl_packet.right_y = clients[obj_id].m_rightvec.y;
		rotate_pl_packet.right_z = clients[obj_id].m_rightvec.z;

		rotate_pl_packet.up_x = clients[obj_id].m_upvec.x;
		rotate_pl_packet.up_y = clients[obj_id].m_upvec.y;
		rotate_pl_packet.up_z = clients[obj_id].m_upvec.z;

		rotate_pl_packet.look_x = clients[obj_id].m_lookvec.x;
		rotate_pl_packet.look_y = clients[obj_id].m_lookvec.y;
		rotate_pl_packet.look_z = clients[obj_id].m_lookvec.z;
		break;

	case TARGET_NPC:
		rotate_pl_packet.id = obj_id;

		rotate_pl_packet.right_x = npcs[obj_id].m_rightvec.x;
		rotate_pl_packet.right_y = npcs[obj_id].m_rightvec.y;
		rotate_pl_packet.right_z = npcs[obj_id].m_rightvec.z;

		rotate_pl_packet.up_x = npcs[obj_id].m_upvec.x;
		rotate_pl_packet.up_y = npcs[obj_id].m_upvec.y;
		rotate_pl_packet.up_z = npcs[obj_id].m_upvec.z;

		rotate_pl_packet.look_x = npcs[obj_id].m_lookvec.x;
		rotate_pl_packet.look_y = npcs[obj_id].m_lookvec.y;
		rotate_pl_packet.look_z = npcs[obj_id].m_lookvec.z;
		break;
	}
	do_send(&rotate_pl_packet);
}
void SESSION::send_move_rotate_packet(int obj_id, short obj_type, short dir)
{
	SC_MOVE_ROTATE_OBJECT_PACKET update_pl_packet;
	update_pl_packet.size = sizeof(SC_MOVE_ROTATE_OBJECT_PACKET);
	update_pl_packet.type = SC_MOVE_ROTATE_OBJECT;
	update_pl_packet.target = obj_type;
	update_pl_packet.direction = dir;

	switch (obj_type) {
	case TARGET_PLAYER:
		update_pl_packet.id = clients[obj_id].id;
		
		update_pl_packet.x = clients[obj_id].pos.x;
		update_pl_packet.y = clients[obj_id].pos.y;
		update_pl_packet.z = clients[obj_id].pos.z;

		update_pl_packet.right_x = clients[obj_id].m_rightvec.x;
		update_pl_packet.right_y = clients[obj_id].m_rightvec.y;
		update_pl_packet.right_z = clients[obj_id].m_rightvec.z;

		update_pl_packet.up_x = clients[obj_id].m_upvec.x;
		update_pl_packet.up_y = clients[obj_id].m_upvec.y;
		update_pl_packet.up_z = clients[obj_id].m_upvec.z;

		update_pl_packet.look_x = clients[obj_id].m_lookvec.x;
		update_pl_packet.look_y = clients[obj_id].m_lookvec.y;
		update_pl_packet.look_z = clients[obj_id].m_lookvec.z;
		break;

	case TARGET_NPC:
		update_pl_packet.id = obj_id;

		update_pl_packet.x = npcs[obj_id].pos.x;
		update_pl_packet.y = npcs[obj_id].pos.y;
		update_pl_packet.z = npcs[obj_id].pos.z;

		update_pl_packet.right_x = npcs[obj_id].m_rightvec.x;
		update_pl_packet.right_y = npcs[obj_id].m_rightvec.y;
		update_pl_packet.right_z = npcs[obj_id].m_rightvec.z;

		update_pl_packet.up_x = npcs[obj_id].m_upvec.x;
		update_pl_packet.up_y = npcs[obj_id].m_upvec.y;
		update_pl_packet.up_z = npcs[obj_id].m_upvec.z;

		update_pl_packet.look_x = npcs[obj_id].m_lookvec.x;
		update_pl_packet.look_y = npcs[obj_id].m_lookvec.y;
		update_pl_packet.look_z = npcs[obj_id].m_lookvec.z;
		break;
	}
	do_send(&update_pl_packet);
}
void SESSION::send_mission_packet(short curr_stage)
{
	SC_MISSION_PACKET mission_packet;
	mission_packet.type = SC_MISSION;
	mission_packet.size = sizeof(SC_MISSION_PACKET);
	mission_packet.stage_num = curr_stage;
	int curr_mission = curr_mission_stage[curr_stage];
	mission_packet.mission_num = curr_mission;

	if (curr_stage == 1) {
		mission_packet.mission_type = stage1_missions[curr_mission].type;
		mission_packet.mission_goal = stage1_missions[curr_mission].goal;
		mission_packet.mission_curr = stage1_missions[curr_mission].curr;
	}
	else if (curr_stage == 2) {
		//mission_packet.mission_type = stage2_missions[curr_mission].type;
		//mission_packet.mission_goal = stage2_missions[curr_mission].goal;
		//mission_packet.mission_curr = stage2_missions[curr_mission].curr;
	}

	do_send(&mission_packet);
}
void SESSION::send_remove_packet(int obj_id, short obj_type)
{
	SC_REMOVE_OBJECT_PACKET remove_packet;
	remove_packet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	remove_packet.type = SC_REMOVE_OBJECT;
	remove_packet.target = obj_type;
	if (obj_type == TARGET_PLAYER)
		remove_packet.id = clients[obj_id].id;
	else if (obj_type == TARGET_NPC)
		remove_packet.id = obj_id;

	do_send(&remove_packet);
}

void SESSION::update_viewlist()
{
	// 1. 플레이어 업데이트
	for (auto& other_pl : clients) {
		if (other_pl.id == id) continue;
		if (other_pl.s_state != ST_INGAME) continue;
		if (other_pl.curr_stage != curr_stage) continue;

		int moved_pl_id = id + PLAYER_ID_START;		// 이동한 객체 (자신)
		int other_pl_id = other_pl.id + PLAYER_ID_START;		// 다른 객체

		// 1) 움직였더니 새로운 객체가 시야 안에 생긴 경우
		if (XMF_Distance(pos, other_pl.pos) <= HUMAN_VIEW_RANGE) {
			// 1-1) 자신의 ViewList에 시야에 새로 들어온 플레이어의 ID 추가
			if (!view_list.count(other_pl_id)) {
				s_lock.lock();
				view_list.insert(other_pl_id);
				s_lock.unlock();
			}
			// 1-2) 시야에 새로 들어온 플레이어의 ViewList에 자신의 ID추가
			if (!other_pl.view_list.count(moved_pl_id)) {
				other_pl.s_lock.lock();
				other_pl.view_list.insert(moved_pl_id);
				other_pl.s_lock.unlock();
			}
		}
		// 2) 움직였더니 원래는 시야에 있었던 객체가 시야에서 사라진 경우
		else {
			// 2-1) 자신의 ViewList에 시야에 새로 들어온 플레이어의 ID 제거
			if (view_list.count(other_pl_id)) {
				s_lock.lock();
				view_list.erase(other_pl_id);
				s_lock.unlock();
			}
			// 2-2) 시야에 새로 들어온 플레이어의 ViewList에 자신의 ID 제거
			if (other_pl.view_list.count(moved_pl_id)) {
				other_pl.s_lock.lock();
				other_pl.view_list.erase(moved_pl_id);
				other_pl.s_lock.unlock();
			}
		}
	}

	// 2. NPC 업데이트 (임시로 더미로 대체함.)
	for (auto& npc : npcs) {
		if (npc.id == -1) continue;
		int npc_id = npc.id + NPC_ID_START;

		// 1) 움직였더니 새로운 객체가 시야 안에 생긴 경우
		if (XMF_Distance(pos, npc.pos) <= HUMAN_VIEW_RANGE) {
			// 1-1) 자신의 ViewList에 시야에 새로 들어온 플레이어의 ID 추가
			if (!view_list.count(npc_id)) {
				s_lock.lock();
				view_list.insert(npc_id);
				s_lock.unlock();
			}
			// 더미는 View List X
		}
		// 2) 움직였더니 원래는 시야에 있었던 객체가 시야에서 사라진 경우
		else {
			// 2-1) 자신의 ViewList에 시야에 새로 들어온 플레이어의 ID 제거
			if (view_list.count(npc_id)) {
				s_lock.lock();
				view_list.erase(npc_id);
				s_lock.unlock();
			}
			// Npc의 시야는 Npc서버에서 관리.
		}
	}
}

//======================================================================
class HA_SERVER {
	OVER_EX recv_over;

public:
	mutex s_lock;
	SESSION_STATE s_state;
	int id;
	SOCKET socket;
	int remain_size;
	chrono::system_clock::time_point heartbeat_recv_time;
	chrono::system_clock::time_point heartbeat_send_time;

public:
	HA_SERVER() {
		s_state = ST_FREE;
		id = -1;
		socket = 0;
		remain_size = 0;
		heartbeat_recv_time = chrono::system_clock::now();
		heartbeat_send_time = chrono::system_clock::now();
	}
	~HA_SERVER() {}

	void do_recv() {
		DWORD recv_flag = 0;
		memset(&recv_over.overlapped, 0, sizeof(recv_over.overlapped));
		recv_over.wsabuf.len = BUF_SIZE - remain_size;
		recv_over.wsabuf.buf = recv_over.send_buf + remain_size;

		int ret = WSARecv(socket, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSARecv Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
	void do_send(void* packet) {
		OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
		ZeroMemory(&s_data->overlapped, sizeof(s_data->overlapped));

		//cout << "[do_send] Target ID: " << id << "\n" << endl;
		int ret = WSASend(socket, &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, 0);
		if (ret != 0 && GetLastError() != WSA_IO_PENDING) {
			cout << "WSASend Error - " << ret << endl;
			cout << GetLastError() << endl;
		}
	}
};
array<HA_SERVER, MAX_LOGIC_SERVER> extended_servers;	// HA구현을 위해 수평확장된 서버들

//======================================================================
class HEALPACK
{
public:
	XMFLOAT3 pos;
	bool is_used;	// 사용되었는가?
	chrono::system_clock::time_point used_time;	// 사용된 시간
};
array<HEALPACK, 8> g_healpacks;

void initHealpacks() {
	for (int i = 0; i < 8; ++i) {
		g_healpacks[i].is_used = false;
		g_healpacks[i].pos.y = 6.0f;
	}

	g_healpacks[0].pos.x = HEALPACK_0_CENTER_X;
	g_healpacks[0].pos.z = HEALPACK_0_CENTER_Z;

	g_healpacks[1].pos.x = HEALPACK_1_CENTER_X;
	g_healpacks[1].pos.z = HEALPACK_1_CENTER_Z;

	g_healpacks[2].pos.x = HEALPACK_2_CENTER_X;
	g_healpacks[2].pos.z = HEALPACK_2_CENTER_Z;

	g_healpacks[3].pos.x = HEALPACK_3_CENTER_X;
	g_healpacks[3].pos.z = HEALPACK_3_CENTER_Z;

	g_healpacks[4].pos.x = HEALPACK_4_CENTER_X;
	g_healpacks[4].pos.z = HEALPACK_4_CENTER_Z;

	g_healpacks[5].pos.x = HEALPACK_5_CENTER_X;
	g_healpacks[5].pos.z = HEALPACK_5_CENTER_Z;

	g_healpacks[6].pos.x = HEALPACK_6_CENTER_X;
	g_healpacks[6].pos.z = HEALPACK_6_CENTER_Z;

	g_healpacks[7].pos.x = HEALPACK_7_CENTER_X;
	g_healpacks[7].pos.z = HEALPACK_7_CENTER_Z;
}

//======================================================================
HANDLE h_iocp;				// IOCP 핸들
SOCKET g_sc_listensock;		// 클라이언트 통신 listen소켓
SOCKET g_npc_listensock;	// NPC서버 통신 listen소켓
SOCKET g_ss_listensock;		// 수평확장 서버 간 통신 listen 소켓

SOCKET left_ex_server_sock;								// 이전 번호의 서버
SOCKET right_ex_server_sock;							// 다음 번호의 서버

int my_server_id;										// 내 서버 식별번호
bool b_active_server;									// Active 서버인가?
bool b_npcsvr_conn;										// NPC서버가 연결되었는가?

//======================================================================
void resetGame() {	// 한판이 끝날때마다 모든 게임정보를 초기상태로 돌려놓는다.
	cout << "===Reset Game===" << endl;
	SC_RESET_GAME_PACKET reset_game_packet;
	reset_game_packet.size = sizeof(SC_RESET_GAME_PACKET);
	reset_game_packet.type = SC_RESET_GAME;

	// Player
	cout << "Players Info Reset...";
	for (auto& cl : clients) {
		cl.s_lock.lock();
		cl.sessionClear();
		cl.s_lock.unlock();
	}
	cout << " - OK." << endl;

	// Time
	cout << "Server Time Reset...";
	servertime_lock.lock();
	g_s_start_time = system_clock::now();
	g_curr_servertime = milliseconds(0);
	b_isfirstplayer = true;
	servertime_lock.unlock();
	cout << " - OK." << endl;

	// Missions
	cout << "Missions Info Reset...";
	mission_lock.lock();
	curr_mission_stage[1] = 0;
	stage1_missions[0].curr = 0.f;
	stage1_missions[1].curr = 0.f;
	stage1_missions[0].start = static_cast<int>(g_curr_servertime.count());
	stage1_missions[1].start = static_cast<int>(g_curr_servertime.count());
	mission_lock.unlock();
	cout << " - OK." << endl;

	// NPC
	cout << "NPCs Info Reset...";
	for (auto& npc : npcs) {
		npc.s_lock.lock();
		npc.sessionClear();
		npc.s_lock.unlock();
	}
	if (b_npcsvr_conn) {
		lock_guard<mutex> lg{ npc_server.s_lock };
		npc_server.do_send(&reset_game_packet);
	}
	cout << " - OK." << endl;

	cout << "====\n" << endl;
}

//======================================================================
void disconnect(int target_id, int target)
{
	switch (target) {
	case SESSION_CLIENT:
	{
		clients[target_id].s_lock.lock();
		if (clients[target_id].s_state == ST_FREE) {
			clients[target_id].s_lock.unlock();
			return;
		}
		closesocket(clients[target_id].socket);
		clients[target_id].s_state = ST_FREE;
		clients[target_id].s_lock.unlock();

		cout << "Player[ID: " << clients[target_id].id << ", name: " << clients[target_id].name << "] is log out\n" << endl;	// server message

		// 남아있는 모든 클라이언트들에게 target_id번 클라이언트가 접속 종료한 사실을 알립니다.
		int remain_user_cnt = 0;
		for (int i = 0; i < MAX_USER; i++) {
			auto& pl = clients[i];

			if (pl.id == target_id) continue;

			pl.s_lock.lock();
			if (pl.s_state != ST_INGAME) {
				pl.s_lock.unlock();
				continue;
			}
			remain_user_cnt++;
			pl.send_remove_packet(target_id, TARGET_PLAYER);
			pl.s_lock.unlock();
		}

		// NPC 서버에게도 target_id번 클라이언트가 접속 종료한 사실을 알립니다.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_remove_packet(target_id, TARGET_PLAYER);
		}

		// 만약 남아있는 플레이어가 없다면 게임을 초기화합니다.
		if (remain_user_cnt == 0) {
			cout << "방금 접속 종료한 클라이언트가 마지막 클라이언트였습니다. 게임을 초기화합니다." << endl;
			resetGame();
		}

		break;
	}
	case SESSION_NPC:
		cout << "NPC 서버가 다운되었습니다." << endl;
		closesocket(npc_server.socket);
		npc_server.socket = 0;
		b_npcsvr_conn = false;
		break;

	case SESSION_EXTENDED_SERVER:
	{
		extended_servers[target_id].s_lock.lock();
		if (extended_servers[target_id].s_state == ST_FREE) {
			extended_servers[target_id].s_lock.unlock();
			return;
		}
		closesocket(extended_servers[target_id].socket);
		extended_servers[target_id].s_state = ST_FREE;
		extended_servers[target_id].s_lock.unlock();

		cout << "Server[" << extended_servers[target_id].id << "]의 다운이 감지되었습니다." << endl;	// server message

		// 서버 재실행
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + target_id);	// 십의자리: Actvie여부(S: 1, A: 2), 일의자리: 서버ID

		// XD폴더 내에서 동작할 때(내부 테스트)와 외부에서 실행할 때를 구분해줍니다.
		string XDFolderKeyword = "XD";
		if (filesystem::current_path().string().find(XDFolderKeyword) != string::npos) {
			ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../../../Execute/Execute_S", SW_SHOW);	// 내부 테스트용
		}
		else {
			ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L".", SW_SHOW);					// 외부 수출용 (exe로 실행될때)
		}

		// 원격 이중화를 위해선 실행되는 PC의 "외부 IP"를 알아야 한다.


		// 클라이언트에게 Active서버가 다운되었다고 알려줌.
		if (!b_active_server) {	// 내가 Active가 아니면 상대가 Active임. (서버가 2개밖에 없기 때문)
			b_active_server = true;
			cout << "현재 Server[" << my_server_id << "] 가 Active 서버로 승격되었습니다. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// 만약 자신의 오른쪽 서버가 다운되었는데, 그 서버가 서버군의 마지막 서버인 경우 재실행된 서버에게 ConnectEx 요청을 보냅니다.
		if (target_id == MAX_LOGIC_SERVER - 1) {
			if (my_server_id < target_id) {
				// ConnectEx
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				int key = CP_KEY_LOGIC2EXLOGIC + MAX_LOGIC_SERVER - 1;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_LOGIC2EXLOGIC + HA_PORTNUM_S0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버
				inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect 재시도 중...\n" << endl;
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
		}
		break;
	}
	}
}

//======================================================================
void process_packet(int client_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* login_packet = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		clients[client_id].s_lock.lock();
		if (clients[client_id].s_state == ST_FREE) {
			clients[client_id].s_lock.unlock();
			break;
		}
		if (clients[client_id].s_state == ST_INGAME) {
			clients[client_id].s_lock.unlock();
			disconnect(client_id, SESSION_CLIENT);
			break;
		}

		// 서버구동 이후 첫번째 클라이언트의 접속이라면 그때부터 서버시간이 흐르기 시작합니다.
		if (b_isfirstplayer) {
			cout << "서버 시간이 흐르기 시작합니다.\n" << endl;
			g_s_start_time = system_clock::now();
			b_isfirstplayer = false;
		}

		// 새로 접속한 플레이어의 초기 정보를 설정합니다.
		clients[client_id].id = login_packet->inroom_index;	// client_id (서버에서 쓰는 id)와 클라이언트 인덱스는 다르다!
		clients[client_id].s_state = ST_INGAME;
		clients[client_id].pl_state = PL_ST_IDLE;
		clients[client_id].curr_stage = 1;
		clients[client_id].hp = HELI_MAXHP;
		clients[client_id].role = login_packet->role;
		strcpy_s(clients[client_id].name, login_packet->name);

		if (login_packet->role == ROLE_RIFLE) {
			clients[client_id].pos.x = SPAWN_POS_X_HUMAN + 15 * client_id;
			clients[client_id].pos.y = SPAWN_POS_Y_HUMAN;
			clients[client_id].pos.z = SPAWN_POS_Z_HUMAN;
			clients[client_id].remain_bullet = MAX_BULLET;
		}
		else if (login_packet->role == ROLE_HELI) {
			clients[client_id].pos.x = SPAWN_POS_X_HELI;
			clients[client_id].pos.y = SPAWN_POS_Y_HELI;
			clients[client_id].pos.z = SPAWN_POS_Z_HELI;
			clients[client_id].remain_bullet = MAX_BULLET_HELI;
		}

		clients[client_id].m_rightvec = XMFLOAT3{ 1.0f, 0.0f, 0.0f };
		clients[client_id].m_upvec = XMFLOAT3{ 0.0f, 1.0f, 0.0f };
		clients[client_id].m_lookvec = XMFLOAT3{ 0.0f, 0.0f, 1.0f };

		clients[client_id].setBB();

		clients[client_id].send_login_packet();
		clients[client_id].s_lock.unlock();
		cout << "Player[서버내 ID: " << client_id << ", 클라 인덱스: " << clients[client_id].id
			<< ", name: " << clients[client_id].name << "]이(가) 접속하였습니다." << endl;	// server message

		if (!b_active_server) {
			cout << "Stand-By서버는 대기 상태를 유지합니다." << endl;
			break;	// Active서버가 아니라면, 클라이언트가 연결되었음을 사용자에게 알리기만 하고 아무일도 하지 않습니다.
		}

		cout << "새로운 오브젝트가 생성되었습니다! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ").\n" << endl;

		clients[client_id].update_viewlist();

		//====================
		// 스테이지1 미션 전달
		{
			lock_guard<mutex> lg{ clients[client_id].s_lock };
			clients[client_id].send_mission_packet(clients[client_id].curr_stage);
		}

		stage1_missions[0].start = static_cast<int>(g_curr_servertime.count());
		cout << "[" << stage1_missions[0].start << "]  새로운 미션 추가: ";
		switch (stage1_missions[0].type) {
		case MISSION_KILL:
			cout << "[처치] ";
			break;
		case MISSION_OCCUPY:
			cout << "[점령] ";
			break;
		}
		cout << stage1_missions[0].curr << " / " << stage1_missions[0].goal << "\n" << endl;

		//====================
		// 1. 맵 정보
		// 새로 접속한 클라이언트에게 맵 정보를 보내줍니다.
		for (auto& building : mapobj_info) {
			SC_MAP_OBJINFO_PACKET building_packet;
			building_packet.type = SC_MAP_OBJINFO;
			building_packet.size = sizeof(SC_MAP_OBJINFO_PACKET);

			building_packet.center_x = building.getPosX();
			building_packet.center_y = building.getPosY();
			building_packet.center_z = building.getPosZ();

			building_packet.scale_x = building.getScaleX();
			building_packet.scale_y = building.getScaleY();
			building_packet.scale_z = building.getScaleZ();

			building_packet.forward_x = building.getLocalForward().x;
			building_packet.forward_y = building.getLocalForward().y;
			building_packet.forward_z = building.getLocalForward().z;

			building_packet.right_x = building.getLocalRight().x;
			building_packet.right_y = building.getLocalRight().y;
			building_packet.right_z = building.getLocalRight().z;

			building_packet.rotate_x = building.getLocalRotate().x;
			building_packet.rotate_y = building.getLocalRotate().y;
			building_packet.rotate_z = building.getLocalRotate().z;

			building_packet.aob = building.getAngleAOB();
			building_packet.boc = building.getAngleBOC();

			clients[client_id].do_send(&building_packet);
		}

		//====================
		// 2. Player 객체 정보
		//  1) Clients
		// 현재 접속해 있는 모든 클라이언트에게 새로운 클라이언트(client_id)의 정보를 전송합니다.
		for (auto& pl : clients) {
			if (pl.s_state != ST_INGAME) continue;
			if (pl.id == client_id) continue;

			lock_guard<mutex> lg{ pl.s_lock };
			pl.send_add_obj_packet(client_id, TARGET_PLAYER);
		}

		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 클라이언트의 정보를 전송합니다.
		for (auto& pl : clients) {
			if (pl.s_state != ST_INGAME) continue;
			if (pl.id == client_id) continue;

			lock_guard<mutex> lg{ pl.s_lock };
			clients[client_id].send_add_obj_packet(pl.id, TARGET_PLAYER);
		}

		//  2) NPCs
		// 새로 접속한 클라이언트에게 현재 접속해 있는 모든 NPC의 정보를 전송합니다.
		for (auto& npc : npcs) {
			if (npc.id == -1) continue;

			lock_guard<mutex> lg{ clients[client_id].s_lock};
			clients[client_id].send_add_obj_packet(npc.id, TARGET_NPC);
		}

		//  3) NPC서버로 새로 접속한 클라이언트의 정보를 전송합니다.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_add_obj_packet(client_id, TARGET_PLAYER);
		}


		break;
	}// CS_LOGIN end
	case CS_MOVE:
	{
		if (!b_active_server) break;
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽은 자는 움직일 수 없다.

		CS_MOVE_PACKET* cl_move_packet = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		
		// 1. 헬기는 바닥에 닿으면 폭사한다.
		if (clients[client_id].role == ROLE_HELI && clients[client_id].pos.y <= 8.5f) {
			if (clients[client_id].pos.y <= 9.f) {		
				// 우선 NPC서버에게 플레이어 낙사 정보를 보내줍니다.
				SC_DAMAGED_PACKET damaged_packet;
				damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
				damaged_packet.type = SC_DAMAGED;
				damaged_packet.target = TARGET_PLAYER;
				damaged_packet.id = client_id;
				damaged_packet.damage = HELI_MAXHP;
				{
					lock_guard<mutex> lg{ npc_server.s_lock };
					npc_server.do_send(&damaged_packet);
				}

				// 사망처리
				clients[client_id].s_lock.lock();
				clients[client_id].hp = 0;
				clients[client_id].pl_state = PL_ST_DEAD;
				clients[client_id].s_lock.unlock();
				cout << "Player[" << client_id << "]의 헬리콥터가 지면에 추락하여 폭발하였다." << endl;

				SC_OBJECT_STATE_PACKET heli_fall_packet;
				heli_fall_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
				heli_fall_packet.type = SC_OBJECT_STATE;
				heli_fall_packet.target = TARGET_PLAYER;
				heli_fall_packet.id = clients[client_id].id;
				heli_fall_packet.state = PL_ST_DEAD;
				for (auto& send_cl : clients) {
					if (send_cl.s_state != ST_INGAME) continue;
					if (send_cl.curr_stage == 0) continue;

					lock_guard<mutex> lg{ send_cl.s_lock };
					send_cl.do_send(&heli_fall_packet);
				}

				break;
			}
		}

		// 2. 정상적인 이동이라면 세션 정보를 업데이트 한다.
		clients[client_id].s_lock.lock();
		clients[client_id].pos = { cl_move_packet->x, cl_move_packet->y, cl_move_packet->z };
		clients[client_id].setBB();
		clients[client_id].pl_state = cl_move_packet->direction + 1;	// MV_FRONT = 0, MV_BACK = 1, MV_SIDE = 2; PL_ST_MOVE_FRONT = 1, PL_ST_MOVE_BACK = 2, PL_ST_MOVE_SIDE = 3;
		if (clients[client_id].role == ROLE_HELI) {
			if (clients[client_id].height_alert && clients[client_id].pos.y > 30.0f) {	// 경보가 울리고 있다가 고도가 일정 높이 이상 올라오면 경보를 해제한다.
				clients[client_id].height_alert = false;
				SC_HEIGHT_ALERT_PACKET alert_cancel_packet;
				alert_cancel_packet.size = sizeof(SC_HEIGHT_ALERT_PACKET);
				alert_cancel_packet.type = SC_HEIGHT_ALERT;
				alert_cancel_packet.alert_on = 0;
				clients[client_id].do_send(&alert_cancel_packet);
			}
			else if (!clients[client_id].height_alert && clients[client_id].pos.y <= 30.0f) {	// 고도가 일정 높이 미만 내려가면 경보를 울린다.
				clients[client_id].height_alert = true;
				SC_HEIGHT_ALERT_PACKET alert_start_packet;
				alert_start_packet.size = sizeof(SC_HEIGHT_ALERT_PACKET);
				alert_start_packet.type = SC_HEIGHT_ALERT;
				alert_start_packet.alert_on = 1;
				clients[client_id].do_send(&alert_start_packet);
			}
		}
		clients[client_id].s_lock.unlock();

		//cout << "[Move TEST] Player[" << client_id << "]가 " << cl_move_packet->direction << " 방향으로 이동하여 POS가 ("
		//	<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.x << ")가 되었음.\n" << endl;

		// 3. View List를 업데이트한다.
		clients[client_id].update_viewlist();

		// 4. 다른 클라이언트에게 플레이어가 이동한 위치를 알려준다.
		for (auto& vl_key : clients[client_id].view_list) {
			if (!(PLAYER_ID_START <= vl_key && vl_key <= PLAYER_ID_END)) continue;	// Player가 아니면 continue

			int pl_id = vl_key - PLAYER_ID_START;
			if (pl_id == client_id) continue;
			if (clients[pl_id].s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ clients[pl_id].s_lock };
			clients[pl_id].send_move_packet(client_id, TARGET_PLAYER, cl_move_packet->direction);

		}

		// 5. NPC 서버에게도 플레이어가 이동한 위치를 알려준다.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_move_packet(client_id, TARGET_PLAYER, cl_move_packet->direction);
		}

		// 6. 점령미션 중이라면 점령지역에 있는지 확인한다.
		short curr_mission = curr_mission_stage[clients[client_id].curr_stage];
		if (clients[client_id].curr_stage == 1 && stage1_missions[curr_mission].type == MISSION_OCCUPY) {
			// 점령지역 세팅
			float occupy_leftup_x = occupy_areas[1].getPosX() - occupy_areas[1].getScaleX() / 2.0f;
			float occupy_leftup_z = occupy_areas[1].getPosZ() - occupy_areas[1].getScaleZ() / 2.0f;
			float occupy_rightbottom_x = occupy_areas[1].getPosX() + occupy_areas[1].getScaleX() / 2.0f;
			float occupy_rightbottom_z = occupy_areas[1].getPosZ() + occupy_areas[1].getScaleZ() / 2.0f;

			// 점령 지역에 있는지 검사
			if (occupy_leftup_x <= clients[client_id].pos.x && clients[client_id].pos.x <= occupy_rightbottom_x && \
				occupy_leftup_z <= clients[client_id].pos.z && clients[client_id].pos.z <= occupy_rightbottom_z) {
				// 안에 있다면
				if (!clients[client_id].b_occupying) {		// 점령지역에 새로 들어온 경우
					clients[client_id].s_lock.lock();
					clients[client_id].b_occupying = true;	// 점령 상태 On
					cout << "Client[" << client_id << "]가 점령을 시작합니다. (START TIME: " << static_cast<int>(g_curr_servertime.count()) << ")\n" << endl;
					clients[client_id].s_lock.unlock();
				}
			}
			else {
				// 밖에 있다면
				if (clients[client_id].b_occupying) {		// 점령지역 밖으로 탈출한 경우
					clients[client_id].s_lock.lock();
					clients[client_id].b_occupying = false;	// 점령 상태 Off
					cout << "Client[" << client_id << "]가 점령을 종료합니다. (END TIME: " << static_cast<int>(g_curr_servertime.count()) << ")\n" << endl;
					clients[client_id].s_lock.unlock();
				}
			}
		}
		else if (clients[client_id].curr_stage == 2) {
		}

		// 7. 스폰지역에 있는지 확인한다.
		if (clients[client_id].curr_stage == 1) {
			float spawn_leftup_x = STAGE1_SPAWN_AREA_POS_X - STAGE1_SPAWN_AREA_SIZE_X / 2.0f;
			float spawn_leftup_z = STAGE1_SPAWN_AREA_POS_Z - STAGE1_SPAWN_AREA_SIZE_Z / 2.0f;
			float spawn_rightbottom_x = STAGE1_SPAWN_AREA_POS_X + STAGE1_SPAWN_AREA_SIZE_X / 2.0f;
			float spawn_rightbottom_z = STAGE1_SPAWN_AREA_POS_Z + STAGE1_SPAWN_AREA_SIZE_Z / 2.0f;
			if (spawn_leftup_x <= clients[client_id].pos.x && clients[client_id].pos.x <= spawn_rightbottom_x\
				&& spawn_leftup_z <= clients[client_id].pos.z && clients[client_id].pos.z <= spawn_rightbottom_z) {
				clients[client_id].s_lock.lock();
				clients[client_id].in_spawn_area = true;
				clients[client_id].s_lock.unlock();
			}
			else {
				clients[client_id].s_lock.lock();
				clients[client_id].in_spawn_area = false;
				clients[client_id].s_lock.unlock();
			}
		}

		// 8. 힐팩 위치에 있는지 확인한다.
		if (clients[client_id].curr_stage == 1 && clients[client_id].role == ROLE_RIFLE) {	// 힐팩은 소총수만 먹을 수 있다.
			bool collided_healpack = false;
			for (int i = 0; i < 8; ++i) {
				if (g_healpacks[i].is_used) continue;
				if (g_healpacks[i].pos.x - HEALPACK_SIZE / 2.0f <= clients[client_id].pos.x && clients[client_id].pos.x <= g_healpacks[i].pos.x + HEALPACK_SIZE / 2.0f\
					&& g_healpacks[i].pos.z - HEALPACK_SIZE / 2.0f <= clients[client_id].pos.z && clients[client_id].pos.z <= g_healpacks[i].pos.z + HEALPACK_SIZE / 2.0f) {
					collided_healpack = true;

					g_healpacks[i].is_used = true;
					g_healpacks[i].used_time = system_clock::now();

					// 힐팩은 사용한 사실을 모든 클라에게 알려줍니다. (힐팩 이펙트 OFF를 위함)
					SC_HEALPACK_PACKET healpack_use_packet;
					healpack_use_packet.size = sizeof(SC_HEALPACK_PACKET);
					healpack_use_packet.type = SC_HEALPACK;
					healpack_use_packet.healpack_id = i;
					healpack_use_packet.isused = 1;
					for (auto& send_cl : clients) {
						if (send_cl.s_state != ST_INGAME) continue;
						if (send_cl.curr_stage == 0) continue;

						lock_guard<mutex> lg{ send_cl.s_lock };
						send_cl.do_send(&healpack_use_packet);
					}
					break;
				}
			}

			if (collided_healpack && clients[client_id].hp < HUMAN_MAXHP) {	// 힐팩 위치에 있다면 그 클라이언트를 치료해줍니다.
				clients[client_id].s_lock.lock();
				clients[client_id].hp += HEALPACK_RECOVER_HP;
				if (clients[client_id].hp > HUMAN_MAXHP) clients[client_id].hp = HUMAN_MAXHP;
				clients[client_id].s_lock.unlock();
				cout << "Player[" << client_id << "]가 힐팩을 얻어 HP를 (+" << HEALPACK_RECOVER_HP << ")만큼 회복하였다. (HP: " << clients[client_id].hp << " 남음)\n" << endl;

				SC_HEALING_PACKET healing_packet;
				healing_packet.size = sizeof(SC_HEALING_PACKET);
				healing_packet.type = SC_HEALING;
				healing_packet.id = clients[client_id].id;
				healing_packet.value = HEALPACK_RECOVER_HP;

				// 우선 NPC서버에게 플레이어 힐링 정보를 보내줍니다.
				{
					lock_guard<mutex> lg{ npc_server.s_lock };
					npc_server.do_send(&healing_packet);
				}

				// 모든 클라이언트한테도 보내줍니다.
				for (auto& send_cl : clients) {
					if (send_cl.s_state != ST_INGAME) continue;
					if (send_cl.curr_stage == 0) continue;

					lock_guard<mutex> lg{ send_cl.s_lock };
					send_cl.do_send(&healing_packet);
				}
			}
		}

		break;
	}// CS_MOVE end
	case CS_ROTATE:
	{
		if (!b_active_server) break;
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽은 자는 움직일 수 없다.

		CS_ROTATE_PACKET* cl_rotate_packet = reinterpret_cast<CS_ROTATE_PACKET*>(packet);

		// 1. 세션 정보를 업데이트 한다.
		clients[client_id].s_lock.lock();
		clients[client_id].m_rightvec = { cl_rotate_packet->right_x, cl_rotate_packet->right_y, cl_rotate_packet->right_z };
		clients[client_id].m_upvec = { cl_rotate_packet->up_x, cl_rotate_packet->up_y, cl_rotate_packet->up_z };
		clients[client_id].m_lookvec = { cl_rotate_packet->look_x, cl_rotate_packet->look_y, cl_rotate_packet->look_z };
		clients[client_id].m_cam_lookvec = { cl_rotate_packet->cam_look_x, cl_rotate_packet->cam_look_y, cl_rotate_packet->cam_look_z };
		clients[client_id].setBB();
		clients[client_id].s_lock.unlock();

		// 2. 다른 클라이언트에게 플레이어가 회전한 방향을 알려준다.
		for (auto& vl_key : clients[client_id].view_list) {
			if (!(PLAYER_ID_START <= vl_key && vl_key <= PLAYER_ID_END)) continue;;	// Player가 아니면 break

			int pl_id = vl_key - PLAYER_ID_START;
			if (pl_id == client_id) continue;
			if (clients[pl_id].s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ clients[pl_id].s_lock };
			clients[pl_id].send_rotate_packet(client_id, TARGET_PLAYER);
		}

		// 3. NPC 서버에게도 플레이어가 회전한 방향를 알려준다.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_rotate_packet(client_id, TARGET_PLAYER);
		}

		break;
	}// CS_ROTATE end
	case CS_ATTACK:
	{
		if (!b_active_server) break;
		if (clients[client_id].curr_stage == 0) break;
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽은 자는 움직일 수 없다.

		CS_ATTACK_PACKET* cl_attack_packet = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		//==============================
		// 1. 총알
		// Bullet 개수 체크
		if (clients[client_id].remain_bullet <= 0) break;

		// 총알 개수 업데이트
		clients[client_id].s_lock.lock();
		if (clients[client_id].remain_bullet > 1)
			clients[client_id].remain_bullet -= 1;
		else
			clients[client_id].remain_bullet = 0;
		clients[client_id].s_lock.unlock();

		// 총알을 발사했다는 정보를 다른 클라이언트에게 알려줍니다.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_stage == 0) continue;

			// 사운드 볼륨 계산을 위해 거리를 측정합니다
			float dist = XMF_Distance(cl.pos, clients[client_id].pos);
			if (dist > ATKSOUND_MAX_DISTANCE) continue;

			char atksound_vol = VOL_MUTE;
			if (0 <= dist && dist < ATKSOUND_NEAR_DISTANCE)
				atksound_vol = VOL_HIGH;
			else if (ATKSOUND_NEAR_DISTANCE <= dist && dist < ATKSOUND_MID_DISTANCE)
				atksound_vol = VOL_MID;
			else if (ATKSOUND_MID_DISTANCE <= dist)
				atksound_vol = VOL_LOW;

			SC_ATTACK_PACKET atk_pack;
			atk_pack.size = sizeof(SC_ATTACK_PACKET);
			atk_pack.type = SC_ATTACK;
			atk_pack.obj_type = TARGET_PLAYER;
			atk_pack.id = clients[client_id].id;
			atk_pack.sound_volume = atksound_vol;
			atk_pack.atklook_x = clients[client_id].m_cam_lookvec.x;
			atk_pack.atklook_y = clients[client_id].m_cam_lookvec.y;
			atk_pack.atklook_z = clients[client_id].m_cam_lookvec.z;
			lock_guard<mutex> lg{ cl.s_lock };
			cl.do_send(&atk_pack);
		}

		//==============================
		// 2. 충돌검사
		bool b_collide = false;

		// 야매방법 (추후에 반드시 레이캐스트로 바꿔야함!!!)
		SESSION bullet;
		bullet.pos = clients[client_id].pos;
		bullet.pos.y += 2.8f;
		bullet.m_lookvec = clients[client_id].m_cam_lookvec;
		bullet.m_xoobb = BoundingOrientedBox(XMFLOAT3(bullet.pos.x, bullet.pos.y, bullet.pos.z)\
			, XMFLOAT3(0.1f, 0.1f, 0.3f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		XMFLOAT3 bullet_initpos = bullet.pos;
		XMFLOAT3 collide_pos = XMF_fault;
		int collided_obj = C_OBJ_NONCOLLIDE;
		int collided_npc_id = -1;
		while (XMF_Distance(bullet.pos, bullet_initpos) <= BULLET_RANGE) {
			
			// 1. 맵 지형(건물, 나무, 박스, 차, ...)과 검사
			for (auto& mapobj : mapobj_info) {
				// 거리가 너무 멀면 검사 X
				if (XMF_Distance(bullet.pos, mapobj.getPos()) > BULLET_RANGE) continue;

				if (bullet.m_xoobb.Intersects(mapobj.m_xoobb)) {
					if (collide_pos == XMF_fault) {	// 첫 검사는 무조건 업데이트
						b_collide = true;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_MAPOBJ;
						//cout << "맵 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// 저장해둔 충돌점보다 가까이에 있으면 업데이트
							b_collide = true;

							collide_pos = bullet.pos;
							collided_obj = C_OBJ_MAPOBJ;
							//cout << "맵 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
						}
					}
				}
			}
			
			// 2. NPC랑 검사
			for (auto& vl_key : clients[client_id].view_list) {
				if (!(NPC_ID_START <= vl_key && vl_key <= NPC_ID_END)) continue;

				int npc_id = vl_key - NPC_ID_START;
				if (npcs[npc_id].pl_state == PL_ST_DEAD) continue;

				if (bullet.m_xoobb.Intersects(npcs[npc_id].m_xoobb)) {
					if (collide_pos == XMF_fault) {	// 첫 검사는 무조건 업데이트
						b_collide = true;
						collided_npc_id = npc_id;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_NPC;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// 저장해둔 충돌점보다 가까이에 있으면 업데이트
							b_collide = true;
							collided_npc_id = npc_id;

							collide_pos = bullet.pos;
							collided_obj = C_OBJ_NPC;
						}
						else {
							continue;
						}
					}
				}
			}

			// 3. 바닥이랑 검사
			if (bullet.pos.y <= 6.0f) {
				if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// 저장해둔 충돌점보다 가까이에 있으면 업데이트
					b_collide = true;

					collide_pos = bullet.pos;
					collided_obj = C_OBJ_GROUND;
				}
			}

			// 충돌했다면 패킷을 보낸다.
			if (b_collide) {
				switch (collided_obj) {
				case C_OBJ_MAPOBJ:
					//cout << "맵 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					SC_BULLET_COLLIDE_POS_PACKET map_collide_pack;
					map_collide_pack.size = sizeof(SC_BULLET_COLLIDE_POS_PACKET);
					map_collide_pack.type = SC_BULLET_COLLIDE_POS;
					map_collide_pack.attacker = TARGET_PLAYER;
					map_collide_pack.collide_target = C_OBJ_MAPOBJ;
					map_collide_pack.x = collide_pos.x;
					map_collide_pack.y = collide_pos.y;
					map_collide_pack.z = collide_pos.z;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != clients[client_id].curr_stage) continue;
						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&map_collide_pack);
					}

					break;
				case C_OBJ_GROUND:
					//cout << "바닥 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					SC_BULLET_COLLIDE_POS_PACKET ground_collide_pack;
					ground_collide_pack.size = sizeof(SC_BULLET_COLLIDE_POS_PACKET);
					ground_collide_pack.type = SC_BULLET_COLLIDE_POS;
					ground_collide_pack.attacker = TARGET_PLAYER;
					ground_collide_pack.collide_target = C_OBJ_GROUND;
					ground_collide_pack.x = collide_pos.x;
					ground_collide_pack.y = collide_pos.y;
					ground_collide_pack.z = collide_pos.z;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != clients[client_id].curr_stage) continue;
						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&ground_collide_pack);
					}

					break;
				case C_OBJ_NPC:
					// 데미지 계산
					int damage = 0;
					if (clients[client_id].oneshot_onekill_cheat) {	// 원샷원킬 치트키 적용중일때
						damage = 9999;
					}
					else {
						float dec_damage = static_cast<float>(RIFLE_DAMAGE);
						if (XMF_Distance(npcs[collided_npc_id].pos, bullet_initpos) > 10) {	// 멀리 떨어질 수록 데미지가 조금씩 감소한다.
							float dist = static_cast<int>(XMF_Distance(npcs[collided_npc_id].pos, bullet_initpos));
							dec_damage = dec_damage / 100.f * (dist / 10);
						}
						damage = static_cast<int>(RIFLE_DAMAGE - dec_damage);
						if (collided_npc_id < MAX_NPC_HELI) {	// 헬기는 좀 덜 아프게 맞는다.
							damage = damage - static_cast<int>(damage * 0.3f);
						}
						if (damage < 1) {	// 최소한 1 데미지는 주도록
							damage = 1;
						}
					}

					// 우선 맞아서 죽든 안죽든 피격 위치를 클라이언트에게 알려줍니다. (피터지는? 연출을 위함)
					SC_DAMAGED_PACKET npc_damaged_pack;
					npc_damaged_pack.size = sizeof(SC_DAMAGED_PACKET);
					npc_damaged_pack.type = SC_DAMAGED;
					npc_damaged_pack.target = TARGET_NPC;
					npc_damaged_pack.id = collided_npc_id;
					npc_damaged_pack.damage = damage;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != clients[client_id].curr_stage) continue;

						float dist = XMF_Distance(cl.pos, npcs[collided_npc_id].pos);
						if (dist < DAMAGEDSOUND_NEAR_DISTANCE)
							npc_damaged_pack.sound_volume = VOL_HIGH;
						else if (DAMAGEDSOUND_NEAR_DISTANCE <= dist && dist < DAMAGEDSOUND_MID_DISTANCE)
							npc_damaged_pack.sound_volume = VOL_MID;
						else if (DAMAGEDSOUND_MID_DISTANCE <= dist && dist < DAMAGEDSOUND_MAX_DISTANCE)
							npc_damaged_pack.sound_volume = VOL_LOW;

						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&npc_damaged_pack);
					}
					{
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.do_send(&npc_damaged_pack);
					}

					// npc 데미지 처리
					if (npcs[collided_npc_id].hp > damage) {
						npcs[collided_npc_id].s_lock.lock();
						npcs[collided_npc_id].hp -= damage;
						npcs[collided_npc_id].s_lock.unlock();
						//cout << "Player[" << client_id << "]의 공격에 NPC[" << collided_npc_id << "]가 피해(-" << damage << ")를 입었다."
						//	<< " (남은 HP : " << npcs[collided_npc_id].hp << ")\n" << endl;
					}
					else {	// npc 사망
						npcs[collided_npc_id].s_lock.lock();
						npcs[collided_npc_id].hp = 0;
						npcs[collided_npc_id].pl_state = PL_ST_DEAD;
						npcs[collided_npc_id].s_lock.unlock();
						cout << "Player[" << client_id << "]의 공격에 NPC[" << collided_npc_id << "]가 사망하였다." << endl;

						if (collided_npc_id < STAGE1_MAX_HELI) {
							// 1) 헬기NPC인 경우 NPC서버에게 상태패킷을 보냅니다.
							SC_OBJECT_STATE_PACKET npc_die_packet;
							npc_die_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
							npc_die_packet.type = SC_OBJECT_STATE;
							npc_die_packet.target = TARGET_NPC;
							npc_die_packet.id = collided_npc_id;
							npc_die_packet.state = PL_ST_DEAD;
							if (b_npcsvr_conn) {
								lock_guard<mutex> lg{ npc_server.s_lock };
								npc_server.do_send(&npc_die_packet);
							}

							// (임시) npc서버에서 헬기 낙하 모션 구현전까지 클라이언트에서 scale을 0으로 낮추는 야매방법 임시 채택
							SC_OBJECT_STATE_PACKET temp_packet;
							temp_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
							temp_packet.type = SC_OBJECT_STATE;
							temp_packet.target = TARGET_NPC;
							temp_packet.id = collided_npc_id;
							temp_packet.state = PL_ST_DEAD;
							for (auto& cl : clients) {
								if (cl.s_state != ST_INGAME) continue;
								if (cl.curr_stage == 0) continue;

								lock_guard<mutex> lg{ cl.s_lock };
								cl.do_send(&temp_packet);
							}
						}
						else {
							// 2) 인간NPC인 경우 NPC서버에게 제거패킷, 클라이언트에게 상태패킷을 보냅니다.
							//   (NPC서버에선 동작을 멈추게하고, 클라에선 애니메이션을 하게됩니다.)

							// 제거 패킷 (to. NPC서버)
							if (b_npcsvr_conn) {
								lock_guard<mutex> lg{ npc_server.s_lock };
								npc_server.send_remove_packet(collided_npc_id, TARGET_NPC);
							}

							// 상태패킷 (to. 클라이언트)
							SC_OBJECT_STATE_PACKET npc_die_packet;
							npc_die_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
							npc_die_packet.type = SC_OBJECT_STATE;
							npc_die_packet.target = TARGET_NPC;
							npc_die_packet.id = collided_npc_id;
							npc_die_packet.state = PL_ST_DEAD;
							for (auto& cl : clients) {
								if (cl.s_state != ST_INGAME) continue;
								if (cl.curr_stage == 0) continue;

								lock_guard<mutex> lg{ cl.s_lock };
								cl.do_send(&npc_die_packet);
							}
						}

						// 미션 업데이트
						short curr_mission = curr_mission_stage[clients[client_id].curr_stage];
						bool mission_clear = false;
						if (clients[client_id].curr_stage == 1) {
							stage1_missions[curr_mission].curr++;

							if (stage1_missions[curr_mission].curr == stage1_missions[curr_mission].goal) {
								mission_clear = true;
							}
						}
						else if (clients[client_id].curr_stage == 2) {
							//stage2_missions[clients[client_id].curr_mission].curr++;
						}

						for (auto& cl : clients) {
							if (cl.s_state != ST_INGAME) continue;
							if (cl.curr_stage != clients[client_id].curr_stage) continue;
							lock_guard<mutex> lg{ cl.s_lock };
							cl.send_mission_packet(clients[client_id].curr_stage);
						}

						if (mission_clear) {
							if (clients[client_id].curr_stage == 1) {
								cout << "스테이지[1]의 미션[" << curr_mission << "] 완료!" << endl;

								// 미션 완료 패킷
								SC_MISSION_COMPLETE_PACKET mission_complete;
								mission_complete.type = SC_MISSION_COMPLETE;
								mission_complete.size = sizeof(SC_MISSION_COMPLETE_PACKET);
								mission_complete.stage_num = clients[client_id].curr_stage;
								mission_complete.mission_num = curr_mission;
								for (auto& cl : clients) {
									if (cl.s_state != ST_INGAME) continue;
									if (cl.curr_stage != clients[client_id].curr_stage) continue;
									lock_guard<mutex> lg{ cl.s_lock };
									cl.do_send(&mission_complete);
								}

								if (curr_mission + 1 >= ST1_MISSION_NUM) {	// 모든 미션 완료
									cout << "스테이지[1]의 모든 미션을 완료하였습니다.\n" << endl;
								}
								else {
									curr_mission_stage[clients[client_id].curr_stage]++;
									curr_mission = curr_mission_stage[clients[client_id].curr_stage];

									// 다음 미션 전달
									for (auto& cl : clients) {
										if (cl.s_state != ST_INGAME) continue;
										if (cl.curr_stage != clients[client_id].curr_stage) continue;
										lock_guard<mutex> lg{ cl.s_lock };
										cl.send_mission_packet(clients[client_id].curr_stage);
									}

									stage1_missions[curr_mission].start = static_cast<int>(g_curr_servertime.count());
									cout << "[" << stage1_missions[curr_mission].start << "] 새로운 미션 추가: ";
									switch (stage1_missions[curr_mission].type) {
									case MISSION_KILL:
										cout << "[처치] ";
										break;
									case MISSION_OCCUPY:
										cout << "[점령] ";
										break;
									}
									cout << stage1_missions[curr_mission].curr << " / " << stage1_missions[curr_mission].goal << "\n" << endl;
								}
							}
							else if (clients[client_id].curr_stage == 2) {
								//cout << "스테이지[2] 미션-" << curr_mission << " 완료";
							}
						}
						else {
							if (clients[client_id].curr_stage == 1) {
								cout << "스테이지[1] 현재 미션 업데이트: ";
								switch (stage1_missions[curr_mission].type) {
								case MISSION_KILL:
									cout << "[처치] ";
									break;
								case MISSION_OCCUPY:
									cout << "[점령] ";
									break;
								}
								cout << stage1_missions[curr_mission].curr << " / "
									<< stage1_missions[curr_mission].goal << "\n" << endl;
							}
							else if (clients[client_id].curr_stage == 2) {
								//cout << "스테이지[2] 미션 업데이트: ";
							}
						}

					}

					break;
				}
				break;
			}
			else {
				bullet.pos = XMF_Add(bullet.pos, XMF_MultiplyScalar(bullet.m_lookvec, 1.f));
				bullet.m_xoobb = BoundingOrientedBox(XMFLOAT3(bullet.pos.x, bullet.pos.y, bullet.pos.z)\
					, XMFLOAT3(0.2f, 0.2f, 0.6f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			}
		}

		/* My Raycast 원래는 raycast로 해야함.
		XMFLOAT3 human_intersection\
			= Intersect_Ray_Box(clients[client_id].pos, clients[client_id].m_lookvec, BULLET_RANGE, dummy_humanoid.pos, HUMAN_BBSIZE_X, HUMAN_BBSIZE_Y, HUMAN_BBSIZE_Z);
		if (human_intersection != XMF_fault) {
			cout << "[CS_ATTACK] 사람 더미와 충돌하였음." << endl;//test
			b_collide = true;
		}

		XMFLOAT3 heli_intersection\
			= Intersect_Ray_Box(clients[client_id].pos, clients[client_id].m_lookvec, BULLET_RANGE, dummy_helicopter.pos, HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z);
		if (heli_intersection != XMF_fault) {
			cout << "[CS_ATTACK] 헬기 더미와 충돌하였음." << endl;//test
			b_collide = true;
		}
		*/


		break;
	}// CS_ATTACK end
	case CS_INPUT_KEYBOARD:
	{
		if (!b_active_server) break;		// Active 서버만 패킷을 처리합니다.
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		float sign = 1.0f;					// right/up/look벡터 방향으로 움직이는지, 반대 방향으로 움직이는지
		switch (inputkey_p->keytype) {
		case PACKET_KEY_R:
			milliseconds reload_term = duration_cast<milliseconds>(system_clock::now() - clients[client_id].reload_time);
			if (clients[client_id].role == ROLE_RIFLE && reload_term < milliseconds(RELOAD_TIME)) break;
			if (clients[client_id].role == ROLE_HELI && reload_term < milliseconds(RELOAD_TIME_HELI)) break;

			if (clients[client_id].role == ROLE_RIFLE && clients[client_id].remain_bullet == MAX_BULLET) break;
			if (clients[client_id].role == ROLE_HELI && clients[client_id].remain_bullet == MAX_BULLET_HELI) break;

			clients[client_id].s_lock.lock();
			if (clients[client_id].role == ROLE_RIFLE)
				clients[client_id].remain_bullet = MAX_BULLET;
			else if (clients[client_id].role == ROLE_HELI)
				clients[client_id].remain_bullet = MAX_BULLET_HELI;
			clients[client_id].reload_time = system_clock::now();
			clients[client_id].s_lock.unlock();

			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage == 0) continue;
				int dist = XMF_Distance(cl.pos, clients[client_id].pos);
				if (dist > RELOADSOUND_MAX_DISTANCE) continue;

				SC_RELOAD_PACKET reload_packet;
				reload_packet.size = sizeof(SC_RELOAD_PACKET);
				reload_packet.type = SC_RELOAD;
				reload_packet.bullet_cnt = clients[client_id].remain_bullet;
				reload_packet.id = clients[client_id].id;
				if (0 <= dist && dist < RELOADSOUND_NEAR_DISTANCE)
					reload_packet.sound_volume = VOL_HIGH;
				else if (RELOADSOUND_NEAR_DISTANCE <= dist && dist < RELOADSOUND_MID_DISTANCE)
					reload_packet.sound_volume = VOL_MID;
				else if (RELOADSOUND_MID_DISTANCE <= dist && dist < RELOADSOUND_MAX_DISTANCE)
					reload_packet.sound_volume = VOL_LOW;

				lock_guard<mutex> lg{ cl.s_lock };
				cl.do_send(&reload_packet);
			}
			break;

		case PACKET_KEYUP_MOVEKEY:
			if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽은 자는 움직일 수 없다.
			if (clients[client_id].pl_state == PL_ST_MOVE_FRONT || clients[client_id].pl_state == PL_ST_MOVE_BACK || clients[client_id].pl_state == PL_ST_MOVE_SIDE) {
				clients[client_id].s_lock.lock();
				clients[client_id].pl_state = PL_ST_IDLE;
				clients[client_id].s_lock.unlock();

				SC_OBJECT_STATE_PACKET change2idle_pack;
				change2idle_pack.size = sizeof(SC_OBJECT_STATE_PACKET);
				change2idle_pack.type = SC_OBJECT_STATE;
				change2idle_pack.target = TARGET_PLAYER;
				change2idle_pack.id = clients[client_id].id;
				change2idle_pack.state = PL_ST_IDLE;

				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage == 0) continue;
					if (cl.id == client_id) continue;

					lock_guard<mutex> lg{ cl.s_lock };
					cl.do_send(&change2idle_pack);
				}
			}
			break;

		case PACKET_KEY_INSERT:		// 무적 치트키
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// 잘못된 요청
			if (clients[client_id].curr_stage == 0) break;	// 잘못된 요청
			if (!b_active_server) break;	// 잘못된 요청

			if (clients[client_id].immortal_cheat == true) {
				cout << "==무적 치트키 해제==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].immortal_cheat = false;
				clients[client_id].s_lock.unlock();
			}
			else {
				cout << "==무적 치트키==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].immortal_cheat = true;
				clients[client_id].s_lock.unlock();
			}

			break;
		}// PACKET_KEY_INSERT case end
		case PACKET_KEY_DELETE:		// 원샷원킬 치트키
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// 잘못된 요청
			if (clients[client_id].curr_stage == 0) break;	// 잘못된 요청
			if (!b_active_server) break;	// 잘못된 요청

			if (clients[client_id].oneshot_onekill_cheat == true) {
				cout << "==원샷원킬 치트키 해제==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].oneshot_onekill_cheat = false;
				clients[client_id].s_lock.unlock();
			}
			else {
				cout << "==원샷원킬 치트키==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].oneshot_onekill_cheat = true;
				clients[client_id].s_lock.unlock();
			}

			break;
		}// PACKET_KEY_DELETE case end
		case PACKET_KEY_END:	// 몰살 치트키
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// 잘못된 요청
			if (clients[client_id].curr_stage == 0) break;	// 잘못된 요청
			if (!b_active_server) break;	// 잘못된 요청
			if (!b_npcsvr_conn) break;	// 죽일 NPC가 없음.
			if (curr_mission_stage[clients[client_id].curr_stage] != 0) break;	// 잘못된 요청

			cout << "==몰살 치트키==" << endl;

			for (auto& npc : npcs) {
				if (npc.pl_state == PL_ST_DEAD) continue;

				npc.s_lock.lock();
				npc.hp = 0;
				npc.pl_state = PL_ST_DEAD;
				npc.s_lock.unlock();
				//cout << "[치트키] Player[" << client_id << "]의 치트키에 NPC[" << npc.id << "]가 사망하였다." << endl;

				if (npc.id < STAGE1_MAX_HELI) {
					// 1) 헬기NPC인 경우 NPC서버에게 상태패킷을 보냅니다.
					SC_OBJECT_STATE_PACKET npc_die_packet;
					npc_die_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
					npc_die_packet.type = SC_OBJECT_STATE;
					npc_die_packet.target = TARGET_NPC;
					npc_die_packet.id = npc.id;
					npc_die_packet.state = PL_ST_DEAD;
					if (b_npcsvr_conn) {
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.do_send(&npc_die_packet);
					}

					// 헬기 추락모션
					SC_OBJECT_STATE_PACKET temp_packet;
					temp_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
					temp_packet.type = SC_OBJECT_STATE;
					temp_packet.target = TARGET_NPC;
					temp_packet.id = npc.id;
					temp_packet.state = PL_ST_DEAD;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage == 0) continue;

						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&temp_packet);
					}
				}
				else {
					// 2) 인간NPC인 경우 NPC서버에게 제거패킷, 클라이언트에게 상태패킷을 보냅니다.
					//   (NPC서버에선 동작을 멈추게하고, 클라에선 애니메이션을 하게됩니다.)

					// 제거 패킷 (to. NPC서버)
					if (b_npcsvr_conn) {
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.send_remove_packet(npc.id, TARGET_NPC);
					}

					// 상태패킷 (to. 클라이언트)
					SC_OBJECT_STATE_PACKET npc_die_packet;
					npc_die_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
					npc_die_packet.type = SC_OBJECT_STATE;
					npc_die_packet.target = TARGET_NPC;
					npc_die_packet.id = npc.id;
					npc_die_packet.state = PL_ST_DEAD;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage == 0) continue;

						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&npc_die_packet);
					}
				}
			}

			// 미션 강제 완료
			short curr_mission = curr_mission_stage[clients[client_id].curr_stage];
			bool mission_clear = false;
			if (clients[client_id].curr_stage == 1) {
				stage1_missions[curr_mission].curr = stage1_missions[curr_mission].goal;
				mission_clear = true;
			}

			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage != clients[client_id].curr_stage) continue;
				lock_guard<mutex> lg{ cl.s_lock };
				cl.send_mission_packet(clients[client_id].curr_stage);
			}

			if (clients[client_id].curr_stage == 1) {
				cout << "스테이지[1]의 미션[" << curr_mission << "] 완료!" << endl;

				// 미션 완료 패킷
				SC_MISSION_COMPLETE_PACKET mission_complete;
				mission_complete.type = SC_MISSION_COMPLETE;
				mission_complete.size = sizeof(SC_MISSION_COMPLETE_PACKET);
				mission_complete.stage_num = clients[client_id].curr_stage;
				mission_complete.mission_num = curr_mission;
				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage != clients[client_id].curr_stage) continue;
					lock_guard<mutex> lg{ cl.s_lock };
					cl.do_send(&mission_complete);
				}

				curr_mission_stage[clients[client_id].curr_stage]++;
				curr_mission = curr_mission_stage[clients[client_id].curr_stage];

				// 다음 미션 전달
				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage != clients[client_id].curr_stage) continue;
					lock_guard<mutex> lg{ cl.s_lock };
					cl.send_mission_packet(clients[client_id].curr_stage);
				}

				stage1_missions[curr_mission].start = static_cast<int>(g_curr_servertime.count());
				cout << "[" << stage1_missions[curr_mission].start << "] 새로운 미션 추가: ";
				switch (stage1_missions[curr_mission].type) {
				case MISSION_KILL:
					cout << "[처치] ";
					break;
				case MISSION_OCCUPY:
					cout << "[점령] ";
					break;
				}
				cout << stage1_missions[curr_mission].curr << " / " << stage1_missions[curr_mission].goal << "\n" << endl;
			}
			break;
		}// PACKET_KEY_END case end
		case PACKET_KEY_PGUP:		// 강제회복 치트키
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// 잘못된 요청
			if (clients[client_id].curr_stage == 0) break;	// 잘못된 요청
			if (!b_active_server) break;	// 잘못된 요청
			if (clients[client_id].hp == HUMAN_MAXHP) break;	// 이미 풀HP임.

			int damaged_hp = HUMAN_MAXHP - clients[client_id].hp;

			cout << "==힐링 치트키==\n" << endl;
			clients[client_id].s_lock.lock();
			clients[client_id].hp += damaged_hp;
			clients[client_id].s_lock.unlock();
			cout << "Player[" << client_id << "]가 HP를 (+" << damaged_hp << ")만큼 회복하였다. (HP: "	<< clients[client_id].hp << " 남음)\n" << endl;

			SC_HEALING_PACKET healing_packet;
			healing_packet.size = sizeof(SC_HEALING_PACKET);
			healing_packet.type = SC_HEALING;
			healing_packet.id = clients[client_id].id;
			healing_packet.value = damaged_hp;

			// 우선 NPC서버에게 플레이어 데미지 정보를 보내줍니다.
			{
				lock_guard<mutex> lg{ npc_server.s_lock };
				npc_server.do_send(&healing_packet);
			}

			// 모든 클라이언트한테도 보내줍니다.
			for (auto& send_cl : clients) {
				if (send_cl.s_state != ST_INGAME) continue;
				if (send_cl.curr_stage == 0) continue;

				lock_guard<mutex> lg{ send_cl.s_lock };
				send_cl.do_send(&healing_packet);
			}			

			break;
		}// PACKET_KEY_PGUP case end
		}// switch end

		break;
	}// CS_INPUT_KEYBOARD end
	case CS_INPUT_MOUSE:
	{
		if (!b_active_server) break;		// Active 서버만 패킷을 처리합니다.
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽은 자는 움직일 수 없다.

		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->buttontype == PACKET_NONCLICK) {			// 버튼 안누름 (그냥 이동만 한 경우)
		}
		else if (rt_p->buttontype == PACKET_BUTTON_L) {			// 마우스 좌클릭
			// 1스테이지 로직
			if (clients[client_id].curr_stage == 1) {

			}
			// 2스테이지에선 ATTACK_PACKET을 보냄.
		}
		else if (rt_p->buttontype == PACKET_BUTTON_R) {		// 마우스 우클릭 드래그: 기능 미정.
			// 1스테이지 로직
			if (clients[client_id].curr_stage == 1) {

			}
			// 2스테이지 로직
			else if (clients[client_id].curr_stage == 2) {

			}
		}

		break;
	}// CS_INPUT_MOUSE end
	case CS_CHAT:
	{
		CS_CHAT_PACKET* recv_chat_pack = reinterpret_cast<CS_CHAT_PACKET*>(packet);

		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_stage == 0) continue;

			SC_CHAT_PACKET send_chat_pack;
			send_chat_pack.size = sizeof(SC_CHAT_PACKET);
			send_chat_pack.type = SC_CHAT;
			strcpy_s(send_chat_pack.name, clients[client_id].name);
			strcpy_s(send_chat_pack.msg, recv_chat_pack->msg);
			cl.do_send(&send_chat_pack);
		}

		break;
	}
	case CS_PARTICLE_COLLIDE:
	{
		CS_PARTICLE_COLLIDE_PACKET* particle_pack = reinterpret_cast<CS_PARTICLE_COLLIDE_PACKET*>(packet);

		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽으면 충돌X
		if (clients[client_id].immortal_cheat) break;	// 무적은 충돌X

		// 데미지 계산
		int damage = static_cast<int>(PARTICLE_BASIC_DAMAGE * particle_pack->particle_mass);
		if (clients[client_id].role == ROLE_HELI) {	// 헬기 플레이어는 덜 아프게 맞는다.
			damage = damage - static_cast<int>(damage * 0.4f);
		}
		int after_hp = clients[client_id].hp - damage;	//

		// 우선 NPC서버에게 플레이어 데미지 정보를 보내줍니다.
		SC_DAMAGED_PACKET damaged_packet;
		damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
		damaged_packet.type = SC_DAMAGED;
		damaged_packet.target = TARGET_PLAYER;
		damaged_packet.id = clients[client_id].inserver_index;
		damaged_packet.damage = damage;
		{
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.do_send(&damaged_packet);
		}

		// 데미지 처리
		if (after_hp > 0) {		// 데미지 처리
			clients[client_id].s_lock.lock();
			clients[client_id].hp -= damage;
			clients[client_id].s_lock.unlock();
			cout << "파티클에 충돌하여 Player[" << client_id << "]가 피해(-" << damage << ")를 입었다. (HP: " << clients[client_id].hp << " 남음)\n" << endl;

			SC_DAMAGED_PACKET damaged_packet;
			damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
			damaged_packet.type = SC_DAMAGED;
			damaged_packet.target = TARGET_PLAYER;
			damaged_packet.id = clients[client_id].id;
			damaged_packet.damage = damage;
			for (auto& send_cl : clients) {
				if (send_cl.s_state != ST_INGAME) continue;
				if (send_cl.curr_stage == 0) continue;

				float dist = XMF_Distance(send_cl.pos, clients[client_id].pos);
				if (dist < DAMAGEDSOUND_NEAR_DISTANCE)
					damaged_packet.sound_volume = VOL_HIGH;
				else if (DAMAGEDSOUND_NEAR_DISTANCE <= dist && dist < DAMAGEDSOUND_MID_DISTANCE)
					damaged_packet.sound_volume = VOL_MID;
				else if (DAMAGEDSOUND_MID_DISTANCE <= dist && dist < DAMAGEDSOUND_MAX_DISTANCE)
					damaged_packet.sound_volume = VOL_LOW;

				lock_guard<mutex> lg{ send_cl.s_lock };
				send_cl.do_send(&damaged_packet);
			}
		}
		else {					// 사망
			clients[client_id].s_lock.lock();
			clients[client_id].hp = 0;
			clients[client_id].pl_state = PL_ST_DEAD;
			clients[client_id].s_lock.unlock();
			cout << "파티클에 충돌하여 Player[" << clients[client_id].id << "]가 사망하였다." << endl;

			SC_OBJECT_STATE_PACKET death_packet;
			death_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
			death_packet.type = SC_OBJECT_STATE;
			death_packet.target = TARGET_PLAYER;
			death_packet.id = clients[client_id].id;
			death_packet.state = PL_ST_DEAD;
			for (auto& send_cl : clients) {
				if (send_cl.s_state != ST_INGAME) continue;
				if (send_cl.curr_stage == 0) continue;

				{
					lock_guard<mutex> lg{ send_cl.s_lock };
					send_cl.do_send(&death_packet);
				}
			}
		}

		break;
	}// CS_PARTICLE_COLLIDE end
	case CS_HELI_MAP_COLLIDE:
	{
		CS_HELI_MAP_COLLIDE_PACKET* particle_pack = reinterpret_cast<CS_HELI_MAP_COLLIDE_PACKET*>(packet);
		if (clients[client_id].role != ROLE_HELI) break;	// 잘못된 요청
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// 죽으면 충돌X
		if (clients[client_id].immortal_cheat) break;	// 무적은 충돌X
		
		// 데미지 계산
		int damage = static_cast<int>(HUMAN_MAXHP / 3) + 1;
		int after_hp = clients[client_id].hp - damage;

		// 우선 NPC서버에게 플레이어 데미지 정보를 보내줍니다.
		SC_DAMAGED_PACKET damaged_packet;
		damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
		damaged_packet.type = SC_DAMAGED;
		damaged_packet.target = TARGET_PLAYER;
		damaged_packet.id = clients[client_id].inserver_index;
		damaged_packet.damage = damage;
		{
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.do_send(&damaged_packet);
		}

		// 데미지 처리
		if (after_hp > 0) {		// 데미지 처리
			clients[client_id].s_lock.lock();
			clients[client_id].hp -= damage;
			clients[client_id].s_lock.unlock();
			cout << "벽에 충돌하여 Player[" << clients[client_id].id << "]가 피해(-" << damage << ")를 입었다. (HP: " << clients[client_id].hp << " 남음)\n" << endl;

			SC_DAMAGED_PACKET damaged_packet;
			damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
			damaged_packet.type = SC_DAMAGED;
			damaged_packet.target = TARGET_PLAYER;
			damaged_packet.id = clients[client_id].id;
			damaged_packet.damage = damage;
			for (auto& send_cl : clients) {
				if (send_cl.s_state != ST_INGAME) continue;
				if (send_cl.curr_stage == 0) continue;

				float dist = XMF_Distance(send_cl.pos, clients[client_id].pos);
				if (dist < DAMAGEDSOUND_NEAR_DISTANCE)
					damaged_packet.sound_volume = VOL_HIGH;
				else if (DAMAGEDSOUND_NEAR_DISTANCE <= dist && dist < DAMAGEDSOUND_MID_DISTANCE)
					damaged_packet.sound_volume = VOL_MID;
				else if (DAMAGEDSOUND_MID_DISTANCE <= dist && dist < DAMAGEDSOUND_MAX_DISTANCE)
					damaged_packet.sound_volume = VOL_LOW;

				lock_guard<mutex> lg{ send_cl.s_lock };
				send_cl.do_send(&damaged_packet);
			}
		}
		else {					// 사망
			clients[client_id].s_lock.lock();
			clients[client_id].hp = 0;
			clients[client_id].pl_state = PL_ST_DEAD;
			clients[client_id].s_lock.unlock();
			cout << "벽에 충돌하여 Player[" << clients[client_id].id << "]가 사망하였다." << endl;

			SC_OBJECT_STATE_PACKET death_packet;
			death_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
			death_packet.type = SC_OBJECT_STATE;
			death_packet.target = TARGET_PLAYER;
			death_packet.id = clients[client_id].id;
			death_packet.state = PL_ST_DEAD;
			for (auto& send_cl : clients) {
				if (send_cl.s_state != ST_INGAME) continue;
				if (send_cl.curr_stage == 0) continue;

				{
					lock_guard<mutex> lg{ send_cl.s_lock };
					send_cl.do_send(&death_packet);
				}
			}
		}

	}// CS_HELI_MAP_COLLIDE end
	case CS_PING:
	{
		CS_PING_PACKET* re_login_pack = reinterpret_cast<CS_PING_PACKET*>(packet);

		SC_PING_RETURN_PACKET ping_ret_pack;
		ping_ret_pack.type = SC_PING_RETURN;
		ping_ret_pack.size = sizeof(SC_PING_RETURN_PACKET);
		ping_ret_pack.ping_sender_id = clients[client_id].id;
		clients[client_id].do_send(&ping_ret_pack);

		break;
	}// CS_PING end
	case CS_RELOGIN:
	{
		CS_RELOGIN_PACKET* re_login_pack = reinterpret_cast<CS_RELOGIN_PACKET*>(packet);

		int re_login_id = re_login_pack->id;
		clients[re_login_id].s_lock.lock();
		clients[re_login_id].s_state = ST_INGAME;
		clients[re_login_id].setBB();
		clients[re_login_id].s_lock.unlock();

		cout << "[HA] Clients[" << re_login_id << "]와 다시 연결되었습니다.\n" << endl;

		break;
	}// CS_RELOGIN end
	case SS_HEARTBEAT:
	{
		SS_HEARTBEAT_PACKET* heartbeat_pack = reinterpret_cast<SS_HEARTBEAT_PACKET*>(packet);
		int recv_id = heartbeat_pack->sender_id;

		extended_servers[recv_id].heartbeat_recv_time = chrono::system_clock::now();
		if (recv_id < my_server_id) {	// A->B->A로 heartbeat의 한 사이클이 끝나도록하기 위함. (즉, 오른쪽 서버로부터 Heartbeat를 받으면 한 사이클의 끝으로 판단)
			// Heartbeat를 먼저 보낸 서버에게 자신의 Heartbeat를 전송합니다.
			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[recv_id].do_send(&hb_packet);										// 자신에게 Heartbeat를 보낸 서버에게 전송합니다.
			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// 전송한 시간을 업데이트
		}
		break;
	}// SS_HEARTBEAT end
	case SS_DATA_REPLICA:
	{
		SS_DATA_REPLICA_PACKET* replica_pack = reinterpret_cast<SS_DATA_REPLICA_PACKET*>(packet);

		int replica_id = replica_pack->id;
		clients[replica_id].s_lock.lock();
		clients[replica_id].id = replica_id;
		clients[replica_id].s_state = ST_FREE;
		strcpy_s(clients[replica_id].name, replica_pack->name);

		clients[replica_id].pos = { replica_pack->x, replica_pack->y, replica_pack->z };

		clients[replica_id].m_rightvec = { replica_pack->right_x, replica_pack->right_y, replica_pack->right_z };
		clients[replica_id].m_upvec = { replica_pack->up_x, replica_pack->up_y, replica_pack->up_z };
		clients[replica_id].m_lookvec = { replica_pack->look_x, replica_pack->look_y, replica_pack->look_z };

		clients[replica_id].pl_state = replica_pack->state;
		clients[replica_id].hp = replica_pack->hp;
		clients[replica_id].remain_bullet = replica_pack->bullet_cnt;
		clients[replica_id].curr_stage = replica_pack->curr_stage;

		clients[replica_id].s_lock.unlock();

		cout << "Client[" << replica_id << "]의 데이터가 복제되었습니다." << endl;
		cout << "===================================" << endl;
		cout << "Name: " << clients[replica_id].name << endl;
		cout << "Stage: " << clients[replica_id].curr_stage << endl;
		cout << "State: " << clients[replica_id].pl_state << endl;
		cout << "Pos: " << clients[replica_id].pos.x << ", " << clients[replica_id].pos.y << ", " << clients[replica_id].pos.z << endl;
		cout << "LookVec: " << clients[replica_id].m_lookvec.x << ", " << clients[replica_id].m_lookvec.y << ", " << clients[replica_id].m_lookvec.z << endl;
		cout << "STime: " << replica_pack->curr_stage << "ms." << endl;
		cout << "===================================\n" << endl;
		break;
	}// SS_DATA_REPLICA end
	case NPC_FULL_INFO:
	{
		NPC_FULL_INFO_PACKET* npc_info_pack = reinterpret_cast<NPC_FULL_INFO_PACKET*>(packet);

		short npc_id = npc_info_pack->n_id;

		npcs[npc_id].s_lock.lock();
		npcs[npc_id].hp = npc_info_pack->hp;
		strcpy_s(npcs[npc_id].name, npc_info_pack->name);
		npcs[npc_id].id = npc_info_pack->n_id;
		npcs[npc_id].pos = { npc_info_pack->x, npc_info_pack->y, npc_info_pack->z };
		npcs[npc_id].m_rightvec = { npc_info_pack->right_x, npc_info_pack->right_y, npc_info_pack->right_z };
		npcs[npc_id].m_upvec = { npc_info_pack->up_x, npc_info_pack->up_y, npc_info_pack->up_z };
		npcs[npc_id].m_lookvec = { npc_info_pack->look_x, npc_info_pack->look_y, npc_info_pack->look_z };

		if (npc_info_pack->ishuman) {
			npcs[npc_id].setBB();
		}
		else {
			npcs[npc_id].m_xoobb = BoundingOrientedBox(XMFLOAT3(npcs[npc_id].pos.x, npcs[npc_id].pos.y, npcs[npc_id].pos.z)
				, XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		npcs[npc_id].s_lock.unlock();

		if (npc_id == 0) {
			cout << "[Init NPCs...] ";
		}
		else if (npc_id == MAX_NPCS - 1) {
			cout << "---- OK." << endl;
		}

		break;
	}// NPC_FULL_INFO end
	case NPC_MOVE:
	{
		NPC_MOVE_PACKET* npc_move_pack = reinterpret_cast<NPC_MOVE_PACKET*>(packet);

		short npc_id = npc_move_pack->n_id;
		npcs[npc_id].s_lock.lock();
		npcs[npc_id].pos = { npc_move_pack->x, npc_move_pack->y, npc_move_pack->z };
		if (npc_id < 5) {	// 헬기
			npcs[npc_id].m_xoobb = BoundingOrientedBox(XMFLOAT3(npcs[npc_id].pos.x, npcs[npc_id].pos.y, npcs[npc_id].pos.z)
				, XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else {	// 사람
			npcs[npc_id].setBB();
		}
		npcs[npc_id].s_lock.unlock();

		// 클라이언트로 NPC좌표를 보내는 것은 1초에 한번씩
		// 클라이언트에서 보간한 좌표와의 오차를 비교하기 위해 보내는 것임.
		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.do_send(npc_move_pack);
		}

		break;
	}// NPC_MOVE end
	case NPC_ROTATE:
	{
		NPC_ROTATE_PACKET* npc_rotate_pack = reinterpret_cast<NPC_ROTATE_PACKET*>(packet);

		short npc_id = npc_rotate_pack->n_id;

		// 서버 내 NPC 정보 업데이트
		npcs[npc_id].s_lock.lock();
		npcs[npc_id].pos = { npc_rotate_pack->x, npc_rotate_pack->y, npc_rotate_pack->z };
		npcs[npc_id].m_rightvec = { npc_rotate_pack->right_x, npc_rotate_pack->right_y, npc_rotate_pack->right_z };
		npcs[npc_id].m_upvec = { npc_rotate_pack->up_x, npc_rotate_pack->up_y, npc_rotate_pack->up_z };
		npcs[npc_id].m_lookvec = { npc_rotate_pack->look_x, npc_rotate_pack->look_y, npc_rotate_pack->look_z };
		if (npc_id < 5) {	// 헬기
			npcs[npc_id].m_xoobb = BoundingOrientedBox(XMFLOAT3(npcs[npc_id].pos.x, npcs[npc_id].pos.y, npcs[npc_id].pos.z)
				, XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else {	// 사람
			npcs[npc_id].setBB();
		}
		npcs[npc_id].s_lock.unlock();
		//cout << "NPC[" << npc_id << "]가 Look(" << npcs[npc_id].m_lookvec.x << ", " << npcs[npc_id].m_lookvec.y << ", " << npcs[npc_id].m_lookvec.z
		//	<< ") 방향으로 회전하였습니다.\n" << endl;

		// 클라로 패킷 전달
		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.do_send(npc_rotate_pack);
		}

		break;
	}// NPC_ROTATE end
	case NPC_REMOVE:
	{
		NPC_REMOVE_PACKET* npc_remove_pack = reinterpret_cast<NPC_REMOVE_PACKET*>(packet);

		short npc_id = npc_remove_pack->n_id;

		npcs[npc_id].s_lock.lock();
		npcs[npc_id].sessionClear();
		npcs[npc_id].s_lock.unlock();
		//cout << "NPC[" << npc_id << "]가 삭제되었습니다.\n" << endl;

		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 NPC 제작 전까지 사용되는 임시코드
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.send_remove_packet(npc_id, TARGET_NPC);
		}

		break;
	}// NPC_REMOVE end
	case NPC_ATTACK:
	{
		NPC_ATTACK_PACKET* recv_attack_pack = reinterpret_cast<NPC_ATTACK_PACKET*>(packet);

		// 총알을 발사했다는 정보를 모든 클라이언트에게 알려줍니다.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_stage == 0) continue;

			// 사운드 볼륨 계산을 위해 거리를 측정합니다
			float dist = XMF_Distance(cl.pos, npcs[recv_attack_pack->n_id].pos);
			char atksound_vol = VOL_MUTE;
			if (0 <= dist && dist < ATKSOUND_NEAR_DISTANCE)
				atksound_vol = VOL_HIGH;
			else if (ATKSOUND_NEAR_DISTANCE <= dist && dist < ATKSOUND_MID_DISTANCE)
				atksound_vol = VOL_MID;
			else if (ATKSOUND_MID_DISTANCE <= dist)
				atksound_vol = VOL_LOW;

			SC_ATTACK_PACKET atk_pack;
			atk_pack.size = sizeof(SC_ATTACK_PACKET);
			atk_pack.type = SC_ATTACK;
			atk_pack.obj_type = TARGET_NPC;
			atk_pack.id = recv_attack_pack->n_id;
			atk_pack.atklook_x = recv_attack_pack->atklook_x;
			atk_pack.atklook_y = recv_attack_pack->atklook_y;
			atk_pack.atklook_z = recv_attack_pack->atklook_z;
			atk_pack.sound_volume = atksound_vol;
			lock_guard<mutex> lg{ cl.s_lock };
			cl.do_send(&atk_pack);
		}

		// 2. 충돌검사
		int npc_id = recv_attack_pack->n_id;
		bool b_collide = false;

		// 야매방법 (추후에 반드시 레이캐스트로 바꿔야함!!!)
		SESSION bullet;
		bullet.pos = npcs[npc_id].pos;
		if (MAX_NPC_HELI <= npcs[recv_attack_pack->n_id].id) bullet.pos.y += 2.8f;
		bullet.m_lookvec = XMFLOAT3{ recv_attack_pack->atklook_x, recv_attack_pack->atklook_y, recv_attack_pack->atklook_z };
		bullet.m_xoobb = BoundingOrientedBox(XMFLOAT3(bullet.pos.x, bullet.pos.y, bullet.pos.z)\
			, XMFLOAT3(0.1f, 0.1f, 0.3f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		//cout << "총알 진행방향: " << bullet.m_lookvec.x << ", " << bullet.m_lookvec.y << ", " << bullet.m_lookvec.z << endl;

		XMFLOAT3 bullet_initpos = bullet.pos;
		XMFLOAT3 collide_pos = XMF_fault;
		int collided_obj = C_OBJ_NONCOLLIDE;
		int collided_cl_id = -1;
		while (XMF_Distance(bullet.pos, bullet_initpos) <= BULLET_RANGE) {

			// 1. 맵 지형(건물, 나무, 박스, 차, ...)과 검사
			for (auto& mapobj : mapobj_info) {
				// 거리가 너무 멀면 검사 X
				if (XMF_Distance(bullet.pos, mapobj.getPos()) > BULLET_RANGE) continue;

				if (bullet.m_xoobb.Intersects(mapobj.m_xoobb)) {
					if (collide_pos == XMF_fault) {	// 첫 검사는 무조건 업데이트
						b_collide = true;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_MAPOBJ;
						//cout << "맵 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// 저장해둔 충돌점보다 가까이에 있으면 업데이트
							b_collide = true;

							collide_pos = bullet.pos;
							collided_obj = C_OBJ_MAPOBJ;
							//cout << "맵 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
						}
					}
				}
			}

			// 2. 플레이어랑 검사
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.pl_state == PL_ST_DEAD) continue;
				if (XMF_Distance(bullet.pos, cl.pos) > BULLET_RANGE) continue;

				if (bullet.m_xoobb.Intersects(cl.m_xoobb)) {
					if (collide_pos == XMF_fault) {	// 첫 검사는 무조건 업데이트
						b_collide = true;
						collided_cl_id = cl.inserver_index;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_PLAYER;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// 저장해둔 충돌점보다 가까이에 있으면 업데이트
							b_collide = true;
							collided_cl_id = cl.inserver_index;

							collide_pos = bullet.pos;
							collided_obj = C_OBJ_PLAYER;
						}
						else {
							continue;
						}
					}
				}
			}

			// 3. 바닥이랑 검사
			if (bullet.pos.y <= 0.0f) {
				if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// 저장해둔 충돌점보다 가까이에 있으면 업데이트
					b_collide = true;

					collide_pos = bullet.pos;
					collided_obj = C_OBJ_GROUND;
				}
			}

			// 충돌했다면 패킷을 보낸다.
			if (b_collide) {
				switch (collided_obj) {
				case C_OBJ_MAPOBJ:
					//cout << "맵 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					SC_BULLET_COLLIDE_POS_PACKET map_collide_pack;
					map_collide_pack.size = sizeof(SC_BULLET_COLLIDE_POS_PACKET);
					map_collide_pack.type = SC_BULLET_COLLIDE_POS;
					map_collide_pack.attacker = TARGET_PLAYER;
					map_collide_pack.collide_target = C_OBJ_MAPOBJ;
					map_collide_pack.x = collide_pos.x;
					map_collide_pack.y = collide_pos.y;
					map_collide_pack.z = collide_pos.z;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != clients[client_id].curr_stage) continue;
						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&map_collide_pack);
					}

					break;
				case C_OBJ_GROUND:
					//cout << "바닥 오브젝트와 충돌하였음 (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					SC_BULLET_COLLIDE_POS_PACKET ground_collide_pack;
					ground_collide_pack.size = sizeof(SC_BULLET_COLLIDE_POS_PACKET);
					ground_collide_pack.type = SC_BULLET_COLLIDE_POS;
					ground_collide_pack.attacker = TARGET_PLAYER;
					ground_collide_pack.collide_target = C_OBJ_GROUND;
					ground_collide_pack.x = collide_pos.x;
					ground_collide_pack.y = collide_pos.y;
					ground_collide_pack.z = collide_pos.z;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != clients[client_id].curr_stage) continue;
						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&ground_collide_pack);
					}

					break;
				case C_OBJ_PLAYER:
					// 만약 무적 치트키를 쓴 유저라면 건너뛴다
					if (clients[collided_cl_id].immortal_cheat == true) {
						cout << "Client[" << collided_cl_id << "]는 무적이다." << endl;
						break;
					}

					// 데미지 계산
					int damage = 0;
					if (recv_attack_pack->n_id < MAX_NPC_HELI) {
						damage = NPC_VALKAN_DAMAGE;
					}
					else {
						damage = NPC_RIFLE_DAMAGE;
					}
					if (clients[collided_cl_id].role == ROLE_HELI) {	// 헬기 플레이어는 덜 아프게 맞는다.
						damage = damage / 2;
					}
					int after_hp = clients[collided_cl_id].hp - damage;	//

					// 우선 NPC서버에게 플레이어 데미지 정보를 보내줍니다.
					SC_DAMAGED_PACKET damaged_packet;
					damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
					damaged_packet.type = SC_DAMAGED;
					damaged_packet.target = TARGET_PLAYER;
					damaged_packet.id = collided_cl_id;
					damaged_packet.damage = damage;
					{
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.do_send(&damaged_packet);
					}

					// 데미지 처리
					if (after_hp > 0) {		// 데미지 처리
						clients[collided_cl_id].s_lock.lock();
						clients[collided_cl_id].hp -= damage;
						clients[collided_cl_id].s_lock.unlock();
						//cout << "NPC[" << recv_attack_pack->n_id << "]의 공격에 Player[" << collided_cl_id << "]가 피해(-" << damage << ")를 입었다. (HP: "
						//	<< clients[collided_cl_id].hp << " 남음)\n" << endl;

						SC_DAMAGED_PACKET damaged_packet;
						damaged_packet.size = sizeof(SC_DAMAGED_PACKET);
						damaged_packet.type = SC_DAMAGED;
						damaged_packet.target = TARGET_PLAYER;
						damaged_packet.id = clients[collided_cl_id].id;
						damaged_packet.damage = damage;
						for (auto& send_cl : clients) {
							if (send_cl.s_state != ST_INGAME) continue;
							if (send_cl.curr_stage == 0) continue;

							float dist = XMF_Distance(send_cl.pos, clients[collided_cl_id].pos);
							if (dist < DAMAGEDSOUND_NEAR_DISTANCE)
								damaged_packet.sound_volume = VOL_HIGH;
							else if (DAMAGEDSOUND_NEAR_DISTANCE <= dist && dist < DAMAGEDSOUND_MID_DISTANCE)
								damaged_packet.sound_volume = VOL_MID;
							else if (DAMAGEDSOUND_MID_DISTANCE <= dist && dist < DAMAGEDSOUND_MAX_DISTANCE)
								damaged_packet.sound_volume = VOL_LOW;

							lock_guard<mutex> lg{ send_cl.s_lock };
							send_cl.do_send(&damaged_packet);
						}
					}
					else {					// 사망
						clients[collided_cl_id].s_lock.lock();
						clients[collided_cl_id].hp = 0;
						clients[collided_cl_id].pl_state = PL_ST_DEAD;
						clients[collided_cl_id].s_lock.unlock();
						//cout << "Npc[" << recv_attack_pack->n_id << "]의 공격에 Player[" << collided_cl_id << "]가 사망하였다." << endl;

						SC_OBJECT_STATE_PACKET death_packet;
						death_packet.size = sizeof(SC_OBJECT_STATE_PACKET);
						death_packet.type = SC_OBJECT_STATE;
						death_packet.target = TARGET_PLAYER;
						death_packet.id = clients[collided_cl_id].id;
						death_packet.state = PL_ST_DEAD;
						for (auto& send_cl : clients) {
							if (send_cl.s_state != ST_INGAME) continue;
							if (send_cl.curr_stage == 0) continue;

							{
								lock_guard<mutex> lg{ send_cl.s_lock };
								send_cl.do_send(&death_packet);
							}
						}
					}

					break;
				}// switch end
				break;
			}
			else {
				bullet.pos = XMF_Add(bullet.pos, XMF_MultiplyScalar(bullet.m_lookvec, 1.f));
				bullet.m_xoobb = BoundingOrientedBox(XMFLOAT3(bullet.pos.x, bullet.pos.y, bullet.pos.z)\
					, XMFLOAT3(0.1f, 0.1f, 0.3f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			}
		}

		break;
	}
	case NPC_CHANGE_STATE:
	{
		NPC_CHANGE_STATE_PACKET* npc_chgstate_pack = reinterpret_cast<NPC_CHANGE_STATE_PACKET*>(packet);

		break;
	}// NPC_CHANGE_STATE_PACKET end
	}
}

//======================================================================
int get_new_client_id()	// clients의 비어있는 칸을 찾아서 새로운 client의 아이디를 할당해주는 함수
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].s_lock.lock();
		if (clients[i].s_state == ST_FREE) {
			clients[i].s_state = ST_ACCEPTED;
			clients[i].s_lock.unlock();
			return i;
		}
		clients[i].s_lock.unlock();
	}
	return -1;
}

//======================================================================
int find_empty_extended_server() {	// ex_servers의 비어있는 칸을 찾아서 새로운 Server_ex의 아이디를 할당해주는 함수
	for (int i = 0; i < MAX_USER; ++i) {
		extended_servers[i].s_lock.lock();
		if (extended_servers[i].s_state == ST_FREE) {
			extended_servers[i].s_state = ST_ACCEPTED;
			extended_servers[i].heartbeat_recv_time = chrono::system_clock::now();
			extended_servers[i].heartbeat_send_time = chrono::system_clock::now();
			extended_servers[i].s_lock.unlock();
			return i;
		}
		extended_servers[i].s_lock.unlock();
	}
	return -1;
}

//======================================================================
void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EX* ex_over = reinterpret_cast<OVER_EX*>(over);
		if (FALSE == ret) {
			if (ex_over->process_type == OP_ACCEPT) {
				cout << "Accept Error";
			}
			else if (ex_over->process_type == OP_CONNECT) {
				//cout << "Connect Error" << endl;

				// 비동기Conn를 다시 시도합니다.
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
				int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
				if (ret != 0) {
					cout << "Bind Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				OVER_EX* con_over = new OVER_EX;
				con_over->process_type = OP_CONNECT;
				HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key, 0);
				if (NULL == hret) {
					cout << "CreateIoCompletoinPort Error - " << ret << endl;
					cout << WSAGetLastError() << endl;
					exit(-1);
				}

				int target_portnum = key - CP_KEY_LOGIC2EXLOGIC + HA_PORTNUM_S0;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				ha_server_addr.sin_port = htons(target_portnum);
				inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no) {
						//cout << "Server Connect 재시도 중...\n" << endl;
					}
					else {
						cout << "ConnectEX Error - " << err_no << endl;
						cout << WSAGetLastError() << endl;
					}
				}
			}
			else {
				// 1. Client Error
				if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
					//cout << "GQCS Error ( client[" << key << "] )" << endl;
					disconnect(static_cast<int>(key - CP_KEY_LOGIC2CLIENT), SESSION_CLIENT);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				// 2. NPC Server Error
				else if (key >= CP_KEY_LOGIC2NPC && key <= CP_KEY_LISTEN_NPC) {
					disconnect(0, SESSION_NPC);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
				// 3. Ex_Server Error
				else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
					//cout << WSAGetLastError() << endl;
					disconnect(static_cast<int>(key - CP_KEY_LOGIC2EXLOGIC), SESSION_EXTENDED_SERVER);
					if (ex_over->process_type == OP_SEND) delete ex_over;
					continue;
				}
			}
		}

		switch (ex_over->process_type) {
		case OP_ACCEPT: {
			// 1. Client Accept
			if (key == CP_KEY_LISTEN_CLIENT) {
				SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				int client_id = get_new_client_id();
				if (client_id != -1) {
					// 클라이언트 id, 소켓
					clients[client_id].s_lock.lock();
					clients[client_id].inserver_index = client_id;
					clients[client_id].remain_size = 0;
					clients[client_id].socket = c_socket;
					clients[client_id].s_lock.unlock();
					int new_key = client_id + CP_KEY_LOGIC2CLIENT;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, new_key, 0);
					clients[client_id].do_recv();
					c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "어떤 Client의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
				int addr_size = sizeof(SOCKADDR_IN);

				int option = TRUE;//Nagle
				setsockopt(g_sc_listensock, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
				AcceptEx(g_sc_listensock, c_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}
			// 2. Npc Server Accept
			else if (key == CP_KEY_LISTEN_NPC) {
				SOCKET npc_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				npc_server.socket = npc_socket;
				cout << "NPC 서버와 연결되었습니다." << endl;
				b_npcsvr_conn = true;
				int new_key = CP_KEY_LOGIC2NPC;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(npc_socket), h_iocp, new_key, 0);
				npc_server.do_recv();
				npc_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(npc_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_npc_listensock, npc_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);

				// NPC서버에게 맵 정보를 보내줍니다.
				for (auto& mapobj : mapobj_info) {
					SC_MAP_OBJINFO_PACKET mapobj_packet;
					mapobj_packet.type = SC_MAP_OBJINFO;
					mapobj_packet.size = sizeof(SC_MAP_OBJINFO_PACKET);

					mapobj_packet.center_x = mapobj.getPosX();
					mapobj_packet.center_y = mapobj.getPosY();
					mapobj_packet.center_z = mapobj.getPosZ();
					
					mapobj_packet.scale_x = mapobj.getScaleX();
					mapobj_packet.scale_y = mapobj.getScaleY();
					mapobj_packet.scale_z = mapobj.getScaleZ();
					
					mapobj_packet.forward_x = mapobj.getLocalForward().x;
					mapobj_packet.forward_y = mapobj.getLocalForward().y;
					mapobj_packet.forward_z = mapobj.getLocalForward().z;
					
					mapobj_packet.right_x = mapobj.getLocalRight().x;
					mapobj_packet.right_y = mapobj.getLocalRight().y;
					mapobj_packet.right_z = mapobj.getLocalRight().z;
					
					mapobj_packet.rotate_x = mapobj.getLocalRotate().x;
					mapobj_packet.rotate_y = mapobj.getLocalRotate().y;
					mapobj_packet.rotate_z = mapobj.getLocalRotate().z;
					
					mapobj_packet.aob = mapobj.getAngleAOB();
					mapobj_packet.boc = mapobj.getAngleBOC();

					npc_server.do_send(&mapobj_packet);
				}
			}
			// 3. Ex_Server Accept
			else if (key == CP_KEY_LISTEN_EXLOGIC) {
				SOCKET extended_server_socket = reinterpret_cast<SOCKET>(ex_over->wsabuf.buf);
				left_ex_server_sock = extended_server_socket;
				int new_id = find_empty_extended_server();
				if (new_id != -1) {
					//cout << "Sever[" << new_id << "]의 연결요청을 받았습니다.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_LOGIC2EXLOGIC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_iocp, new_key, 0);
					extended_servers[new_id].do_recv();
					extended_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "다른 Sever의 연결요청을 받았으나, 현재 서버가 꽉 찼습니다.\n" << endl;
				}

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(extended_server_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_ss_listensock, extended_server_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);
			}

			break;
		}//OP_ACPT end
		case OP_RECV: {
			// 1. Client Recv
			if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
				int recved_id = key - CP_KEY_LOGIC2CLIENT;
				if (0 == num_bytes) disconnect(recved_id, SESSION_CLIENT);

				int remain_data = num_bytes + clients[recved_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(recved_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				clients[recved_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				clients[recved_id].do_recv();
			}
			// 2. NPC Server Recv
			else if (key >= CP_KEY_LOGIC2NPC && key < CP_KEY_LISTEN_NPC) {
				if (0 == num_bytes) disconnect(key, SESSION_NPC);

				int remain_data = num_bytes + npc_server.remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(0, p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				npc_server.remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				npc_server.do_recv();
			}
			// 3. Ex_Server Recv
			else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				if (0 == num_bytes) disconnect(key, SESSION_EXTENDED_SERVER);
				int server_id = key - CP_KEY_LOGIC2EXLOGIC;

				int remain_data = num_bytes + extended_servers[server_id].remain_size;
				char* p = ex_over->send_buf;
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						process_packet(static_cast<int>(server_id), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				extended_servers[server_id].remain_size = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->send_buf, p, remain_data);
				}
				extended_servers[server_id].do_recv();
				break;
			}

			break;
		}//OP_RECV end
		case OP_SEND: {
			// 1. Client Send
			if (key >= CP_KEY_LOGIC2CLIENT && key <= CP_KEY_LISTEN_CLIENT) {
				if (0 == num_bytes) disconnect(key - CP_KEY_LOGIC2CLIENT, SESSION_CLIENT);
				delete ex_over;
			}
			// 2. NPC Server Send
			else if (key >= CP_KEY_LOGIC2NPC && key <= CP_KEY_LISTEN_NPC) {
				if (0 == num_bytes) disconnect(0, SESSION_NPC);
				delete ex_over;
			}
			// 3. Ex_Server Send
			else if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				int server_id = key - CP_KEY_LOGIC2EXLOGIC;
				if (0 == num_bytes) disconnect(server_id, SESSION_EXTENDED_SERVER);
				delete ex_over;
			}

			break;
		}//OP_SEND end
		case OP_CONNECT: {
			// 1. Ex_Server Conn_Ex
			if (key >= CP_KEY_LOGIC2EXLOGIC && key <= CP_KEY_LISTEN_EXLOGIC) {
				if (FALSE != ret) {
					int server_id = key - CP_KEY_LOGIC2EXLOGIC;
					std::cout << "성공적으로 Server[" << server_id << "]에 연결되었습니다.\n" << endl;
					extended_servers[server_id].id = server_id;
					extended_servers[server_id].remain_size = 0;
					extended_servers[server_id].socket = right_ex_server_sock;
					extended_servers[server_id].s_state = ST_ACCEPTED;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, NULL, 0);
					delete ex_over;
					extended_servers[server_id].do_recv();
				}
			}
			break;
		}//OP_CONN end
		}
	}
}

//======================================================================
void timerFunc() {
	while (true) {
		auto start_t = system_clock::now();
		// ================================
		if (b_active_server && !b_isfirstplayer) {
			// 서버 시간 업데이트
			servertime_lock.lock();
			g_curr_servertime = duration_cast<milliseconds>(start_t - g_s_start_time);
			servertime_lock.unlock();
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage == 0) continue;

				SC_TIME_TICKING_PACKET ticking_packet;
				ticking_packet.size = sizeof(SC_TIME_TICKING_PACKET);
				ticking_packet.type = SC_TIME_TICKING;
				int left_time;
				switch (cl.curr_stage) {
				case 1:
					left_time = STAGE1_TIMELIMIT * 1000 - static_cast<int>(g_curr_servertime.count());
					if (left_time <= 0) {
						ticking_packet.servertime_ms = STAGE1_TIMELIMIT * 1000;
					}
					else {
						ticking_packet.servertime_ms = static_cast<int>(g_curr_servertime.count());
					}
					break;
				default:
					left_time = -1;
				}

				if (left_time != -1) {
					cl.do_send(&ticking_packet);
				}
			}

			// 스폰지역에서는 HP를 조금씩 회복한다.
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage == 0) continue;
				if (cl.pl_state == PL_ST_DEAD) continue;

				if (cl.in_spawn_area == true && cl.hp < 100) {
					cl.s_lock.lock();
					cl.hp += 1;
					if (cl.hp >= 100) cl.hp = 100;
					cl.s_lock.unlock();

					SC_HEALING_PACKET healing_packet;
					healing_packet.size = sizeof(SC_HEALING_PACKET);
					healing_packet.type = SC_HEALING;
					healing_packet.id = cl.id;
					healing_packet.value = 1;

					// 우선 NPC서버에게 플레이어 데미지 정보를 보내줍니다.
					{
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.do_send(&healing_packet);
					}

					// 모든 클라이언트한테도 보내줍니다.
					for (auto& send_cl : clients) {
						if (send_cl.s_state != ST_INGAME) continue;
						if (send_cl.curr_stage == 0) continue;

						lock_guard<mutex> lg{ send_cl.s_lock };
						send_cl.do_send(&healing_packet);
					}
				}
			}

			// 힐팩 리스폰
			for (int i = 0; i < 8; ++i) {
				if (!g_healpacks[i].is_used) continue;
				if (system_clock::now() > g_healpacks[i].used_time + milliseconds(HEALPACK_RESPAWN_TIME)) {
					g_healpacks[i].is_used = false;	// 재생성

					// 힐팩이 생성된 사실을 모든 클라에게 알려줍니다. (힐팩 이펙트 On을 위함)
					SC_HEALPACK_PACKET healpack_use_packet;
					healpack_use_packet.size = sizeof(SC_HEALPACK_PACKET);
					healpack_use_packet.type = SC_HEALPACK;
					healpack_use_packet.healpack_id = i;
					healpack_use_packet.isused = 0;
					for (auto& send_cl : clients) {
						if (send_cl.s_state != ST_INGAME) continue;
						if (send_cl.curr_stage == 0) continue;

						lock_guard<mutex> lg{ send_cl.s_lock };
						send_cl.do_send(&healpack_use_packet);
					}
				}
			}

			// 만약 현재 미션이 점령 중이라면 점령 관련 계산을 한다.
			int occupy_member_cnt = 0;
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage == 0) continue;

				// 점령 인원 계산
				if (cl.curr_stage == 1 && cl.b_occupying) {
					occupy_member_cnt++;
				}
			}

			if (occupy_member_cnt >= 1) {	// 한 명 이상 점령 중이라면
				int curr_mission_id = curr_mission_stage[1];
				if (stage1_missions[curr_mission_id].curr >= stage1_missions[curr_mission_id].goal) {	// 미션 완료
					// 미션 완료 패킷
					SC_MISSION_COMPLETE_PACKET mission_complete;
					mission_complete.type = SC_MISSION_COMPLETE;
					mission_complete.size = sizeof(SC_MISSION_COMPLETE_PACKET);
					mission_complete.stage_num = 1;
					mission_complete.mission_num = curr_mission_id;
					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != 1) continue;
						lock_guard<mutex> lg{ cl.s_lock };
						cl.do_send(&mission_complete);
					}
				}
				else {	// 아직 점령 진행중
					// 미션 진행 업데이트
					mission_lock.lock();
					stage1_missions[curr_mission_id].curr += 30 * occupy_member_cnt * occupy_member_cnt;	// 점령중인 사람이 많을 수록 게이지가 빨리 차오른다.
					mission_lock.unlock();

					for (auto& cl : clients) {
						if (cl.s_state != ST_INGAME) continue;
						if (cl.curr_stage != 1) continue;
						lock_guard<mutex> lg{ cl.s_lock };
						cl.send_mission_packet(1);
					}
				}
			}
		}

		// ================================
		// --- 업데이트


		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}

//======================================================================
void heartBeatFunc() {	// Heartbeat관련 스레드 함수
	while (true) {
		auto start_t = system_clock::now();

		// ================================
		// 1. Heartbeat 전송
		// : 오른쪽 서버로 Heartbeat를 보냅니다. (왼쪽 서버가 오른쪽 서버로 전송하기 때문에 가장 마지막 서버는 보내지 않습니다.)
		if (my_server_id != MAX_LOGIC_SERVER - 1) {
			if (extended_servers[my_server_id].s_state != ST_ACCEPTED)	continue;
			if (extended_servers[my_server_id + 1].s_state != ST_ACCEPTED) continue;

			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[my_server_id + 1].do_send(&hb_packet);	// 오른쪽 서버에 전송합니다.

			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// 전송한 시간을 업데이트
		}

		// ================================
		// 2. Heartbeat 수신검사
		// 오랫동안 Heartbeat를 받지 못한 서버구성원이 있는지 확인합니다.
		//   1) 오른쪽 서버 검사 (가장 왼쪽에 있는 서버 구성원은 검사를 하지 않습니다.)
		if (my_server_id != 0) {
			if (extended_servers[my_server_id - 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id - 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id - 1 << "]에게 Heartbeat를 오랫동안 받지 못했습니다. 서버 다운으로 간주합니다." << endl;
					disconnect(my_server_id - 1, SESSION_EXTENDED_SERVER);
				}
			}
		}
		//   2) 왼쪽 서버 검사 (가장 오른쪽에 있는 서버 구성원은 검사를 하지 않습니다.)
		if (my_server_id != MAX_LOGIC_SERVER - 1) {
			if (extended_servers[my_server_id + 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id + 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id + 1 << "]에게 Heartbeat를 오랫동안 받지 못했습니다. 서버 다운으로 간주합니다." << endl;
					disconnect(my_server_id + 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		// ================================
		// 3. Data Replica 전송 (자신이 Active서버 일때에만)
		if (b_active_server) {
			// 데이터복제 패킷을 받게될 Standby서버의 id를 알아냅니다.
			int standby_id = -1;
			if (my_server_id == 0)
				standby_id = 1;
			else
				standby_id = 0;

			if (extended_servers[standby_id].s_state == ST_ACCEPTED) {
				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage == 0) continue;

					SS_DATA_REPLICA_PACKET replica_pack;
					replica_pack.type = SS_DATA_REPLICA;
					replica_pack.size = sizeof(SS_DATA_REPLICA_PACKET);

					replica_pack.target = TARGET_PLAYER;
					replica_pack.id = cl.id;
					strcpy_s(replica_pack.name, cl.name);

					replica_pack.x = cl.pos.x;
					replica_pack.y = cl.pos.y;
					replica_pack.z = cl.pos.z;

					replica_pack.right_x = cl.m_rightvec.x;
					replica_pack.right_y = cl.m_rightvec.y;
					replica_pack.right_z = cl.m_rightvec.z;

					replica_pack.up_x = cl.m_upvec.x;
					replica_pack.up_y = cl.m_upvec.y;
					replica_pack.up_z = cl.m_upvec.z;

					replica_pack.look_x = cl.m_lookvec.x;
					replica_pack.look_y = cl.m_lookvec.y;
					replica_pack.look_z = cl.m_lookvec.z;

					replica_pack.state = cl.pl_state;
					replica_pack.hp = cl.hp;
					replica_pack.bullet_cnt = cl.remain_bullet;
					replica_pack.curr_stage = cl.curr_stage;

					extended_servers[standby_id].do_send(&replica_pack);

					cout << "[REPLICA TEST] Client[" << cl.id << "]의 정보를 Sever[" << standby_id << "]에게 전달합니다. - line: 1413" << endl;
					cout << "Stage: " << replica_pack.curr_stage << ", State: " << replica_pack.state
						<< ", Pos: " << replica_pack.x << ", " << replica_pack.y << ", " << replica_pack.z
						<< ", Look: " << replica_pack.look_x << ", " << replica_pack.look_y << ", " << replica_pack.look_z << "\n" << endl;
				}
			}
		}

		// ================================
		// 4. 스레드 대기 (과부하 방지)
		auto curr_t = system_clock::now();
		if (curr_t - start_t < static_cast<milliseconds>(HB_SEND_CYCLE)) {
			this_thread::sleep_for(static_cast<milliseconds>(HB_SEND_CYCLE) - (curr_t - start_t));
		}
	}
}

//======================================================================
int main(int argc, char* argv[])
{
	b_isfirstplayer = true;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//======================================================================
	// [ HA - 서버 ID, 포트번호 지정 ]
	// 어떤 서버를 가동할 것인지를 명령행 인수로 입력받아서 그 서버에 포트 번호를 부여합니다.
	my_server_id = 0;		// 서버 구분을 위한 지정번호
	int sc_portnum = -1;	// 클라이언트 통신용 포트번호
	int ss_portnum = -1;	// 서버 간 통신용 포트번호
	int snpc_portnum = -1;	// npc서버 통신용 포트번호
	if (argc > 1) {			// 입력된 명령행 인수가 있을 때 (주로 서버다운으로 인한 서버 재실행때 사용됨)
		// Serve ID지정
		my_server_id = atoi(argv[1]) % 10;

		// Active 여부 결정
		int is_active = atoi(argv[1]) / 10;	// 십의자리가 1: Standby, 2: Active
		if (is_active == 0) {	// 서버의 첫 실행은 ID에 따라 구분
			if (my_server_id == 0) {
				b_active_server = false;
			}
			else if (my_server_id == 1) {
				b_active_server = true;
			}
		}
		else if (is_active == 1) {	// 강제 Standby모드 실행
			b_active_server = false;
		}
		else if (is_active == 2) {	// 강제 Active모드 실행
			b_active_server = true;
		}
		else {
			cout << "[Server ID Error] Unknown ID.\n" << endl;
			return -1;
		}
	}
	else {				// 만약 입력된 명령행 인수가 없다면 입력을 받습니다.
		cout << "실행할 서버: ";
		cin >> my_server_id;

		// Active 여부 결정
		switch (my_server_id) {
		case 0:	// 0번 서버
			b_active_server = false;
			break;
		case 1:	// 1번 서버
			b_active_server = true;
			break;
		case 10:	// 0번 서버 (강제 Standby)
		case 11:	// 1번 서버 (강제 Standby)
			b_active_server = false;
			break;
		case 20:	// 0번 서버 (강제 Active)
		case 21:	// 1번 서버 (강제 Active)
			b_active_server = true;
			break;
		default:
			cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
			return 0;
		}
	}

	// 서버번호에 따라 포트번호를 지정해줍니다.
	switch (my_server_id % 10) {
	case 0:	// 0번 서버
		sc_portnum = PORTNUM_LOGIC_0;
		ss_portnum = HA_PORTNUM_S0;
		snpc_portnum = PORTNUM_LGCNPC_0;
		break;
	case 1:	// 1번 서버
		sc_portnum = PORTNUM_LOGIC_1;
		ss_portnum = HA_PORTNUM_S1;
		snpc_portnum = PORTNUM_LGCNPC_1;
		break;
	default:
		cout << "잘못된 값이 입력되었습니다. 프로그램을 종료합니다." << endl;
		return 0;
	}
	cout << "Server[" << my_server_id << "] 가 가동되었습니다. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else cout << "Stand-By";
	cout << " / S - C PORT : " << sc_portnum << " / S - S PORT : " << ss_portnum << " / S - NPC PORT : " << snpc_portnum << " ]" << endl;


	//======================================================================
	// [ HA - 서버 수평확장 ]
	// HA Listen Socket (서버 간 통신을 위한 Listen소켓)
	g_ss_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN ha_server_addr;
	memset(&ha_server_addr, 0, sizeof(ha_server_addr));
	ha_server_addr.sin_family = AF_INET;
	ha_server_addr.sin_port = htons(ss_portnum);
	ha_server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_ss_listensock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(ha_server_addr));
	listen(g_ss_listensock, SOMAXCONN);
	SOCKADDR_IN ha_addr;
	int ha_addr_size = sizeof(ha_addr);

	// HA Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_ss_listensock), h_iocp, CP_KEY_LISTEN_EXLOGIC, 0);
	right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX ha_over;
	ha_over.process_type = OP_ACCEPT;
	ha_over.wsabuf.buf = reinterpret_cast<CHAR*>(right_ex_server_sock);
	AcceptEx(g_ss_listensock, right_ex_server_sock, ha_over.send_buf, 0, ha_addr_size + 16, ha_addr_size + 16, 0, &ha_over.overlapped);

	// 수평확장된 서버의 마지막 구성원이 아니라면, 오른쪽에 있는 서버에 비동기connect 요청을 보냅니다.
	if (my_server_id < MAX_LOGIC_SERVER - 1) {
		int right_servernum = my_server_id + 1;
		int right_portnum = ss_portnum + 1;
		cout << "다른 이중화 서버(Server[" << right_servernum << "] (S-S PORT: " << right_portnum << ")에 비동기Connect를 요청합니다." << endl;

		// ConnectEx
		SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		GUID guid = WSAID_CONNECTEX;
		DWORD bytes = 0;
		LPFN_CONNECTEX connectExFP;
		::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
		closesocket(temp_s);

		SOCKADDR_IN ha_server_addr;
		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // 실제 연결할 소켓
		int ret = ::bind(right_ex_server_sock, reinterpret_cast<LPSOCKADDR>(&ha_server_addr), sizeof(ha_server_addr));
		if (ret != 0) {
			cout << "Bind Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		OVER_EX* con_over = new OVER_EX;
		con_over->process_type = OP_CONNECT;
		int key_num = CP_KEY_LOGIC2EXLOGIC + right_servernum;
		HANDLE hret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(right_ex_server_sock), h_iocp, key_num, 0);
		if (NULL == hret) {
			cout << "CreateIoCompletoinPort Error - " << ret << endl;
			cout << WSAGetLastError() << endl;
			exit(-1);
		}

		ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
		ha_server_addr.sin_family = AF_INET;
		ha_server_addr.sin_port = htons(right_portnum);	// 수평확장된 서버군에서 자기 오른쪽에 있는 서버

		// 루프백
		inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

		// 원격
		/*if (my_server_id == 0) {
			inet_pton(AF_INET, IPADDR_LOGIC1, &ha_server_addr.sin_addr);
		}
		else if (my_server_id == 1) {
			inet_pton(AF_INET, IPADDR_LOGIC0, &ha_server_addr.sin_addr);
		}
		*/

		BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
		if (FALSE == bret) {
			int err_no = GetLastError();
			if (ERROR_IO_PENDING == err_no)
				cout << "Server Connect 시도 중...\n" << endl;
			else {
				cout << "ConnectEX Error - " << err_no << endl;
				cout << WSAGetLastError() << endl;
			}
		}
	}
	else {
		cout << "마지막 서버구성원이므로 Connect를 수행하지않습니다.\n" << endl;
	}
	extended_servers[my_server_id].id = my_server_id;
	extended_servers[my_server_id].s_state = ST_ACCEPTED;

	//======================================================================
	// [ HA - 서버 매니저 연결 ]
	manager_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN manager_addr;
	ZeroMemory(&manager_addr, sizeof(manager_addr));
	manager_addr.sin_family = AF_INET;
	manager_addr.sin_port = htons(PORTNUM_MANAGER);
	inet_pton(AF_INET, "127.0.0.1", &manager_addr.sin_addr);
	connect(manager_socket, reinterpret_cast<sockaddr*>(&manager_addr), sizeof(manager_addr));


	//======================================================================
	// [ Main ]

	// [ Main - 맵(건물) 정보 로드 ]
	cout << "[Import Map Data...]" << endl;
	// 1. 디렉토리 검색
	string filename;
	vector<string> readTargets;

	filesystem::path collidebox_path(".\\Collideboxes");
	if (filesystem::exists(collidebox_path)) {
		filesystem::recursive_directory_iterator itr(collidebox_path);
		while (itr != filesystem::end(itr)) {
			const filesystem::directory_entry& entry = *itr;
			//cout << entry.path().string() << endl;
			string path_name = entry.path().string();
			if (path_name.find(".txt") != string::npos) {	// .txt 가 들어간 파일만 저장합니다. (디렉토리 이름만 있는 path 배제)
				readTargets.push_back(path_name);
			}
			itr++;
		}
	}
	else {
		cout << "[Directory Search Error] Unknown Directory." << endl;
	}

	// 파일 읽기
	for (auto& fname : readTargets) {
		cout << "  Import From File \'" << fname << "\'... ";
		//string fname = readTargets[0];
		ifstream txtfile(fname);

		Building tmp_mapobj;

		string word;
		int word_cnt = 0;

		bool b_center = false;
		bool b_scale = false;
		bool b_local_forward = false;
		bool b_local_right = false;
		bool b_local_rotate = false;
		bool b_angle_aob = false;
		bool b_angle_boc = false;

		float tmp_buf[3] = { 0.f, 0.f, 0.f }; // 뽑은 데이터를 임시 저장할 공간, 3개 꽉차면 벡터에 넣어주고 비워두자.
		int buf_count = 0;
		if (txtfile.is_open()) {
			while (txtfile >> word) {
				if (word == "Center:") {
					b_center = true;
					buf_count = 0;
				}
				else if (word == "Scale:") {
					b_scale = true;
					buf_count = 0;
				}
				else if (word == "Forward:") {
					b_local_forward = true;
					buf_count = 0;
				}
				else if (word == "Right:") {
					b_local_right = true;
					buf_count = 0;
				}
				else if (word == "Rotate:") {
					b_local_rotate = true;
					buf_count = 0;
				}
				else if (word == "AOB:") {
					b_angle_aob = true;
					buf_count = 0;
				}
				else if (word == "BOC:") {
					b_angle_boc = true;
					buf_count = 0;
				}
				else {
					if (!(b_center || b_scale || b_local_forward || b_local_right || b_local_rotate || b_angle_aob || b_angle_boc)) continue;

					if (b_angle_aob || b_angle_boc) {
						if (b_angle_aob) {
							float aob = string2data(word);
							tmp_mapobj.setAngleAOB(aob);
							b_angle_aob = false;
						}
						else if (b_angle_boc) {
							float boc = string2data(word);
							tmp_mapobj.setAngleBOC(boc);
							b_angle_boc = false;

							// BOC가 건물정보의 마지막이므로 마지막 정보는 BoundingBox 업데이트
							tmp_mapobj.setBB();

							// 그리고 완성된 tmpbuilding 을 mapobj_info에 추가
							mapobj_info.push_back(tmp_mapobj);
						}
						continue;
					}

					tmp_buf[buf_count] = string2data(word);
					if (buf_count == 2) {
						XMFLOAT3 tmp_flt3(tmp_buf[0], tmp_buf[1], tmp_buf[2]);
						if (b_center) {
							tmp_mapobj.setPos2(tmp_flt3);
							b_center = false;
						}
						else if (b_scale) {
							tmp_mapobj.setScale(tmp_flt3.x / 2.0f, tmp_flt3.y / 2.0f, tmp_flt3.z / 2.0f);
							b_scale = false;
						}
						else if (b_local_forward) {
							tmp_mapobj.setLocalForward(tmp_flt3);
							b_local_forward = false;
						}
						else if (b_local_right) {
							tmp_mapobj.setLocalRight(tmp_flt3);
							b_local_right = false;
						}
						else if (b_local_rotate) {
							tmp_mapobj.setLocalRotate(tmp_flt3);
							b_local_rotate = false;
						}
						memset(tmp_buf, 0, sizeof(tmp_buf));
						buf_count = 0;
					}
					else {
						buf_count++;
					}
				}
				word_cnt++;
			}
		}
		else {
			cout << "[Error] Unknown File." << endl;
		}
		cout << "---- OK." << endl;
		txtfile.close();
	}

	//======================================================================
	// [ Main - 정보 초기화 ]
	setMissions();//미션 설정
	setOccupyAreas();//점령지역 설정
	cout << "\n";

	initHealpacks();	// 힐팩 설치

	//======================================================================
	// [ Main - 클라이언트 연결 ]
	// Client Listen Socket (클라이언트-서버 통신을 위한 Listen소켓)
	g_sc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(sc_portnum);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_sc_listensock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_sc_listensock, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	// Client Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_sc_listensock), h_iocp, CP_KEY_LISTEN_CLIENT, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over;
	a_over.process_type = OP_ACCEPT;
	a_over.wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);

	int option = TRUE;//Nagle
	setsockopt(g_sc_listensock, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	AcceptEx(g_sc_listensock, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over.overlapped);

	//======================================================================
	// [ Main - NPC서버 연결 ]
	// NPC Listen Socket (로직서버-NPC서버 통신을 위한 Listen소켓)
	g_npc_listensock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr2;
	memset(&server_addr, 0, sizeof(server_addr2));
	server_addr2.sin_family = AF_INET;
	server_addr2.sin_port = htons(snpc_portnum);
	server_addr2.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_npc_listensock, reinterpret_cast<sockaddr*>(&server_addr2), sizeof(server_addr2));
	listen(g_npc_listensock, SOMAXCONN);
	SOCKADDR_IN npc_addr;
	int addr_size2 = sizeof(npc_addr);
	int npc_id = 0;

	// NPC Accept
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_npc_listensock), h_iocp, CP_KEY_LISTEN_NPC, 0);
	SOCKET npc_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EX a_over2;
	a_over2.process_type = OP_ACCEPT;
	a_over2.wsabuf.buf = reinterpret_cast<CHAR*>(npc_socket);
	AcceptEx(g_npc_listensock, npc_socket, a_over2.send_buf, 0, addr_size2 + 16, addr_size2 + 16, 0, &a_over2.overlapped);

	//======================================================================
	// [ Main - 스레드 생성 ]
	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);			// 클라이언트-서버 통신용 Worker스레드

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);				// 클라이언트 로직 타이머스레드
	timer_threads.emplace_back(heartBeatFunc);			// 서버 간 Heartbeat교환 스레드

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_sc_listensock);
	WSACleanup();
}