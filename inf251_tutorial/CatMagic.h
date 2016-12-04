#pragma once
#include "SingleTextureObject.h"
#include <random>
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>
#include "GLLocStructs.h"

static const int MAX_SUBCATS = 20;

class CatMagic {
private:
	BoundingBox bounds;
	vec3 masterPosition = vec3(0,-50,0); // The "source" position
	vec3 masterDirection;

	float yTrajectoryAngleDegrees;

	float gravity;
	float speed;
	
	vec3 displacements[MAX_SUBCATS]; // relative positions of all but the master position
	vec3 directions[MAX_SUBCATS];

	int activeSubCats = 0;

	vec3 generateRandomDirection(float yTrajectoryUpAngle);

public:
	SingleTextureObject cat;
	void step();
	void init(vec3 masterPosition, float yTrajectoryUpAngle, float gravityValue, float speed, vec3 viewDirection, vec3 strafeDirection);
	void load();
	
	// angle according to y is ok , random initial anglerelative to [0,0,1]
	void drawAll(VertexGLLocs vLocs, MaterialGLLocs mLocs, 
		mat4 VMatrix, mat4 PMatrix,
		GLint MMatrixLoc, GLint MVMatrixLoc, GLint MVPMatrixLoc);

	//void resetToMasterPosition(vec3 masterPosition, vec3 viewDirection, vec3 strafeDirection);

	/* Spawns new cats at current master position*/
	void giveBirth();
	void drawChildren();

	void generateRandomDirection(vec3& base);
	void stepChildren();
	void stepMasterCat(vec3& position, vec3& direction);
};