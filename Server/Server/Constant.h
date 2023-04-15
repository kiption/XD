#pragma once

#define PI 3.141592654f

#define FLY_MIN_HEIGHT 100.f		// 비행가능한 최저높이
#define FLY_MAX_HEIGHT 2000.f		// 비행가능한 최고높이

#define MOVE_SCALAR_FB 4.0f			// 이동 힘 (앞뒤)
#define MOVE_SCALAR_LR 1.4f			// 이동 힘 (좌우)

#define SHOOTDOWN_MV_SCALAR 1.0f	// 격추되었을 때 낙하하는 힘
#define SHOOTDOWN_RT_DEGREE 10.0f	// 격추되었을 때 회전하는 각도

#define PITCH_ROTATE_SCALAR 0.15f	// pitch회전 힘
#define YAW_ROTATE_SCALAR 0.8f		// yaw회전 힘
#define ROLL_ROTATE_SCALAR 0.2f		// roll회전 힘

#define ENGINE_SCALAR 0.5f			// 헬기 엔진 출력 (상승, 하강 정도조절)

#define SENSITIVITY 0.25f			// 마우스 감도

#define PITCH_LIMIT 30.f			// 회전 각도제한
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 45.f			

#define COLLIDE_PLAYER_DAMAGE 100		// 플레이어간 충돌데미지

#define SHOOT_COOLDOWN_BULLET 100		// 총알 발사 쿨타임(ms)
#define BULLET_MOVE_SCALAR 20.0f		// 총알 이동 힘
#define BULLET_RANGE 800.f				// 총알 사정거리
#define BULLET_DAMAGE 10				// 총알 피격데미지

#define RESPAWN_TIME 5000			// 리스폰 시간 (ms)

#define HELI_BBSIZE_X 2.2f				// 헬기 BB크기
#define HELI_BBSIZE_Y 2.5f
#define HELI_BBSIZE_Z 2.5f

#define HELI_MAXHP 100

#define VULCAN_BULLET_BBSIZE_X 2.0f		// 공중전(기관총) 탄 BB크기
#define VULCAN_BULLET_BBSIZE_Y 2.0f
#define VULCAN_BULLET_BBSIZE_Z 2.0f

#define RIFFLE_BULLET_BBSIZE_X 3.5f		// 지상전(소총) 탄 BB크기
#define RIFFLE_BULLET_BBSIZE_Y 3.5f
#define RIFFLE_BULLET_BBSIZE_Z 3.5f

