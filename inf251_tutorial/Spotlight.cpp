#include "Spotlight.h"

Spotlight::Spotlight() : 
	bOn(1), fConeAngle(10),fLinearAtt(0),
	vColor(0.0f, 0.0f, 0.2f),
	vDirection(0, 0, -1),
	vPosition(0,0,0)
					{
	fConeCosine = glm::cos(fConeAngle);
}

void Spotlight::toggleOnOff() {
	bOn = (bOn == 1 ? 0 : 1);
}

void Spotlight::loadToUniformAt(GLuint shaderProgram, std::string uniformName) {
	std::string vDirLoc = uniformName + ".vDirection",
		vPositionLoc = uniformName + ".vPosition",
		vColorLoc = uniformName + ".vColor",

		bOnLoc = uniformName + ".bOn",

		fConeAngleLoc = uniformName + ".fConeAngle",
		fConeCosineLoc = uniformName + ".fConeCosine",
		fLinearAttLoc = uniformName + ".fLinearAtt";

	GLuint vDirSLoc = glGetUniformLocation(shaderProgram, &vDirLoc[0]),

		   vPositionSLoc = glGetUniformLocation(shaderProgram, &vPositionLoc[0]),
		   vColorSLoc = glGetUniformLocation(shaderProgram, &vColorLoc[0]),
		   
		   fConeAngleSLoc = glGetUniformLocation(shaderProgram, &fConeAngleLoc[0]),
		   fConeCosineSLoc = glGetUniformLocation(shaderProgram, &fConeCosineLoc[0]),
		   fLinearAttSLoc = glGetUniformLocation(shaderProgram, &fLinearAttLoc[0]),
		   
		   bOnSLoc = glGetUniformLocation(shaderProgram, &bOnLoc[0]);

		   glUniform3fv(vDirSLoc, 1, &vDirection[0]);
		   glUniform3fv(vPositionSLoc, 1, &vPosition[0]);
		   glUniform3fv(vColorSLoc, 1, &vColor[0]);

		   glUniform1f(fConeAngleSLoc, fConeAngle);
		   glUniform1f(fConeCosineSLoc, fConeCosine);
		   glUniform1f(fLinearAttSLoc, fLinearAtt);
		   
		   glUniform1i(bOnSLoc, bOn);
}