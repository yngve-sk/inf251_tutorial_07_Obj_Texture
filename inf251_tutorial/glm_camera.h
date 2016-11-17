#pragma once
#include <cmath>

#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;
using namespace std;

class GLMCamera
{
private:
	vec3 position,
		 target,
	     up;

	float fov, ar, zNear, zFar, zoom;

	float MOVEMENT_SPEED = 0.15f,
		ROTATIONAL_SPEED = 0.003f,
		ZOOM_SPEED = 0.05f;
	bool perspectiveProjection = true;

public:

	GLMCamera();
	void setAspectRatio(int width, int height);

	/* Compute world to projection matrix */
	mat4 computeCameraTransform();

	mat4 getWorldToViewMatrix();

	mat4 getViewToProjectionMatrix();

	vec3 getPosition();

	void switchPerspective();

	bool isProjectionPerspective();
	
	void moveForward();
	
	void moveBackwards();

	void strafeLeft();

	void strafeRight();
 
	void moveDown();

	void moveUp();

	void translate(const vec2& oldMousePosition, const vec2& newMousePosition);

	void rotate(const vec2& oldMousePosition, const vec2& newMousePosition);

	void flip();

	void adjustZoom(const vec2& oldMousePosition, const vec2& newMousePosition);

	void adjustFov(float delta);

	void adjustZNear(float delta);

	void adjustZFar(float delta);
};

