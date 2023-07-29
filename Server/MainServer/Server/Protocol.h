#pragma once
//======================================================================
// IP �ּ�
const char* IPADDR_LOOPBACK = "127.0.0.1";		// ������

//======================================================================
// ��Ʈ ��ȣ
// 1. Ŭ���̾�Ʈ-�������� ���
constexpr int MAX_LOGIN_SERVER = 2;
constexpr int PORTNUM_LOGIN_0 = 9900;
constexpr int PORTNUM_LOGIN_1 = 9901;

// 2. Ŭ���̾�Ʈ-�κ񼭹� ���
constexpr int MAX_LOBBY_SERVER = 2;
constexpr int PORTNUM_LOBBY_0 = 9910;
constexpr int PORTNUM_LOBBY_1 = 9911;

// 3. Ŭ���̾�Ʈ-�������� ���
constexpr int MAX_LOGIC_SERVER = 2;
constexpr int PORTNUM_LOGIC_0 = 9920;
constexpr int PORTNUM_LOGIC_1 = 9921;

// 4. ��������-NPC���� ��� (NPC �����и�)
constexpr int MAX_NPC_SERVER = 2;
constexpr int PORTNUM_LGCNPC_0 = 9930;
constexpr int PORTNUM_LGCNPC_1 = 9931;

// 5. �κ񼭹�-�κ񼭹� ��� (���� ����Ȯ��)
constexpr int HA_PORTNUM_LBY0 = 10010;
constexpr int HA_PORTNUM_LBY1 = 10011;

// 6. ��������-�������� ��� (���� ����Ȯ��)
constexpr int HA_PORTNUM_S0 = 10020;
constexpr int HA_PORTNUM_S1 = 10021;

// 7. NPC����-NPC���� ��� (���� ����Ȯ��)
constexpr int HA_PORTNUM_NPC0 = 10030;
constexpr int HA_PORTNUM_NPC1 = 10031;

//======================================================================
// HA ����
constexpr int SERIAL_NUM_CLIENT = 0;
constexpr int SERIAL_NUM_EXSERVER = 1000;

constexpr int HB_SEND_CYCLE = 1000;		// Heartbeat�� ������ �ֱ� (����: millisec)
constexpr int HB_GRACE_PERIOD = 3000;	// Heartbeat�� �� �� �Ѿ ���� ������ �����ٿ����� ������ (����: millisec)

constexpr int HA_REPLICA_CYCLE = 333;	// ������ �����ͺ��� �ֱ� (����: millisec)

//======================================================================
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 12;
constexpr int CHAT_SIZE = 60;
constexpr int ROOM_NAME_SIZE = 20;

constexpr int MAX_ROOM = 100;
constexpr int MAX_USER = 3;
constexpr int MAX_NPCS = 25;
constexpr int MAX_NPC_HELI = 5;
constexpr int MAX_NPC_HUMAN = 20;
constexpr int MAX_BULLET = 20;
constexpr int MAX_BULLET_HELI = 30;

constexpr int PLAYER_ID_START = 1000;		// Player ID ���۰�. (ID_VALUE_PLAYER + client_id)
constexpr int PLAYER_ID_END   = 1999;		// Player ID ����
constexpr int NPC_ID_START    = 2000;		// Npc ID ���۰�. (ID_VALUE_NPC + npc_id)
constexpr int NPC_ID_END      = 2999;		// Npc ID ����

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

constexpr int TOTAL_STAGE = 1;
constexpr int ST1_MISSION_NUM = 2;		// Stage1 �� �̼� ��

//======================================================================
// key value
constexpr char INPUT_KEY_W = 0b100000;
constexpr char INPUT_KEY_S = 0b010000;
constexpr char INPUT_KEY_D = 0b001000;
constexpr char INPUT_KEY_A = 0b000100;
constexpr char INPUT_KEY_Q = 0b000010;
constexpr char INPUT_KEY_E = 0b000001;

