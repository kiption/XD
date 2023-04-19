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
#define SHOOT_COOLDOWN_BULLET 100		// �Ѿ� �߻� ��Ÿ��(ms)

#define COLLIDE_PLAYER_DAMAGE 100		// �÷��̾ �浹������
#define BULLET_DAMAGE 10				// �Ѿ� �ǰݵ�����

#define BULLET_MOVE_SCALAR 20.0f		// �Ѿ� �̵� ��
#define BULLET_RANGE 800.f				// �Ѿ� �����Ÿ�

#define VULCAN_BULLET_BBSIZE_X 2.0f		// ������(�����) ź BBũ��
#define VULCAN_BULLET_BBSIZE_Y 2.0f
#define VULCAN_BULLET_BBSIZE_Z 2.0f

#define RIFFLE_BULLET_BBSIZE_X 3.5f		// ������(����) ź BBũ��
#define RIFFLE_BULLET_BBSIZE_Y 3.5f
#define RIFFLE_BULLET_BBSIZE_Z 3.5f


//============================================================
//						������� ���� ���
//============================================================
#define HELI_MAXHP 100

#define HELI_BBSIZE_X 2.2f				// ��� BBũ��
#define HELI_BBSIZE_Y 2.5f
#define HELI_BBSIZE_Z 2.5f

#define AUTO_LEVELOFF_TIME 3000			// �� ��(ms)���� Ű�Է��� ����� �ڵ� ������� ��ȯ�� �Ǵ°�
#define RESPAWN_TIME 5000				// ������ �ð� (ms)

#define RESPAWN_POS_X 512				// clientID * 50 �� �����ָ��.
#define RESPAWN_POS_Y 100
#define RESPAWN_POS_Z 350				// clientID * 50 �� ���ָ��.

//============================================================
//					  �������� ���� ���
//============================================================
#define STAGE1_TIMELIMIT 600		// ��������1 ���ѽð� (����: sec)
#define STAGE2_TIMELIMIT 600		// ��������2 ���ѽð� (����: sec)

