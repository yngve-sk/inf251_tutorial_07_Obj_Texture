#pragma once
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

class GLMCamera
{
private:
	vec3 position,
		viewDirection,
		up;

	float fov, ar, zNear, zFar, zooming;
	
	inline int max(int f1, int f2) {
		return f1 > f2 ? f1 : f2;
	}

	inline float max(float f1, int f2) {
		return f1 > f2 ? f1 : f2;
	}

	inline float max(float f1, float f2) {
		return f1 > f2 ? f1 : f2;
	}
public:

	GLMCamera() :
		position(0.f, 0.f, 0.f),
		viewDirection(0.f, 0.f, -1.f),
		up(0.f, 1.f, 0.f),
		fov(300.0f),
		ar(1.f),
		zNear(0.1f),
		zFar(1000.f),
		zooming(1.f) {}
	

	fmat4 computeCameraTransform() {
		return glm::lookAt(position, position + viewDirection, up);
	}

	void rotate(int x0, int y0, int x, int y) {
		glm::vec2 mouseDelta = vec2((x0 - x), (y0 - y));
		viewDirection = mat3(glm::rotate(mouseDelta.x, up)) * viewDirection;
	}

	void zoom(int x0, int y0, int x, int y) {
		int dx = abs(x0 - x),
			dy = abs(y0 - y);
		
		int maxValue = max(dx, dy);

		zooming = max(0.01f, maxValue);
	}
	
	void translate(int x0, int y0, int x, int y) {
		position += viewDirection * 0.003f * (float)(y0 - y);
		position += viewDirection * 0.003f * (float)(x0 - x);
	}
};