//======================================================================
// Packet ID
enum PacketID {
	CLGN_LOGIN_REQUEST
	, LGNC_LOGIN_RESULT
	, CLBY_CONNECT, CLBY_REQUEST_LOBBYINFO, CLBY_CREATE_ROOM, CLBY_QUICK_MATCH, CLBY_ROOM_ENTER, CLBY_LEAVE_ROOM, CLBY_ROLE_CHANGE, CLBY_GAME_READY, CLBY_GAME_START, CLBY_GAME_EXIT
	, LBYC_MATCH_FAIL, LBYC_ROOM_JOIN, LBYC_ADD_ROOM, LBYC_ROOM_USERCOUNT, LBYC_ROOM_NEW_MEMBER, LBYC_ROOM_LEFT_MEMBER
	, LBYC_LOBBY_CLEAR, LBYC_MEMBER_STATE, LBYC_ROLE_CHANGE, LBYC_GAME_START, LBYC_GAME_EXIT, LBYC_POPUP
	, CS_LOGIN, CS_MOVE, CS_ROTATE, CS_ATTACK, CS_INPUT_KEYBOARD, CS_INPUT_MOUSE, CS_CHAT, CS_PARTICLE_COLLIDE, CS_HELI_MAP_COLLIDE, CS_PING, CS_RELOGIN
	, SC_LOGIN_INFO, SC_ADD_OBJECT, SC_REMOVE_OBJECT, SC_MOVE_OBJECT, SC_ROTATE_OBJECT, SC_MOVE_ROTATE_OBJECT, SC_HEIGHT_ALERT
	, SC_DAMAGED, SC_HEALING, SC_HEALPACK, SC_ATTACK, SC_RELOAD, SC_CHANGE_SCENE, SC_OBJECT_STATE, SC_TIMEOUT, SC_BULLET_COLLIDE_POS, SC_MISSION, SC_MISSION_COMPLETE
	, SC_TIME_TICKING, SC_CHAT, SC_MAP_OBJINFO, SC_PING_RETURN, SC_ACTIVE_DOWN, SC_RESET_GAME
	, SS_CONNECT, SS_HEARTBEAT, SS_DATA_REPLICA
	, NPC_FULL_INFO, NPC_MOVE, NPC_ROTATE, NPC_CHECK_POS, NPC_REMOVE, NPC_ATTACK, NPC_CHANGE_STATE
};

//======================================================================
enum PLAYER_STATE { PL_ST_IDLE, PL_ST_MOVE_FRONT, PL_ST_MOVE_BACK, PL_ST_MOVE_SIDE, PL_ST_FLY, PL_ST_CHASE, PL_ST_ATTACK, PL_ST_DEAD, PL_ST_GAMEOVER };

//======================================================================
enum ROOM_STATE { R_ST_WAIT, R_ST_FULL, R_ST_INGAME };

//======================================================================
enum TargetType { TARGET_PLAYER, TARGET_BULLET, TARGET_NPC };

//======================================================================
enum EnumFor1ByteBoolean { b_FALSE, b_TRUE };

//======================================================================
// Packets ( CS: Client->Server, SC: Server->Client, SS: Server->Other Server )
#pragma pack (push, 1)
// ================================
//     Ŭ���̾�Ʈ - �α��μ���
// ================================
// 1. Ŭ�� -> �α��μ���
struct CLGN_LOGIN_REQUEST_PACKET {
	unsigned char size;
	char type;
	char login_id[20];
	char login_pw[20];
};

// 2. �α��μ��� -> Ŭ��
struct LGNC_LOGIN_RESULT_PACKET {
	unsigned char size;
	char type;
	char login_accept;	// 0: Deny, 1: Accept
};


//================================
//      Ŭ���̾�Ʈ - �κ񼭹�
//================================
// 1. Ŭ�� -> �κ񼭹�
struct CLBY_CONNECT_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

struct CLBY_REQUEST_LOBBYINFO_PACKET {
	unsigned char size;
	char type;
};

struct CLBY_CREATE_ROOM_PACKET {
	unsigned char size;
	char type;
	char room_name[ROOM_NAME_SIZE];
};

struct CLBY_QUICK_MATCH_PACKET {
	unsigned char size;
	char type;
};

struct CLBY_ROOM_ENTER_PACKET {
	unsigned char size;
	char type;
	char room_id;
};

struct CLBY_LEAVE_ROOM_PACKET {
	unsigned char size;
	char type;
};

struct CLBY_ROLE_CHANGE_PACKET {
	unsigned char size;
	char type;
	char role;
};

struct CLBY_GAME_READY_PACKET {
	unsigned char size;
	char type;
};

struct CLBY_GAME_START_PACKET {
	unsigned char size;
	char type;
};

struct CLBY_GAME_EXIT_PACKET {
	unsigned char size;
	char type;
};

// 2. �κ񼭹� -> Ŭ��
enum MATCH_FAIL_REASON { MATCH_FAIL_UNKNOWN, MATCH_FAIL_NOEMPTYROOM };
struct LBYC_MATCH_FAIL_PACKET {
	unsigned char size;
	char type;
	char fail_reason;	// 0: Unknown, 1: No Empty Room
};

enum ROOM_MEMBER_STATE { RM_ST_EMPTY, RM_ST_NONREADY, RM_ST_READY, RM_ST_MANAGER }; // ��ĭ, �غ�X, �غ�O, ����
struct LBYC_ROOM_JOIN_PACKET {
	unsigned char size;
	char type;
	short room_id;
	char room_name[ROOM_NAME_SIZE];
	short member_count;
	char member_name[MAX_USER][NAME_SIZE];
	char member_state[MAX_USER];
	char member_role[MAX_USER];
	short your_roomindex;
	char b_manager;	// �ʰ� �������� (0: �ƴ�, 1: ����)
};

