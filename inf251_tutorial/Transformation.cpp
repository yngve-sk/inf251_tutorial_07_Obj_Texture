#include "Transformation.h"


void Transformation::translate(glm::vec3 trans) {
	translationMatrix = glm::translate(translationMatrix, trans);
}

void Transformation::adjustScale(float newScale) {
	scaleMatrix = scaleMatrix * glm::scale(vec3(newScale));
}

void Transformation::setScale(float newScale) {
	scaleMatrix = glm::scale(vec3(newScale));
}

void Transformation::resetScale() {
	setScale(1.f);
}

void Transformation::rotate(float angle, glm::vec3 aroundAxis) {
	rotationMatrix = glm::rotate(rotationMatrix, angle, aroundAxis);
}

mat4 Transformation::getTransformationMatrix() {
	return scaleMatrix * rotationMatrix * translationMatrix;
}

void Transformation::loadToUniformLoc(GLuint loc) {
	mat4 transformation = getTransformationMatrix();
	glUniformMatrix4fv(loc, 1, GL_FALSE, &transformation[0][0]);
}