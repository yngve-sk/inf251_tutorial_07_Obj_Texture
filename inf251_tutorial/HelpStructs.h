#pragma once
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

struct MaterialGLLocs {
	GLint MaterialAColorLoc = -1;
	GLint MaterialDColorLoc = -1;
	GLint MaterialSColorLoc = -1;
	GLint MaterialShineLoc = -1;
};

struct VertexGLLocs {
	GLint posLoc, 
		  texLoc, 
		  normalLoc;
};