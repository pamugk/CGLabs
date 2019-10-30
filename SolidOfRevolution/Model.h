#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "MVPMatrix.h"
#include "Point2D.h"
#include "Segment.h"
#include "tbezier.h"
#include <vector>

using namespace std;

#pragma once
class Model
{
	const int countOfVerticesPerAPoint = 4;

	struct Point3D
	{
		double x;
		double y;
		double z;

		Point3D(){}

		Point3D(double x, double y, double z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		Point3D(Point2D point, double z)
		{
			x = point.x;
			y = point.y;
			this->z = z;
		}

		Point3D multiply(MVPMatrix m)
		{
			float* content = m.getContent();
			double newPoint[3] = 
			{ 
				content[0] * x + content[4] * y + content[8] * z, 
				content[1] * x + content[5] * y + content[9] * z,
				content[2] * x + content[6] * y + content[10] * z
			};
			return Point3D(newPoint[0], newPoint[1], newPoint[2]);
		}

		Point3D operator - (const Point3D& p)
		{
			return Point3D(x - p.x, y - p.y, z - p.z);
		}
	};

	vector<Point2D> basePoints;
	vector<Point3D> curvePoints;
	Point3D rotationAxis;

	GLfloat color[3] = { 0.5, 0.5, 0.5 };

	void preparePoint(Point3D point, vector<GLfloat>& vertices);
	void traversePoint(int pos, vector<GLuint>& indices);
	void preparePoints(vector<Point3D> points, vector<GLuint>& indices, vector<GLfloat>& vertices, int startI);
	void triangulizeCurve(vector<Point3D> curve, vector<GLuint>& curveIndices, int startI);
	void findCurveEnds(Point3D& begin, Point3D& end);
public:
	const int verCoordCount = 3;
	const int colorCount = 3;
	const int vertexSize = verCoordCount + colorCount;

	GLuint vao;
	GLuint ibo;
	GLuint vbo;

	vector<GLuint> baseIndices;
	vector<GLuint> curveIndices;
	vector<GLuint> modelIndices;

	vector<GLfloat> baseVertices;
	vector<GLfloat> curveVertices;
	vector<GLfloat> modelVertices;

	MVPMatrix m;

	double pointRadius;

	Model();

	void rotateAboutAxis(float degree);
	void rotate(float x, float y, float z, float degree);
	void rotateAboutX(float degree);
	void rotateAboutY(float degree);
	void rotateAboutZ(float degree);

	void addBasePoint(Point2D point);
	void clearBasePoints();

	void formCurve();
	void clearCurve();

	void formSolidOfRevolution(float rotationDegree);
	void clearSolidOfRevolution();

	void setColor(GLfloat r, GLfloat g, GLfloat b);
};