constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 10;
constexpr int MAX_NPCS = 10;

constexpr int WORLD_X_POS = 2000;
constexpr int WORLD_Y_POS = 2000;
constexpr int WORLD_Z_POS = 2000;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ROTATE = 2;
constexpr char SC_LOGIN_INFO = 3;
constexpr char SC_ADD_PLAYER = 4;
constexpr char SC_REMOVE_PLAYER = 5;
constexpr char SC_MOVE_PLAYER = 6;
constexpr char SC_ROTATE_PLAYER = 7;

// Packets ( CS: Client->Server, SC: Server->Client )
#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

enum move_dir { MV_FORWARD, MV_BACK, MV_LEFT, MV_RIGHT, MV_DOWN, MV_UP };
struct CS_MOVE_PACKET {
	unsigned char size;
	char type;
	char direction;
};

struct CS_ROTATE_PACKET {
	unsigned char size;
	char type;
	float roll, pitch, yaw;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char type;
	short id;
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
	char name[NAME_SIZE];
	float x, y, z;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
	float x, y, z;
};

struct SC_ROTATE_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
	float right_x, right_y, right_z;
	float up_x, up_y, up_z;
	float look_x, look_y, look_z;
};

#pragma pack (pop)