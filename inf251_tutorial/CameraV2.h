#pragma once
#include <glm\glm.hpp>
#include <glm/gtx/transform.hpp>

class Camera {
	float fov, ar, zNear, zFar, zoom;

	float MOVEMENT_SPEED = 10.95f,
		ROTATIONAL_SPEED = 0.015f,
		ZOOM_SPEED = 0.05f;
	bool perspectiveProjection = true;

	glm::vec3 position;
	glm::vec3 viewDirection;
	const glm::vec3 UP;
	glm::vec2 oldMousePosition;
	glm::vec3 strafeDirection;
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
	bool isProjectionPerspective();
	void switchPerspective();
};
