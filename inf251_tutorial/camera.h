#pragma once
#include <cmath>
#include "Vector3.h"
#include "Matrix4.h"
class Camera
{
private:
	Vector3f position,
		target,
		const up;

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

	Camera() {
		position.set(0.f, 0.f, 0.f);
		target.set(0.f, 0.f, -1.f);
		up.set(0.f, 1.f, 0.f);
		fov = 300.0f;
		ar = 1.f;
		zNear = 0.1f;
		zFar = 1000.f;
		zooming = 1.f;
	}

	Matrix4f computeCameraTransform() {
		Vector3f t = target.getNormalized();
		Vector3f u = up.getNormalized();
		Vector3f r = t.cross(u);
		Matrix4f camR(r.x(), r.y(), r.z(), 0.f,
			u.x(), u.y(), u.z(), 0.f,
			-t.x(), -t.y(), -t.z(), 0.f,
			0.f, 0.f, 0.f, 1.f);

		Matrix4f camT = Matrix4f::createTranslation(position);

		Matrix4f prj = Matrix4f::createPerspectivePrj(fov, ar, zNear, zFar);

		Matrix4f camZoom = Matrix4f::createScaling(zooming, zooming, 1.f);

		return camZoom * prj * camR * camT;
	}

	void rotate(int x0, int y0, int x, int y) {
		Matrix4f rx, ry;
		ry.rotate(0.1f * (x0 - x), Vector3f(1, 0, 0));
		target = ry * target;
		up = ry * up;

		rx.rotate(0.1f * (y0 - y), Vector3f(0, 1, 0));
		target = rx * target;
		up = rx * up;
	}

	void zoom(int x0, int y0, int x, int y) {
		int dx = abs(x0 - x),
			dy = abs(y0 - y);
		
		int maxValue = max(dx, dy);

		zooming = max(0.01f, maxValue);
	}
	
	void translate(int x0, int y0, int x, int y) {
		position += target * 0.003f * (y0 - y);
		position += target * 0.003f * (x0 - x);
	}
};

