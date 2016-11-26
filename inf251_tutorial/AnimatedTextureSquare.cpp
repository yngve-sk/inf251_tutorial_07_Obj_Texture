#include "AnimatedTextureSquare.h"

AnimatedTextureSquare::AnimatedTextureSquare(vec3 position,
											float width, 
											float height, 
											int numberOfFrames, 
											int numberOfSteps, 
											int frameWait, 
											const char* textureDirectory) :
	position(position),
	width(width),
	height(height),
	numberOfFrames(numberOfFrames),
	numberOfSteps(numberOfSteps),
	frameWait(frameWait),
	textureDirectory(textureDirectory){

	loadSquare();
	loadTextures();
}

void AnimatedTextureSquare::loadSquare() {


	square[0].position[0] = position.x;
	square[0].position[1] = position.y;
	square[0].position[2] = position.z;
							
	square[1].position[0] = position.x + width;
	square[1].position[1] = position.y;
	square[1].position[2] = position.z;
	
	square[2].position[0] = position.x + width;
	square[2].position[1] = position.y + height;
	square[2].position[2] = position.z;
	
	square[3].position[0] = position.x;
	square[3].position[1] = position.y + height;
	square[3].position[2] = position.z;
	
	square[0].texCoord[0] = 0.f;
	square[0].texCoord[1] = 0.f;
	
	square[1].texCoord[0] = 1.f;
	square[1].texCoord[1] = 0.f;
	
	square[2].texCoord[0] = 1.f;
	square[2].texCoord[1] = 1.f;
	
	square[3].texCoord[0] = 0.f;
	square[3].texCoord[1] = 1.f;

	// Generate a VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		4 * sizeof(ModelOBJ::Vertex),
		square,
		GL_STATIC_DRAW);

	// Create an array of indices representing the triangles (faces of the cube)
	unsigned int canvasTris[3 * 4] = {
		0, 1, 2,
		2, 3, 0
	};

	// Create an IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		3 * vertexNumber * sizeof(unsigned int),
		canvasTris,
		GL_DYNAMIC_DRAW);

	return;
}

void AnimatedTextureSquare::loadTextures() {

	int step = sizeof(GLuint);

	for (int i = 1; i < numberOfFrames + 1; i++) {
		string path = textureDirectory + std::to_string(i) + ".png";
		loadTexture(&path[0], textureObjects[i - 1]);
		//cout << "loading texture " << i << " filepath: " << path << endl;
	}

	//print array
	for (int i = 0; i < numberOfFrames; i++) {
		//cout << "arr[" << std::to_string(i) << "] = " << start[i] << endl;
	}
}

void AnimatedTextureSquare::loadTexture(const char* texturePath, GLuint& textureObject) {
	
	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0,
		TextureHeight = 0;

	if (TextureData != nullptr)
		free(TextureData);
	unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
		texturePath, LCT_RGB, 8);

	if (fail != 0) {
		cerr << "Error: cant load the texture at loc " << textureDirectory << endl;
	}

	if (textureObject != 0)
		glDeleteTextures(1, &textureObject);
	glGenTextures(1, &textureObject);

	// Bind it as a 2D texture (note that other types of textures are supported as well)
	glBindTexture(GL_TEXTURE_2D, textureObject);

	// Set the texture data
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGB,		// remember to check this
		TextureWidth, TextureHeight, 0,
		GL_RGB,		// remember to check this
		GL_UNSIGNED_BYTE, TextureData);

	// Configure texture parameter
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void AnimatedTextureSquare::loadBumpMap(const char* textureDirectory) {

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0;
	unsigned int TextureHeight = 0;

	cout << "loading material: " << textureDirectory << endl;

	// Load the texture
	if (TextureData != nullptr)
		free(TextureData);

	//cout << "round " << i << " trying to load @ file path " << "House-Model\\" << model.getMaterial(i).colorMapFilename.c_str() << endl;
	unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
		textureDirectory,
		LCT_RGB,
		8); // Remember to check the last 2 parameters
	if (fail != 0) {
		cerr << "Error: cannot load texture file "
			<< textureDirectory << endl;
		return;
	}

	// Create the texture object
	if (bumpMapObject != 0)
		glDeleteTextures(1, &bumpMapObject);
	glGenTextures(1, &bumpMapObject);

	// Bind it as a 2D texture (note that other types of textures are supported as well)
	glBindTexture(GL_TEXTURE_2D_ARRAY, bumpMapObject);

	// Set the texture data
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,			// remember to check this
		TextureWidth,
		TextureHeight,
		0,
		GL_RGB,			// remember to check this
		GL_UNSIGNED_BYTE,
		TextureData
		);

	// Configure texture parameter
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void AnimatedTextureSquare::drawObject(VertexGLLocs vertexGLLocs, MaterialGLLocs materialGLLocs) {

	//ModelOBJ::Material material = model.getMaterial(0);

	// Set material parameters for house
	//glUniform3f(materialGLLocs.aColorLoc, material.ambient[0], material.ambient[1], material.ambient[2]);
	//glUniform3f(materialGLLocs.dColorLoc, material.diffuse[0], material.diffuse[1], material.diffuse[2]);
	//glUniform3f(materialGLLocs.sColorLoc, material.specular[0], material.specular[1], material.specular[2]);
	//glUniform1f(materialGLLocs.shineLoc, material.shininess);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, activeTextureObject);

	if (usingBumpMapping) {
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		//int normal_location = glGetUniformLocation(ShaderProgram, "normal_texture");
		//glUniform1i(normal_location, 1);
		glBindTexture(GL_TEXTURE_2D, bumpMapObject);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glVertexAttribPointer(vertexGLLocs.posLoc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(vertexGLLocs.texLoc, 2, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
	if (vertexGLLocs.normalLoc != -1) {
		glVertexAttribPointer(vertexGLLocs.normalLoc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(5 * sizeof(float)));
	}
	glDrawElements(GL_TRIANGLES, numberOfVertices, GL_UNSIGNED_INT, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	if (usingBumpMapping) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

void AnimatedTextureSquare::toogleAnimate() {
	animate = !animate;
}

void AnimatedTextureSquare::stepAnimation() {

	if (--numberOfSteps == 0) {
		canvasFrame = (++canvasFrame) % 173;
		numberOfSteps = frameWait;
	}
	activeTextureObject = textureObjects[canvasFrame];
}


