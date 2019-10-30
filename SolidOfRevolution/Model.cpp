#include "Model.h"

Model::Model()
{
	m = MVPMatrix::getIdentityMatrix();
	pointRadius = 0.0015;
}

void Model::rotateAboutAxis(float degree)
{
	m = m.rotate(rotationAxis.x, rotationAxis.y, rotationAxis.z, degree);
}

void Model::rotate(float x, float y, float z, float degree)
{
	m = m.rotate(x, y, z, degree);
}

void Model::rotateAboutX(float degree)
{
	m = m.rotateAboutX(degree);
}

void Model::rotateAboutY(float degree)
{
	m = m.rotateAboutY(degree);
}

void Model::rotateAboutZ(float degree)
{
	m = m.rotateAboutZ(degree);
}

void Model::preparePoint(Point3D point, vector<GLfloat>& vertices)
{
	double dx = -pointRadius, dy = -pointRadius;

	for (int j = 0; j < countOfVerticesPerAPoint; j++)
	{
		vertices.push_back(point.x + dx);
		vertices.push_back(point.y + dy);
		vertices.push_back(point.z);

		if (j % 3 == 0 || j % 3 == 2)
			dy *= -1;
		if (j % 3 == 1)
			dx *= -1;

		for (int i = 0; i < 3; i++)
			vertices.push_back(color[i]);
	}
}

void Model::traversePoint(int pos, vector<GLuint>& indices)
{
	indices.push_back(countOfVerticesPerAPoint * pos);
	indices.push_back(countOfVerticesPerAPoint * pos + 1);
	indices.push_back(countOfVerticesPerAPoint * pos + 2);
	indices.push_back(countOfVerticesPerAPoint * pos + 2);
	indices.push_back(countOfVerticesPerAPoint * pos + 3);
	indices.push_back(countOfVerticesPerAPoint * pos);
}

void Model::preparePoints(vector<Point3D> points, vector<GLuint>& indices, vector<GLfloat>& vertices, int startI)
{
	for (int i = 0; i < points.size(); i++)
	{
		preparePoint(points[i], vertices);
		traversePoint(startI + i, indices);
	}
}

void Model::triangulizeCurve(vector<Point3D> curve, vector<GLuint>& curveIndices, int startI)
{
	if (curvePoints.size() < 2)
		return;
	Point3D currentPoint = curvePoints[0];
	const double eps = 0.0005;
	for (int i = 0; i < curvePoints.size() - 1; i++)
	{
		int nextI = startI + i + 1;
		Point3D nextPoint = curvePoints[i + 1];
		if (currentPoint.x >= nextPoint.x && currentPoint.y <= nextPoint.y ||
			currentPoint.x <= nextPoint.x && currentPoint.y >= nextPoint.y)
		{
			curveIndices.push_back(countOfVerticesPerAPoint * (startI + i));
			curveIndices.push_back(countOfVerticesPerAPoint * nextI);
			curveIndices.push_back(countOfVerticesPerAPoint * (startI + i) + 2);
			curveIndices.push_back(countOfVerticesPerAPoint * (startI + i) + 2);
			curveIndices.push_back(countOfVerticesPerAPoint * nextI + 2);
			curveIndices.push_back(countOfVerticesPerAPoint * nextI);
		}
		else
		{
			curveIndices.push_back(countOfVerticesPerAPoint * (startI + i) + 3);
			curveIndices.push_back(countOfVerticesPerAPoint * nextI + 3);
			curveIndices.push_back(countOfVerticesPerAPoint * (startI + i) + 1);
			curveIndices.push_back(countOfVerticesPerAPoint * nextI + 3);
			curveIndices.push_back(countOfVerticesPerAPoint * nextI + 1);
			curveIndices.push_back(countOfVerticesPerAPoint * (startI + i) + 1);
		}
		currentPoint = nextPoint;
	}
}

void Model::findCurveEnds(Point3D& begin, Point3D& end)
{
	begin = curvePoints[0];
	end = curvePoints[0];
	for (int i = 1; i < curvePoints.size(); i++)
	{
		if (curvePoints[i].x < begin.x)
			begin = curvePoints[i];
		if (curvePoints[i].x > end.x)
			end = curvePoints[i];
	}
}