struct LBYC_ADD_ROOM_PACKET {
	unsigned char size;
	char type;
	short room_id;
	char room_name[ROOM_NAME_SIZE];
	char room_state;
	short user_count;
};

struct LBYC_ROOM_USERCOUNT_PACKET {
	unsigned char size;
	char type;
	short room_id;
	short user_count;
};

struct LBYC_ROOM_NEW_MEMBER_PACKET {
	unsigned char size;
	char type;
	short new_member_roomindex;
	char new_member_name[NAME_SIZE];
};

struct LBYC_ROOM_LEFT_MEMBER_PACKET {
	unsigned char size;
	char type;
	short left_member_roomindex;
	char left_member_name[NAME_SIZE];
};

struct LBYC_LOBBY_CLEAR_PACKET {
	unsigned char size;
	char type;
};

struct LBYC_ROOM_INFO_PACKET {
	unsigned char size;
	char type;
	short room_id;
	char room_name[ROOM_NAME_SIZE];
	char room_state;
	short user_count;
};

struct LBYC_MEMBER_STATE_PACKET {
	unsigned char size;
	char type;
	short member_id;
	char member_state;
};

enum PLAYER_ROLE { ROLE_NOTCHOOSE, ROLE_RIFLE, ROLE_HELI };
struct LBYC_ROLE_CHANGE_PACKET {
	unsigned char size;
	char type;
	short member_id;
	char role;
};

struct LBYC_GAME_START_PACKET {
	unsigned char size;
	char type;
};

struct LBYC_GAME_EXIT_PACKET {
	unsigned char size;
	char type;
};

enum LOBBY_POPUP_MSG { POPUPMSG_PLZCHOOSEROLE, POPUPMSG_ANYONENOTREADY };
struct LBYC_POPUP_PACKET {
	unsigned char size;
	char type;
	char msg;
};


// ================================
//     Ŭ���̾�Ʈ - ��������
// ================================
// 1. Ŭ�� -> ��������
// 1) �α��� ���� ��Ŷ
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
	char role;
	short room_id;
	short inroom_index;
};

// 2) ���� ���� ��Ŷ
enum MoveDirection { MV_FRONT, MV_BACK, MV_SIDE };
struct CS_MOVE_PACKET {
	unsigned char size;
	char type;
	float x, y, z;
	short direction;
};

struct CS_ROTATE_PACKET {
	unsigned char size;
	char type;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
	float cam_look_x, cam_look_y, cam_look_z;
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char type;
};

enum PACKET_KEY_TYPE {
	PACKET_KEY_W, PACKET_KEY_A, PACKET_KEY_S, PACKET_KEY_D, PACKET_KEY_R,
	PACKET_KEY_Q, PACKET_KEY_E,
	PACKET_KEY_SPACEBAR,
	PACKET_KEYUP_MOVEKEY,
	PACKET_KEY_INSERT, PACKET_KEY_DELETE, PACKET_KEY_HOME, PACKET_KEY_END, PACKET_KEY_PGUP
};
struct CS_INPUT_KEYBOARD_PACKET {
	unsigned char size;
	char type;
	short keytype;
};

enum PACKET_MOUSE_BUTTON { PACKET_NONCLICK, PACKET_BUTTON_L, PACKET_BUTTON_R };
struct CS_INPUT_MOUSE_PACKET {
	unsigned char size;
	char type;
	char buttontype;
	float delta_x, delta_y;
	//float roll, pitch, yaw;
};

struct CS_CHAT_PACKET {
	unsigned char size;
	char type;
	char msg[CHAT_SIZE];
};

struct CS_PARTICLE_COLLIDE_PACKET {
	unsigned char size;
	char type;
	float particle_mass;
};

struct CS_HELI_MAP_COLLIDE_PACKET {
	unsigned char size;
	char type;
};

// 3) ����ȭ ���� ��Ŷ
struct CS_PING_PACKET {
	unsigned char size;
	char type;
};

struct CS_RELOGIN_PACKET {
	unsigned char size;
	char type;
	short id;
};

// 2. �������� -> Ŭ��
// 1) ��ü ���� �� ���� ���� ��Ŷ
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char type;
	char name[20];
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
	int hp;
	int life;
	int remain_bullet;
};

struct SC_ADD_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	short obj_state;
	char name[NAME_SIZE];
	char role;
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
};

// 2) �⺻ ���� ���� ��Ŷ
struct SC_MOVE_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	float x, y, z;
	short direction;
};

struct SC_ROTATE_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct SC_MOVE_ROTATE_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
	short direction;
};

