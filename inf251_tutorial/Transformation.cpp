#include "Transformation.h"


void Transformation::translate(glm::vec3 trans) {
	translationMatrix = glm::translate(translationMatrix, trans);
}

void Transformation::setScale(float newScale) {
	scaleMatrix = scaleMatrix * glm::scale(vec3(newScale));
}

void Transformation::rotate(float angle, glm::vec3 aroundAxis) {
	rotationMatrix = glm::rotate(rotationMatrix, angle, aroundAxis);
}

mat4 Transformation::getTransformationMatrix() {
	return scaleMatrix * rotationMatrix * translationMatrix;
}