void Model::addBasePoint(Point2D point)
{
	basePoints.push_back(point);
	preparePoint(Point3D(point, 0.0), baseVertices);
	traversePoint(basePoints.size() - 1, baseIndices);
}

void Model::clearBasePoints()
{
	basePoints.clear();
	baseIndices.clear();
	baseVertices.clear();
}

void Model::formCurve()
{
	vector<Segment> curve;
	tbezierSO0(basePoints, curve);

	clearCurve();

	if (curve.size() == 0)
		return;

	for (auto s : curve)
	{
		for (int i = 0; i < RESOLUTION; ++i)
		{
			Point2D p = s.calc((double)i / (double)RESOLUTION);
			Point2D temp(p.x, p.y);
			curvePoints.push_back(Point3D(temp, 1.0));
		}
	}

	preparePoints(curvePoints, curveIndices, curveVertices, 0);
	triangulizeCurve(curvePoints, curveIndices, 0);
}

void Model::clearCurve()
{
	curvePoints.clear();
	curveIndices.clear();
	curveVertices.clear();
}

void Model::formSolidOfRevolution(float rotationDegree)
{
	clearSolidOfRevolution();

	Point3D begin, end;
	findCurveEnds(begin, end);
	rotationAxis = end - begin;
	Point2D normalizedAxis = Point2D(rotationAxis.x, rotationAxis.y);
	normalizedAxis.normalize();
	rotationAxis = Point3D(normalizedAxis, 0.0);

	vector<Point3D> currentCurve(curvePoints);
	MVPMatrix rotationMatrix =
		MVPMatrix::getIdentityMatrix()
		//.rotate(rotationAxis.x, rotationAxis.y, rotationAxis.z, rotationDegree);
		.rotateAboutX(rotationDegree);
	int rotatesCount = 360 / rotationDegree;
	int sizeOfCurve = curvePoints.size();
	int shift = 0;
	for (int i = 0; i < rotatesCount; i++)
	{
		vector<Point3D> nextCurve;
		for (int j = 0; j < sizeOfCurve - 1; j++)
			nextCurve.push_back(currentCurve[j].multiply(rotationMatrix));

		preparePoints(currentCurve, modelIndices, modelVertices, shift);
		triangulizeCurve(currentCurve, modelIndices, shift);

		//0: 0, 1, 2, ..., sizeOfCurve - 1
		//1: sizeOfCurve, sizeOfCurve+1, ... 2 * sizeOfCurve - 1
		int I = shift + i;
		int nextI = sizeOfCurve + i + 1;
		for (int j = 0; j < sizeOfCurve; j++)
		{
			modelIndices.push_back(countOfVerticesPerAPoint * (j + shift) + 1);
			modelIndices.push_back(countOfVerticesPerAPoint * (j + shift + sizeOfCurve) + 1);
			modelIndices.push_back(countOfVerticesPerAPoint * (j + shift + sizeOfCurve) + 3);
			modelIndices.push_back(countOfVerticesPerAPoint * (j + shift + sizeOfCurve) + 3);
			modelIndices.push_back(countOfVerticesPerAPoint * (j + shift) + 2);
			modelIndices.push_back(countOfVerticesPerAPoint * (j + shift) + 1);
		}

		shift += sizeOfCurve;
		currentCurve = nextCurve;
	}

	/*for (int j = 0; j < sizeOfCurve; j++)
	{
		modelIndices.push_back(countOfVerticesPerAPoint * j + 1);
		modelIndices.push_back(countOfVerticesPerAPoint * (shift + j - sizeOfCurve) + 1);
		modelIndices.push_back(countOfVerticesPerAPoint * (shift + j - sizeOfCurve) + 3);
		modelIndices.push_back(countOfVerticesPerAPoint * (shift + j - sizeOfCurve) + 3);
		modelIndices.push_back(countOfVerticesPerAPoint * j + 2);
		modelIndices.push_back(countOfVerticesPerAPoint * j + 1);
	}*/
}

void Model::clearSolidOfRevolution()
{
	modelIndices.clear();
	modelVertices.clear();
	m = MVPMatrix::getIdentityMatrix();
}

void Model::setColor(GLfloat r, GLfloat g, GLfloat b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
}