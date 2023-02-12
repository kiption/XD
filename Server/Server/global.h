#pragma once

#define PI 3.141592654f

#define MOVE_SCALAR_FB 0.003f			// 이동 힘 (앞뒤)
#define MOVE_SCALAR_LR 0.0007f			// 이동 힘 (좌우)

#define PITCH_ROTATE_SCALAR 0.15f	// pitch회전 힘
#define YAW_ROTATE_SCALAR 0.8f		// yaw회전 힘
#define ROLL_ROTATE_SCALAR 0.2f		// roll회전 힘

#define ENGINE_SCALAR 2.5f			// 헬기 엔진 출력 (상승, 하강 정도조절)

#define SENSITIVITY 0.6f			// 마우스 감도

#define PITCH_LIMIT 45.f			// 회전 각도제한
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 60.f			

#define COLLIDE_PLAYER_DAMAGE 100		// 플레이어간 충돌데미지

#define SHOOT_COOLDOWN_BULLET 200		// 총알 발사 쿨타임(ms)
#define BULLET_MOVE_SCALAR 0.005f		// 총알 이동 힘
#define BULLET_RANGE 300.f				// 총알 사정거리
#define BULLET_DAMAGE 10				// 총알 피격데미지

#define RESPAWN_TIME 10000.f			// 리스폰 시간 (ms)
