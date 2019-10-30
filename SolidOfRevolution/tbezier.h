#pragma once
#include <vector>
#include "Point2D.h"
#include "Segment.h"

using namespace std;
/**
 * Amount of lines representing each Bezier segment.
 */
const auto RESOLUTION = 32;

/**
 * Paramenet affecting curvature, should be in [2; +inf).
 */
const auto C = 2.0;

/**
 * Build an interpolation curve with smoothness order 0 based on cubic Bezier according to given point set.
 *
 * @param values - input array of points to interpolate.
 * @param curve - output array of curve segments.
 * @return true if interpolation successful, false if not.
 */
bool tbezierSO0(const vector<Point2D>& values, vector<Segment>& curve);

/**
 * Build an interpolation curve with smoothness order 1 based on cubic Bezier according to given point set.
 *
 * @param values - input array of points to interpolate.
 * @param curve - output array of curve segments.
 * @return true if interpolation successful, false if not.
 */
bool tbezierSO1(const vector<Point2D>& values, vector<Segment>& curve);