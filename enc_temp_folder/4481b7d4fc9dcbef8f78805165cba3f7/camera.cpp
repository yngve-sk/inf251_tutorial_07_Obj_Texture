#include "glm_camera.h"

GLMCamera::GLMCamera() :
	position(0.f, 0.f, 0.f),
	target(0.f, 0.f, -1.f),
	up(0.f, 1.f, 0.f),
	fov(30.0f),
	ar(1.f),
	zNear(0.1f),
	zFar(4000.f),
	zoom(1.f) {};

void GLMCamera::setAspectRatio(int width, int height) {
	ar = (1.0f * width) / height;
}

mat4 GLMCamera::getViewToProjectionMatrix() {
	
	mat4 prj;
	if (perspectiveProjection) {
		prj = perspective(fov, ar, zNear, zFar);
	}
	else {
		prj = ortho(-1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar);
	}

	return prj;
}

mat4 GLMCamera::getCameraZoom() {
	return  glm::scale(vec3(zoom, zoom, 1.f));
}



mat4 GLMCamera::getWorldToViewMatrix() {
	vec3 t = normalize(target),
		u = normalize(up),
		r = cross(t, u); // switch t and u

	//cout << "t: " << t.x << ", " << t.y << ", " << t.z << endl;
	//cout << "u: " << u.x << ", " << u.y << ", " << u.z << endl;
	//cout << "r: " << r.x << ", " << r.y << ", " << r.z << endl;

	mat4 camR = mat4(r.x, r.y, r.z, 0.f,
		u.x, u.y, u.z, 0.f,
		-t.x, -t.y, -t.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	);

	mat4 camT = glm::translate(-position);

	return camR * camT;
}

vec3 GLMCamera::getPosition() {
	return position;
}

void GLMCamera::switchPerspective() {
	perspectiveProjection = !perspectiveProjection;
}

bool GLMCamera::isProjectionPerspective() {
	return perspectiveProjection;
}

void GLMCamera::moveForward() {
	position += target * MOVEMENT_SPEED;
}

void GLMCamera::moveBackwards() {
	position -= target * MOVEMENT_SPEED;
}

void GLMCamera::strafeLeft() {
	vec3 right = cross(target, up); //vec3(1, 0, 0);
	position -= right * MOVEMENT_SPEED;
}

void GLMCamera::strafeRight() {
	vec3 right = cross(target, up);
	position += right * MOVEMENT_SPEED;
}

void GLMCamera::moveDown() {
	position -= up * MOVEMENT_SPEED;
}

void GLMCamera::moveUp() {
	position += up * MOVEMENT_SPEED;
}

void GLMCamera::translate(const vec2& oldMousePosition, const vec2& newMousePosition) {
	position += target * MOVEMENT_SPEED * (oldMousePosition.y - newMousePosition.y);
	position += cross(target, up) * MOVEMENT_SPEED * (newMousePosition.x - oldMousePosition.x);
}

void GLMCamera::rotate(const vec2& oldMousePosition, const vec2& newMousePosition) {
	mat4 ry, rx;

	int dx = newMousePosition.x - oldMousePosition.x,
		dy = newMousePosition.y - oldMousePosition.y;

	// "horizontal" rotation
	ry = glm::rotate(ROTATIONAL_SPEED * dx, vec3(0, 1, 0));
	target = mat3(ry) * target;
	up = mat3(ry) * up;

	// "vertical" rotation
	rx = glm::rotate(ROTATIONAL_SPEED * dy, cross(target, up));
	up = mat3(rx) * up;
	target = mat3(rx) * target;



	//rx = glm::rotate(-ROTATIONAL_SPEED * dy, vec3(1, 0, 0));
	//ry = glm::rotate(-ROTATIONAL_SPEED * dx, vec3(0, 1, 0));

	//up = normalize(vec3(ry*rx*vec4(up, 1.f)));
	//target = normalize(vec3(ry*rx*vec4(target, 1.f)));

}

void GLMCamera::flip() {
	up[1] = -up[1];
}

void GLMCamera::adjustZoom(const vec2& oldMousePosition, const vec2& newMousePosition) {
	zoom = max(0.001f, zoom + ZOOM_SPEED * (newMousePosition.y - oldMousePosition.y));
}

void GLMCamera::adjustFov(float delta) {
	fov += delta;
}

void GLMCamera::adjustZNear(float delta) {
	zNear += delta;
}

void GLMCamera::adjustZFar(float delta) {
	zFar += delta;
}

void GLMCamera::loadToUniformsInShader(GLuint shaderProgram) {
	GLuint loc = glGetUniformLocation(shaderProgram, "camera_position");
	glUniform3fv(loc, 1, &position[0]);
}