#include "Point2D.h"

Point2D::Point2D() { x = y = 0.0; }
Point2D::Point2D(double _x, double _y) { x = _x; y = _y; }

Point2D Point2D::operator +(const Point2D& p) const { return Point2D(x + p.x, y + p.y); }
Point2D Point2D::operator -(const Point2D& p) const { return Point2D(x - p.x, y - p.y); }
Point2D Point2D::operator *(double v) const { return Point2D(x * v, y * v); }

void Point2D::normalize()
{
	double l = sqrt(x * x + y * y);
	if (IS_ZERO(l))
		x = y = 0.0;
	else
	{
		x /= l;
		y /= l;
	}
};

Point2D Point2D::absMin(const Point2D& p1, const Point2D& p2)
{
	return Point2D(abs(p1.x) < abs(p2.x) ? p1.x : p2.x, abs(p1.y) < abs(p2.y) ? p1.y : p2.y);
}