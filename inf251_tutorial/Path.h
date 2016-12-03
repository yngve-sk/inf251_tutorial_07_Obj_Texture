#pragma once
#include <glm\glm.hpp>

using namespace glm;
class Path {
private:
	int* Coefficients; // Binomial coefficients
	
	vec3* bezierPoints; // points of curve is stored here
	int currentPointIndex = 0;

	int numCurvePoints = 0;

	bool isReversed = false;


public:
	void computeBinomialCoeffs(int n);

	void bezier(vec3* controlPoints, int numControlPoints, 
		int numBezierPoints);
	
	void computeBezierPoint(float u, 
						    vec3& bezierPoint, 
						    int numControlPoints, 
						    vec3* controlPoints, 
						    int* Coefficients);

	void reverseDirection();
	vec3 getNextCurvePoint();
};