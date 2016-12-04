#pragma once
#include <glm\glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "BoundingBox.h"
#include "Path.h"

class Camera {
	float fov, ar, zNear, zFar, zoom;

	float MOVEMENT_SPEED = 10.95f,
		ROTATIONAL_SPEED = 0.015f,
		ZOOM_SPEED = 0.05f;
	bool perspectiveProjection = true;

	// Specifies much up/down the camera is allowed
	// to look. dotThreshold is the cosine of the angle
	// a between the up[0,1,0] or down[0,-1,0]
	// and the viewing direction. I.e
	// the cosine must not exceed the threshold.
	// prevents weird behavior
	float dotThreshold = 0.95; 

	glm::vec3 position;
	glm::vec3 viewDirection;
	const glm::vec3 UP;
	glm::vec2 oldMousePosition;
	glm::vec3 strafeDirection;

	void moveInDirection(float speed, vec3 direction);

public:
	Camera();
	void mouseUpdate(const glm::vec2& newMousePosition);
	glm::mat4 getWorldToViewMatrix() const;
	glm::mat4 getViewToProjectionMatrix() const;

	void moveForward();
	void moveBackward();
	void strafeLeft();
	void strafeRight();
	void moveUp();
	void moveDown();
	glm::vec3 getPosition() const { return position; }
	void translate(const vec2& oldMousePosition, const vec2& newMousePosition);
	bool isProjectionPerspective();
	void switchPerspective();
	void setPosition(glm::vec3 newPos) {
		position = newPos;
	}
	BoundingBox bounds;

	void generateCircularBezierAroundCurrentPosition(Path& writeTo, float hRadius, float dy);
	void setLookAtPoint(vec3 point);
};
