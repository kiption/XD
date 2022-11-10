#pragma once
#include <math.h>
#include "MyVectors.h"

MyVector3D myNormalize(MyVector3D vec);

MyVector3D calcMove(MyVector3D vec1, MyVector3D vec2, float scalar);
MyVector3D calcRotate(MyVector3D vec, float roll, float pitch, float yaw);
