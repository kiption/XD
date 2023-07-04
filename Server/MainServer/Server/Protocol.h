#pragma once
//======================================================================
// IP 주소
const char* IPADDR_LOOPBACK = "127.0.0.1";		// 루프백
const char* IPADDR_LOGIC0 = "127.0.0.1"; //"112.152.39.13";		// 원격 접속
const char* IPADDR_LOGIC1 = "127.0.0.1"; //"218.101.227.89";	// 원격 접속

//======================================================================
// 포트 번호
// 1. 클라이언트-인증서버 통신
constexpr int MAX_LOGIN_SERVER = 2;
constexpr int PORTNUM_LOGIN_0 = 9900;
constexpr int PORTNUM_LOGIN_1 = 9901;

// 2. 클라이언트-로비서버 통신
constexpr int MAX_LOBBY_SERVER = 2;
constexpr int PORTNUM_LOBBY_0 = 9910;
constexpr int PORTNUM_LOBBY_1 = 9911;

// 3. 클라이언트-로직서버 통신
constexpr int MAX_LOGIC_SERVER = 2;
constexpr int PORTNUM_LOGIC_0 = 9920;
constexpr int PORTNUM_LOGIC_1 = 9921;

// 4. 로직서버-NPC서버 통신 (NPC 서버분리)
constexpr int MAX_NPC_SERVER = 2;
constexpr int PORTNUM_LGCNPC_0 = 9930;
constexpr int PORTNUM_LGCNPC_1 = 9931;

// 5. 로직서버-로직서버 통신 (서버 수평확장)
constexpr int HA_PORTNUM_S0 = 10020;
constexpr int HA_PORTNUM_S1 = 10021;

//======================================================================
// HA 관련
constexpr int SERIAL_NUM_CLIENT = 0;
constexpr int SERIAL_NUM_EXSERVER = 1000;

constexpr int HB_SEND_CYCLE = 1000;		// Heartbeat를 보내는 주기 (단위: millisec)
constexpr int HB_GRACE_PERIOD = 3000;	// Heartbeat가 몇 초 넘어도 오지 않으면 서버다운으로 간주함 (단위: millisec)

constexpr int HA_REPLICA_CYCLE = 333;	// 서버간 데이터복제 주기 (단위: millisec)

//======================================================================
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 60;

constexpr int MAX_USER = 3;
constexpr int MAX_NPCS = 25;
constexpr int MAX_BULLET = 10;

constexpr int PLAYER_ID_START = 1000;		// Player ID 시작값. (ID_VALUE_PLAYER + client_id)
constexpr int PLAYER_ID_END   = 1999;		// Player ID 끝값
constexpr int NPC_ID_START    = 2000;		// Npc ID 시작값. (ID_VALUE_NPC + npc_id)
constexpr int NPC_ID_END      = 2999;		// Npc ID 끝값

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

constexpr int TOTAL_STAGE = 1;
constexpr int ST1_MISSION_NUM = 2;		// Stage1 총 미션 수

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
	CLGN_LOGIN_REQUEST, LGNC_LOGIN_RESULT, LGNC_USER_MATCH_PACKET
	, CLBY_MATCH_REQUEST, LBYC_MATCH_RESULT
	, CS_LOGIN, CS_MOVE, CS_ROTATE, CS_ATTACK, CS_INPUT_KEYBOARD, CS_INPUT_MOUSE, CS_CHAT, CS_PING, CS_RELOGIN
	, SC_LOGIN_INFO, SC_ADD_OBJECT, SC_REMOVE_OBJECT, SC_MOVE_OBJECT, SC_ROTATE_OBJECT, SC_MOVE_ROTATE_OBJECT
	, SC_DAMAGED, SC_CHANGE_SCENE, SC_OBJECT_STATE, SC_RESPAWN, SC_BULLET_COUNT, SC_MISSION, SC_MISSION_COMPLETE
	, SC_TIME_TICKING, SC_CHAT, SC_MAP_OBJINFO, SC_PING_RETURN, SC_ACTIVE_DOWN
	, SS_CONNECT, SS_HEARTBEAT, SS_DATA_REPLICA
	, NPC_FULL_INFO, NPC_MOVE, NPC_ROTATE, NPC_CHECK_POS, NPC_REMOVE, NPC_ATTACK, NPC_CHANGE_STATE
};

