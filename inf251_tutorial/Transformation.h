#pragma once
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

using namespace glm;
class Transformation {
private:
	mat4 rotationMatrix = mat4(), translationMatrix = mat4();
	mat4 scaleMatrix = mat4();
public:
	void translate(vec3 trans);
	void setScale(float newScale);
	void rotate(float angle, glm::vec3 aroundAxis);
	mat4 getTransformationMatrix();
};