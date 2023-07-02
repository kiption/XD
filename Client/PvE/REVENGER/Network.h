#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <chrono>
#include "ObjectsInfo.h"
#include "GameSound.h"

GameSound gamesound;

#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

const char* SERVER_ADDR = IPADDR_LOOPBACK;   // 루프백 용
const char* LOGIC0_ADDR = IPADDR_LOGIC0;   // 리모트 용
const char* LOGIC1_ADDR = IPADDR_LOGIC1;   // 리모트 용

SOCKET s_socket;
short active_servernum = 1;
array<SOCKET, MAX_LOGIC_SERVER> sockets;
int my_id;

//==================================================
float servertime_ms;    // 실제 서버시간

int timelimit_ms;       // 스테이지별 제한시간
int timelimit_sec;

volatile bool stage1_enter_ok;
volatile bool stage2_enter_ok;

volatile bool respawn_trigger = false;

//==================================================
int curr_connection_num = 1;

chrono::system_clock::time_point last_ping;   // ping을 서버로 보낸 시간
chrono::system_clock::time_point last_pong;   // 서버의 ping에 대한 응답을 받은 시간

//==================================================
class Mission
{
public:
	short type;
	float goal;
	float curr;

public:
	Mission() {
		type = MISSION_KILL;
		goal = 0.0f;
		curr = 0.0f;
	}
};
array<Mission, TOTAL_STAGE + 1> stage_missions;
bool trigger_mission_complete = false;
bool trigger_stage_clear = false;
short curr_mission_num = 0;

//==================================================
struct message {
	char name[NAME_SIZE];
	char msg[CHAT_SIZE];
};
constexpr int MAX_SAVED_MSG = 8;
message chat_logs;
bool chat_comeTome = true;

//==================================================
enum PACKET_PROCESS_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };
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

void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED lp_over, DWORD s_flag);
OVER_EX g_recv_over;
void recvPacket(short servernum)
{
	//cout << "Do RECV" << endl;
	DWORD recv_flag = 0;

	memset(&g_recv_over.overlapped, 0, sizeof(g_recv_over.overlapped));
	g_recv_over.wsabuf.len = BUF_SIZE;
	g_recv_over.wsabuf.buf = g_recv_over.send_buf;
	if (WSARecv(sockets[servernum], &g_recv_over.wsabuf, 1, 0, &recv_flag, &g_recv_over.overlapped, recvCallback) == SOCKET_ERROR) {
		if (GetLastError() != WSA_IO_PENDING)
			cout << "[WSARecv Error] code: " << GetLastError() << "\n" << endl;
	}
}
void sendPacket(void* packet, short servernum)
{
	//cout << "Do SEND" << endl;
	OVER_EX* s_data = new OVER_EX{ reinterpret_cast<char*>(packet) };
	if (WSASend(sockets[servernum], &s_data->wsabuf, 1, 0, 0, &s_data->overlapped, sendCallback) == SOCKET_ERROR) {
		int err_no = GetLastError();
		if (err_no == WSAECONNRESET) {   // 서버가 끊어진 상황
			closesocket(sockets[active_servernum]);

			int new_portnum = 0;
			switch (servernum) {
			case 0:   // Active: 0 -> 1
				active_servernum = 1;
				new_portnum = PORTNUM_LOGIC_1;
				break;
			case 1:   // Active: 1 -> 0
				active_servernum = 0;
				new_portnum = PORTNUM_LOGIC_0;
				break;
			}

			sockets[active_servernum] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			SOCKADDR_IN newserver_addr;
			ZeroMemory(&newserver_addr, sizeof(newserver_addr));
			newserver_addr.sin_family = AF_INET;
			newserver_addr.sin_port = htons(new_portnum);
			//inet_pton(AF_INET, SERVER_ADDR, &newserver_addr.sin_addr);//루프백

			// REMOTE
			if (active_servernum == 0) {
				inet_pton(AF_INET, IPADDR_LOGIC0, &newserver_addr.sin_addr);
			}
			else if (active_servernum == 1) {
				inet_pton(AF_INET, IPADDR_LOGIC1, &newserver_addr.sin_addr);
			}
			connect(sockets[active_servernum], reinterpret_cast<sockaddr*>(&newserver_addr), sizeof(newserver_addr));

			CS_RELOGIN_PACKET re_login_pack;
			re_login_pack.size = sizeof(CS_RELOGIN_PACKET);
			re_login_pack.type = CS_RELOGIN;
			re_login_pack.id = my_id;
			sendPacket(&re_login_pack, active_servernum);
			recvPacket(active_servernum);
		}
		cout << "[WSASend Error] code: " << err_no << "\n" << endl;
	}
}

