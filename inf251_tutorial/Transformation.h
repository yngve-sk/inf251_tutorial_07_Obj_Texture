#pragma once
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <string>
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

using namespace glm;

#define PI 3.14159265

class Transformation {
private:
	mat4 rotationMatrix = mat4(), translationMatrix = mat4();
	mat4 scaleMatrix = mat4();
public:
	void translate(vec3);
	void setPosition(vec3);
	void adjustScale(float);
	void setScale(float);
	void resetScale();
	void rotate(float, glm::vec3);
	mat4 getTransformationMatrix();
	void loadToUniformLoc(GLuint);
	void flip(vec3);
};
