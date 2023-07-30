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
chrono::system_clock::time_point g_s_start_time;	// ���� ���۽ð�  (����: ms)
milliseconds g_curr_servertime;
bool b_isfirstplayer;	// ù player��������. (ù Ŭ�� ���Ӻ��� �����ð��� �帣���� �ϱ� ����)
mutex servertime_lock;	// �����ð� lock

//======================================================================
class Mission
{
public:
	short type;
	float goal;
	float curr;
	int start;				// �̼� ���� �ð�

public:
	Mission() {
		type = MISSION_KILL;
		goal = 0.0f;
		curr = 0.0f;
	}
};
mutex mission_lock;			// �̼� lock
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
	// ��������1 �̼�
	stage1_missions[0] = setMission(MISSION_KILL, STAGE1_MISSION1_GOAL, 0.0f);
	stage1_missions[1] = setMission(MISSION_OCCUPY, STAGE1_MISSION2_GOAL * 1'000.f, 0.0f);

	cout << " ---- OK." << endl;
}

//======================================================================
array<MapObject, TOTAL_STAGE + 1> occupy_areas;	// ���������� ��������
void setOccupyAreas() {
	cout << "[Init Occupy Areas...]";
	// ��������0 ���� ���������� ����.
	occupy_areas[0] = MapObject{ -9999.f, -9999.f, -9999.f, 0.f, 0.f, 0.f };

	// ��������1 ��������
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
	XMFLOAT3 m_rightvec, m_upvec, m_lookvec;		// ���� Look, Right, Up Vectors
	XMFLOAT3 m_cam_lookvec;							// ī�޶� �躤�� (������ ���⿡ �ϰ� ����)

	bool b_occupying;	// ���� ������
	bool height_alert;	// ���� ������ �Ʒ��� �������� �溸���� �︰��.

	chrono::system_clock::time_point shoot_time;	// ���� �߻��� �ð�
	chrono::system_clock::time_point reload_time;	// �Ѿ� ���� ���۽ð�

	short curr_stage;

	bool in_spawn_area;		// ���������� ������ ���ݾ� ȸ���ȴ�.

	BoundingOrientedBox m_xoobb;	// Bounding Box

	unordered_set<int> view_list;	// �þ�ó�� (id���� �־��ְ� �ȴ�.) => �׷� ������ �ؾ��ϳ�: �÷��̾�, npc���� ID�� ���� �ٸ������� ����������Ѵ�.

	bool oneshot_onekill_cheat;		//[ġƮŰ] ���� ��ų
	bool immortal_cheat;			//[ġƮŰ] ����

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

		oneshot_onekill_cheat = false;	// ������ų ġƮŰ
		immortal_cheat = false;	// ���� ġƮŰ
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
	// 1. �÷��̾� ������Ʈ
	for (auto& other_pl : clients) {
		if (other_pl.id == id) continue;
		if (other_pl.s_state != ST_INGAME) continue;
		if (other_pl.curr_stage != curr_stage) continue;

		int moved_pl_id = id + PLAYER_ID_START;		// �̵��� ��ü (�ڽ�)
		int other_pl_id = other_pl.id + PLAYER_ID_START;		// �ٸ� ��ü

		// 1) ���������� ���ο� ��ü�� �þ� �ȿ� ���� ���
		if (XMF_Distance(pos, other_pl.pos) <= HUMAN_VIEW_RANGE) {
			// 1-1) �ڽ��� ViewList�� �þ߿� ���� ���� �÷��̾��� ID �߰�
			if (!view_list.count(other_pl_id)) {
				s_lock.lock();
				view_list.insert(other_pl_id);
				s_lock.unlock();
			}
			// 1-2) �þ߿� ���� ���� �÷��̾��� ViewList�� �ڽ��� ID�߰�
			if (!other_pl.view_list.count(moved_pl_id)) {
				other_pl.s_lock.lock();
				other_pl.view_list.insert(moved_pl_id);
				other_pl.s_lock.unlock();
			}
		}
		// 2) ���������� ������ �þ߿� �־��� ��ü�� �þ߿��� ����� ���
		else {
			// 2-1) �ڽ��� ViewList�� �þ߿� ���� ���� �÷��̾��� ID ����
			if (view_list.count(other_pl_id)) {
				s_lock.lock();
				view_list.erase(other_pl_id);
				s_lock.unlock();
			}
			// 2-2) �þ߿� ���� ���� �÷��̾��� ViewList�� �ڽ��� ID ����
			if (other_pl.view_list.count(moved_pl_id)) {
				other_pl.s_lock.lock();
				other_pl.view_list.erase(moved_pl_id);
				other_pl.s_lock.unlock();
			}
		}
	}

	// 2. NPC ������Ʈ (�ӽ÷� ���̷� ��ü��.)
	for (auto& npc : npcs) {
		if (npc.id == -1) continue;
		int npc_id = npc.id + NPC_ID_START;

		// 1) ���������� ���ο� ��ü�� �þ� �ȿ� ���� ���
		if (XMF_Distance(pos, npc.pos) <= HUMAN_VIEW_RANGE) {
			// 1-1) �ڽ��� ViewList�� �þ߿� ���� ���� �÷��̾��� ID �߰�
			if (!view_list.count(npc_id)) {
				s_lock.lock();
				view_list.insert(npc_id);
				s_lock.unlock();
			}
			// ���̴� View List X
		}
		// 2) ���������� ������ �þ߿� �־��� ��ü�� �þ߿��� ����� ���
		else {
			// 2-1) �ڽ��� ViewList�� �þ߿� ���� ���� �÷��̾��� ID ����
			if (view_list.count(npc_id)) {
				s_lock.lock();
				view_list.erase(npc_id);
				s_lock.unlock();
			}
			// Npc�� �þߴ� Npc�������� ����.
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
array<HA_SERVER, MAX_LOGIC_SERVER> extended_servers;	// HA������ ���� ����Ȯ��� ������

//======================================================================
class HEALPACK
{
public:
	XMFLOAT3 pos;
	bool is_used;	// ���Ǿ��°�?
	chrono::system_clock::time_point used_time;	// ���� �ð�
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
HANDLE h_iocp;				// IOCP �ڵ�
SOCKET g_sc_listensock;		// Ŭ���̾�Ʈ ��� listen����
SOCKET g_npc_listensock;	// NPC���� ��� listen����
SOCKET g_ss_listensock;		// ����Ȯ�� ���� �� ��� listen ����

SOCKET left_ex_server_sock;								// ���� ��ȣ�� ����
SOCKET right_ex_server_sock;							// ���� ��ȣ�� ����

int my_server_id;										// �� ���� �ĺ���ȣ
bool b_active_server;									// Active �����ΰ�?
bool b_npcsvr_conn;										// NPC������ ����Ǿ��°�?

//======================================================================
void resetGame() {	// ������ ���������� ��� ���������� �ʱ���·� �������´�.
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

		// �����ִ� ��� Ŭ���̾�Ʈ�鿡�� target_id�� Ŭ���̾�Ʈ�� ���� ������ ����� �˸��ϴ�.
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

		// NPC �������Ե� target_id�� Ŭ���̾�Ʈ�� ���� ������ ����� �˸��ϴ�.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_remove_packet(target_id, TARGET_PLAYER);
		}

		// ���� �����ִ� �÷��̾ ���ٸ� ������ �ʱ�ȭ�մϴ�.
		if (remain_user_cnt == 0) {
			cout << "��� ���� ������ Ŭ���̾�Ʈ�� ������ Ŭ���̾�Ʈ�����ϴ�. ������ �ʱ�ȭ�մϴ�." << endl;
			resetGame();
		}

		break;
	}
	case SESSION_NPC:
		cout << "NPC ������ �ٿ�Ǿ����ϴ�." << endl;
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

		cout << "Server[" << extended_servers[target_id].id << "]�� �ٿ��� �����Ǿ����ϴ�." << endl;	// server message

		// ���� �����
		wchar_t wchar_buf[10];
		wsprintfW(wchar_buf, L"%d", 10 + target_id);	// �����ڸ�: Actvie����(S: 1, A: 2), �����ڸ�: ����ID

		// XD���� ������ ������ ��(���� �׽�Ʈ)�� �ܺο��� ������ ���� �������ݴϴ�.
		string XDFolderKeyword = "XD";
		if (filesystem::current_path().string().find(XDFolderKeyword) != string::npos) {
			ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L"../../../Execute/Execute_S", SW_SHOW);	// ���� �׽�Ʈ��
		}
		else {
			ShellExecute(NULL, L"open", L"Server.exe", wchar_buf, L".", SW_SHOW);					// �ܺ� ����� (exe�� ����ɶ�)
		}

		// ���� ����ȭ�� ���ؼ� ����Ǵ� PC�� "�ܺ� IP"�� �˾ƾ� �Ѵ�.


		// Ŭ���̾�Ʈ���� Active������ �ٿ�Ǿ��ٰ� �˷���.
		if (!b_active_server) {	// ���� Active�� �ƴϸ� ��밡 Active��. (������ 2���ۿ� ���� ����)
			b_active_server = true;
			cout << "���� Server[" << my_server_id << "] �� Active ������ �°ݵǾ����ϴ�. [ MODE: Stand-by -> Active ]\n" << endl;
		}

		// ���� �ڽ��� ������ ������ �ٿ�Ǿ��µ�, �� ������ �������� ������ ������ ��� ������ �������� ConnectEx ��û�� �����ϴ�.
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
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
				ha_server_addr.sin_port = htons(target_portnum);	// ����Ȯ��� ���������� �ڱ� �����ʿ� �ִ� ����
				inet_pton(AF_INET, IPADDR_LOOPBACK, &ha_server_addr.sin_addr);

				BOOL bret = connectExFP(right_ex_server_sock, reinterpret_cast<sockaddr*>(&ha_server_addr), sizeof(SOCKADDR_IN), nullptr, 0, nullptr, &con_over->overlapped);
				if (FALSE == bret) {
					int err_no = GetLastError();
					if (ERROR_IO_PENDING == err_no)
						cout << "Server Connect ��õ� ��...\n" << endl;
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

		// �������� ���� ù��° Ŭ���̾�Ʈ�� �����̶�� �׶����� �����ð��� �帣�� �����մϴ�.
		if (b_isfirstplayer) {
			cout << "���� �ð��� �帣�� �����մϴ�.\n" << endl;
			g_s_start_time = system_clock::now();
			b_isfirstplayer = false;
		}

		// ���� ������ �÷��̾��� �ʱ� ������ �����մϴ�.
		clients[client_id].id = login_packet->inroom_index;	// client_id (�������� ���� id)�� Ŭ���̾�Ʈ �ε����� �ٸ���!
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
		cout << "Player[������ ID: " << client_id << ", Ŭ�� �ε���: " << clients[client_id].id
			<< ", name: " << clients[client_id].name << "]��(��) �����Ͽ����ϴ�." << endl;	// server message

		if (!b_active_server) {
			cout << "Stand-By������ ��� ���¸� �����մϴ�." << endl;
			break;	// Active������ �ƴ϶��, Ŭ���̾�Ʈ�� ����Ǿ����� ����ڿ��� �˸��⸸ �ϰ� �ƹ��ϵ� ���� �ʽ��ϴ�.
		}

		cout << "���ο� ������Ʈ�� �����Ǿ����ϴ�! - POS:(" << clients[client_id].pos.x
			<< "," << clients[client_id].pos.y << "," << clients[client_id].pos.z << ").\n" << endl;

		clients[client_id].update_viewlist();

		//====================
		// ��������1 �̼� ����
		{
			lock_guard<mutex> lg{ clients[client_id].s_lock };
			clients[client_id].send_mission_packet(clients[client_id].curr_stage);
		}

		stage1_missions[0].start = static_cast<int>(g_curr_servertime.count());
		cout << "[" << stage1_missions[0].start << "]  ���ο� �̼� �߰�: ";
		switch (stage1_missions[0].type) {
		case MISSION_KILL:
			cout << "[óġ] ";
			break;
		case MISSION_OCCUPY:
			cout << "[����] ";
			break;
		}
		cout << stage1_missions[0].curr << " / " << stage1_missions[0].goal << "\n" << endl;

		//====================
		// 1. �� ����
		// ���� ������ Ŭ���̾�Ʈ���� �� ������ �����ݴϴ�.
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
		// 2. Player ��ü ����
		//  1) Clients
		// ���� ������ �ִ� ��� Ŭ���̾�Ʈ���� ���ο� Ŭ���̾�Ʈ(client_id)�� ������ �����մϴ�.
		for (auto& pl : clients) {
			if (pl.s_state != ST_INGAME) continue;
			if (pl.id == client_id) continue;

			lock_guard<mutex> lg{ pl.s_lock };
			pl.send_add_obj_packet(client_id, TARGET_PLAYER);
		}

		// ���� ������ Ŭ���̾�Ʈ���� ���� ������ �ִ� ��� Ŭ���̾�Ʈ�� ������ �����մϴ�.
		for (auto& pl : clients) {
			if (pl.s_state != ST_INGAME) continue;
			if (pl.id == client_id) continue;

			lock_guard<mutex> lg{ pl.s_lock };
			clients[client_id].send_add_obj_packet(pl.id, TARGET_PLAYER);
		}

		//  2) NPCs
		// ���� ������ Ŭ���̾�Ʈ���� ���� ������ �ִ� ��� NPC�� ������ �����մϴ�.
		for (auto& npc : npcs) {
			if (npc.id == -1) continue;

			lock_guard<mutex> lg{ clients[client_id].s_lock};
			clients[client_id].send_add_obj_packet(npc.id, TARGET_NPC);
		}

		//  3) NPC������ ���� ������ Ŭ���̾�Ʈ�� ������ �����մϴ�.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_add_obj_packet(client_id, TARGET_PLAYER);
		}


		break;
	}// CS_LOGIN end
	case CS_MOVE:
	{
		if (!b_active_server) break;
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ���� �ڴ� ������ �� ����.

		CS_MOVE_PACKET* cl_move_packet = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		
		// 1. ���� �ٴڿ� ������ �����Ѵ�.
		if (clients[client_id].role == ROLE_HELI && clients[client_id].pos.y <= 8.5f) {
			if (clients[client_id].pos.y <= 9.f) {		
				// �켱 NPC�������� �÷��̾� ���� ������ �����ݴϴ�.
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

				// ���ó��
				clients[client_id].s_lock.lock();
				clients[client_id].hp = 0;
				clients[client_id].pl_state = PL_ST_DEAD;
				clients[client_id].s_lock.unlock();
				cout << "Player[" << client_id << "]�� �︮���Ͱ� ���鿡 �߶��Ͽ� �����Ͽ���." << endl;

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

		// 2. �������� �̵��̶�� ���� ������ ������Ʈ �Ѵ�.
		clients[client_id].s_lock.lock();
		clients[client_id].pos = { cl_move_packet->x, cl_move_packet->y, cl_move_packet->z };
		clients[client_id].setBB();
		clients[client_id].pl_state = cl_move_packet->direction + 1;	// MV_FRONT = 0, MV_BACK = 1, MV_SIDE = 2; PL_ST_MOVE_FRONT = 1, PL_ST_MOVE_BACK = 2, PL_ST_MOVE_SIDE = 3;
		if (clients[client_id].role == ROLE_HELI) {
			if (clients[client_id].height_alert && clients[client_id].pos.y > 30.0f) {	// �溸�� �︮�� �ִٰ� ���� ���� ���� �̻� �ö���� �溸�� �����Ѵ�.
				clients[client_id].height_alert = false;
				SC_HEIGHT_ALERT_PACKET alert_cancel_packet;
				alert_cancel_packet.size = sizeof(SC_HEIGHT_ALERT_PACKET);
				alert_cancel_packet.type = SC_HEIGHT_ALERT;
				alert_cancel_packet.alert_on = 0;
				clients[client_id].do_send(&alert_cancel_packet);
			}
			else if (!clients[client_id].height_alert && clients[client_id].pos.y <= 30.0f) {	// ���� ���� ���� �̸� �������� �溸�� �︰��.
				clients[client_id].height_alert = true;
				SC_HEIGHT_ALERT_PACKET alert_start_packet;
				alert_start_packet.size = sizeof(SC_HEIGHT_ALERT_PACKET);
				alert_start_packet.type = SC_HEIGHT_ALERT;
				alert_start_packet.alert_on = 1;
				clients[client_id].do_send(&alert_start_packet);
			}
		}
		clients[client_id].s_lock.unlock();

		//cout << "[Move TEST] Player[" << client_id << "]�� " << cl_move_packet->direction << " �������� �̵��Ͽ� POS�� ("
		//	<< clients[client_id].pos.x << ", " << clients[client_id].pos.y << ", " << clients[client_id].pos.x << ")�� �Ǿ���.\n" << endl;

		// 3. View List�� ������Ʈ�Ѵ�.
		clients[client_id].update_viewlist();

		// 4. �ٸ� Ŭ���̾�Ʈ���� �÷��̾ �̵��� ��ġ�� �˷��ش�.
		for (auto& vl_key : clients[client_id].view_list) {
			if (!(PLAYER_ID_START <= vl_key && vl_key <= PLAYER_ID_END)) continue;	// Player�� �ƴϸ� continue

			int pl_id = vl_key - PLAYER_ID_START;
			if (pl_id == client_id) continue;
			if (clients[pl_id].s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ clients[pl_id].s_lock };
			clients[pl_id].send_move_packet(client_id, TARGET_PLAYER, cl_move_packet->direction);

		}

		// 5. NPC �������Ե� �÷��̾ �̵��� ��ġ�� �˷��ش�.
		if (b_npcsvr_conn) {
			lock_guard<mutex> lg{ npc_server.s_lock };
			npc_server.send_move_packet(client_id, TARGET_PLAYER, cl_move_packet->direction);
		}

		// 6. ���ɹ̼� ���̶�� ���������� �ִ��� Ȯ���Ѵ�.
		short curr_mission = curr_mission_stage[clients[client_id].curr_stage];
		if (clients[client_id].curr_stage == 1 && stage1_missions[curr_mission].type == MISSION_OCCUPY) {
			// �������� ����
			float occupy_leftup_x = occupy_areas[1].getPosX() - occupy_areas[1].getScaleX() / 2.0f;
			float occupy_leftup_z = occupy_areas[1].getPosZ() - occupy_areas[1].getScaleZ() / 2.0f;
			float occupy_rightbottom_x = occupy_areas[1].getPosX() + occupy_areas[1].getScaleX() / 2.0f;
			float occupy_rightbottom_z = occupy_areas[1].getPosZ() + occupy_areas[1].getScaleZ() / 2.0f;

			// ���� ������ �ִ��� �˻�
			if (occupy_leftup_x <= clients[client_id].pos.x && clients[client_id].pos.x <= occupy_rightbottom_x && \
				occupy_leftup_z <= clients[client_id].pos.z && clients[client_id].pos.z <= occupy_rightbottom_z) {
				// �ȿ� �ִٸ�
				if (!clients[client_id].b_occupying) {		// ���������� ���� ���� ���
					clients[client_id].s_lock.lock();
					clients[client_id].b_occupying = true;	// ���� ���� On
					cout << "Client[" << client_id << "]�� ������ �����մϴ�. (START TIME: " << static_cast<int>(g_curr_servertime.count()) << ")\n" << endl;
					clients[client_id].s_lock.unlock();
				}
			}
			else {
				// �ۿ� �ִٸ�
				if (clients[client_id].b_occupying) {		// �������� ������ Ż���� ���
					clients[client_id].s_lock.lock();
					clients[client_id].b_occupying = false;	// ���� ���� Off
					cout << "Client[" << client_id << "]�� ������ �����մϴ�. (END TIME: " << static_cast<int>(g_curr_servertime.count()) << ")\n" << endl;
					clients[client_id].s_lock.unlock();
				}
			}
		}
		else if (clients[client_id].curr_stage == 2) {
		}

		// 7. ���������� �ִ��� Ȯ���Ѵ�.
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

		// 8. ���� ��ġ�� �ִ��� Ȯ���Ѵ�.
		if (clients[client_id].curr_stage == 1 && clients[client_id].role == ROLE_RIFLE) {	// ������ ���Ѽ��� ���� �� �ִ�.
			bool collided_healpack = false;
			for (int i = 0; i < 8; ++i) {
				if (g_healpacks[i].is_used) continue;
				if (g_healpacks[i].pos.x - HEALPACK_SIZE / 2.0f <= clients[client_id].pos.x && clients[client_id].pos.x <= g_healpacks[i].pos.x + HEALPACK_SIZE / 2.0f\
					&& g_healpacks[i].pos.z - HEALPACK_SIZE / 2.0f <= clients[client_id].pos.z && clients[client_id].pos.z <= g_healpacks[i].pos.z + HEALPACK_SIZE / 2.0f) {
					collided_healpack = true;

					g_healpacks[i].is_used = true;
					g_healpacks[i].used_time = system_clock::now();

					// ������ ����� ����� ��� Ŭ�󿡰� �˷��ݴϴ�. (���� ����Ʈ OFF�� ����)
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

			if (collided_healpack && clients[client_id].hp < HUMAN_MAXHP) {	// ���� ��ġ�� �ִٸ� �� Ŭ���̾�Ʈ�� ġ�����ݴϴ�.
				clients[client_id].s_lock.lock();
				clients[client_id].hp += HEALPACK_RECOVER_HP;
				if (clients[client_id].hp > HUMAN_MAXHP) clients[client_id].hp = HUMAN_MAXHP;
				clients[client_id].s_lock.unlock();
				cout << "Player[" << client_id << "]�� ������ ��� HP�� (+" << HEALPACK_RECOVER_HP << ")��ŭ ȸ���Ͽ���. (HP: " << clients[client_id].hp << " ����)\n" << endl;

				SC_HEALING_PACKET healing_packet;
				healing_packet.size = sizeof(SC_HEALING_PACKET);
				healing_packet.type = SC_HEALING;
				healing_packet.id = clients[client_id].id;
				healing_packet.value = HEALPACK_RECOVER_HP;

				// �켱 NPC�������� �÷��̾� ���� ������ �����ݴϴ�.
				{
					lock_guard<mutex> lg{ npc_server.s_lock };
					npc_server.do_send(&healing_packet);
				}

				// ��� Ŭ���̾�Ʈ���׵� �����ݴϴ�.
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
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ���� �ڴ� ������ �� ����.

		CS_ROTATE_PACKET* cl_rotate_packet = reinterpret_cast<CS_ROTATE_PACKET*>(packet);

		// 1. ���� ������ ������Ʈ �Ѵ�.
		clients[client_id].s_lock.lock();
		clients[client_id].m_rightvec = { cl_rotate_packet->right_x, cl_rotate_packet->right_y, cl_rotate_packet->right_z };
		clients[client_id].m_upvec = { cl_rotate_packet->up_x, cl_rotate_packet->up_y, cl_rotate_packet->up_z };
		clients[client_id].m_lookvec = { cl_rotate_packet->look_x, cl_rotate_packet->look_y, cl_rotate_packet->look_z };
		clients[client_id].m_cam_lookvec = { cl_rotate_packet->cam_look_x, cl_rotate_packet->cam_look_y, cl_rotate_packet->cam_look_z };
		clients[client_id].setBB();
		clients[client_id].s_lock.unlock();

		// 2. �ٸ� Ŭ���̾�Ʈ���� �÷��̾ ȸ���� ������ �˷��ش�.
		for (auto& vl_key : clients[client_id].view_list) {
			if (!(PLAYER_ID_START <= vl_key && vl_key <= PLAYER_ID_END)) continue;;	// Player�� �ƴϸ� break

			int pl_id = vl_key - PLAYER_ID_START;
			if (pl_id == client_id) continue;
			if (clients[pl_id].s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ clients[pl_id].s_lock };
			clients[pl_id].send_rotate_packet(client_id, TARGET_PLAYER);
		}

		// 3. NPC �������Ե� �÷��̾ ȸ���� ���⸦ �˷��ش�.
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
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ���� �ڴ� ������ �� ����.

		CS_ATTACK_PACKET* cl_attack_packet = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
		//==============================
		// 1. �Ѿ�
		// Bullet ���� üũ
		if (clients[client_id].remain_bullet <= 0) break;

		// �Ѿ� ���� ������Ʈ
		clients[client_id].s_lock.lock();
		if (clients[client_id].remain_bullet > 1)
			clients[client_id].remain_bullet -= 1;
		else
			clients[client_id].remain_bullet = 0;
		clients[client_id].s_lock.unlock();

		// �Ѿ��� �߻��ߴٴ� ������ �ٸ� Ŭ���̾�Ʈ���� �˷��ݴϴ�.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_stage == 0) continue;

			// ���� ���� ����� ���� �Ÿ��� �����մϴ�
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
		// 2. �浹�˻�
		bool b_collide = false;

		// �߸Ź�� (���Ŀ� �ݵ�� ����ĳ��Ʈ�� �ٲ����!!!)
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
			
			// 1. �� ����(�ǹ�, ����, �ڽ�, ��, ...)�� �˻�
			for (auto& mapobj : mapobj_info) {
				// �Ÿ��� �ʹ� �ָ� �˻� X
				if (XMF_Distance(bullet.pos, mapobj.getPos()) > BULLET_RANGE) continue;

				if (bullet.m_xoobb.Intersects(mapobj.m_xoobb)) {
					if (collide_pos == XMF_fault) {	// ù �˻�� ������ ������Ʈ
						b_collide = true;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_MAPOBJ;
						//cout << "�� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// �����ص� �浹������ �����̿� ������ ������Ʈ
							b_collide = true;

							collide_pos = bullet.pos;
							collided_obj = C_OBJ_MAPOBJ;
							//cout << "�� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
						}
					}
				}
			}
			
			// 2. NPC�� �˻�
			for (auto& vl_key : clients[client_id].view_list) {
				if (!(NPC_ID_START <= vl_key && vl_key <= NPC_ID_END)) continue;

				int npc_id = vl_key - NPC_ID_START;
				if (npcs[npc_id].pl_state == PL_ST_DEAD) continue;

				if (bullet.m_xoobb.Intersects(npcs[npc_id].m_xoobb)) {
					if (collide_pos == XMF_fault) {	// ù �˻�� ������ ������Ʈ
						b_collide = true;
						collided_npc_id = npc_id;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_NPC;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// �����ص� �浹������ �����̿� ������ ������Ʈ
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

			// 3. �ٴ��̶� �˻�
			if (bullet.pos.y <= 6.0f) {
				if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// �����ص� �浹������ �����̿� ������ ������Ʈ
					b_collide = true;

					collide_pos = bullet.pos;
					collided_obj = C_OBJ_GROUND;
				}
			}

			// �浹�ߴٸ� ��Ŷ�� ������.
			if (b_collide) {
				switch (collided_obj) {
				case C_OBJ_MAPOBJ:
					//cout << "�� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
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
					//cout << "�ٴ� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
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
					// ������ ���
					int damage = 0;
					if (clients[client_id].oneshot_onekill_cheat) {	// ������ų ġƮŰ �������϶�
						damage = 9999;
					}
					else {
						float dec_damage = static_cast<float>(RIFLE_DAMAGE);
						if (XMF_Distance(npcs[collided_npc_id].pos, bullet_initpos) > 10) {	// �ָ� ������ ���� �������� ���ݾ� �����Ѵ�.
							float dist = static_cast<int>(XMF_Distance(npcs[collided_npc_id].pos, bullet_initpos));
							dec_damage = dec_damage / 100.f * (dist / 10);
						}
						damage = static_cast<int>(RIFLE_DAMAGE - dec_damage);
						if (collided_npc_id < MAX_NPC_HELI) {	// ���� �� �� ������ �´´�.
							damage = damage - static_cast<int>(damage * 0.3f);
						}
						if (damage < 1) {	// �ּ��� 1 �������� �ֵ���
							damage = 1;
						}
					}

					// �켱 �¾Ƽ� �׵� ���׵� �ǰ� ��ġ�� Ŭ���̾�Ʈ���� �˷��ݴϴ�. (��������? ������ ����)
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

					// npc ������ ó��
					if (npcs[collided_npc_id].hp > damage) {
						npcs[collided_npc_id].s_lock.lock();
						npcs[collided_npc_id].hp -= damage;
						npcs[collided_npc_id].s_lock.unlock();
						//cout << "Player[" << client_id << "]�� ���ݿ� NPC[" << collided_npc_id << "]�� ����(-" << damage << ")�� �Ծ���."
						//	<< " (���� HP : " << npcs[collided_npc_id].hp << ")\n" << endl;
					}
					else {	// npc ���
						npcs[collided_npc_id].s_lock.lock();
						npcs[collided_npc_id].hp = 0;
						npcs[collided_npc_id].pl_state = PL_ST_DEAD;
						npcs[collided_npc_id].s_lock.unlock();
						cout << "Player[" << client_id << "]�� ���ݿ� NPC[" << collided_npc_id << "]�� ����Ͽ���." << endl;

						if (collided_npc_id < STAGE1_MAX_HELI) {
							// 1) ���NPC�� ��� NPC�������� ������Ŷ�� �����ϴ�.
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

							// (�ӽ�) npc�������� ��� ���� ��� ���������� Ŭ���̾�Ʈ���� scale�� 0���� ���ߴ� �߸Ź�� �ӽ� ä��
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
							// 2) �ΰ�NPC�� ��� NPC�������� ������Ŷ, Ŭ���̾�Ʈ���� ������Ŷ�� �����ϴ�.
							//   (NPC�������� ������ ���߰��ϰ�, Ŭ�󿡼� �ִϸ��̼��� �ϰԵ˴ϴ�.)

							// ���� ��Ŷ (to. NPC����)
							if (b_npcsvr_conn) {
								lock_guard<mutex> lg{ npc_server.s_lock };
								npc_server.send_remove_packet(collided_npc_id, TARGET_NPC);
							}

							// ������Ŷ (to. Ŭ���̾�Ʈ)
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

						// �̼� ������Ʈ
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
								cout << "��������[1]�� �̼�[" << curr_mission << "] �Ϸ�!" << endl;

								// �̼� �Ϸ� ��Ŷ
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

								if (curr_mission + 1 >= ST1_MISSION_NUM) {	// ��� �̼� �Ϸ�
									cout << "��������[1]�� ��� �̼��� �Ϸ��Ͽ����ϴ�.\n" << endl;
								}
								else {
									curr_mission_stage[clients[client_id].curr_stage]++;
									curr_mission = curr_mission_stage[clients[client_id].curr_stage];

									// ���� �̼� ����
									for (auto& cl : clients) {
										if (cl.s_state != ST_INGAME) continue;
										if (cl.curr_stage != clients[client_id].curr_stage) continue;
										lock_guard<mutex> lg{ cl.s_lock };
										cl.send_mission_packet(clients[client_id].curr_stage);
									}

									stage1_missions[curr_mission].start = static_cast<int>(g_curr_servertime.count());
									cout << "[" << stage1_missions[curr_mission].start << "] ���ο� �̼� �߰�: ";
									switch (stage1_missions[curr_mission].type) {
									case MISSION_KILL:
										cout << "[óġ] ";
										break;
									case MISSION_OCCUPY:
										cout << "[����] ";
										break;
									}
									cout << stage1_missions[curr_mission].curr << " / " << stage1_missions[curr_mission].goal << "\n" << endl;
								}
							}
							else if (clients[client_id].curr_stage == 2) {
								//cout << "��������[2] �̼�-" << curr_mission << " �Ϸ�";
							}
						}
						else {
							if (clients[client_id].curr_stage == 1) {
								cout << "��������[1] ���� �̼� ������Ʈ: ";
								switch (stage1_missions[curr_mission].type) {
								case MISSION_KILL:
									cout << "[óġ] ";
									break;
								case MISSION_OCCUPY:
									cout << "[����] ";
									break;
								}
								cout << stage1_missions[curr_mission].curr << " / "
									<< stage1_missions[curr_mission].goal << "\n" << endl;
							}
							else if (clients[client_id].curr_stage == 2) {
								//cout << "��������[2] �̼� ������Ʈ: ";
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

		/* My Raycast ������ raycast�� �ؾ���.
		XMFLOAT3 human_intersection\
			= Intersect_Ray_Box(clients[client_id].pos, clients[client_id].m_lookvec, BULLET_RANGE, dummy_humanoid.pos, HUMAN_BBSIZE_X, HUMAN_BBSIZE_Y, HUMAN_BBSIZE_Z);
		if (human_intersection != XMF_fault) {
			cout << "[CS_ATTACK] ��� ���̿� �浹�Ͽ���." << endl;//test
			b_collide = true;
		}

		XMFLOAT3 heli_intersection\
			= Intersect_Ray_Box(clients[client_id].pos, clients[client_id].m_lookvec, BULLET_RANGE, dummy_helicopter.pos, HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z);
		if (heli_intersection != XMF_fault) {
			cout << "[CS_ATTACK] ��� ���̿� �浹�Ͽ���." << endl;//test
			b_collide = true;
		}
		*/


		break;
	}// CS_ATTACK end
	case CS_INPUT_KEYBOARD:
	{
		if (!b_active_server) break;		// Active ������ ��Ŷ�� ó���մϴ�.
		CS_INPUT_KEYBOARD_PACKET* inputkey_p = reinterpret_cast<CS_INPUT_KEYBOARD_PACKET*>(packet);

		float sign = 1.0f;					// right/up/look���� �������� �����̴���, �ݴ� �������� �����̴���
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
			if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ���� �ڴ� ������ �� ����.
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

		case PACKET_KEY_INSERT:		// ���� ġƮŰ
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// �߸��� ��û
			if (clients[client_id].curr_stage == 0) break;	// �߸��� ��û
			if (!b_active_server) break;	// �߸��� ��û

			if (clients[client_id].immortal_cheat == true) {
				cout << "==���� ġƮŰ ����==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].immortal_cheat = false;
				clients[client_id].s_lock.unlock();
			}
			else {
				cout << "==���� ġƮŰ==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].immortal_cheat = true;
				clients[client_id].s_lock.unlock();
			}

			break;
		}// PACKET_KEY_INSERT case end
		case PACKET_KEY_DELETE:		// ������ų ġƮŰ
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// �߸��� ��û
			if (clients[client_id].curr_stage == 0) break;	// �߸��� ��û
			if (!b_active_server) break;	// �߸��� ��û

			if (clients[client_id].oneshot_onekill_cheat == true) {
				cout << "==������ų ġƮŰ ����==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].oneshot_onekill_cheat = false;
				clients[client_id].s_lock.unlock();
			}
			else {
				cout << "==������ų ġƮŰ==\n" << endl;
				clients[client_id].s_lock.lock();
				clients[client_id].oneshot_onekill_cheat = true;
				clients[client_id].s_lock.unlock();
			}

			break;
		}// PACKET_KEY_DELETE case end
		case PACKET_KEY_END:	// ���� ġƮŰ
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// �߸��� ��û
			if (clients[client_id].curr_stage == 0) break;	// �߸��� ��û
			if (!b_active_server) break;	// �߸��� ��û
			if (!b_npcsvr_conn) break;	// ���� NPC�� ����.
			if (curr_mission_stage[clients[client_id].curr_stage] != 0) break;	// �߸��� ��û

			cout << "==���� ġƮŰ==" << endl;

			for (auto& npc : npcs) {
				if (npc.pl_state == PL_ST_DEAD) continue;

				npc.s_lock.lock();
				npc.hp = 0;
				npc.pl_state = PL_ST_DEAD;
				npc.s_lock.unlock();
				//cout << "[ġƮŰ] Player[" << client_id << "]�� ġƮŰ�� NPC[" << npc.id << "]�� ����Ͽ���." << endl;

				if (npc.id < STAGE1_MAX_HELI) {
					// 1) ���NPC�� ��� NPC�������� ������Ŷ�� �����ϴ�.
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

					// ��� �߶����
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
					// 2) �ΰ�NPC�� ��� NPC�������� ������Ŷ, Ŭ���̾�Ʈ���� ������Ŷ�� �����ϴ�.
					//   (NPC�������� ������ ���߰��ϰ�, Ŭ�󿡼� �ִϸ��̼��� �ϰԵ˴ϴ�.)

					// ���� ��Ŷ (to. NPC����)
					if (b_npcsvr_conn) {
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.send_remove_packet(npc.id, TARGET_NPC);
					}

					// ������Ŷ (to. Ŭ���̾�Ʈ)
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

			// �̼� ���� �Ϸ�
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
				cout << "��������[1]�� �̼�[" << curr_mission << "] �Ϸ�!" << endl;

				// �̼� �Ϸ� ��Ŷ
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

				// ���� �̼� ����
				for (auto& cl : clients) {
					if (cl.s_state != ST_INGAME) continue;
					if (cl.curr_stage != clients[client_id].curr_stage) continue;
					lock_guard<mutex> lg{ cl.s_lock };
					cl.send_mission_packet(clients[client_id].curr_stage);
				}

				stage1_missions[curr_mission].start = static_cast<int>(g_curr_servertime.count());
				cout << "[" << stage1_missions[curr_mission].start << "] ���ο� �̼� �߰�: ";
				switch (stage1_missions[curr_mission].type) {
				case MISSION_KILL:
					cout << "[óġ] ";
					break;
				case MISSION_OCCUPY:
					cout << "[����] ";
					break;
				}
				cout << stage1_missions[curr_mission].curr << " / " << stage1_missions[curr_mission].goal << "\n" << endl;
			}
			break;
		}// PACKET_KEY_END case end
		case PACKET_KEY_PGUP:		// ����ȸ�� ġƮŰ
		{
			if (clients[client_id].s_state != ST_INGAME) break;	// �߸��� ��û
			if (clients[client_id].curr_stage == 0) break;	// �߸��� ��û
			if (!b_active_server) break;	// �߸��� ��û
			if (clients[client_id].hp == HUMAN_MAXHP) break;	// �̹� ǮHP��.

			int damaged_hp = HUMAN_MAXHP - clients[client_id].hp;

			cout << "==���� ġƮŰ==\n" << endl;
			clients[client_id].s_lock.lock();
			clients[client_id].hp += damaged_hp;
			clients[client_id].s_lock.unlock();
			cout << "Player[" << client_id << "]�� HP�� (+" << damaged_hp << ")��ŭ ȸ���Ͽ���. (HP: "	<< clients[client_id].hp << " ����)\n" << endl;

			SC_HEALING_PACKET healing_packet;
			healing_packet.size = sizeof(SC_HEALING_PACKET);
			healing_packet.type = SC_HEALING;
			healing_packet.id = clients[client_id].id;
			healing_packet.value = damaged_hp;

			// �켱 NPC�������� �÷��̾� ������ ������ �����ݴϴ�.
			{
				lock_guard<mutex> lg{ npc_server.s_lock };
				npc_server.do_send(&healing_packet);
			}

			// ��� Ŭ���̾�Ʈ���׵� �����ݴϴ�.
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
		if (!b_active_server) break;		// Active ������ ��Ŷ�� ó���մϴ�.
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ���� �ڴ� ������ �� ����.

		CS_INPUT_MOUSE_PACKET* rt_p = reinterpret_cast<CS_INPUT_MOUSE_PACKET*>(packet);

		if (clients[client_id].s_state == ST_FREE) {
			//clients[client_id].s_lock.lock();
			//clients[client_id].s_lock.unlock();
			break;
		}

		if (rt_p->buttontype == PACKET_NONCLICK) {			// ��ư �ȴ��� (�׳� �̵��� �� ���)
		}
		else if (rt_p->buttontype == PACKET_BUTTON_L) {			// ���콺 ��Ŭ��
			// 1�������� ����
			if (clients[client_id].curr_stage == 1) {

			}
			// 2������������ ATTACK_PACKET�� ����.
		}
		else if (rt_p->buttontype == PACKET_BUTTON_R) {		// ���콺 ��Ŭ�� �巡��: ��� ����.
			// 1�������� ����
			if (clients[client_id].curr_stage == 1) {

			}
			// 2�������� ����
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

		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ������ �浹X
		if (clients[client_id].immortal_cheat) break;	// ������ �浹X

		// ������ ���
		int damage = static_cast<int>(PARTICLE_BASIC_DAMAGE * particle_pack->particle_mass);
		if (clients[client_id].role == ROLE_HELI) {	// ��� �÷��̾�� �� ������ �´´�.
			damage = damage - static_cast<int>(damage * 0.4f);
		}
		int after_hp = clients[client_id].hp - damage;	//

		// �켱 NPC�������� �÷��̾� ������ ������ �����ݴϴ�.
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

		// ������ ó��
		if (after_hp > 0) {		// ������ ó��
			clients[client_id].s_lock.lock();
			clients[client_id].hp -= damage;
			clients[client_id].s_lock.unlock();
			cout << "��ƼŬ�� �浹�Ͽ� Player[" << client_id << "]�� ����(-" << damage << ")�� �Ծ���. (HP: " << clients[client_id].hp << " ����)\n" << endl;

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
		else {					// ���
			clients[client_id].s_lock.lock();
			clients[client_id].hp = 0;
			clients[client_id].pl_state = PL_ST_DEAD;
			clients[client_id].s_lock.unlock();
			cout << "��ƼŬ�� �浹�Ͽ� Player[" << clients[client_id].id << "]�� ����Ͽ���." << endl;

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
		if (clients[client_id].role != ROLE_HELI) break;	// �߸��� ��û
		if (clients[client_id].pl_state == PL_ST_DEAD) break;	// ������ �浹X
		if (clients[client_id].immortal_cheat) break;	// ������ �浹X
		
		// ������ ���
		int damage = static_cast<int>(HUMAN_MAXHP / 3) + 1;
		int after_hp = clients[client_id].hp - damage;

		// �켱 NPC�������� �÷��̾� ������ ������ �����ݴϴ�.
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

		// ������ ó��
		if (after_hp > 0) {		// ������ ó��
			clients[client_id].s_lock.lock();
			clients[client_id].hp -= damage;
			clients[client_id].s_lock.unlock();
			cout << "���� �浹�Ͽ� Player[" << clients[client_id].id << "]�� ����(-" << damage << ")�� �Ծ���. (HP: " << clients[client_id].hp << " ����)\n" << endl;

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
		else {					// ���
			clients[client_id].s_lock.lock();
			clients[client_id].hp = 0;
			clients[client_id].pl_state = PL_ST_DEAD;
			clients[client_id].s_lock.unlock();
			cout << "���� �浹�Ͽ� Player[" << clients[client_id].id << "]�� ����Ͽ���." << endl;

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

		cout << "[HA] Clients[" << re_login_id << "]�� �ٽ� ����Ǿ����ϴ�.\n" << endl;

		break;
	}// CS_RELOGIN end
	case SS_HEARTBEAT:
	{
		SS_HEARTBEAT_PACKET* heartbeat_pack = reinterpret_cast<SS_HEARTBEAT_PACKET*>(packet);
		int recv_id = heartbeat_pack->sender_id;

		extended_servers[recv_id].heartbeat_recv_time = chrono::system_clock::now();
		if (recv_id < my_server_id) {	// A->B->A�� heartbeat�� �� ����Ŭ�� ���������ϱ� ����. (��, ������ �����κ��� Heartbeat�� ������ �� ����Ŭ�� ������ �Ǵ�)
			// Heartbeat�� ���� ���� �������� �ڽ��� Heartbeat�� �����մϴ�.
			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[recv_id].do_send(&hb_packet);										// �ڽſ��� Heartbeat�� ���� �������� �����մϴ�.
			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// ������ �ð��� ������Ʈ
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

		cout << "Client[" << replica_id << "]�� �����Ͱ� �����Ǿ����ϴ�." << endl;
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
		if (npc_id < 5) {	// ���
			npcs[npc_id].m_xoobb = BoundingOrientedBox(XMFLOAT3(npcs[npc_id].pos.x, npcs[npc_id].pos.y, npcs[npc_id].pos.z)
				, XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else {	// ���
			npcs[npc_id].setBB();
		}
		npcs[npc_id].s_lock.unlock();

		// Ŭ���̾�Ʈ�� NPC��ǥ�� ������ ���� 1�ʿ� �ѹ���
		// Ŭ���̾�Ʈ���� ������ ��ǥ���� ������ ���ϱ� ���� ������ ����.
		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 ���� ������ ���Ǵ� �ӽ��ڵ�
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

		// ���� �� NPC ���� ������Ʈ
		npcs[npc_id].s_lock.lock();
		npcs[npc_id].pos = { npc_rotate_pack->x, npc_rotate_pack->y, npc_rotate_pack->z };
		npcs[npc_id].m_rightvec = { npc_rotate_pack->right_x, npc_rotate_pack->right_y, npc_rotate_pack->right_z };
		npcs[npc_id].m_upvec = { npc_rotate_pack->up_x, npc_rotate_pack->up_y, npc_rotate_pack->up_z };
		npcs[npc_id].m_lookvec = { npc_rotate_pack->look_x, npc_rotate_pack->look_y, npc_rotate_pack->look_z };
		if (npc_id < 5) {	// ���
			npcs[npc_id].m_xoobb = BoundingOrientedBox(XMFLOAT3(npcs[npc_id].pos.x, npcs[npc_id].pos.y, npcs[npc_id].pos.z)
				, XMFLOAT3(HELI_BBSIZE_X, HELI_BBSIZE_Y, HELI_BBSIZE_Z), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else {	// ���
			npcs[npc_id].setBB();
		}
		npcs[npc_id].s_lock.unlock();
		//cout << "NPC[" << npc_id << "]�� Look(" << npcs[npc_id].m_lookvec.x << ", " << npcs[npc_id].m_lookvec.y << ", " << npcs[npc_id].m_lookvec.z
		//	<< ") �������� ȸ���Ͽ����ϴ�.\n" << endl;

		// Ŭ��� ��Ŷ ����
		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 ���� ������ ���Ǵ� �ӽ��ڵ�
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
		//cout << "NPC[" << npc_id << "]�� �����Ǿ����ϴ�.\n" << endl;

		for (auto& cl : clients) {
			if (cl.curr_stage != 1) continue;	// Stage2 NPC ���� ������ ���Ǵ� �ӽ��ڵ�
			if (cl.s_state != ST_INGAME) continue;

			lock_guard<mutex> lg{ cl.s_lock };
			cl.send_remove_packet(npc_id, TARGET_NPC);
		}

		break;
	}// NPC_REMOVE end
	case NPC_ATTACK:
	{
		NPC_ATTACK_PACKET* recv_attack_pack = reinterpret_cast<NPC_ATTACK_PACKET*>(packet);

		// �Ѿ��� �߻��ߴٴ� ������ ��� Ŭ���̾�Ʈ���� �˷��ݴϴ�.
		for (auto& cl : clients) {
			if (cl.s_state != ST_INGAME) continue;
			if (cl.curr_stage == 0) continue;

			// ���� ���� ����� ���� �Ÿ��� �����մϴ�
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

		// 2. �浹�˻�
		int npc_id = recv_attack_pack->n_id;
		bool b_collide = false;

		// �߸Ź�� (���Ŀ� �ݵ�� ����ĳ��Ʈ�� �ٲ����!!!)
		SESSION bullet;
		bullet.pos = npcs[npc_id].pos;
		if (MAX_NPC_HELI <= npcs[recv_attack_pack->n_id].id) bullet.pos.y += 2.8f;
		bullet.m_lookvec = XMFLOAT3{ recv_attack_pack->atklook_x, recv_attack_pack->atklook_y, recv_attack_pack->atklook_z };
		bullet.m_xoobb = BoundingOrientedBox(XMFLOAT3(bullet.pos.x, bullet.pos.y, bullet.pos.z)\
			, XMFLOAT3(0.1f, 0.1f, 0.3f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		//cout << "�Ѿ� �������: " << bullet.m_lookvec.x << ", " << bullet.m_lookvec.y << ", " << bullet.m_lookvec.z << endl;

		XMFLOAT3 bullet_initpos = bullet.pos;
		XMFLOAT3 collide_pos = XMF_fault;
		int collided_obj = C_OBJ_NONCOLLIDE;
		int collided_cl_id = -1;
		while (XMF_Distance(bullet.pos, bullet_initpos) <= BULLET_RANGE) {

			// 1. �� ����(�ǹ�, ����, �ڽ�, ��, ...)�� �˻�
			for (auto& mapobj : mapobj_info) {
				// �Ÿ��� �ʹ� �ָ� �˻� X
				if (XMF_Distance(bullet.pos, mapobj.getPos()) > BULLET_RANGE) continue;

				if (bullet.m_xoobb.Intersects(mapobj.m_xoobb)) {
					if (collide_pos == XMF_fault) {	// ù �˻�� ������ ������Ʈ
						b_collide = true;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_MAPOBJ;
						//cout << "�� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// �����ص� �浹������ �����̿� ������ ������Ʈ
							b_collide = true;

							collide_pos = bullet.pos;
							collided_obj = C_OBJ_MAPOBJ;
							//cout << "�� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
						}
					}
				}
			}

			// 2. �÷��̾�� �˻�
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.pl_state == PL_ST_DEAD) continue;
				if (XMF_Distance(bullet.pos, cl.pos) > BULLET_RANGE) continue;

				if (bullet.m_xoobb.Intersects(cl.m_xoobb)) {
					if (collide_pos == XMF_fault) {	// ù �˻�� ������ ������Ʈ
						b_collide = true;
						collided_cl_id = cl.inserver_index;

						collide_pos = bullet.pos;
						collided_obj = C_OBJ_PLAYER;
					}
					else {
						if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// �����ص� �浹������ �����̿� ������ ������Ʈ
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

			// 3. �ٴ��̶� �˻�
			if (bullet.pos.y <= 0.0f) {
				if (XMF_Distance(bullet.pos, bullet_initpos) < XMF_Distance(collide_pos, bullet_initpos)) {	// �����ص� �浹������ �����̿� ������ ������Ʈ
					b_collide = true;

					collide_pos = bullet.pos;
					collided_obj = C_OBJ_GROUND;
				}
			}

			// �浹�ߴٸ� ��Ŷ�� ������.
			if (b_collide) {
				switch (collided_obj) {
				case C_OBJ_MAPOBJ:
					//cout << "�� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
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
					//cout << "�ٴ� ������Ʈ�� �浹�Ͽ��� (POS: " << collide_pos.x << ", " << collide_pos.y << ", " << collide_pos.z << ")\n" << endl;
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
					// ���� ���� ġƮŰ�� �� ������� �ǳʶڴ�
					if (clients[collided_cl_id].immortal_cheat == true) {
						cout << "Client[" << collided_cl_id << "]�� �����̴�." << endl;
						break;
					}

					// ������ ���
					int damage = 0;
					if (recv_attack_pack->n_id < MAX_NPC_HELI) {
						damage = NPC_VALKAN_DAMAGE;
					}
					else {
						damage = NPC_RIFLE_DAMAGE;
					}
					if (clients[collided_cl_id].role == ROLE_HELI) {	// ��� �÷��̾�� �� ������ �´´�.
						damage = damage / 2;
					}
					int after_hp = clients[collided_cl_id].hp - damage;	//

					// �켱 NPC�������� �÷��̾� ������ ������ �����ݴϴ�.
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

					// ������ ó��
					if (after_hp > 0) {		// ������ ó��
						clients[collided_cl_id].s_lock.lock();
						clients[collided_cl_id].hp -= damage;
						clients[collided_cl_id].s_lock.unlock();
						//cout << "NPC[" << recv_attack_pack->n_id << "]�� ���ݿ� Player[" << collided_cl_id << "]�� ����(-" << damage << ")�� �Ծ���. (HP: "
						//	<< clients[collided_cl_id].hp << " ����)\n" << endl;

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
					else {					// ���
						clients[collided_cl_id].s_lock.lock();
						clients[collided_cl_id].hp = 0;
						clients[collided_cl_id].pl_state = PL_ST_DEAD;
						clients[collided_cl_id].s_lock.unlock();
						//cout << "Npc[" << recv_attack_pack->n_id << "]�� ���ݿ� Player[" << collided_cl_id << "]�� ����Ͽ���." << endl;

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
int get_new_client_id()	// clients�� ����ִ� ĭ�� ã�Ƽ� ���ο� client�� ���̵� �Ҵ����ִ� �Լ�
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
int find_empty_extended_server() {	// ex_servers�� ����ִ� ĭ�� ã�Ƽ� ���ο� Server_ex�� ���̵� �Ҵ����ִ� �Լ�
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

				// �񵿱�Conn�� �ٽ� �õ��մϴ�.
				SOCKET temp_s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				GUID guid = WSAID_CONNECTEX;
				DWORD bytes = 0;
				LPFN_CONNECTEX connectExFP;
				::WSAIoctl(temp_s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFP, sizeof(connectExFP), &bytes, nullptr, nullptr);
				closesocket(temp_s);

				SOCKADDR_IN ha_server_addr;
				ZeroMemory(&ha_server_addr, sizeof(ha_server_addr));
				ha_server_addr.sin_family = AF_INET;
				right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
						//cout << "Server Connect ��õ� ��...\n" << endl;
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
					// Ŭ���̾�Ʈ id, ����
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
					cout << "� Client�� �����û�� �޾�����, ���� ������ �� á���ϴ�.\n" << endl;
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
				cout << "NPC ������ ����Ǿ����ϴ�." << endl;
				b_npcsvr_conn = true;
				int new_key = CP_KEY_LOGIC2NPC;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(npc_socket), h_iocp, new_key, 0);
				npc_server.do_recv();
				npc_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

				ZeroMemory(&ex_over->overlapped, sizeof(ex_over->overlapped));
				ex_over->wsabuf.buf = reinterpret_cast<CHAR*>(npc_socket);
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_npc_listensock, npc_socket, ex_over->send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->overlapped);

				// NPC�������� �� ������ �����ݴϴ�.
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
					//cout << "Sever[" << new_id << "]�� �����û�� �޾ҽ��ϴ�.\n" << endl;
					extended_servers[new_id].id = new_id;
					extended_servers[new_id].remain_size = 0;
					extended_servers[new_id].socket = extended_server_socket;
					int new_key = new_id + CP_KEY_LOGIC2EXLOGIC;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(extended_server_socket), h_iocp, new_key, 0);
					extended_servers[new_id].do_recv();
					extended_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else {
					cout << "�ٸ� Sever�� �����û�� �޾�����, ���� ������ �� á���ϴ�.\n" << endl;
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
					std::cout << "���������� Server[" << server_id << "]�� ����Ǿ����ϴ�.\n" << endl;
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
			// ���� �ð� ������Ʈ
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

			// �������������� HP�� ���ݾ� ȸ���Ѵ�.
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

					// �켱 NPC�������� �÷��̾� ������ ������ �����ݴϴ�.
					{
						lock_guard<mutex> lg{ npc_server.s_lock };
						npc_server.do_send(&healing_packet);
					}

					// ��� Ŭ���̾�Ʈ���׵� �����ݴϴ�.
					for (auto& send_cl : clients) {
						if (send_cl.s_state != ST_INGAME) continue;
						if (send_cl.curr_stage == 0) continue;

						lock_guard<mutex> lg{ send_cl.s_lock };
						send_cl.do_send(&healing_packet);
					}
				}
			}

			// ���� ������
			for (int i = 0; i < 8; ++i) {
				if (!g_healpacks[i].is_used) continue;
				if (system_clock::now() > g_healpacks[i].used_time + milliseconds(HEALPACK_RESPAWN_TIME)) {
					g_healpacks[i].is_used = false;	// �����

					// ������ ������ ����� ��� Ŭ�󿡰� �˷��ݴϴ�. (���� ����Ʈ On�� ����)
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

			// ���� ���� �̼��� ���� ���̶�� ���� ���� ����� �Ѵ�.
			int occupy_member_cnt = 0;
			for (auto& cl : clients) {
				if (cl.s_state != ST_INGAME) continue;
				if (cl.curr_stage == 0) continue;

				// ���� �ο� ���
				if (cl.curr_stage == 1 && cl.b_occupying) {
					occupy_member_cnt++;
				}
			}

			if (occupy_member_cnt >= 1) {	// �� �� �̻� ���� ���̶��
				int curr_mission_id = curr_mission_stage[1];
				if (stage1_missions[curr_mission_id].curr >= stage1_missions[curr_mission_id].goal) {	// �̼� �Ϸ�
					// �̼� �Ϸ� ��Ŷ
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
				else {	// ���� ���� ������
					// �̼� ���� ������Ʈ
					mission_lock.lock();
					stage1_missions[curr_mission_id].curr += 30 * occupy_member_cnt * occupy_member_cnt;	// �������� ����� ���� ���� �������� ���� ��������.
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
		// --- ������Ʈ


		auto curr_t = system_clock::now();
		if (curr_t - start_t < 33ms) {
			this_thread::sleep_for(33ms - (curr_t - start_t));
		}
	}
}

//======================================================================
void heartBeatFunc() {	// Heartbeat���� ������ �Լ�
	while (true) {
		auto start_t = system_clock::now();

		// ================================
		// 1. Heartbeat ����
		// : ������ ������ Heartbeat�� �����ϴ�. (���� ������ ������ ������ �����ϱ� ������ ���� ������ ������ ������ �ʽ��ϴ�.)
		if (my_server_id != MAX_LOGIC_SERVER - 1) {
			if (extended_servers[my_server_id].s_state != ST_ACCEPTED)	continue;
			if (extended_servers[my_server_id + 1].s_state != ST_ACCEPTED) continue;

			SS_HEARTBEAT_PACKET hb_packet;
			hb_packet.size = sizeof(SS_HEARTBEAT_PACKET);
			hb_packet.type = SS_HEARTBEAT;
			hb_packet.sender_id = my_server_id;
			extended_servers[my_server_id + 1].do_send(&hb_packet);	// ������ ������ �����մϴ�.

			extended_servers[my_server_id].heartbeat_send_time = chrono::system_clock::now();	// ������ �ð��� ������Ʈ
		}

		// ================================
		// 2. Heartbeat ���Ű˻�
		// �������� Heartbeat�� ���� ���� ������������ �ִ��� Ȯ���մϴ�.
		//   1) ������ ���� �˻� (���� ���ʿ� �ִ� ���� �������� �˻縦 ���� �ʽ��ϴ�.)
		if (my_server_id != 0) {
			if (extended_servers[my_server_id - 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id - 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id - 1 << "]���� Heartbeat�� �������� ���� ���߽��ϴ�. ���� �ٿ����� �����մϴ�." << endl;
					disconnect(my_server_id - 1, SESSION_EXTENDED_SERVER);
				}
			}
		}
		//   2) ���� ���� �˻� (���� �����ʿ� �ִ� ���� �������� �˻縦 ���� �ʽ��ϴ�.)
		if (my_server_id != MAX_LOGIC_SERVER - 1) {
			if (extended_servers[my_server_id + 1].s_state == ST_ACCEPTED) {
				if (chrono::system_clock::now() > extended_servers[my_server_id + 1].heartbeat_recv_time + chrono::milliseconds(HB_GRACE_PERIOD)) {
					cout << "Server[" << my_server_id + 1 << "]���� Heartbeat�� �������� ���� ���߽��ϴ�. ���� �ٿ����� �����մϴ�." << endl;
					disconnect(my_server_id + 1, SESSION_EXTENDED_SERVER);
				}
			}
		}

		// ================================
		// 3. Data Replica ���� (�ڽ��� Active���� �϶�����)
		if (b_active_server) {
			// �����ͺ��� ��Ŷ�� �ްԵ� Standby������ id�� �˾Ƴ��ϴ�.
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

					cout << "[REPLICA TEST] Client[" << cl.id << "]�� ������ Sever[" << standby_id << "]���� �����մϴ�. - line: 1413" << endl;
					cout << "Stage: " << replica_pack.curr_stage << ", State: " << replica_pack.state
						<< ", Pos: " << replica_pack.x << ", " << replica_pack.y << ", " << replica_pack.z
						<< ", Look: " << replica_pack.look_x << ", " << replica_pack.look_y << ", " << replica_pack.look_z << "\n" << endl;
				}
			}
		}

		// ================================
		// 4. ������ ��� (������ ����)
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
	// [ HA - ���� ID, ��Ʈ��ȣ ���� ]
	// � ������ ������ �������� ����� �μ��� �Է¹޾Ƽ� �� ������ ��Ʈ ��ȣ�� �ο��մϴ�.
	my_server_id = 0;		// ���� ������ ���� ������ȣ
	int sc_portnum = -1;	// Ŭ���̾�Ʈ ��ſ� ��Ʈ��ȣ
	int ss_portnum = -1;	// ���� �� ��ſ� ��Ʈ��ȣ
	int snpc_portnum = -1;	// npc���� ��ſ� ��Ʈ��ȣ
	if (argc > 1) {			// �Էµ� ����� �μ��� ���� �� (�ַ� �����ٿ����� ���� ���� ����ට ����)
		// Serve ID����
		my_server_id = atoi(argv[1]) % 10;

		// Active ���� ����
		int is_active = atoi(argv[1]) / 10;	// �����ڸ��� 1: Standby, 2: Active
		if (is_active == 0) {	// ������ ù ������ ID�� ���� ����
			if (my_server_id == 0) {
				b_active_server = false;
			}
			else if (my_server_id == 1) {
				b_active_server = true;
			}
		}
		else if (is_active == 1) {	// ���� Standby��� ����
			b_active_server = false;
		}
		else if (is_active == 2) {	// ���� Active��� ����
			b_active_server = true;
		}
		else {
			cout << "[Server ID Error] Unknown ID.\n" << endl;
			return -1;
		}
	}
	else {				// ���� �Էµ� ����� �μ��� ���ٸ� �Է��� �޽��ϴ�.
		cout << "������ ����: ";
		cin >> my_server_id;

		// Active ���� ����
		switch (my_server_id) {
		case 0:	// 0�� ����
			b_active_server = false;
			break;
		case 1:	// 1�� ����
			b_active_server = true;
			break;
		case 10:	// 0�� ���� (���� Standby)
		case 11:	// 1�� ���� (���� Standby)
			b_active_server = false;
			break;
		case 20:	// 0�� ���� (���� Active)
		case 21:	// 1�� ���� (���� Active)
			b_active_server = true;
			break;
		default:
			cout << "�߸��� ���� �ԷµǾ����ϴ�. ���α׷��� �����մϴ�." << endl;
			return 0;
		}
	}

	// ������ȣ�� ���� ��Ʈ��ȣ�� �������ݴϴ�.
	switch (my_server_id % 10) {
	case 0:	// 0�� ����
		sc_portnum = PORTNUM_LOGIC_0;
		ss_portnum = HA_PORTNUM_S0;
		snpc_portnum = PORTNUM_LGCNPC_0;
		break;
	case 1:	// 1�� ����
		sc_portnum = PORTNUM_LOGIC_1;
		ss_portnum = HA_PORTNUM_S1;
		snpc_portnum = PORTNUM_LGCNPC_1;
		break;
	default:
		cout << "�߸��� ���� �ԷµǾ����ϴ�. ���α׷��� �����մϴ�." << endl;
		return 0;
	}
	cout << "Server[" << my_server_id << "] �� �����Ǿ����ϴ�. [ MODE: ";
	if (b_active_server) cout << "Acitve";
	else cout << "Stand-By";
	cout << " / S - C PORT : " << sc_portnum << " / S - S PORT : " << ss_portnum << " / S - NPC PORT : " << snpc_portnum << " ]" << endl;


	//======================================================================
	// [ HA - ���� ����Ȯ�� ]
	// HA Listen Socket (���� �� ����� ���� Listen����)
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

	// ����Ȯ��� ������ ������ �������� �ƴ϶��, �����ʿ� �ִ� ������ �񵿱�connect ��û�� �����ϴ�.
	if (my_server_id < MAX_LOGIC_SERVER - 1) {
		int right_servernum = my_server_id + 1;
		int right_portnum = ss_portnum + 1;
		cout << "�ٸ� ����ȭ ����(Server[" << right_servernum << "] (S-S PORT: " << right_portnum << ")�� �񵿱�Connect�� ��û�մϴ�." << endl;

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
		right_ex_server_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);     // ���� ������ ����
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
		ha_server_addr.sin_port = htons(right_portnum);	// ����Ȯ��� ���������� �ڱ� �����ʿ� �ִ� ����

		// ������
		inet_pton(AF_INET, "127.0.0.1", &ha_server_addr.sin_addr);

		// ����
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
				cout << "Server Connect �õ� ��...\n" << endl;
			else {
				cout << "ConnectEX Error - " << err_no << endl;
				cout << WSAGetLastError() << endl;
			}
		}
	}
	else {
		cout << "������ �����������̹Ƿ� Connect�� ���������ʽ��ϴ�.\n" << endl;
	}
	extended_servers[my_server_id].id = my_server_id;
	extended_servers[my_server_id].s_state = ST_ACCEPTED;

	//======================================================================
	// [ HA - ���� �Ŵ��� ���� ]
	manager_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN manager_addr;
	ZeroMemory(&manager_addr, sizeof(manager_addr));
	manager_addr.sin_family = AF_INET;
	manager_addr.sin_port = htons(PORTNUM_MANAGER);
	inet_pton(AF_INET, "127.0.0.1", &manager_addr.sin_addr);
	connect(manager_socket, reinterpret_cast<sockaddr*>(&manager_addr), sizeof(manager_addr));


	//======================================================================
	// [ Main ]

	// [ Main - ��(�ǹ�) ���� �ε� ]
	cout << "[Import Map Data...]" << endl;
	// 1. ���丮 �˻�
	string filename;
	vector<string> readTargets;

	filesystem::path collidebox_path(".\\Collideboxes");
	if (filesystem::exists(collidebox_path)) {
		filesystem::recursive_directory_iterator itr(collidebox_path);
		while (itr != filesystem::end(itr)) {
			const filesystem::directory_entry& entry = *itr;
			//cout << entry.path().string() << endl;
			string path_name = entry.path().string();
			if (path_name.find(".txt") != string::npos) {	// .txt �� �� ���ϸ� �����մϴ�. (���丮 �̸��� �ִ� path ����)
				readTargets.push_back(path_name);
			}
			itr++;
		}
	}
	else {
		cout << "[Directory Search Error] Unknown Directory." << endl;
	}

	// ���� �б�
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

		float tmp_buf[3] = { 0.f, 0.f, 0.f }; // ���� �����͸� �ӽ� ������ ����, 3�� ������ ���Ϳ� �־��ְ� �������.
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

							// BOC�� �ǹ������� �������̹Ƿ� ������ ������ BoundingBox ������Ʈ
							tmp_mapobj.setBB();

							// �׸��� �ϼ��� tmpbuilding �� mapobj_info�� �߰�
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
	// [ Main - ���� �ʱ�ȭ ]
	setMissions();//�̼� ����
	setOccupyAreas();//�������� ����
	cout << "\n";

	initHealpacks();	// ���� ��ġ

	//======================================================================
	// [ Main - Ŭ���̾�Ʈ ���� ]
	// Client Listen Socket (Ŭ���̾�Ʈ-���� ����� ���� Listen����)
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
	// [ Main - NPC���� ���� ]
	// NPC Listen Socket (��������-NPC���� ����� ���� Listen����)
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
	// [ Main - ������ ���� ]
	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(do_worker);			// Ŭ���̾�Ʈ-���� ��ſ� Worker������

	vector<thread> timer_threads;
	timer_threads.emplace_back(timerFunc);				// Ŭ���̾�Ʈ ���� Ÿ�̸ӽ�����
	timer_threads.emplace_back(heartBeatFunc);			// ���� �� Heartbeat��ȯ ������

	for (auto& th : worker_threads)
		th.join();
	for (auto& timer_th : timer_threads)
		timer_th.join();

	closesocket(g_sc_listensock);
	WSACleanup();
}