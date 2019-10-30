#include "Segment.h"

Point2D Segment::calc(double t) const
{
	double t2 = t * t;
	double t3 = t2 * t;
	double nt = 1.0 - t;
	double nt2 = nt * nt;
	double nt3 = nt2 * nt;
	return Point2D(nt3 * points[0].x + 3.0 * t * nt2 * points[1].x + 3.0 * t2 * nt * points[2].x + t3 * points[3].x,
		nt3 * points[0].y + 3.0 * t * nt2 * points[1].y + 3.0 * t2 * nt * points[2].y + t3 * points[3].y);
};