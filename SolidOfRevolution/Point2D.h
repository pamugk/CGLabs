#pragma once
#include <cmath>

/**
 * Test if real value is zero.
 */
#define IS_ZERO(v) (abs(v) < 1.0e-5)

/**
 * The Point2D struct provides methods to store and handle 2d points.
 */
struct Point2D
{
	/**
	 * Point coordinates.
	 */
	double x, y;

	/**
	 *Point2D constructor.
	 */
	Point2D();
	/**
	 * Point2D constructor.
	 *
	 * @param x - x coordinate of the point.
	 * @param y - y coordinate of the point.
	 */
	Point2D(double, double);

	/**
	 * Add other point to the current one.
	 *
	 * @param p - point to add.
	 * @return summ of the current point and the given one.
	 */
	Point2D operator +(const Point2D& p) const;
	/**
	 * Subtract other point from the current one.
	 *
	 * @param p - point to subtract.
	 * @return difference of the current point and the given one.
	 */
	Point2D operator -(const Point2D& p) const;
	/**
	 * Multiply current point by the real value.
	 *
	 * @param v - value to multiply by.
	 * @return current point multiplied by the given value.
	 */
	Point2D operator *(double) const;

	/**
	 * Safely normalize current point.
	 */
	void normalize();
	/**
	 * Get the absolute minimum of two given points.
	 *
	 * @param p1 - first point.
	 * @param p2 - second point.
	 * @return absolute minimum of the given points' coordinates.
	 */
	static Point2D absMin(const Point2D&, const Point2D&);
};