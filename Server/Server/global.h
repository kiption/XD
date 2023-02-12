#pragma once

#define PI 3.141592654f

#define MOVE_SCALAR_FB 0.7f			// 이동 힘 (앞뒤)
#define MOVE_SCALAR_LR 0.3f			// 이동 힘 (좌우)

#define PITCH_ROTATE_SCALAR 0.2f	// pitch회전 힘
#define YAW_ROTATE_SCALAR 0.3f		// yaw회전 힘
#define ROLL_ROTATE_SCALAR 0.3f		// roll회전 힘

#define ENGINE_SCALAR 1.6f			// 헬기 엔진 출력 (상승, 하강 정도조절)

#define SENSITIVITY 0.6f			// 마우스 감도

#define PITCH_LIMIT 45.f			// 회전 각도제한
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 30.f			

#define SHOOT_COOLDOWN_BULLET 200		// 총알 발사 쿨타임(ms)

#define BULLET_MOVE_SCALAR 1.5f			// 총알 이동 힘
#define BULLET_RANGE 200.f				// 총알 사정거리
