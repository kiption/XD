#pragma once
// IP 주소
const char* IPADDR_LOOPBACK = "127.0.0.1";		// 루프백
const char* IPADDR_LOGIC0 = "127.0.0.1"; //"112.152.39.13";	// 원격 접속
const char* IPADDR_LOGIC1 = "127.0.0.1";	// 원격 접속

// 클라이언트-인증서버 통신
constexpr int MAX_LOGIN_SERVER = 2;
constexpr int PORTNUM_LOGIN_0 = 9900;
constexpr int PORTNUM_LOGIN_1 = 9901;

// 클라이언트-로비서버 통신
constexpr int MAX_LOBBY_SERVER = 2;
constexpr int PORTNUM_LOBBY_0 = 9910;
constexpr int PORTNUM_LOBBY_1 = 9911;

// 클라이언트-로직서버 통신
constexpr int MAX_LOGIC_SERVER = 2;
constexpr int PORTNUM_LOGIC_0 = 9920;
constexpr int PORTNUM_LOGIC_1 = 9921;

// [수평확장] 로직서버-로직서버 통신
constexpr int HA_PORTNUM_S0 = 10020;
constexpr int HA_PORTNUM_S1 = 10021;

constexpr int SERIAL_NUM_CLIENT = 0;
constexpr int SERIAL_NUM_EXSERVER = 1000;

constexpr int HB_SEND_CYCLE = 1000;		// Heartbeat를 보내는 주기 (단위: millisec)
constexpr int HB_GRACE_PERIOD = 3000;	// Heartbeat가 몇 초 넘어도 오지 않으면 서버다운으로 간주함 (단위: millisec)

constexpr int HA_REPLICA_CYCLE = 333;	// 서버간 데이터복제 주기 (단위: millisec)
//
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 3;
constexpr int MAX_NPCS = 5;
constexpr int MAX_BULLET = 10;

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

// key value
constexpr char INPUT_KEY_W = 0b100000;
constexpr char INPUT_KEY_S = 0b010000;
constexpr char INPUT_KEY_D = 0b001000;
constexpr char INPUT_KEY_A = 0b000100;
constexpr char INPUT_KEY_Q = 0b000010;
constexpr char INPUT_KEY_E = 0b000001;

// Packet ID
enum PacketID {
	CS_LOGIN, CS_MOVE, CS_ROTATE, CS_ATTACK, CS_INPUT_KEYBOARD, CS_INPUT_MOUSE, CS_PING, CS_RELOGIN
	, SC_LOGIN_INFO, SC_ADD_OBJECT, SC_REMOVE_OBJECT, SC_MOVE_OBJECT, SC_ROTATE_OBJECT, SC_MOVE_ROTATE_OBJECT
	, SC_DAMAGED, SC_CHANGE_SCENE, SC_OBJECT_STATE, SC_BULLET_COUNT, SC_TIME_TICKING, SC_MAP_OBJINFO, SC_PING_RETURN, SC_ACTIVE_DOWN
	, SS_CONNECT, SS_HEARTBEAT, SS_DATA_REPLICA
};

enum PLAYER_STATE { PL_ST_IDLE, PL_ST_MOVE, PL_ST_FLY, PL_ST_CHASE, PL_ST_ATTACK, PL_ST_DEAD };

// Target Type
enum TargetType { TARGET_PLAYER, TARGET_BULLET, TARGET_NPC };

// Packets ( CS: Client->Server, SC: Server->Client, SS: Server->Other Server )
#pragma pack (push, 1)
// ================================
//			1. CS Packet
// ================================
// 1) 로그인 관련 패킷
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

// 2) 조작 관련 패킷
struct CS_MOVE_PACKET {
	unsigned char size;
	char type;
	float x, y, z;
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
	PACKET_KEY_W, PACKET_KEY_A, PACKET_KEY_S, PACKET_KEY_D,
	PACKET_KEY_UP, PACKET_KEY_LEFT, PACKET_KEY_DOWN, PACKET_KEY_RIGHT,
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

// ================================
//			2. SC Packet
// ================================
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

// ================================
// 2) 기본 조작 관련 패킷
struct SC_MOVE_OBJECT_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	float x, y, z;
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
};

// ================================
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

struct SC_BULLET_COUNT_PACKET {
	unsigned char size;
	char type;
	int bullet_cnt;
};

// ================================
// 4) UI 관련 패킷
struct SC_TIME_TICKING_PACKET {
	unsigned char size;
	char type;
	int servertime_ms;	// 단위: ms
};

// ================================
// 4) 맵 정보 관련 패킷
struct SC_MAP_OBJINFO_PACKET {
	unsigned char size;
	char type;
	float pos_x, pos_y, pos_z;
	float scale_x, scale_y, scale_z;
};

// ================================
// 5) 이중화 관련 패킷
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
//			3. SS Packet
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
#pragma pack (pop)