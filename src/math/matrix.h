#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
	float data[16];
} Matrix;

Matrix projectionMatrix(int w, int h);
Matrix viewMatrix(float x, float y);
Matrix modelMatrix(float x, float y, float rot, int w, int h);

#endif // !MATRIX_H
