#pragma once
#include <glm\glm.hpp>


class Path {
private:
	glm::vec3 points[100]; //max 100 points
	int numPoints = 0;
public:
	void setPoints(glm::vec3 start, int numPoints);
	//glm::vec3 getNextPoint();
	void interpolate();
};