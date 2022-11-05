#pragma once
#include <math.h>
#include "MyFloat3.h"
enum POS { X, Y, Z, W };		// vec[0] ���� vec[X] �� �˾ƺ��� ���ϱ� ������ ����.

MyFloat3 myNormalize(MyFloat3 vec);

MyFloat3 calcMove(MyFloat3 vec1, MyFloat3 vec2, float scalar);
MyFloat3 calcRotate(MyFloat3 vec, float roll, float pitch, float yaw);

MyFloat3 roll(MyFloat3 vec);
