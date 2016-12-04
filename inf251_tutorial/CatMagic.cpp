#pragma once
#include "CatMagic.h"
#include <glm\glm.hpp>

void CatMagic::stepMasterCat(vec3& position, vec3& direction) {
	position += speed * direction;
	cat.transformation.setPosition(position);
	
	direction.y += gravity;

	if (position.y >= -30) {
		direction = -direction;
		generateRandomDirection(direction);

		giveBirth();
	
	}
}

void CatMagic::step() {
	// 1. Step the master cat
	stepMasterCat(masterPosition, masterDirection);

	for (int i = 0; i < activeSubCats; i++) {
		displacements[i] += speed * directions[i];
		directions[i].y += gravity;

		if ((masterPosition + displacements[i]).y >= -30) {
			directions[i] = -directions[i];
			generateRandomDirection(directions[i]);
		}
	}
	//masterPosition += speed * masterDirection;
	////masterPosition = masterPosition - vec3(0, gravity, 0);
	//cat.transformation.setPosition(masterPosition);
	//
	//masterDirection.y += pow(gravity,2);
	//
	//if (masterPosition.y >= -30) {
	//	masterDirection = -masterDirection;
	//	generateRandomDirection(masterDirection);
	//	
	//	giveBirth();
	//}
	//
	//stepChildren();
}

void CatMagic::load() {
	cat.init("Objects\\cat\\cat.obj",
		"Objects\\cat\\cat_diff.png",
		"Objects\\cat\\cat_norm.png");
	//cat.transformation.rotate(180, vec3(1, 0, 0));
	//cat.transformation.translate(vec3(0, 0, 0));
	cat.transformation.setScale(500);
}

void CatMagic::init(vec3 initMasterPosition, float yTrajectoryUpAngle, float gravityValue, float speedValue, vec3 viewDirection, vec3 strafeDirection) {
	masterPosition = initMasterPosition;
	yTrajectoryAngleDegrees = yTrajectoryUpAngle;
	activeSubCats = 0;
	gravity = -gravityValue;
	speed = speedValue;

	

	//masterDirection = generateRandomDirection(yTrajectoryUpAngle);

	masterDirection = viewDirection; //rotate(-yTrajectoryUpAngle, strafeDirection) * vec4(viewDirection,1.);
	cat.transformation.setPosition(initMasterPosition);
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

void CatMagic::drawAll(VertexGLLocs vLocs, MaterialGLLocs mLocs,
	 mat4 VMatrix, mat4 PMatrix,
	GLint MMatrixLoc, GLint MVMatrixLoc, GLint MVPMatrixLoc) {
	cat.drawObject(vLocs, mLocs); // start just drawing one
	
	for (int i = 0; i < activeSubCats; i++) {

		mat4 MMMatrix = cat.transformation.getTransformationMatrix() * translate(displacements[i]);
		mat4 MVMatrix = VMatrix * MMMatrix;
		mat4 MVPMatrix = PMatrix * MVMatrix;

		glUniformMatrix4fv(MMatrixLoc, 1, GL_FALSE, &MMMatrix[0][0]);
		glUniformMatrix4fv(MVMatrixLoc, 1, GL_FALSE, &MVMatrix[0][0]);
		glUniformMatrix4fv(MVPMatrixLoc, 1, GL_FALSE, &MVPMatrix[0][0]);


		cat.drawObject(vLocs, mLocs);
	}
}

void CatMagic::drawChildren() {
	
}

void CatMagic::giveBirth() {
	if (activeSubCats >= MAX_SUBCATS) {
		return; // too many cats dont make any more... 
	}

	int index = activeSubCats++;

	displacements[index] = vec3(0); // Starts at master position
	directions[index] = vec3() + masterDirection;
	generateRandomDirection(directions[index]);
}


void CatMagic::generateRandomDirection(vec3& base) {
	float maxscale = 0.8;

	vec3 rVec = vec3(maxscale*rand() / RAND_MAX, -maxscale*rand() / RAND_MAX, maxscale*rand() / RAND_MAX);

	bool invertXZ = (rand() / RAND_MAX) < 0.5;

	base.y *= 0.8;
	base.x /= 1.4 * (invertXZ ? 1 : -1);
	base.z /= 1.4 * (invertXZ ? 1 : -1);

	base += rVec;
}

void CatMagic::stepChildren() {
	for (int i = 0; i < activeSubCats; i++) {
		// reduce their y-component
		directions[i].y += gravity;
		displacements[i] += speed * directions[i];

		if (masterPosition.y + displacements[i].y >= -30) {
			directions[i] = -directions[i];
			generateRandomDirection(directions[i]);
		}
	}
}