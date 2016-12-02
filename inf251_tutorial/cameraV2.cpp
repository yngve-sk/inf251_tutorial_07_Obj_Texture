#include "CameraV2.h"
#include <glm\gtx\transform.hpp>

Camera::Camera() :
	viewDirection(0.f, 0.f, -1.f),
	position(0.f, 0.f, 0.f),
	UP(0.0f, 1.0f, 0.0f), fov(30.0f),
	ar(1.f),
	zNear(0.1f),
	zFar(10000.f),
	zoom(1.f) {
}

void Camera::mouseUpdate(const glm::vec2& newMousePosition) {
	glm::vec2 dxdy = newMousePosition - oldMousePosition;
	if (glm::length(dxdy) > 50.0f)
	{
		oldMousePosition = newMousePosition;
		return;
	}
	strafeDirection = glm::cross(viewDirection, UP);
	glm::mat4 rotator = glm::rotate(-dxdy.x * ROTATIONAL_SPEED, UP) *
		glm::rotate(-dxdy.y * ROTATIONAL_SPEED, strafeDirection);

	viewDirection = glm::mat3(rotator) * viewDirection;

	oldMousePosition = newMousePosition;
}

glm::mat4 Camera::getWorldToViewMatrix() const {
	return glm::lookAt(position, position + viewDirection, UP);
}

glm::mat4 Camera::getViewToProjectionMatrix() const{
	glm::mat4 prj;
	if (perspectiveProjection) {
		prj = glm::perspective(fov, ar, zNear, zFar);
	}
	else {
		prj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar);
	}

	return prj;
}

void Camera::moveForward() {
	position += MOVEMENT_SPEED * viewDirection;
}

void Camera::moveBackward() {
	position += -MOVEMENT_SPEED * viewDirection;
}

void Camera::strafeLeft() {
	position += MOVEMENT_SPEED * strafeDirection;
}

void Camera::strafeRight() {
	position += -MOVEMENT_SPEED * strafeDirection;
}

void Camera::moveUp() {
	position += MOVEMENT_SPEED * UP;
}

void Camera::moveDown() {
	position += -MOVEMENT_SPEED * UP;
}

void Camera::switchPerspective() {
	perspectiveProjection = !perspectiveProjection;
}

bool Camera::isProjectionPerspective() {
	return perspectiveProjection;
}
