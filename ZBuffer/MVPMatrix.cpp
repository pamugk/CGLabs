#define _USE_MATH_DEFINES
#include <cmath>
#include "MVPMatrix.h"

MVPMatrix::MVPMatrix() {}

MVPMatrix::MVPMatrix(float content[N])
{
	for (int i = 0; i < N; i++)
		this->content[i] = content[i];
}

MVPMatrix MVPMatrix::transpose()
{
	float transposedContent[N];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			transposedContent[i * n + j] = content[j * n + i];
	return MVPMatrix(transposedContent);
}

float* MVPMatrix::getContent()
{
	return content;
}

float* MVPMatrix::getNMatrix()
{
	const int n1 = 3;
	const int N1 = n1 * n1;
	float m[N1];
	for (int i = 0; i < n1; i++)
		for (int j = 0; j < n1; j++)
			m[i * n1 + j] = content[i * n + j];

	float det = 
		m[0] * m[4] * m[8] + m[3] * m[7] * m[2] + m[1] * m[5] * m[6] 
		- m[6] * m[4] * m[2] - m[1] * m[3] * m[8] - m[5] * m[7] * m[0];

	float* nMatrixContent = new float[N1]
	{
		m[4] * m[8] - m[5] * m[7], m[7] * m[2] - m[1] * m[8], m[1] * m[5] - m[4] * m[2],
		m[6] * m[5] - m[3] * m[8], m[0] * m[8] - m[2] * m[6], m[3] * m[2] - m[0] * m[5],
		m[3] * m[7] - m[6] * m[4], m[1] * m[6] - m[0] * m[7], m[0] * m[4] - m[3] * m[1]
	};

	for (int i = 0; i < N1; i++)
	{
		nMatrixContent[i] /= det;
		float q = nMatrixContent[i];
		int a = 0;
	}
	return nMatrixContent;
}

MVPMatrix MVPMatrix::move(float x, float y, float z)
{
	float moveContent[N] =
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		x, y, z, 1.0f
	};
	return MVPMatrix(moveContent) * (*this);
}

MVPMatrix MVPMatrix::scale(float sx, float sy, float sz)
{
	float scaleContent[N] = 
	{
		sx, 0.f, 0.f, 0.f,
		0.f, sy, 0.f, 0.f,
		0.f, 0.f, sz, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	return MVPMatrix(scaleContent) * (*this);
}

MVPMatrix MVPMatrix::rotateAboutX(float degree)
{
	return rotate(1.f, 0.f, 0.f, degree);
}

MVPMatrix MVPMatrix::rotateAboutY(float degree)
{
	return rotate(0.f, 1.f, 0.f, degree);
}

MVPMatrix MVPMatrix::rotateAboutZ(float degree)
{
	return rotate(0.f, 0.f, 1.f, degree);
}

MVPMatrix MVPMatrix::rotate(float x, float y, float z, float degree)
{
	float c = cos(degree);
	float s = sin(degree);
	float rotateContent[N] =
	{
		x * x * (1.f - c) + c, y * x * (1.f - c) + z * s, x * z * (1.f - c) - y * s, 0.f,
		x * y * (1.f - c) - z * s, y * y * (1.f - c) + c, y * z * (1.f - c) + x * s, 0.f,
		x * z * (1.f - c) + y * s, y * z * (1.f - c) - x * s, z * z * (1.f - c) + c, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	return MVPMatrix(rotateContent) * (*this);
}

MVPMatrix MVPMatrix::getIdentityMatrix()
{
	float E[16] =
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	return MVPMatrix(E);
}

MVPMatrix MVPMatrix::getParallelProjectionMatrix(float l, float r, float b, float t, float n, float f)
{
	float parallelMatrixContent[N] =
	{
		2.f / (r - l), 0.f, 0.f, 0.f,
		0.f, 2.f / (t - b), 0.f, 0.f,
		0.f, 0.f, -2.f / (f - n), 0.f,
		-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1.f
	};
	return MVPMatrix(parallelMatrixContent);
}

MVPMatrix MVPMatrix::getPerspectiveProjectionMatrix(float l, float r, float b, float t, float n, float f)
{
	float perspectiveMatrixContent[N] =
	{
		2.f * n / (r - l), 0.f, 0.f, 0.f,
		0.f, 2.f * n / (t - b), 0.f, 0.f,
		(r + l) / (r - l), (t + b) / (t - b), - (f + n) / (f - n), -1.f,
		0.f, 0.f, -2.f *f * n / (f - n), 0.f
	};
	return MVPMatrix(perspectiveMatrixContent);
}

MVPMatrix MVPMatrix::getPerspectiveProjectionMatrix(float n, float f, float w, float h, float fovAngle)
{
	float tg = tanf(M_PI / 180.f * fovAngle / 2.f);
	return MVPMatrix::getPerspectiveProjectionMatrix(-n * tg, n * tg, -n * w / h *tg, n * w / h * tg, n, f);
}

bool MVPMatrix::operator==(const MVPMatrix & m)
{
	bool result = true;
	int i = 0;
	while (result && i < N)
	{
		result = result && content[i] == m.content[i];
		i++;
	}
	return result;
}

bool MVPMatrix::operator!=(const MVPMatrix & m)
{
	bool result = true;
	int i = 0;
	while (result && i < N)
	{
		result = result && content[i] == m.content[i];
		i++;
	}
	return !result;
}

MVPMatrix MVPMatrix::operator+(const MVPMatrix & m)
{
	float newContent[N];
	for (int i = 0; i < N; i++)
		newContent[i] = content[i] + m.content[i];
	return MVPMatrix(newContent);
}

MVPMatrix MVPMatrix::operator-(const MVPMatrix & m)
{
	float newContent[N];
	for (int i = 0; i < N; i++)
		newContent[i] = content[i] - m.content[i];
	return MVPMatrix(newContent);
}

MVPMatrix MVPMatrix::operator*(const MVPMatrix & m)
{
	float newContent[N];
	for(int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
		{
			 float sum = 0;
			 for (int k = 0; k < n; k++)
				 sum += content[k * n + i] * m.content[j * n + k];
			 newContent[j * n + i] = sum;
		}
	return MVPMatrix(newContent);
}

MVPMatrix MVPMatrix::operator*(float num)
{
	float newContent[N];
	for (int i = 0; i < N; i++)
		newContent[i] = content[i] * num;
	return MVPMatrix(newContent);
}

MVPMatrix MVPMatrix::operator/(float num)
{
	float newContent[N];
	for (int i = 0; i < N; i++)
		newContent[i] = content[i] / num;
	return MVPMatrix(newContent);
}