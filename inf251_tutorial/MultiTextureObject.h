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

using namespace std;
using namespace glm;

class MultiTextureObject {

public:
	ModelOBJ model;

	GLuint VBO = 0;
	GLuint IBO = 0;

	GLuint TextureObjects[64];

	mat4 LocalRotation;
	mat4 LocalTranslation;
	mat4 LocalScale;

	bool usingBumpMapping;

	void drawObject(VertexGLLocs, MaterialGLLocs);
	void loadMaterials(const char*);
	void loadObject(const char*);

private:
	void loadMaterial(const char*, GLuint&, int, const ModelOBJ::Material&);
	void drawMesh(int, GLuint, GLuint, GLuint&, GLuint&, VertexGLLocs);
};
