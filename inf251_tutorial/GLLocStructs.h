#pragma once
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

struct MaterialGLLocs {
	GLint aColorLoc = -1;
	GLint dColorLoc = -1;
	GLint sColorLoc = -1;
	GLint shineLoc = -1;
};

struct VertexGLLocs {
	GLint posLoc, 
		  texLoc, 
		  normalLoc;
};