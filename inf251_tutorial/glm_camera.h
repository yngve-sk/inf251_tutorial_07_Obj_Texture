#pragma once
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

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

	GLMCamera() :
		position(0.f, 0.f, 0.f),
		target(0.f, 0.f, 1.f),
		up(0.f, -1.f, 0.f),
		fov(30.0f),
		ar(1.f),
		zNear(0.1f),
		zFar(4000.f),
		zoom(1.f) {}

	void setAspectRatio(int width, int height) {
		ar = (1.0f * width) / height;
	}

	mat4 computeCameraTransform() {
		vec3 t = normalize(target),
			u = normalize(up),
			r = cross(t, u);

		mat4 camR = mat4(r.x,   r.y,  r.z, 0.f,
						 u.x,   u.y,  u.z, 0.f,
						 -t.x, -t.y, -t.z, 0.f,
						  0.f,  0.f,  0.f, 1.f
		);

		mat4 camT = glm::translate(-position);

		mat4 prj;
		if (perspectiveProjection) {
			prj = perspective(fov, ar, zNear, zFar);
		} else {
			prj = ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 100.0f);
		}
		
		mat4 camZoom = glm::scale(vec3(zoom, zoom, 1.f));

//		return camZoom * prj * camR * camT;
		return camZoom * prj * camR * camT;
	}

	vec3 getPosition() {
		return position;
	}

	void switchPerspective() {
		perspectiveProjection = !perspectiveProjection;
	}

	bool isProjectionPerspective() {
		return perspectiveProjection;
	}
	
	void moveForward() {
		position += target * MOVEMENT_SPEED;
	}
	
	void moveBackwards() {
		position -= (target * MOVEMENT_SPEED);
	}

	void strafeLeft() {
		vec3 right = normalize(cross(target, up)); //vec3(1, 0, 0);
		position += right * MOVEMENT_SPEED;
	}

	void strafeRight() {
		vec3 right = normalize(cross(target, up));
		position -= right * MOVEMENT_SPEED;
	}
 
	void moveDown() {
		position -= up * MOVEMENT_SPEED;
	}

	void moveUp() {
		position += up * MOVEMENT_SPEED;
	}

	void translate(const vec2& oldMousePosition, const vec2& newMousePosition) {
		position += target * MOVEMENT_SPEED * (oldMousePosition.y - newMousePosition.y);
		position += cross(target, up) * MOVEMENT_SPEED * (newMousePosition.x - oldMousePosition.x);
	}

	void rotate(const vec2& oldMousePosition, const vec2& newMousePosition) {
	//mat4 ry, rx;
	//
		int dx = oldMousePosition.x - newMousePosition.x,
			dy = oldMousePosition.y - newMousePosition.y;
	//
	//ry = glm::rotate(ROTATIONAL_SPEED * dx, vec3(0,1,0));
	//target = mat3(ry) * target;
	//up = mat3(ry) * up;
	//
	//rx = glm::rotate(-ROTATIONAL_SPEED * dy, vec3(1, 0, 0));
	//up = mat3(rx) * up;
	//target = vec3(rx * vec4(target, 1.0f));

		float rotY = ROTATIONAL_SPEED * dx,
			rotX = ROTATIONAL_SPEED * dy;

		mat4 rx, rz, ry;
		rx = mat4(1.f,	   0.f,			 0.f, 0.f, 
				  0.f, cos(rotX), -sin(rotX), 0.f,
				  0.f, sin(rotX),  cos(rotX), 0.f,
				  0.f,		 0.f,			 0.f, 1.f
			);

		ry = mat4( cos(rotY), 0.f, sin(rotY), 0.f, 
						   0.f, 1.f,		 0.f, 0.f,
				  -sin(rotY), 0.f, cos(rotY), 0.f,
						   0.f, 0.f,		 0, 1.f
			);

		mat4 rt = ry*rx;

		// ry * up, rx * up
		up = mat3(rt)*up;
		up.x = 0; // no rotation around z-axis, //TODO!
		target = vec3(rt * vec4(target, 1.0f));
	}

	void adjustZoom(const vec2& oldMousePosition, const vec2& newMousePosition) {
		zoom = max(0.001f, zoom + ZOOM_SPEED * (newMousePosition.y - oldMousePosition.y));
	}

	void adjustFov(float delta) {
		fov += delta;
	}

	void adjustZNear(float delta) {
		zNear += delta;
	}

	void adjustZFar(float delta) {
		zFar += delta;
	}
};

