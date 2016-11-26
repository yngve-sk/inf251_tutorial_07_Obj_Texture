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

class SingleTextureObject {

public:
	ModelOBJ model;

	GLuint VBO = -1;
	GLuint IBO = -1;

	GLuint textureObject;
	GLuint bumpMapObject;

	Transformation transformation;

	bool usingBumpMapping;

	void drawObject(VertexGLLocs, MaterialGLLocs);
	void loadObject(const char*);
	void loadMaterial(const char*);
	void loadBumpMap(const char*);
	
	SingleTextureObject(const char*, const char*, const char*);

private:
};