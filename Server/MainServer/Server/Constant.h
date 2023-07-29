//======================================================================
//							[ Constant.h ]
//			������Ʈ���� ���Ǵ� ������� �����س��� ����Դϴ�.
//======================================================================
#pragma once
#define PI 3.141592654f

//============================================================
//						�̵� ���� ���
//============================================================
#define MOVE_SCALAR_FB 4.0f			// �̵� �� (�յ�)
#define MOVE_SCALAR_LR 1.4f			// �̵� �� (�¿�)

#define FLY_MIN_HEIGHT 20.f		// ���డ���� ��������
#define FLY_MAX_HEIGHT 330.f		// ���డ���� �ְ����

#define ENGINE_SCALAR 0.5f			// ��� ���� ��� (���, �ϰ� ��������)

#define SHOOTDOWN_MV_SCALAR 1.0f	// ���ߵǾ��� �� �����ϴ� ��


//============================================================
//						ȸ�� ���� ���
//============================================================
#define SENSITIVITY 0.25f			// ���콺 ����

#define PITCH_ROTATE_SCALAR 0.15f	// pitchȸ�� ��
#define YAW_ROTATE_SCALAR 0.8f		// yawȸ�� ��
#define ROLL_ROTATE_SCALAR 0.2f		// rollȸ�� ��

#define PITCH_LIMIT 30.f			// ȸ�� ��������
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 45.f			

#define SHOOTDOWN_RT_DEGREE 10.0f	// ���ߵǾ��� �� ȸ���ϴ� ����


//============================================================
//						���� ���� ���
//============================================================
#define SHOOT_COOLDOWN_BULLET 33		// �Ѿ� �߻� ��Ÿ��(ms)
#define RELOAD_TIME 2000				// �����Ϸ���� �ɸ��� �ð�(ms)
#define RELOAD_TIME_HELI 5000			// ��� �����Ϸ���� �ɸ��� �ð�(ms)

#define COLLIDE_PLAYER_DAMAGE 100		// �÷��̾ �浹������
#define RIFLE_DAMAGE 20					// �Ѿ� �ǰݵ�����
#define VALKAN_DAMAGE 25				// �Ѿ� �ǰݵ�����
#define NPC_RIFLE_DAMAGE 13				// NPC(����) �ǰݵ�����
#define NPC_VALKAN_DAMAGE 18			// NPC(���) �ǰݵ�����

#define BULLET_RANGE 400				// �Ѿ� �ִ��̵��Ÿ�

#define ATKSOUND_MAX_DISTANCE 400		// �� �Ҹ��� ���� �ִ� �Ÿ�
#define ATKSOUND_FAR_DISTANCE 290		// ���� �� �Ҹ�
#define ATKSOUND_MID_DISTANCE 170		// ������ �� �Ҹ�
#define ATKSOUND_NEAR_DISTANCE 50		// ū �� �Ҹ�

#define RELOADSOUND_MAX_DISTANCE 160	// ���� �Ҹ��� ���� �ִ� �Ÿ�
#define RELOADSOUND_FAR_DISTANCE 120	// �۰� ���� �����Ҹ�
#define RELOADSOUND_MID_DISTANCE 80	// ������ ���� �Ҹ�
#define RELOADSOUND_NEAR_DISTANCE 40	// ū ���� �Ҹ�

#define DAMAGEDSOUND_MAX_DISTANCE 120	// �ǰ� �Ҹ��� ���� �ִ� �Ÿ�
#define DAMAGEDSOUND_FAR_DISTANCE 90	// �۰� ���� �ǰݼҸ�
#define DAMAGEDSOUND_MID_DISTANCE 60	// ������ �ǰ� �Ҹ�
#define DAMAGEDSOUND_NEAR_DISTANCE 30	// ū �ǰ� �Ҹ�

#define PARTICLE_BASIC_DAMAGE 5			// ��ƼŬ �浹 �⺻ ������ (���� �������� ���⿡ ������ ���Ѱ�)

