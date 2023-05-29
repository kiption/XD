//======================================================================
//							[ Constant.h ]
//			프로젝트에서 사용되는 상수들을 정의해놓은 헤더입니다.
//======================================================================
#pragma once
#define PI 3.141592654f

//============================================================
//						이동 관련 상수
//============================================================
#define MOVE_SCALAR_FB 4.0f			// 이동 힘 (앞뒤)
#define MOVE_SCALAR_LR 1.4f			// 이동 힘 (좌우)

#define FLY_MIN_HEIGHT 20.f		// 비행가능한 최저높이
#define FLY_MAX_HEIGHT 330.f		// 비행가능한 최고높이

#define ENGINE_SCALAR 0.5f			// 헬기 엔진 출력 (상승, 하강 정도조절)

#define SHOOTDOWN_MV_SCALAR 1.0f	// 격추되었을 때 낙하하는 힘


//============================================================
//						회전 관련 상수
//============================================================
#define SENSITIVITY 0.25f			// 마우스 감도

#define PITCH_ROTATE_SCALAR 0.15f	// pitch회전 힘
#define YAW_ROTATE_SCALAR 0.8f		// yaw회전 힘
#define ROLL_ROTATE_SCALAR 0.2f		// roll회전 힘

#define PITCH_LIMIT 30.f			// 회전 각도제한
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 45.f			

#define SHOOTDOWN_RT_DEGREE 10.0f	// 격추되었을 때 회전하는 각도


//============================================================
//						공격 관련 상수
//============================================================
#define SHOOT_COOLDOWN_BULLET 33		// 총알 발사 쿨타임(ms)
#define RELOAD_TIME 1500				// 장전완료까지 걸리는 시간(ms)

#define COLLIDE_PLAYER_DAMAGE 100		// 플레이어간 충돌데미지
#define BULLET_DAMAGE 1					// 총알 피격데미지

#define BULLET_RANGE 200				// 총알 최대이동거리


//============================================================
//						인간정보 관련 상수
//============================================================
#define HUMAN_MAXHP 100

#define HUMAN_BBSIZE_X 2.5f				// 인간 BB크기 (CollideBox)
#define HUMAN_BBSIZE_Y 2.7f
#define HUMAN_BBSIZE_Z 2.7f
		
#define HUMAN_BOXSIZE_X 10.0f			// 인간 Box크기 (Raycast)
#define HUMAN_BOXSIZE_Y 20.0f
#define HUMAN_BOXSIZE_Z 10.0f

#define RESPAWN_POS_X 850.0f
#define RESPAWN_POS_Y 13.0f
#define RESPAWN_POS_Z 150.0f

//============================================================
//						헬기정보 관련 상수
//============================================================
#define HELI_MAXHP 100

#define HELI_BBSIZE_X 2.5f				// 헬기 BB크기 (CollideBox)
#define HELI_BBSIZE_Y 2.7f
#define HELI_BBSIZE_Z 2.7f

#define HELI_BOXSIZE_X 10.0f			// 헬기 Box크기 (Raycast)
#define HELI_BOXSIZE_Y 9.5f
#define HELI_BOXSIZE_Z 30.0f

#define AUTO_LEVELOFF_TIME 3000			// 몇 초(ms)동안 키입력이 없어야 자동 수평비행 전환이 되는가
#define RESPAWN_TIME 5000				// 리스폰 시간 (ms)

//============================================================
//					  스테이지 관련 상수
//============================================================
#define STAGE1_TIMELIMIT 600		// 스테이지1 제한시간 (단위: sec)
#define STAGE2_TIMELIMIT 600		// 스테이지2 제한시간 (단위: sec)