//======================================================================
enum PLAYER_STATE { PL_ST_IDLE, PL_ST_MOVE_FRONT, PL_ST_MOVE_BACK, PL_ST_MOVE_SIDE, PL_ST_FLY, PL_ST_CHASE, PL_ST_ATTACK, PL_ST_DEAD };

//======================================================================
enum TargetType { TARGET_PLAYER, TARGET_BULLET, TARGET_NPC, TARGET_DUMMY };	//[TEST] DUMMY는 임시로 넣었다. NPC 구현 다 되면 빼자.

//======================================================================
// Packets ( CS: Client->Server, SC: Server->Client, SS: Server->Other Server )
#pragma pack (push, 1)
// ================================
//     클라이언트 - 로그인서버
// ================================
// 1. 클라 -> 로그인서버
struct CLGN_LOGIN_REQUEST_PACKET {
	unsigned char size;
	char type;
	char login_id[20];
	char login_pw[20];
};

// 2. 로그인서버 -> 클라
struct LGNC_LOGIN_RESULT_PACKET {
	unsigned char size;
	char type;
	char login_accept;	// 0: Deny, 1: Accept
};


//================================
//      클라이언트 - 로비서버
//================================
// 1. 클라 -> 로비서버
struct CLBY_MATCH_REQUEST_PACKET {
	unsigned char size;
	char type;
};

// 2. 로비서버 -> 클라
struct LBYC_MATCH_RESULT_PACKET {
	unsigned char size;
	char type;
	char result;	// 0: fail, 1: success
};


// ================================
//     클라이언트 - 로직서버
// ================================
// 1. 클라 -> 로직서버
// 1) 로그인 관련 패킷
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

// 2) 조작 관련 패킷
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
};

struct CS_ATTACK_PACKET {
	unsigned char size;
	char type;
};

enum PACKET_KEY_TYPE {
	PACKET_KEY_NUM1, PACKET_KEY_NUM2,
	PACKET_KEY_W, PACKET_KEY_A, PACKET_KEY_S, PACKET_KEY_D, PACKET_KEY_R,
	PACKET_KEY_SPACEBAR,
	PACKET_KEYUP_MOVEKEY
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

// 3) 이중화 관련 패킷
struct CS_PING_PACKET {
	unsigned char size;
	char type;
};

struct CS_RELOGIN_PACKET {
	unsigned char size;
	char type;
	short id;
};

// 2. 로직서버 -> 클라
// 1) 객체 생성 및 제거 관련 패킷
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char type;
	short id;
	char name[20];
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
	int hp;
	int remain_bullet;
};

struct SC_ADD_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	short obj_state;
	char name[NAME_SIZE];
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

// 2) 기본 조작 관련 패킷
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

// 3) 컨텐츠 관련 패킷
struct SC_DAMAGED_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	int damage;
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

struct SC_RESPAWN_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	int hp;
	short state;
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct SC_BULLET_COUNT_PACKET {
	unsigned char size;
	char type;
	int bullet_cnt;
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

// 4) UI 관련 패킷
#define STAGE1_TIMELIMIT 600 // 스테이지1 제한시간 (단위: sec)
struct SC_TIME_TICKING_PACKET {
	unsigned char size;
	char type;
	int servertime_ms;	// 단위: ms
};

struct SC_CHAT_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
	char msg[CHAT_SIZE];
};

// 5) 맵 정보 관련 패킷
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

// 6) 이중화 관련 패킷
struct SC_PING_RETURN_PACKET {	// 현재는 클라-서버 -> 추후에 클라-릴레이서버 로 바꿀 예정.
	unsigned char size;
	char type;
	short ping_sender_id;
};
struct SC_ACTIVE_DOWN_PACKET {	// 현재는 클라-서버 -> 추후에 클라-릴레이서버 로 바꿀 예정.
	unsigned char size;
	char type;
	short prev_s_id;
	short my_s_id;
};


// ================================
//           서버 - 서버
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
//          NPC 관련 패킷
// ================================
struct NPC_FULL_INFO_PACKET {
	unsigned char size;
	char type;
	short n_id;
	char ishuman;	// 0: 헬기, 1: 사람
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