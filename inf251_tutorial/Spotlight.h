#pragma once
#include <glm\glm.hpp>
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>
#include <string>

class Spotlight {
private:
	int bOn;
	float fConeAngle, 
		  fConeCosine, 
		  fLinearAtt;
public:
	glm::vec3 vColor,
		      vPosition,
		      vDirection;

	void loadToUniformAt(GLuint shaderProgram, std::string uniformName);
	void toggleOnOff();
};