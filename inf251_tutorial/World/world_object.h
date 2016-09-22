#pragma once

#include "../Model/Vector3.h"
#include "../Matrix4.h"
#include "../Model/model_obj.h"

class WorldObject {
private:
	Matrix4f rotate, scale;
	Vector3f translate;
	Vector3f anchor;
	Vector3f size;

	const ModelOBJ& model;

public:

	WorldObject(const ModelOBJ& model) {

	}

	void f() {

	}
};