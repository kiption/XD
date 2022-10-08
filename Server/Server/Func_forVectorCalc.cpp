#include "Func_forVectorCalc.h"

void calcMove(float* vec1, float* vec2, float scalar)
{
	vec1[X] = vec1[X] + vec2[X] * scalar;
	vec1[Y] = vec1[Y] + vec2[Y] * scalar;
	vec1[Z] = vec1[Z] + vec2[Z] * scalar;
}