//============================================================
//						�ΰ����� ���� ���
//============================================================
#define HUMAN_MAXHP 100

#define HUMAN_BBSIZE_X 4.0f			// �ΰ� BBũ�� (CollideBox)
#define HUMAN_BBSIZE_Y 10.0f
#define HUMAN_BBSIZE_Z 4.0f

#define SPAWN_POS_X_HUMAN -15.0f
#define SPAWN_POS_Y_HUMAN 6.0f
#define SPAWN_POS_Z_HUMAN 750.0f

#define RESPAWN_TIME 3000			// ������ �ð� (ms)

#define HUMAN_VIEW_RANGE 500.0f

//============================================================
//						������� ���� ���
//============================================================
#define HELI_MAXHP 100

#define HELI_BBSIZE_X 10.0f				// ��� BBũ��
#define HELI_BBSIZE_Y 10.0f
#define HELI_BBSIZE_Z 15.0f

#define SPAWN_POS_X_HELI -43.5f
#define SPAWN_POS_Y_HELI 25.0f
#define SPAWN_POS_Z_HELI -750.0f

#define AUTO_LEVELOFF_TIME 3000			// �� ��(ms)���� Ű�Է��� ����� �ڵ� ������� ��ȯ�� �Ǵ°�

#define RESPAWN_TIME_HELI 10000			// ��� ������ �ð� (ms)

//============================================================
//					  �������� ���� ���
//============================================================
#define STAGE1_MISSION1_GOAL 25.0f		// ��������1 �̼�1 ��ǥ (óġ ����: ��)
#define STAGE1_MISSION2_GOAL 60.0f		// ��������1 �̼�2 ��ǥ (���� ����: ��)

#define ST1_OCCUPY_AREA_POS_X -45.0f			// ��������1 �������� ��ǥ
#define ST1_OCCUPY_AREA_POS_Z -727.5f
#define ST1_OCCUPY_AREA_SIZE_X 90.0f		// ��������1 �������� ũ��
#define ST1_OCCUPY_AREA_SIZE_Z 95.0f

#define STAGE1_MAX_HELI 5
#define STAGE1_MAX_HUMAN 25

#define STAGE1_SPAWN_AREA_POS_X -35.0f	// �������� ��ǥ
#define STAGE1_SPAWN_AREA_POS_Z 690.0f
#define STAGE1_SPAWN_AREA_SIZE_X 110.0f	// �������� ũ��
#define STAGE1_SPAWN_AREA_SIZE_Z 120.0f

#define HEALPACK_SIZE 10.f				// ���� ũ�� (���簢��)
#define HEALPACK_RECOVER_HP 50			// �������� ȸ���Ǵ� HP��ġ
#define HEALPACK_RESPAWN_TIME 10000		// ���� ����� �ð� (����: ms)

#define HEALPACK_0_CENTER_X 141.3f	// ����7 ��ġ
#define HEALPACK_0_CENTER_Z 169.5f

#define HEALPACK_1_CENTER_X 560.0f	// ����1 ��ġ
#define HEALPACK_1_CENTER_Z 938.0f

#define HEALPACK_2_CENTER_X 141.3f	// ����2 ��ġ
#define HEALPACK_2_CENTER_Z -138.0f

#define HEALPACK_3_CENTER_X 560.0f	// ����3 ��ġ
#define HEALPACK_3_CENTER_Z -902.0f

#define HEALPACK_4_CENTER_X -217.0f	// ����3 ��ġ
#define HEALPACK_4_CENTER_Z 169.5f

#define HEALPACK_5_CENTER_X -630.0f	// ����4 ��ġ
#define HEALPACK_5_CENTER_Z 938.0f

#define HEALPACK_6_CENTER_X -634.0f	// ����5 ��ġ
#define HEALPACK_6_CENTER_Z -905.0f

#define HEALPACK_7_CENTER_X -215.3f	// ����6 ��ġ
#define HEALPACK_7_CENTER_Z -136.5f