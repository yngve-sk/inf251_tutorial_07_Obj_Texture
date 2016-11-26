#pragma once
#include <glm\glm.hpp>
#include "Transformation.h"

class TexturedAnimatedRectangle {
private:
	glm::vec3 origin;
	glm::vec3 line1, line2;
	void loadObject();

public:
	Transformation modelToWorld;

	GLuint VBO = -1;
	GLuint IBO = -1;

	void drawObject(VertexGLLocs);
	

};