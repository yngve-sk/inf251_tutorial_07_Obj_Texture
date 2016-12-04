#include "CameraV2.h"


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
		 rotator = rotator * rotateAroundStrafe;

	vec3 newViewDirection = mat3(rotator) * viewDirection;

	float dotUpDownAbs = abs(dot(newViewDirection, UP));

	if(dotUpDownAbs < dotThreshold)
		viewDirection = newViewDirection;

	//std::cout << "dotThreshold: " << dotThreshold << ", dotUpDownAbs: "<< dotUpDownAbs << std::endl;
	

	oldMousePosition = newMousePosition;
}

void Camera::translate(const vec2& oldMousePosition, const vec2& newMousePosition) {
	position += viewDirection * MOVEMENT_SPEED * (oldMousePosition.y - newMousePosition.y);
	position += cross(viewDirection, UP) * MOVEMENT_SPEED * (newMousePosition.x - oldMousePosition.x);
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
	moveInDirection(MOVEMENT_SPEED, viewDirection);
//	position += MOVEMENT_SPEED * viewDirection;
}

void Camera::moveBackward() {
	moveInDirection(-MOVEMENT_SPEED, viewDirection);
//	position += -MOVEMENT_SPEED * viewDirection;
}

void Camera::strafeLeft() {
	moveInDirection(MOVEMENT_SPEED, strafeDirection);
//	position += MOVEMENT_SPEED * strafeDirection;
}

void Camera::strafeRight() {
	moveInDirection(-MOVEMENT_SPEED, strafeDirection);
//	position += -MOVEMENT_SPEED * strafeDirection;
}

void Camera::moveUp() {
	moveInDirection(MOVEMENT_SPEED, UP);
//	position += MOVEMENT_SPEED * UP;
}

void Camera::moveDown() {
	moveInDirection(MOVEMENT_SPEED, -UP);
//	position += -MOVEMENT_SPEED * UP;
}

void Camera::switchPerspective() {
	perspectiveProjection = !perspectiveProjection;
}

bool Camera::isProjectionPerspective() {
	return perspectiveProjection;
}

void Camera::moveInDirection(float speed, vec3 direction) {
	vec3 proposedPosition = position + speed*direction;
	if (bounds.isWithinBoundingBox(proposedPosition)) {
		position = proposedPosition;
	}
}

void Camera::generateCircularBezierAroundCurrentPosition(Path& curve, float hRadius, float dy) {
	// default: use 8 control points, link
	// point 8 to point 1

	
	const int nControlPoints = 8;
	float angleStepDegrees = 360 / nControlPoints;

	vec3 controlPoints[nControlPoints+1];

	for (int i = 0; i < nControlPoints; i++) {
		float x = hRadius * cos(angleStepDegrees * i);
		float z = hRadius * sin(angleStepDegrees * i);
		controlPoints[i] = (vec3(
			position.x + x, 
			position.y + dy, 
			position.z + z)
			);
	}

	controlPoints[nControlPoints] = controlPoints[0];

	curve.bezier(controlPoints, nControlPoints + 1, 300); // +1 to make it circualr!
}

void Camera::setLookAtPoint(vec3 point) {
	viewDirection = (point - position);
}
