#include "DirectionalLight.h"

using namespace glm;
DirectionalLight::DirectionalLight() : 
				direction(vec3(1,1,1)),
				aColor(vec3(0.3, 0.3, 0.3)),
				dColor(vec3(0.1, 0.1, 0.5)),
				sColor(vec3(0.4, 0.4, 0.4)),
				aIntensity(0.03),
				dIntensity(0.03),
				sIntensity(0.03),
				bOn(1) {}

void DirectionalLight::loadToUniformAt(GLuint shaderProgram, std::string uniformName) {
	std::string dirLoc = uniformName + ".direction",

		aColorLoc = uniformName + ".aColor",
		dColorLoc = uniformName + ".dColor",
		sColorLoc = uniformName + ".sColor",

		aIntensityLoc = uniformName + ".aIntensity",
		dIntensityLoc = uniformName + ".dIntensity",
		sIntensityLoc = uniformName + ".sIntensity",

		bOnLoc = uniformName + ".bOn";

	GLuint dirSLoc = glGetUniformLocation(shaderProgram, &dirLoc[0]),

		aColorSLoc = glGetUniformLocation(shaderProgram, &aColorLoc[0]),
		dColorSLoc = glGetUniformLocation(shaderProgram, &dColorLoc[0]),
		sColorSLoc = glGetUniformLocation(shaderProgram, &sColorLoc[0]),

		aIntensitySLoc = glGetUniformLocation(shaderProgram, &aIntensityLoc[0]),
		dIntensitySLoc = glGetUniformLocation(shaderProgram, &dIntensityLoc[0]),
		sIntensitySLoc = glGetUniformLocation(shaderProgram, &sIntensityLoc[0]),

		bOnSLoc = glGetUniformLocation(shaderProgram, &bOnLoc[0]);

	glUniform3fv(dirSLoc, 1, &direction[0]);
	glUniform3fv(aColorSLoc, 1, &direction[0]);
	glUniform3fv(dColorSLoc, 1, &direction[0]);
	glUniform3fv(sColorSLoc, 1, &direction[0]);

	glUniform1f(aIntensitySLoc, aIntensity);
	glUniform1f(dIntensitySLoc, dIntensity);
	glUniform1f(sIntensitySLoc, sIntensity);

	glUniform1i(bOnSLoc, bOn);


}

void DirectionalLight::toggleOnOff() {
	bOn = (bOn == 1 ? 0 : 1);
}