#pragma once

#include <glm/glm.hpp>
class CameraC
{
private:
	glm::vec3 position;
	glm::vec3 viewDirection;
	const glm::vec3 UP;
public:

	CameraC()
	{
	};

	glm::mat4 getWorldToViewMatrix();

	virtual ~CameraC()
	{
	}
};

