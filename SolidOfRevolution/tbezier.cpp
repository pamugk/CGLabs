/**
 * tbezier.cpp is an implementation of finite discrete point set smooth interpolation algorithms
 * based on cubic Bezier curves with control points calculated according to the tangents to the
 * angles of polygonal line that is built by the linear interpolation of the input points set.
 *
 * Two functions are provided: tbezierSO1 that builds the curve with smoothness order 1 and
 * tbezierSO0 that builds curve with smoothness order 0 and uses special heuristics to
 * reduce lengths of tangents and therefore reduce the difference with linear interpolation
 * making result curve look nicer.
 *
 * tbezierSO1 is recommended for scientific visualization as it uses strict math to balance
 * between smoothness and interpolation accuracy.
 * tbezierSO0 is recommended for advertising purposes as it produces nicer looking curves while
 * the accuracy is in common case lower.
 *
 * Read this for algorithm details: http://sv-journal.org/2017-1/04.php?lang=en
 *
 * Written by Konstantin Ryabinin under terms of MIT license.
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Konstantin Ryabinin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <vector>
#include <iostream>
#include <cmath>
#include "tbezier.h"

using namespace std;

/**
 * Threshold for zero.
 */
const auto EPSILON = 1.0e-5;

/**
 * Test if real value is zero.
 */
#define IS_ZERO(v) (abs(v) < EPSILON)

/**
* Signum function.
*/
#define SIGN(v) (int)(((v) > EPSILON) - ((v) < -EPSILON))

/**
 * Build an interpolation curve with smoothness order 1 based on cubic Bezier according to given point set.
 *
 * @param values - input array of points to interpolate.
 * @param curve - output array of curve segments.
 * @return true if interpolation successful, false if not.
 */
bool tbezierSO1(const vector<Point2D>& values, vector<Segment>& curve)
{
	int n = values.size() - 1;
	if (n < 2)
		return false;
	curve.resize(n);
	Point2D cur, next, tgL, tgR, deltaL, deltaC, deltaR;
	double l1, l2;
	next = values[1] - values[0];
	next.normalize();
	for (int i = 0; i < n; ++i)
	{
		tgL = tgR;
		cur = next;
		deltaC = values[i + 1] - values[i];
		if (i > 0)
			deltaL = Point2D::absMin(deltaC, values[i] - values[i - 1]);
		else
			deltaL = deltaC;
		if (i < n - 1)
		{
			next = values[i + 2] - values[i + 1];
			next.normalize();
			if (IS_ZERO(cur.x) || IS_ZERO(cur.y))
				tgR = cur;
			else if (IS_ZERO(next.x) || IS_ZERO(next.y))
				tgR = next;
			else
				tgR = cur + next;
			tgR.normalize();
			deltaR = Point2D::absMin(deltaC, values[i + 2] - values[i + 1]);
		}
		else
		{
			tgR = Point2D();
			deltaR = deltaC;
		}
		l1 = IS_ZERO(tgL.x) ? 0.0 : deltaL.x / (C * tgL.x);
		l2 = IS_ZERO(tgR.x) ? 0.0 : deltaR.x / (C * tgR.x);
		if (abs(l1 * tgL.y) > abs(deltaL.y))
			l1 = IS_ZERO(tgL.y) ? 0.0 : deltaL.y / tgL.y;
		if (abs(l2 * tgR.y) > abs(deltaR.y))
			l2 = IS_ZERO(tgR.y) ? 0.0 : deltaR.y / tgR.y;
		curve[i].points[0] = values[i];
		curve[i].points[1] = curve[i].points[0] + tgL * l1;
		curve[i].points[3] = values[i + 1];
		curve[i].points[2] = curve[i].points[3] - tgR * l2;
	}
	return true;
}

/**
 * Build an interpolation curve with smoothness order 0 based on cubic Bezier according to given point set.
 *
 * @param values - input array of points to interpolate.
 * @param curve - output array of curve segments.
 * @return true if interpolation successful, false if not.
 */
bool tbezierSO0(const vector<Point2D> & values, vector<Segment> & curve)
{
	int n = values.size() - 1;
	if (n < 2)
		return false;
	curve.resize(n);
	Point2D cur, next, tgL, tgR, deltaC;
	double l1, l2, tmp, x;
	bool zL, zR;
	next = values[1] - values[0];
	next.normalize();
	for (int i = 0; i < n; ++i)
	{
		tgL = tgR;
		cur = next;
		deltaC = values[i + 1] - values[i];
		if (i < n - 1)
		{
			next = values[i + 2] - values[i + 1];
			next.normalize();
			if (IS_ZERO(cur.x) || IS_ZERO(cur.y))
				tgR = cur;
			else if (IS_ZERO(next.x) || IS_ZERO(next.y))
				tgR = next;
			else
				tgR = cur + next;
			tgR.normalize();
		}
		else
			tgR = Point2D();
		// There is actually a little mistake in the white paper (http://sv-journal.org/2017-1/04.php?lang=en):
		// algorithm described after figure 14 implicitly assumes that tangent vectors point inside the
		// A_i and B_i areas (see fig. 14). However in practice they can point outside as well. If so, tangents�
		// coordinates should be clamped to the border of A_i or B_i respectively to keep control points inside
		// the described area and thereby to avoid false extremes and loops on the curve.
		// The clamping is implemented by the next 4 if-statements.
		if (SIGN(tgL.x) != SIGN(deltaC.x))
			tgL.x = 0.0;
		if (SIGN(tgL.y) != SIGN(deltaC.y))
			tgL.y = 0.0;
		if (SIGN(tgR.x) != SIGN(deltaC.x))
			tgR.x = 0.0;
		if (SIGN(tgR.y) != SIGN(deltaC.y))
			tgR.y = 0.0;
		zL = IS_ZERO(tgL.x);
		zR = IS_ZERO(tgR.x);
		l1 = zL ? 0.0 : deltaC.x / (C * tgL.x);
		l2 = zR ? 0.0 : deltaC.x / (C * tgR.x);
		if (abs(l1 * tgL.y) > abs(deltaC.y))
			l1 = IS_ZERO(tgL.y) ? 0.0 : deltaC.y / tgL.y;
		if (abs(l2 * tgR.y) > abs(deltaC.y))
			l2 = IS_ZERO(tgR.y) ? 0.0 : deltaC.y / tgR.y;
		if (!zL && !zR)
		{
			tmp = tgL.y / tgL.x - tgR.y / tgR.x;
			if (!IS_ZERO(tmp))
			{
				x = (values[i + 1].y - tgR.y / tgR.x * values[i + 1].x - values[i].y + tgL.y / tgL.x * values[i].x) / tmp;
				if (x > values[i].x && x < values[i + 1].x)
				{
					if (abs(l1) > abs(l2))
						l1 = 0.0;
					else
						l2 = 0.0;
				}
			}
		}
		curve[i].points[0] = values[i];
		curve[i].points[1] = curve[i].points[0] + tgL * l1;
		curve[i].points[3] = values[i + 1];
		curve[i].points[2] = curve[i].points[3] - tgR * l2;
	}
	return true;
}