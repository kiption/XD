#pragma once
#include <cmath>

double round_digit(double num, int d) {
	double t = pow(10, d - 1);
	return round(num * t) / t;
}