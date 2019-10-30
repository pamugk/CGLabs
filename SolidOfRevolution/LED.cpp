#include "LED.h"

void LED::move(float dx, float dy, float dz)
{
	moveAlongX(dx);
	moveAlongY(dy);
	moveAlongZ(dz);
}

void LED::moveAlongX(float dx)
{
	position[0] += dx;
}

void LED::moveAlongY(float dy)
{
	position[1] += dy;
}

void LED::moveAlongZ(float dz)
{
	position[2] += dz;
}