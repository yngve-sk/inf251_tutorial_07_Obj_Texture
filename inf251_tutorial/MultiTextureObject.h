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

class MultiTextureObject {

public:
	//MultiTextureObject(const char*, const char*);
	MultiTextureObject();
	ModelOBJ model;

	GLuint VBO = -1;
	GLuint IBO = -1;

	GLuint TextureObjects[64];

	Transformation transformation;

	bool usingBumpMapping;

	void drawObject(VertexGLLocs, MaterialGLLocs);
	void loadMaterials(const char*);
	void loadObject(const char*);

private:
	void loadMaterial(const char*, int, const ModelOBJ::Material&);
	void drawMesh(int, GLuint, GLuint, GLuint&, GLuint&, VertexGLLocs);
};