struct SC_HEIGHT_ALERT_PACKET {
	unsigned char size;
	char type;
	char alert_on;	// 0: false, 1: true
};

// 3) ������ ���� ��Ŷ
enum SOUND_VOLUME { VOL_MUTE, VOL_LOW, VOL_MID, VOL_HIGH };
struct SC_DAMAGED_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	int damage;
	char sound_volume;
};

struct SC_HEALING_PACKET {
	unsigned char size;
	char type;
	short id;
	int value;
};

struct SC_HEALPACK_PACKET {
	unsigned char size;
	char type;
	short healpack_id;
	char isused;	// 0: false, 1: true
};

struct SC_ATTACK_PACKET {
	unsigned char size;
	char type;
	short obj_type;
	short id;
	float atklook_x, atklook_y, atklook_z;
	char sound_volume;
};

struct SC_RELOAD_PACKET {
	unsigned char size;
	char type;
	short bullet_cnt;
	short id;
	char sound_volume;
};

struct SC_CHANGE_SCENE_PACKET {
	unsigned char size;
	char type;
	short id;
	short scene_num;
};

struct SC_OBJECT_STATE_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	short state;
};

struct SC_TIMEOUT_PACKET {
	unsigned char size;
	char type;
	short id;
};

enum C_OBJ_TYPE { C_OBJ_NONCOLLIDE, C_OBJ_MAPOBJ, C_OBJ_GROUND, C_OBJ_NPC, C_OBJ_PLAYER };
struct SC_BULLET_COLLIDE_POS_PACKET {
	unsigned char size;
	char type;
	short attacker;			// TargetType(TARGET_PLAYER, TARGET_BULLET, TARGET_NPC) ���
	short collide_target;	// C_OBJ_TYPE ���
	float x, y, z;
};

enum MISSION_TYPE { MISSION_KILL, MISSION_OCCUPY };
struct SC_MISSION_PACKET {
	unsigned char size;
	char type;
	short stage_num;
	short mission_num;
	short mission_type;
	float mission_goal;
	float mission_curr;
};
struct SC_MISSION_COMPLETE_PACKET {
	unsigned char size;
	char type;
	short stage_num;
	short mission_num;
};

// 4) UI ���� ��Ŷ
#define STAGE1_TIMELIMIT 316 // ��������1 ���ѽð� (����: sec)
struct SC_TIME_TICKING_PACKET {
	unsigned char size;
	char type;
	int servertime_ms;	// ����: ms
};

struct SC_CHAT_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
	char msg[CHAT_SIZE];
};

// 5) �� ���� ���� ��Ŷ
struct SC_MAP_OBJINFO_PACKET {
	unsigned char size;
	char type;
	float center_x, center_y, center_z;
	float scale_x, scale_y, scale_z;
	float forward_x, forward_y, forward_z;
	float right_x, right_y, right_z;
	float rotate_x, rotate_y, rotate_z;
	float aob;
	float boc;
};

// 6) ����ȭ ���� ��Ŷ
struct SC_PING_RETURN_PACKET {
	unsigned char size;
	char type;
	short ping_sender_id;
};
struct SC_ACTIVE_DOWN_PACKET {
	unsigned char size;
	char type;
	short prev_s_id;
	short my_s_id;
};

// 7) ���� �ʱ�ȭ
struct SC_RESET_GAME_PACKET {
	unsigned char size;
	char type;
};


// ================================
//           ���� - ����
// ================================
struct SS_CONNECT_PACKET {
	unsigned char size;
	char type;
	short server_id;
	int port_num;
};

struct SS_HEARTBEAT_PACKET {
	unsigned char size;
	char type;
	short sender_id;
};

struct SS_DATA_REPLICA_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	char name[NAME_SIZE];
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
	short state;
	short hp;
	short bullet_cnt;
	short curr_stage;
};


// ================================
//          NPC ���� ��Ŷ
// ================================
struct NPC_FULL_INFO_PACKET {
	unsigned char size;
	char type;
	short n_id;
	char ishuman;	// 0: ���, 1: ���
	char name[20];
	int hp;
	float speed;
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct NPC_MOVE_PACKET {
	unsigned char size;
	char type;
	short n_id;
	float x, y, z;
};

struct NPC_ROTATE_PACKET {
	unsigned char size;
	char type;
	short n_id;
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct NPC_CHECK_POS_PACKET {
	unsigned char size;
	char type;
	short n_id;
	float x, y, z;
};

struct NPC_REMOVE_PACKET {
	unsigned char size;
	char type;
	short n_id;
};

struct NPC_ATTACK_PACKET {
	unsigned char size;
	char type;
	short n_id;
	float atklook_x, atklook_y, atklook_z;
};

struct NPC_CHANGE_STATE_PACKET {
	unsigned char size;
	char type;
	short n_id;
	short state;
};
#pragma pack (pop)