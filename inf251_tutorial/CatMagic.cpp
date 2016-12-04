#pragma once
#include "CatMagic.h"
#include <glm\glm.hpp>

void CatMagic::step() {
	// 1. Step the master cat

	masterPosition += speed * masterDirection;
	masterDirection.y -= gravity;
}

void CatMagic::init(vec3 initMasterPosition, float yTrajectoryUpAngle, float gravityValue, float speedValue) {
	masterPosition = initMasterPosition;
	yTrajectoryAngleDegrees = yTrajectoryUpAngle;
	activeSubCats = 0;
	gravity = -gravityValue;
	speed = speedValue;

	masterDirection = generateRandomDirection(yTrajectoryUpAngle);

	cat.init("Objects\\cat\\cat.obj",
		"Objects\\cat\\cat_diff.png",
		"Objects\\cat\\cat_norm.png");
	cat.transformation.setScale(2.5);
}

vec3 CatMagic::generateRandomDirection(float yTrajectoryUpAngle) {
	float dx = rand() / RAND_MAX,
		  dz = rand() / RAND_MAX;

	// now there is a vector [dx, y, dz], dx and dz is known
	// find y so that the angle between up [0,1,0] is 
	// yTrajectoryAngle

	// dot([dx,y,dz], [0,1,0]) = cos(yTrajectoryAngle)
	// => y = cos(yTrajectoryAngle)

	float y = cos(yTrajectoryUpAngle * (PI / 180));

	return normalize(vec3(dx, y, dz));
}

void CatMagic::drawAll(VertexGLLocs vLocs, MaterialGLLocs mLocs) {
	cat.drawObject(vLocs, mLocs); // start just drawing one
}