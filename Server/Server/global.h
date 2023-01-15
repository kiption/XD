#pragma once

#define PI 3.141592654f

#define MOVE_SCALAR 0.3f		// 이동 힘
#define YAW_ROTATE_SCALAR 0.5f	// yaw회전 힘
#define ENGINE_SCALAR 0.5f		// 헬기 엔진 출력 (상승, 하강 정도조절)

#define SENSITIVITY 0.6f		// 마우스 감도

#define ROLL_LIMIT 60.f			// 회전 각도제한
#define PITCH_LIMIT 20.f
#define YAW_LIMIT 90.f

#define SHOOT_COOLDOWN_BULLET 200		// 총알 발사 쿨타임(ms)

#define BULLET_MOVE_SCALAR 0.8f			// 총알 이동 힘
#define BULLET_RANGE 200.f				// 총알 사정거리
