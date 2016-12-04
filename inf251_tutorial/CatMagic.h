#pragma once
#include "SingleTextureObject.h"
#include <random>
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>
#include "GLLocStructs.h"

static const int MAX_CATS = 20;

class CatMagic {
private:
	vec3 masterPosition; // The "source" position
	vec3 masterDirection;

	float yTrajectoryAngleDegrees;

	float gravity;
	float speed;
	
	vec3 displacements[MAX_CATS]; // relative positions of all but the master position
	vec3 directions[MAX_CATS];

	int activeSubCats = 0;

	vec3 generateRandomDirection(float yTrajectoryUpAngle);

public:
	SingleTextureObject cat;
	void step();
	void init(vec3 masterPosition, float yTrajectoryUpAngle, float gravityValue, float speed); // angle according to y is ok , random initial anglerelative to [0,0,1]
	void drawAll(VertexGLLocs vLocs, MaterialGLLocs mLocs);

};