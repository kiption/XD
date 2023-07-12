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
#define RELOAD_TIME 2500				// �����Ϸ���� �ɸ��� �ð�(ms)

#define COLLIDE_PLAYER_DAMAGE 100		// �÷��̾ �浹������
#define BULLET_DAMAGE 1				// �Ѿ� �ǰݵ�����

#define BULLET_RANGE 500				// �Ѿ� �ִ��̵��Ÿ�


//============================================================
//						�ΰ����� ���� ���
//============================================================
#define HUMAN_MAXHP 100

#define HUMAN_BBSIZE_X 4.0f			// �ΰ� BBũ�� (CollideBox)
#define HUMAN_BBSIZE_Y 30.0f
#define HUMAN_BBSIZE_Z 4.0f

#define RESPAWN_POS_X -15.0f
#define RESPAWN_POS_Y 6.0f
#define RESPAWN_POS_Z 750.0f

#define RESPAWN_TIME 5000			// ������ �ð� (ms)

#define HUMAN_VIEW_RANGE 500.0f

//============================================================
//						������� ���� ���
//============================================================
#define HELI_MAXHP 100

#define HELI_BBSIZE_X 25.f				// ��� BBũ��
#define HELI_BBSIZE_Y 10.f
#define HELI_BBSIZE_Z 40.f

#define AUTO_LEVELOFF_TIME 3000			// �� ��(ms)���� Ű�Է��� ����� �ڵ� ������� ��ȯ�� �Ǵ°�
#define RESPAWN_TIME 5000				// ������ �ð� (ms)

//============================================================
//					  �������� ���� ���
//============================================================
#define STAGE1_MISSION1_GOAL 25.0f		// ��������1 �̼� ��ǥ
#define STAGE1_MISSION2_GOAL 10.0f

#define ST1_OCCUPY_AREA_POS_X -90.0f		// ��������1 �������� ��ǥ
#define ST1_OCCUPY_AREA_POS_Z -795.0f
#define ST1_OCCUPY_AREA_SIZE_X 100.0f		// ��������1 �������� ũ��
#define ST1_OCCUPY_AREA_SIZE_Z 130.0f

#define STAGE1_MAX_HELI 5
#define STAGE1_MAX_HUMAN 25