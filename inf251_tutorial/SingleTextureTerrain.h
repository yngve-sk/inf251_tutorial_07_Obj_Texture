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
#include "vector3.h"

using namespace std;
using namespace glm;

class SingleTextureTerrain {

public:

	//From Sergej
	struct TerrainVertex {
		Vector3f position;
		float tex_coords[2];
		Vector3f normals;
	};

	GLuint VBO = -1;
	GLuint IBO = -1;

	GLuint textureObject;
	GLuint bumpMapObject;
	GLuint bumpMapObjects[10];
	GLuint activeBumpMapObject;

	Transformation transformation;

	bool usingBumpMapping;

	//Terrain Object 
	int colsNum, rowsNum, NO_DATA, numberOfPoints, numberOfTriangles;
	double xLowLeft, yLowLeft, cellSize;
	float *heights;
	Vector3f *vertices;
	unsigned int *trIndices;

	// Animated Texture
	int vertexNumber = 4;
	int numberOfFrames = 173;
	int terrainFrame = 0;
	int numberOfSteps = 10;
	int frameWait = 10;

	TerrainVertex *terrainVertices;
	float *texture_coords;
	Vector3f *vertexNormals;

	bool animate;

	void drawObject(VertexGLLocs, MaterialGLLocs, GLint, GLint);
	void loadTerrain(const char*);
	void generateTerrainBuffers();
	void initTriangles();
	void loadBumpMap(const char*);
	void loadBumpMaps(int numberOfFrames, int frameWait, const char* textureDirectory);
	void loadActiveBumpMap(const char* texturePath, GLuint& textureObject);

	void init(const char*, const char*);
	SingleTextureTerrain();

	void toogleAnimate();
	void stepAnimation();

private:
	void createVertices();
	void createTriangles(int, int, int, int, int);
	int rc2index(int, int);
};
