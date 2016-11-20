#pragma once
#include <glm/glm.hpp>
#include "model_obj.h"
#include <gl/glut.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include ""

// animated multitexture vertex
// multitexture obj
// single texture obj

using namespace glm;
class ModleOBJObjectContainer {
private:
	ModelOBJ obj;
	mat4 rotate, translate;
	float scale;
	int useBumpMapping;

	GLuint IBO = -1;
	GLuint VBO = -1;

	int activeTextureIndex;
	GLuint textureObject[64];
	MaterialParameters materials[64];

};