#include "Point2D.h"
#pragma once
/**
 * The Segment structure provides methods to store and calculate Bezier-based cubic curve segment.
 */
struct Segment
{
	/**
	 * Bezier control points.
	 */
	Point2D points[4];

	/**
	 * Calculate the intermediate curve points.
	 *
	 * @param t - parameter of the curve, should be in [0; 1].
	 * @return intermediate Bezier curve point that corresponds the given parameter.
	 */
	Point2D calc(double) const;
};