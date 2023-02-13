constexpr int PORT_NUM = 9000;
constexpr int PORT_NUM2 = 9001;

constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 3;
constexpr int MAX_NPCS = 10;
constexpr int MAX_BULLET = 30;

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
	, SC_HP_COUNT, SC_PLAYER_STATE, SC_BULLET_COUNT };

// Target Type
enum TargetType { TARGET_PLAYER, TARGET_BULLET, TARGET_NPC };

// Packets ( CS: Client->Server, SC: Server->Client )
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

enum hp_change_cause { CAUSE_DAMAGED_BY_BULLET, CAUSE_DAMAGED_BY_PLAYER, CAUSE_HEAL };
struct SC_HP_COUNT_PACKET {
	unsigned char size;
	char type;
	short id;
	int hp;
	int change_cause;
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
#pragma pack (pop)