#include <GL/glew.h>
#pragma once
class LED
{
public:
	GLint positionHandler;
	GLint colorHandler;
	GLint stateHandler;

	GLfloat position[3] = { 1, 1, 1 }; //Позиция источника света
	GLfloat color[3] = { 1.0f, 1.0f, 1.0f }; //Цвет источника света
	bool isEnabled = true;

	void move(float dx, float dy, float dz);
	void moveAlongX(float dx);
	void moveAlongY(float dy);
	void moveAlongZ(float dz);
};