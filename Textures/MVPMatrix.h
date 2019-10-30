#pragma once
//����� ��� ��������� ������ � ��������� �������������� OpenGL
class MVPMatrix
{
	//����������� �������
	static const int n = 4;
	//����� ��������� � �������
	static const int N = n * n;
	//���������� �������
	float content[N];

public:
	MVPMatrix();
	//����������� �������, ����������� ����������,
	//���������� �� ��������� OpenGL
	MVPMatrix(float content[N]);
	//����� ��� ��������� ����������� �������
	float* getContent();
	//����� ���������� ������ ��� N-������� 
	float* getNMatrix();

	//����� ��� ���������������� �������
	MVPMatrix transpose();

	//����� ��� �������������� ��������
	MVPMatrix move(float x, float y, float z);
	//����� ��� �������������� ���������������
	MVPMatrix scale(float sx, float sy, float sz);
	//����� ��� �������������� �������� ������ X
	MVPMatrix rotateAboutX(float degree);
	//����� ��� �������������� �������� ������ Y
	MVPMatrix rotateAboutY(float degree);
	//����� ��� �������������� �������� ������ Z
	MVPMatrix rotateAboutZ(float degree);
	//����� ��� �������������� �������� (|(x, y, z)| = 1)
	MVPMatrix rotate(float x, float y, float z, float degree);

	//����� ��� ������������ ��������� �������
	static MVPMatrix getIdentityMatrix();
	//����� ��� ������������ ������� ������������ ��������
	static MVPMatrix getParallelProjectionMatrix(float l, float r, float b, float t, float n, float f);
	//����� ��� ������������ ������� ������������� ��������
	static MVPMatrix getPerspectiveProjectionMatrix(float l, float r, float b, float t, float n, float f);
	//����� ��� ������������ ������� ������������� ��������
	static MVPMatrix getPerspectiveProjectionMatrix(float n, float f, float w, float h, float fovAngle);

	//�������� "�����"
	bool operator ==(const MVPMatrix &m);
	//�������� "�� �����"
	bool operator !=(const MVPMatrix &m);

	//�������� �������� ������
	MVPMatrix operator +(const MVPMatrix &m);
	//�������� ��������� ������
	MVPMatrix operator -(const MVPMatrix &m);

	//�������� ��������� ������
	MVPMatrix operator * (const MVPMatrix &m);
	//�������� �������� ������
	MVPMatrix operator *(float num);
	//�������� �������� ������
	MVPMatrix operator /(float num);
};