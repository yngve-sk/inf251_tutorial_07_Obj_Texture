#include "SingleTextureObject.h"

SingleTextureObject::SingleTextureObject(const char* directory, 
										 const char* materialDirectory,
										 const char* bumpMap) {
	init(directory, materialDirectory, bumpMap);
}

SingleTextureObject::SingleTextureObject(){}

void SingleTextureObject::init(const char* directory,
		  const char* materialDirectory,
		  const char* bumpMap) {
		  loadObject(directory);
		  loadMaterial(materialDirectory);
		  loadBumpMap(bumpMap);
}

void SingleTextureObject::loadObject(const char* directory) {

	if (!model.import(directory)) {
		cerr << "Error: cannot load model: " << directory << endl;
		return;
	}

	cout << "Imported model: " << directory << endl;

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		model.getVertexBuffer(),
		GL_STATIC_DRAW);

	// IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		model.getNumberOfIndices() * sizeof(unsigned int),
		model.getIndexBuffer(),
		GL_STATIC_DRAW);

	model.normalize();

	return;
}

void SingleTextureObject::loadMaterial(const char* textureDirectory) {

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
	if (textureObject != 0)
		glDeleteTextures(1, &textureObject);
	glGenTextures(1, &textureObject);

	// Bind it as a 2D texture (note that other types of textures are supported as well)
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureObject);

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

void SingleTextureObject::loadBumpMap(const char* textureDirectory) {

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

void SingleTextureObject::drawObject(VertexGLLocs vertexGLLocs, MaterialGLLocs materialGLLocs) {

	ModelOBJ::Material material = model.getMaterial(0);

	// Set material parameters for house
	glUniform3f(materialGLLocs.aColorLoc, material.ambient[0], material.ambient[1], material.ambient[2]);
	glUniform3f(materialGLLocs.dColorLoc, material.diffuse[0], material.diffuse[1], material.diffuse[2]);
	glUniform3f(materialGLLocs.sColorLoc, material.specular[0], material.specular[1], material.specular[2]);
	glUniform1f(materialGLLocs.shineLoc, material.shininess);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureObject);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	//int normal_location = glGetUniformLocation(ShaderProgram, "normal_texture");
	//glUniform1i(normal_location, 1);
	glBindTexture(GL_TEXTURE_2D, bumpMapObject);

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
	glDrawElements(GL_TRIANGLES, model.getNumberOfIndices(), GL_UNSIGNED_INT, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}


