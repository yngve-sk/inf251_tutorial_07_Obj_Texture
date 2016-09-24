#pragma once

#include "CameraC.h"
#include "glm/gtx/transform.hpp"

CameraC::CameraC() : viewDirection(0.0f, 0.0f, -1.0f), UP(0.0f, 1.0f, 0.0f) {}

glm::mat4 CameraC::getWorldToViewMatrix() {
	return glm::lookAt(position, position + viewDirection, UP);
}