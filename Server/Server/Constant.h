#pragma once

#define PI 3.141592654f

#define MOVE_SCALAR_FB 6.0f			// 이동 힘 (앞뒤)
#define MOVE_SCALAR_LR 1.4f			// 이동 힘 (좌우)

#define PITCH_ROTATE_SCALAR 0.15f	// pitch회전 힘
#define YAW_ROTATE_SCALAR 0.8f		// yaw회전 힘
#define ROLL_ROTATE_SCALAR 0.2f		// roll회전 힘

#define ENGINE_SCALAR 1.0f			// 헬기 엔진 출력 (상승, 하강 정도조절)

#define SENSITIVITY 0.7f			// 마우스 감도

#define PITCH_LIMIT 45.f			// 회전 각도제한
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 60.f			

#define COLLIDE_PLAYER_DAMAGE 100		// 플레이어간 충돌데미지

#define SHOOT_COOLDOWN_BULLET 100		// 총알 발사 쿨타임(ms)
#define BULLET_MOVE_SCALAR 10.0f		// 총알 이동 힘
#define BULLET_RANGE 1200.f				// 총알 사정거리
#define BULLET_DAMAGE 10				// 총알 피격데미지

#define RESPAWN_TIME 10000			// 리스폰 시간 (ms)

#define heli_bbsize_x 2.0f				// 헬기 BB크기
#define heli_bbsize_y 2.0f
#define heli_bbsize_z 2.0f

#define vulcan_bullet_bbsize_x 1.5f		// 공중전(기관총) 탄 BB크기
#define vulcan_bullet_bbsize_y 1.5f
#define vulcan_bullet_bbsize_z 1.5f

#define riffle_bullet_bbsize_x 3.5f		// 지상전(소총) 탄 BB크기
#define riffle_bullet_bbsize_y 3.5f
#define riffle_bullet_bbsize_z 3.5f

