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
#define RELOAD_TIME 2000				// 장전완료까지 걸리는 시간(ms)
#define RELOAD_TIME_HELI 5000			// 헬기 장전완료까지 걸리는 시간(ms)

#define COLLIDE_PLAYER_DAMAGE 100		// 플레이어간 충돌데미지
#define RIFLE_DAMAGE 20					// 총알 피격데미지
#define VALKAN_DAMAGE 25				// 총알 피격데미지
#define NPC_RIFLE_DAMAGE 13				// NPC(소총) 피격데미지
#define NPC_VALKAN_DAMAGE 18			// NPC(헬기) 피격데미지

#define BULLET_RANGE 400				// 총알 최대이동거리

#define ATKSOUND_MAX_DISTANCE 400		// 총 소리가 나는 최대 거리
#define ATKSOUND_FAR_DISTANCE 290		// 작은 총 소리
#define ATKSOUND_MID_DISTANCE 170		// 적당한 총 소리
#define ATKSOUND_NEAR_DISTANCE 50		// 큰 총 소리

#define RELOADSOUND_MAX_DISTANCE 160	// 장전 소리가 나는 최대 거리
#define RELOADSOUND_FAR_DISTANCE 120	// 작게 나는 장전소리
#define RELOADSOUND_MID_DISTANCE 80	// 적당한 장전 소리
#define RELOADSOUND_NEAR_DISTANCE 40	// 큰 장전 소리

#define DAMAGEDSOUND_MAX_DISTANCE 120	// 피격 소리가 나는 최대 거리
#define DAMAGEDSOUND_FAR_DISTANCE 90	// 작게 나는 피격소리
#define DAMAGEDSOUND_MID_DISTANCE 60	// 적당한 피격 소리
#define DAMAGEDSOUND_NEAR_DISTANCE 30	// 큰 피격 소리

#define PARTICLE_BASIC_DAMAGE 5			// 파티클 충돌 기본 데미지 (최종 데미지는 여기에 질량을 곱한값)

//============================================================
//						인간정보 관련 상수
//============================================================
#define HUMAN_MAXHP 100

#define HUMAN_BBSIZE_X 4.0f			// 인간 BB크기 (CollideBox)
#define HUMAN_BBSIZE_Y 10.0f
#define HUMAN_BBSIZE_Z 4.0f

#define SPAWN_POS_X_HUMAN -15.0f
#define SPAWN_POS_Y_HUMAN 6.0f
#define SPAWN_POS_Z_HUMAN 750.0f

#define RESPAWN_TIME 3000			// 리스폰 시간 (ms)

#define HUMAN_VIEW_RANGE 500.0f

//============================================================
//						헬기정보 관련 상수
//============================================================
#define HELI_MAXHP 100

#define HELI_BBSIZE_X 10.0f				// 헬기 BB크기
#define HELI_BBSIZE_Y 10.0f
#define HELI_BBSIZE_Z 15.0f

#define SPAWN_POS_X_HELI -43.5f
#define SPAWN_POS_Y_HELI 25.0f
#define SPAWN_POS_Z_HELI -750.0f

#define AUTO_LEVELOFF_TIME 3000			// 몇 초(ms)동안 키입력이 없어야 자동 수평비행 전환이 되는가

#define RESPAWN_TIME_HELI 10000			// 헬기 리스폰 시간 (ms)

//============================================================
//					  스테이지 관련 상수
//============================================================
#define STAGE1_MISSION1_GOAL 25.0f		// 스테이지1 미션1 목표 (처치 단위: 명)
#define STAGE1_MISSION2_GOAL 60.0f		// 스테이지1 미션2 목표 (점령 단위: 초)

#define ST1_OCCUPY_AREA_POS_X -45.0f			// 스테이지1 점령지역 좌표
#define ST1_OCCUPY_AREA_POS_Z -727.5f
#define ST1_OCCUPY_AREA_SIZE_X 90.0f		// 스테이지1 점령지역 크기
#define ST1_OCCUPY_AREA_SIZE_Z 95.0f

#define STAGE1_MAX_HELI 5
#define STAGE1_MAX_HUMAN 25

#define STAGE1_SPAWN_AREA_POS_X -35.0f	// 스폰지역 좌표
#define STAGE1_SPAWN_AREA_POS_Z 690.0f
#define STAGE1_SPAWN_AREA_SIZE_X 110.0f	// 스폰지역 크기
#define STAGE1_SPAWN_AREA_SIZE_Z 120.0f

#define HEALPACK_SIZE 10.f				// 힐팩 크기 (정사각형)
#define HEALPACK_RECOVER_HP 50			// 힐팩으로 회복되는 HP수치
#define HEALPACK_RESPAWN_TIME 10000		// 힐팩 재생성 시간 (단위: ms)

#define HEALPACK_0_CENTER_X 141.3f	// 힐팩7 위치
#define HEALPACK_0_CENTER_Z 169.5f

#define HEALPACK_1_CENTER_X 560.0f	// 힐팩1 위치
#define HEALPACK_1_CENTER_Z 938.0f

#define HEALPACK_2_CENTER_X 141.3f	// 힐팩2 위치
#define HEALPACK_2_CENTER_Z -138.0f

#define HEALPACK_3_CENTER_X 560.0f	// 힐팩3 위치
#define HEALPACK_3_CENTER_Z -902.0f

#define HEALPACK_4_CENTER_X -217.0f	// 힐팩3 위치
#define HEALPACK_4_CENTER_Z 169.5f

#define HEALPACK_5_CENTER_X -630.0f	// 힐팩4 위치
#define HEALPACK_5_CENTER_Z 938.0f

#define HEALPACK_6_CENTER_X -634.0f	// 힐팩5 위치
#define HEALPACK_6_CENTER_Z -905.0f

#define HEALPACK_7_CENTER_X -215.3f	// 힐팩6 위치
#define HEALPACK_7_CENTER_Z -136.5f