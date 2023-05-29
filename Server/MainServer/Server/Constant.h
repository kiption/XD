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
#define RELOAD_TIME 1500				// �����Ϸ���� �ɸ��� �ð�(ms)

#define COLLIDE_PLAYER_DAMAGE 100		// �÷��̾ �浹������
#define BULLET_DAMAGE 1					// �Ѿ� �ǰݵ�����

#define BULLET_RANGE 200				// �Ѿ� �ִ��̵��Ÿ�


//============================================================
//						�ΰ����� ���� ���
//============================================================
#define HUMAN_MAXHP 100

#define HUMAN_BBSIZE_X 2.5f				// �ΰ� BBũ�� (CollideBox)
#define HUMAN_BBSIZE_Y 2.7f
#define HUMAN_BBSIZE_Z 2.7f
		
#define HUMAN_BOXSIZE_X 10.0f			// �ΰ� Boxũ�� (Raycast)
#define HUMAN_BOXSIZE_Y 20.0f
#define HUMAN_BOXSIZE_Z 10.0f

#define RESPAWN_POS_X 850.0f
#define RESPAWN_POS_Y 13.0f
#define RESPAWN_POS_Z 150.0f

//============================================================
//						������� ���� ���
//============================================================
#define HELI_MAXHP 100

#define HELI_BBSIZE_X 2.5f				// ��� BBũ�� (CollideBox)
#define HELI_BBSIZE_Y 2.7f
#define HELI_BBSIZE_Z 2.7f

#define HELI_BOXSIZE_X 10.0f			// ��� Boxũ�� (Raycast)
#define HELI_BOXSIZE_Y 9.5f
#define HELI_BOXSIZE_Z 30.0f

#define AUTO_LEVELOFF_TIME 3000			// �� ��(ms)���� Ű�Է��� ����� �ڵ� ������� ��ȯ�� �Ǵ°�
#define RESPAWN_TIME 5000				// ������ �ð� (ms)

//============================================================
//					  �������� ���� ���
//============================================================
#define STAGE1_TIMELIMIT 600		// ��������1 ���ѽð� (����: sec)
#define STAGE2_TIMELIMIT 600		// ��������2 ���ѽð� (����: sec)

