#include "Spotlight.h"

Spotlight::Spotlight() : 
	bOn(1), fConeAngle(0), fConeCosine(0), fLinearAtt(0)
					{}

void Spotlight::toggleOnOff() {
	bOn = (bOn == 1 ? 0 : 1);
}

void Spotlight::loadToUniformAt(GLuint shaderProgram, std::string uniformName) {
	std::string vDirLoc = uniformName + ".direction",

		vPositionLoc = uniformName + ".dColor",
		vColorLoc = uniformName + ".aColor",

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