#pragma once
#include <math.h>
#include "MyVectors.h"

XMFLOAT3 myNormalize(XMFLOAT3 vec);

XMFLOAT3 calcMove(XMFLOAT3 vec1, XMFLOAT3 vec2, float scalar);
XMFLOAT3 calcRotate(XMFLOAT3 vec, float roll, float pitch, float yaw);
