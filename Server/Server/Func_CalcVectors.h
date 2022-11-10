#pragma once
#include <math.h>
#include "MyVectors.h"

#define PI 3.141592654f
#define ROLL_LIMIT 30.f
#define PITCH_LIMIT 45.f
#define YAW_LIMIT 90.f

MyVector3D myNormalize(MyVector3D vec);

MyVector3D calcMove(MyVector3D vec1, MyVector3D vec2, float scalar);
MyVector3D calcRotate(MyVector3D vec, float roll, float pitch, float yaw);
