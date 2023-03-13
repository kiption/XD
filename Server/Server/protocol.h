constexpr int PORTNUM_RELAY2CLIENT_0 = 10000;	// 릴레이서버-클라이언트 통신 전용 포트
constexpr int PORTNUM_RELAY2CLIENT_1 = 10001;
constexpr int PORTNUM_RELAY2LOGIN_0 = 10010;	// 릴레이서버-인증서버 통신 전용 포트
constexpr int PORTNUM_RELAY2LOGIN_1 = 10011;
constexpr int PORTNUM_RELAY2LOBBY_0 = 10020;	// 릴레이서버-로비서버 통신 전용 포트
constexpr int PORTNUM_RELAY2LOBBY_1 = 10021;
constexpr int PORTNUM_RELAY2LOGIC_0 = 10030;	// 릴레이서버-로직서버 통신 전용 포트
constexpr int PORTNUM_RELAY2LOGIC_1 = 10031;

constexpr int MAX_SERVER = 2;
constexpr int PORT_NUM_S0 = 11000;		// 서버
constexpr int PORT_NUM_S1 = 11001;
constexpr int HA_PORTNUM_S0 = 11100;		// 서버(HA)
constexpr int HA_PORTNUM_S1 = 11101;

constexpr int SERIAL_NUM_CLIENT = 0;
constexpr int SERIAL_NUM_EXSERVER = 1000;

constexpr int HB_SEND_CYCLE = 1000;		// Heartbeat를 보내는 주기 (단위: millisec)
constexpr int HB_GRACE_PERIOD = 3000;	// Heartbeat가 몇 초 넘어도 오지 않으면 서버다운으로 간주함 (단위: millisec)

constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 5;
constexpr int MAX_NPCS = 5;
constexpr int MAX_BULLET = 100;

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
enum PacketID { CS_LOGIN, CS_INPUT_KEYBOARD, CS_INPUT_MOUSE
	, SC_LOGIN_INFO, SC_ADD_OBJECT, SC_REMOVE_OBJECT, SC_MOVE_OBJECT, SC_ROTATE_OBJECT
	, SC_DAMAGED, SC_PLAYER_STATE, SC_BULLET_COUNT
	, SS_CONNECT, SS_HEARTBEAT };

// Target Type
enum TargetType { TARGET_PLAYER, TARGET_BULLET, TARGET_NPC };

// Packets ( CS: Client->Server, SC: Server->Client, SS: Server->Other Server )
#pragma pack (push, 1)
// ================================
//			1. CS Packet
// ================================
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

struct CS_INPUT_KEYBOARD_PACKET {
	unsigned char size;
	char type;
	char direction;
};

enum rotate_type { RT_LBUTTON, RT_RBUTTON, RT_BOTH };
struct CS_INPUT_MOUSE_PACKET {
	unsigned char size;
	char type;
	char key_val;
	float delta_x, delta_y;
	//float roll, pitch, yaw;
};

// ================================
//			2. SC Packet
// ================================
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char type;
	short id;
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

struct SC_DAMAGED_PACKET {
	unsigned char size;
	char type;
	short target;
	short id;
	int dec_hp;
	float col_pos_x, col_pos_y, col_pos_z;
};

enum player_state { ST_PACK_REVIVAL, ST_PACK_DEAD };
struct SC_PLAYER_STATE_PACKET {
	unsigned char size;
	char type;
	short id;
	char state;
};

struct SC_BULLET_COUNT_PACKET {
	unsigned char size;
	char type;
	short id;
	int bullet_cnt;
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
#pragma pack (pop)