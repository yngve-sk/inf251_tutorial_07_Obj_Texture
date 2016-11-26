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

class AnimatedTextureSquare {

public:

	const int numberOfVertices = 4;
	const int numberOfTriangles = 2;
	const int numberOfIndeces = 6;

	ModelOBJ::Vertex square[4];

	// Square
	int vertexNumber = 4;
	int numberOfFrames = 173;
	int canvasFrame = 0;
	int numberOfSteps = 10;
	int frameWait = 10;
	float width = 36.f*2.f,
		height = 18.5f*2.f;
	vec3 position{ 5.f, 20.f, -40.f };

	const char* textureDirectory;

	GLuint VBO = -1;
	GLuint IBO = -1;

	GLuint textureObjects[256];
	GLuint activeTextureObject;
	GLuint bumpMapObject;

	Transformation transformation;

	bool usingBumpMapping;
	bool animate;

	void drawObject(VertexGLLocs, MaterialGLLocs);
	void loadModel();
	void loadTextures();
	void loadTexture(const char*, GLuint&);
	void loadBumpMap(const char*);
	void toogleAnimate();
	void stepAnimation();

	AnimatedTextureSquare(vec3 position,
		float width,
		float height,
		int numberOfFrames,
		int numberOfSteps,
		int frameWait,
		const char* textureDirectory);

private:
	void loadTexture(GLuint*, const char*);
};