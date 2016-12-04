#include "Path.h"
#include <stdlib.h>
#include <iostream>

using namespace glm;

void Path::bezier(vec3* controlPoints, int numControlPoints,
	int numBezierPoints) {
	//vec3 bezierCurvePoint;
	//free(bezierPoints);
	bezierPoints = new vec3[numBezierPoints];
	bezierPoints[0] = vec3(1, 1, 1);
	float u;
	int k;

	currentPointIndex = 0;
	numCurvePoints = numBezierPoints;

	if (Coefficients) {
		delete[] Coefficients;
	}

	Coefficients = new int[numControlPoints];

	computeBinomialCoeffs(numControlPoints-1);
	for (k = 0; k < numBezierPoints; k++) {
		u = (float) k / numBezierPoints;
		// store all points in an array
		computeBezierPoint(u, bezierPoints[k], numControlPoints, controlPoints);
	}

	int a = 2;

	delete[] Coefficients;
}

void Path::computeBinomialCoeffs(int n) {
	int k, j;
	int val;
	for (k = 0; k <= n; k++) {
		val = 1;
		//Coefficients[k] = 1;
		for (j = n; j >= k + 1; j--)
			val *= j;
		for (j = n - k; j >= 2; j--)
			val /= (float) j;

		//std::cout << std::to_string(val) << std::endl;
		Coefficients[k] = val;
		std::cout << "Coefficients[" << k << "] = " << Coefficients[k] << std::endl;
	}
}

void Path::computeBezierPoint(float u,
						vec3& bezierPoint, 
						int numControlPoints, 
						vec3* controlPoints) {
	int k, n = numControlPoints - 1;
	float bezierBlendFunction;

	float x = 0, y = 0, z = 0;
	
	for (k = 0; k < numControlPoints; k++) {
		bezierBlendFunction = Coefficients[k] * pow(u, k) * pow(1 - u, n - k);
		x += controlPoints[k].x * bezierBlendFunction;
		y += controlPoints[k].y * bezierBlendFunction;
		z += controlPoints[k].z * bezierBlendFunction;
	}
	//std::cout << "p = ( " << x << ", " << y << ", " << z << "), blend = " << bezierBlendFunction << " coeff[k] = " << Coefficients[k] << std::endl;;

	bezierPoint = vec3(x, y, z);
}

void Path::reverseDirection() {
	isReversed = !isReversed;
}

vec3 Path::getNextCurvePoint() {
	vec3 p = bezierPoints[currentPointIndex];
	//std::cout << currentPointIndex << " = ( " << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;;
	if (currentPointIndex == 0) {
		isReversed = false;
		return bezierPoints[++currentPointIndex];
	}
	else if (currentPointIndex == numCurvePoints - 1) {
		//isReversed = true; // reverse and start going back
		//return bezierPoints[--currentPointIndex];
		isReversed = false;
		currentPointIndex = 0;
		return bezierPoints[0];
	}
	else {
		return isReversed ?
			bezierPoints[--currentPointIndex] :
			bezierPoints[++currentPointIndex];
	}
}