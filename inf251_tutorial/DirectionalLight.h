#pragma once
#include <glm\glm.hpp>
#include <string>
#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

class DirectionalLight {
private:
	glm::vec3 direction;
	float aIntensity, dIntensity, sIntensity;

	int bOn;
public:
	glm::vec3 aColor, dColor, sColor;
	
	void loadToUniformsInShader(GLuint shaderProgram, std::string uniformName);
};