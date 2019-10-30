#pragma once
//Класс для упрощения работы с матрицами преобразований OpenGL
class MVPMatrix
{
	//Размерность матрицы
	static const int n = 4;
	//Число элементов в матрице
	static const int N = n * n;
	//Содержимое матрицы
	float content[N];

public:
	MVPMatrix();
	//Конструктор матрицы, принимающий содержимое,
	//записанное по стандарту OpenGL
	MVPMatrix(float content[N]);
	//Метод для получения содержимого матрицы
	float* getContent();
	//Метод извлечения данных для N-матрицы 
	float* getNMatrix();

	//Метод для транспонирования матрицы
	MVPMatrix transpose();

	//Метод для преобразования переноса
	MVPMatrix move(float x, float y, float z);
	//Метод для преобразования масштабирования
	MVPMatrix scale(float sx, float sy, float sz);
	//Метод для преобразования вращения вокруг X
	MVPMatrix rotateAboutX(float degree);
	//Метод для преобразования вращения вокруг Y
	MVPMatrix rotateAboutY(float degree);
	//Метод для преобразования вращения вокруг Z
	MVPMatrix rotateAboutZ(float degree);
	//Метод для преобразования вращения (|(x, y, z)| = 1)
	MVPMatrix rotate(float x, float y, float z, float degree);

	//Метод для формирования единичной матрицы
	static MVPMatrix getIdentityMatrix();
	//Метод для формирования матрицы параллельной проекции
	static MVPMatrix getParallelProjectionMatrix(float l, float r, float b, float t, float n, float f);
	//Метод для формирования матрицы перспективной проекции
	static MVPMatrix getPerspectiveProjectionMatrix(float l, float r, float b, float t, float n, float f);
	//Метод для формирования матрицы перспективной проекции
	static MVPMatrix getPerspectiveProjectionMatrix(float n, float f, float w, float h, float fovAngle);

	//Оператор "равно"
	bool operator ==(const MVPMatrix &m);
	//Оператор "не равно"
	bool operator !=(const MVPMatrix &m);

	//Оператор сложения матриц
	MVPMatrix operator +(const MVPMatrix &m);
	//Оператор вычитания матриц
	MVPMatrix operator -(const MVPMatrix &m);

	//Оператор умножения матриц
	MVPMatrix operator * (const MVPMatrix &m);
	//Оператор сложения матриц
	MVPMatrix operator *(float num);
	//Оператор сложения матриц
	MVPMatrix operator /(float num);
};