void processPacket(char* ptr);
void processData(char* net_buf, size_t io_byte);
void CALLBACK recvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	if (num_bytes == 0) {
		return;
	}

	processData(g_recv_over.send_buf, num_bytes);

	recvPacket(active_servernum);
}
void CALLBACK sendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flag)
{
	delete over;
	return;
}


void processPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* recv_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		// Player 초기정보 설정
		my_id = recv_packet->id;
		players_info[my_id].m_id = recv_packet->id;
		players_info[my_id].m_hp = recv_packet->hp;
		strcpy_s(players_info[my_id].m_name, recv_packet->name);
		players_info[my_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		players_info[my_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		players_info[my_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
		players_info[my_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		players_info[my_id].m_state = OBJ_ST_RUNNING;
		players_info[my_id].m_ingame_state = PL_ST_IDLE;
		players_info[my_id].m_new_state_update = true;
		break;
	}// SC_LOGIN_INFO case end
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Add Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;

			if (0 <= recv_id && recv_id < MAX_USER) {      // Player 추가
				players_info[recv_id].m_id = recv_id;
				strcpy_s(players_info[recv_id].m_name, recv_packet->name);
				players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
				players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
				players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
				players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
				players_info[recv_id].m_state = OBJ_ST_RUNNING;
				players_info[recv_id].m_ingame_state = recv_packet->obj_state;
				players_info[recv_id].m_new_state_update = true;

				curr_connection_num++;
			}
			else {
				cout << "[SC_ADD Error] Unknown ID." << endl;
			}
		}
		// 2. Add NPC (Helicopter)
		else if (recv_packet->target == TARGET_NPC) {
			if (npcs_info[recv_id].m_state != OBJ_ST_EMPTY) break;

			npcs_info[recv_id].m_id = recv_id;
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
			npcs_info[recv_id].m_state = OBJ_ST_RUNNING;
			npcs_info[recv_id].m_ingame_state = recv_packet->obj_state;
			npcs_info[recv_id].m_new_state_update = true;
		}
		else {
			cout << "[ADD ERROR] Unknown Target!" << endl;
		}
		break;
	}// SC_ADD_OBJECT case end
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Move Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;             // 자기자신은 클라에서 움직여주고 있음.
			if (recv_id < 0 || recv_id > MAX_USER) break;   // 잘못된 ID값

			players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };      // 상대방Players Object 이동
			switch (recv_packet->direction) {
			case MV_FRONT:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_FRONT;
				break;
			case MV_BACK:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_BACK;
				break;
			case MV_SIDE:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_SIDE;
				break;
			}
			players_info[recv_id].m_new_state_update = true;
		}
		// 2. Move Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		}
		else {
			cout << "[MOVE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_MOVE_OBJECT case end
	case SC_ROTATE_OBJECT:
	{
		SC_ROTATE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_ROTATE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Rotate Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;            // 자기자신은 클라에서 움직여주고 있음.
			if (recv_id < 0 || recv_id > MAX_USER) break;   // 잘못된 ID값

			// 상대 Object 회전
			players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		// 2. Rotate Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_ROTATE_PLAYER case end
	case SC_MOVE_ROTATE_OBJECT:
	{
		SC_MOVE_ROTATE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_MOVE_ROTATE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Rotate Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) break;            // 자기자신은 클라에서 움직여주고 있음.
			if (recv_id < 0 || recv_id > MAX_USER) break;   // 잘못된 ID값
			players_info[recv_id].m_id = recv_id;
			// 상대 Object 이동
			players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
			switch (recv_packet->direction) {
			case MV_FRONT:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_FRONT;
				break;
			case MV_BACK:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_BACK;
				break;
			case MV_SIDE:
				players_info[recv_id].m_ingame_state = PL_ST_MOVE_SIDE;
				break;
			}
			players_info[recv_id].m_new_state_update = true;
			// 상대 Object 회전
			players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		// 2. Rotate Npc
		else if (recv_packet->target == TARGET_NPC) {
			npcs_info[recv_id].m_id = recv_id;
			npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };

			npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
			npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
			npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		}
		else {
			cout << "[MOVE&ROTATE ERROR] Unknown Target!" << endl;
		}

		break;
	}// SC_MOVE_ROTATE_PLAYER case end
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* recv_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// 1. Remove Player
		if (recv_packet->target == TARGET_PLAYER) {
			if (recv_id == my_id) {
				// 자기자신 없애기
			}
			else if (0 <= recv_id && recv_id < MAX_USER) {
				// 상대 Player 없애기
				players_info[recv_id].m_id = -1;
				players_info[recv_id].m_pos = { 0.f ,0.f ,0.f };
				players_info[recv_id].m_state = OBJ_ST_LOGOUT;

				curr_connection_num--;
				if (curr_connection_num < 1) curr_connection_num = 1;
			}
		}
		// 2. Remove Npc
		else if (recv_packet->target == TARGET_NPC) {
			int npc_id = recv_id - MAX_USER;
			npcs_info[npc_id].m_id = -1;
			npcs_info[npc_id].m_pos = { 0.f ,0.f ,0.f };
			npcs_info[npc_id].m_state = OBJ_ST_LOGOUT;
		}
		else {
			cout << "[REMOVE ERROR] Unknown Target!" << endl;
		}

		break;
	}//SC_REMOVE_PLAYER case end
	case SC_DAMAGED:
	{
		SC_DAMAGED_PACKET* recv_packet = reinterpret_cast<SC_DAMAGED_PACKET*>(ptr);
		int recv_id = recv_packet->id;

		// Player Damaged
		if (recv_packet->target == TARGET_PLAYER) {
			gamesound.collisionSound();
			players_info[recv_id].m_damaged_effect_on = true;
			players_info[recv_id].m_hp -= recv_packet->damage;
			if (players_info[recv_id].m_hp < 0) players_info[recv_packet->id].m_hp = 0;

		}
		// NPC Damaged
		else if (recv_packet->target == TARGET_NPC) {
			gamesound.collisionSound();
			npcs_info[recv_id].m_damaged_effect_on = true;
			npcs_info[recv_id].m_hp -= recv_packet->damage;
			if (npcs_info[recv_id].m_hp < 0) npcs_info[recv_packet->id].m_hp = 0;
		}

		break;
	}//SC_DAMAGED case end
	case SC_CHANGE_SCENE:
	{
		SC_CHANGE_SCENE_PACKET* recv_packet = reinterpret_cast<SC_CHANGE_SCENE_PACKET*>(ptr);

		int recv_id = recv_packet->id;
		players_info[recv_id].curr_scene = recv_packet->scene_num;

		if (recv_id == my_id) {
			if (recv_packet->scene_num == 1) {
				stage1_enter_ok = true;
			}
			else if (recv_packet->scene_num == 2) {
				stage2_enter_ok = true;
			}
		}

		break;
	}//SC_CHANGE_SCENE case end
	case SC_OBJECT_STATE:
	{
		SC_OBJECT_STATE_PACKET* recv_packet = reinterpret_cast<SC_OBJECT_STATE_PACKET*>(ptr);

		int recv_id = recv_packet->id;
		short recvd_target = recv_packet->target;
		short recvd_state = recv_packet->state;

		if (recvd_target == TARGET_PLAYER) {
			players_info[recv_id].m_ingame_state = recvd_state;
			players_info[recv_id].m_new_state_update = true;
			switch (recvd_state) {
			case PL_ST_IDLE:
				break;
			case PL_ST_ATTACK:
				break;
			case PL_ST_DEAD:
				gamesound.collisionSound();

				players_info[recv_id].m_hp = 0;
				players_info[recv_id].m_damaged_effect_on = true;
				break;
			}
		}
		else if (recvd_target == TARGET_NPC) {
			npcs_info[recv_id].m_ingame_state = recvd_state;
			npcs_info[recv_id].m_new_state_update = true;
			switch (recvd_state) {
			case PL_ST_IDLE:
				break;
			case PL_ST_CHASE:
				break;
			case PL_ST_ATTACK:
				break;
			case PL_ST_DEAD:
				gamesound.collisionSound();

				npcs_info[recv_id].m_hp = 0;
				npcs_info[recv_id].m_damaged_effect_on = true;
				break;
			}
		}

		break;
	}//SC_OBJECT_STATE case end
	case SC_RESPAWN:
	{
		SC_RESPAWN_PACKET* recv_packet = reinterpret_cast<SC_RESPAWN_PACKET*>(ptr);

		short recv_id = recv_packet->id;
		players_info[recv_id].m_hp = recv_packet->hp;
		players_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		players_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		players_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
		players_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };
		players_info[recv_id].m_ingame_state = recv_packet->state;

		respawn_trigger = true;
		break;
	}//SC_RESPAWN case end
	case SC_BULLET_COUNT:
	{
		SC_BULLET_COUNT_PACKET* recv_packet = reinterpret_cast<SC_BULLET_COUNT_PACKET*>(ptr);
		players_info[my_id].m_bullet = recv_packet->bullet_cnt;
		if (recv_packet->bullet_cnt == MAX_BULLET) {
			gamesound.reloadSound();
			gamesound.shootingSound(true);
		}
		gamesound.shootingSound(false);
		break;
	}//SC_BULLET_COUNT case end
	case SC_MISSION:
	{
		SC_MISSION_PACKET* recv_packet = reinterpret_cast<SC_MISSION_PACKET*>(ptr);

		short stage_num = recv_packet->stage_num;
		stage_missions[stage_num].type = recv_packet->mission_type;
		stage_missions[stage_num].goal = recv_packet->mission_goal;
		stage_missions[stage_num].curr = recv_packet->mission_curr;

		break;
	}//SC_MISSION case end
	case SC_MISSION_COMPLETE:
	{
		SC_MISSION_COMPLETE_PACKET* recv_packet = reinterpret_cast<SC_MISSION_COMPLETE_PACKET*>(ptr);

		if (recv_packet->stage_num == 1) {
			if (curr_mission_num >= ST1_MISSION_NUM) {
				trigger_stage_clear = true;
				stage_missions[1].curr = 0;
			}
			else {
				trigger_mission_complete = true;
				curr_mission_num = recv_packet->mission_num + 1;
			}
		}
		else if (recv_packet->stage_num == 2) {
			//
		}

		break;
	}//SC_MISSION_COMPLETE case end
	case SC_TIME_TICKING:
	{
		SC_TIME_TICKING_PACKET* recv_packet = reinterpret_cast<SC_TIME_TICKING_PACKET*>(ptr);

		servertime_ms = recv_packet->servertime_ms;
		timelimit_ms = STAGE1_TIMELIMIT * 1000 - servertime_ms;
		timelimit_sec = timelimit_ms / 1000;
		break;
	}//SC_TIME_TICKING case end
	case SC_CHAT:
	{
		SC_CHAT_PACKET* recv_packet = reinterpret_cast<SC_CHAT_PACKET*>(ptr);

		chat_comeTome = true;
		strcpy_s(chat_logs.name, recv_packet->name);
		strcpy_s(chat_logs.msg, recv_packet->msg);

		break;
	}
	case SC_MAP_OBJINFO:
	{
		SC_MAP_OBJINFO_PACKET* recv_packet = reinterpret_cast<SC_MAP_OBJINFO_PACKET*>(ptr);

		MapObjectsInfo temp;
		temp.m_pos = { recv_packet->center_x, recv_packet->center_y, recv_packet->center_z };
		temp.m_scale = { recv_packet->scale_x, recv_packet->scale_y, recv_packet->scale_z };
		temp.m_local_forward = { recv_packet->forward_x, recv_packet->forward_y, recv_packet->forward_z };
		temp.m_local_right = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		temp.m_angle_aob = recv_packet->aob;
		temp.m_angle_boc = recv_packet->boc;
		temp.setBB();
		stage1_mapobj_info.push_back(temp);
		break;
	}//SC_MAP_OBJINFO case end
	case SC_PING_RETURN:
	{
		SC_PING_RETURN_PACKET* recv_packet = reinterpret_cast<SC_PING_RETURN_PACKET*>(ptr);

		if (recv_packet->ping_sender_id == my_id) {
			last_pong = chrono::system_clock::now();
			//cout << "pong\n" << endl;
		}
		break;
	}//SC_PING_RETURN case end
	case SC_ACTIVE_DOWN:
	{
		SC_ACTIVE_DOWN_PACKET* recv_packet = reinterpret_cast<SC_ACTIVE_DOWN_PACKET*>(ptr);

		if (recv_packet->prev_s_id == active_servernum) {
			active_servernum = recv_packet->my_s_id;
		}
		break;
	}//SC_BULLET_COUNT case end
	case NPC_MOVE:
	{
		NPC_MOVE_PACKET* recv_packet = reinterpret_cast<NPC_MOVE_PACKET*>(ptr);

		short recv_id = recv_packet->n_id;
		npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };

		break;
	}
	case NPC_ROTATE:
	{
		NPC_ROTATE_PACKET* recv_packet = reinterpret_cast<NPC_ROTATE_PACKET*>(ptr);

		short recv_id = recv_packet->n_id;
		npcs_info[recv_id].m_pos = { recv_packet->x, recv_packet->y, recv_packet->z };
		npcs_info[recv_id].m_right_vec = { recv_packet->right_x, recv_packet->right_y, recv_packet->right_z };
		npcs_info[recv_id].m_up_vec = { recv_packet->up_x, recv_packet->up_y, recv_packet->up_z };
		npcs_info[recv_id].m_look_vec = { recv_packet->look_x, recv_packet->look_y, recv_packet->look_z };

		break;
	}
	case NPC_ATTACK:
	{
		NPC_ATTACK_PACKET* recv_packet = reinterpret_cast<NPC_ATTACK_PACKET*>(ptr);

		short recv_id = recv_packet->n_id;

		npcs_info[recv_id].m_attack_dir.x = recv_packet->atklook_x;
		npcs_info[recv_id].m_attack_dir.y = recv_packet->atklook_y;
		npcs_info[recv_id].m_attack_dir.z = recv_packet->atklook_z;
		npcs_info[recv_id].m_attack_on = true;
		break;
	}
	}
}

void processData(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			processPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}