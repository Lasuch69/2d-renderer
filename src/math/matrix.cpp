#include <cmath>

#include "matrix.h"

Matrix projectionMatrix(int w, int h) {
	float x = 2.0f / float(w);
	float y = -2.0f / float(h);

	Matrix mat;
	mat.data[0] = x;
	mat.data[1] = 0.0;
	mat.data[2] = 0.0;
	mat.data[3] = 0.0; // int(w / 2);

	mat.data[4] = 0.0;
	mat.data[5] = y;
	mat.data[6] = 0.0;
	mat.data[7] = 0.0; // int(h / 2);

	mat.data[8] = 0.0;
	mat.data[9] = 0.0;
	mat.data[10] = 1.0;
	mat.data[11] = 0.0;

	mat.data[12] = 0.0;
	mat.data[13] = 0.0;
	mat.data[14] = 0.0;
	mat.data[15] = 1.0;

	return mat;
}

Matrix viewMatrix(float x, float y) {
	Matrix mat;
	mat.data[0] = 1.0;
	mat.data[1] = 0.0;
	mat.data[2] = 0.0;
	mat.data[3] = 0.0;

	mat.data[4] = 0.0;
	mat.data[5] = 1.0;
	mat.data[6] = 0.0;
	mat.data[7] = 0.0;

	mat.data[8] = 0.0;
	mat.data[9] = 0.0;
	mat.data[10] = 1.0;
	mat.data[11] = 0.0;

	mat.data[12] = -x;
	mat.data[13] = -y;
	mat.data[14] = 0.0;
	mat.data[15] = 1.0;

	return mat;
}

Matrix modelMatrix(float x, float y, float rot, int w, int h) {
	float sin = std::sin(rot);
	float cos = std::cos(rot);

	Matrix mat;
	mat.data[0] = cos * w;
	mat.data[1] = sin * w;
	mat.data[2] = 0.0;
	mat.data[3] = 0.0;

	mat.data[4] = -sin * h;
	mat.data[5] = cos * h;
	mat.data[6] = 0.0;
	mat.data[7] = 0.0;

	mat.data[8] = 0.0;
	mat.data[9] = 0.0;
	mat.data[10] = 1.0;
	mat.data[11] = 0.0;

	mat.data[12] = x;
	mat.data[13] = y;
	mat.data[14] = 0.0;
	mat.data[15] = 1.0;

	return mat;
}
