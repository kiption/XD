constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 10;

constexpr int WORLD_X_POS = 8;
constexpr int WORLD_Y_POS = 8;
constexpr int WORLD_Z_POS = 8;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;

// Packets ( CS: Client->Server, SC: Server->Client )
#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char type;
	char name[NAME_SIZE];
	short x, y, z;
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char type;
	short x, y, z;
	// 현재는 임시로 물리 계산을 클라이언트에서 하고 있어서 클라에서 계산된 좌표값을 받아오고 그 값을 접속해있는 모든 클라이언트들에게 전달하는 방식으로 되어있습니다.
	// 추후에 서버에서 클라로부터 direction을 받아 물리 계산을 하도록 변경할 예정입니다.
	//char direction;  // 0: North, 1: East, 2: South, 3: West
	//bool bUpdateVelocity;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char type;
	short id;
	short x, y, z;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char type;
	short id;
	short x, y, z;
	char name[NAME_SIZE];
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
	short x, y, z;
};

#pragma pack (pop)