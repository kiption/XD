#pragma once
#include <array>
#include "../Server/Server/protocol.h"

// 서버에서 받은 객체들의 정보를 저장하는 구조체입니다.
struct ObjectsInfo
{
	short m_id;
	short m_x, m_y, m_z;

	ObjectsInfo() {
		m_id = 0;
		m_x = 0, m_y = 0, m_z = 0;
	}
	ObjectsInfo(short id, short x, short y, short z) {
		m_id = id;
		m_x = x, m_y = y, m_z = z;
	}
};

ObjectsInfo my_info;
std::array<ObjectsInfo, MAX_USER> other_players;