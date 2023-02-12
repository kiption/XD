#pragma once

#define PI 3.141592654f

#define MOVE_SCALAR_FB 0.003f			// �̵� �� (�յ�)
#define MOVE_SCALAR_LR 0.0007f			// �̵� �� (�¿�)

#define PITCH_ROTATE_SCALAR 0.15f	// pitchȸ�� ��
#define YAW_ROTATE_SCALAR 0.8f		// yawȸ�� ��
#define ROLL_ROTATE_SCALAR 0.2f		// rollȸ�� ��

#define ENGINE_SCALAR 2.5f			// ��� ���� ��� (���, �ϰ� ��������)

#define SENSITIVITY 0.6f			// ���콺 ����

#define PITCH_LIMIT 45.f			// ȸ�� ��������
#define YAW_LIMIT 90.f
#define ROLL_LIMIT 60.f			

#define COLLIDE_PLAYER_DAMAGE 100		// �÷��̾ �浹������

#define SHOOT_COOLDOWN_BULLET 200		// �Ѿ� �߻� ��Ÿ��(ms)
#define BULLET_MOVE_SCALAR 0.005f		// �Ѿ� �̵� ��
#define BULLET_RANGE 300.f				// �Ѿ� �����Ÿ�
#define BULLET_DAMAGE 10				// �Ѿ� �ǰݵ�����

#define RESPAWN_TIME 10000.f			// ������ �ð� (ms)
