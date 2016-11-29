#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

#include <cstdio>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/extend.hpp>

#include "model_obj.h"
#include "lodepng.h"
#include "GLLocStructs.h"
#include "Transformation.h"

using namespace std;
using namespace glm;

class SingleTextureTerrain {

public:
	ModelOBJ model;

	GLuint VBO = -1;
	GLuint IBO = -1;

	GLuint textureObject;
	GLuint bumpMapObject;

	Transformation transformation;

	bool usingBumpMapping;

	//Terrain Object 
	int colsNum, rowsNum, NO_DATA, numberOfPoints, numberOfTriangles;
	double xLowLeft, yLowLeft, cellSize;
	float *heights;
	vec3 *vertices;
	unsigned int *trIndices;

	void drawObject(VertexGLLocs, MaterialGLLocs);
	void loadObject(const char*);
	void loadTerrain(const char*);
	//void loadBumpMap(const char*);

	//SingleTextureTerrain(const char*, const char*);
	SingleTextureTerrain();

private:
	void createVertices();
	void createTriangles(int, int, int, int, int);
	int rc2index(int, int);
};