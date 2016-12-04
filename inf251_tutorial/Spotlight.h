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
		  fLinearAtt,
		  vIntensity;
public:
	Spotlight();

	glm::vec3 vColor,
		      vPosition,
		      vDirection;
	
	int vRadialAttenuation;

	void loadToUniformAt(GLuint shaderProgram, std::string uniformName);
	void toggleOnOff(bool);
	void setIntensity(float intensity);
	void increaseIntensity();
	void decreaseIntensity();
};