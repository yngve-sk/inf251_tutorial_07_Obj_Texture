#pragma once
#include <glm\glm.hpp>
#include <string>
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

using namespace glm;
class DirectionalLight {
private:
	glm::vec3 direction;
	float aIntensity, dIntensity, sIntensity;

	int bOn;
public:
	DirectionalLight();

	glm::vec3 aColor, dColor, sColor;
	
	void loadToUniformAt(GLuint shaderProgram, std::string uniformName);
	void toggleOnOff();

	/* Returns old intensity, useful for switching between two intensities*/
	vec3 setIntensity(vec3 newIntensity);
};