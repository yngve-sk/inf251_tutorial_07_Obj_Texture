#include "CameraV2.h"
#include <glm\gtx\transform.hpp>

using namespace glm;
Camera::Camera() :
	viewDirection(0.f, 0.f, -1.f),
	position(0.f, 0.f, 0.f),
	UP(0.0f, 1.0f, 0.0f), fov(30.0f),
	ar(1.f),
	zNear(0.1f),
	zFar(10000.f),
	zoom(1.f) {
}

void Camera::mouseUpdate(const vec2& newMousePosition) {
	vec2 dxdy = newMousePosition - oldMousePosition;
	if (length(dxdy) > 50.0f)
	{
		oldMousePosition = newMousePosition;
		return;
	}
	strafeDirection = cross(viewDirection, UP);

	
	mat4 rotateAroundUp = rotate(-dxdy.x * ROTATIONAL_SPEED, UP),
		 rotateAroundStrafe = rotate(-dxdy.y * ROTATIONAL_SPEED, strafeDirection);

	mat4 rotator = rotateAroundUp;
	if(UP.y/glm::length(UP) > 0.1)
		rotator = rotator * rotateAroundStrafe;

	viewDirection = mat3(rotator) * viewDirection;

	oldMousePosition = newMousePosition;
}

mat4 Camera::getWorldToViewMatrix() const {
	return lookAt(position, position + viewDirection, UP);
}

mat4 Camera::getViewToProjectionMatrix() const{
	mat4 prj;
	if (perspectiveProjection) {
		prj = perspective(fov, ar, zNear, zFar);
	}
	else {
		prj = ortho(-1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar);
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
