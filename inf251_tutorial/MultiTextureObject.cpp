#include "MultiTextureObject.h"

MultiTextureObject::MultiTextureObject( const char* directory,
										const char* materialDirectory) {
	loadObject(directory);
	loadMaterials(materialDirectory);
}

void MultiTextureObject::loadObject(const char* directory) {

	if (!model.import(directory)) {
		cerr << "Error: cannot load model." << endl;
		return;
	}

	cout << "Imported model..." << endl;

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

void MultiTextureObject::loadMaterials(const char* TextureDirectory) {
	for (int i = 0; i < model.getNumberOfMeshes(); i++) {
		ModelOBJ::Mesh m = model.getMesh(i);
		ModelOBJ::Material mat = *(m.pMaterial);

		//loadMaterial("House-Model\\", TextureObjects[i], *m.pMaterial);
		loadMaterial(TextureDirectory, i, mat);
	}
}

void MultiTextureObject::loadMaterial(const char* TextureDirectory, int i, const ModelOBJ::Material &material) {

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0;
	unsigned int TextureHeight = 0;

	cout << "loading material: " << material.name << endl;

	// if the current material has a texture
	if (material.colorMapFilename != "") {

		// Load the texture
		if (TextureData != nullptr)
			free(TextureData);

		//cout << "round " << i << " trying to load @ file path " << "House-Model\\" << model.getMaterial(i).colorMapFilename.c_str() << endl;
		unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
			(TextureDirectory + material.colorMapFilename).c_str(),
			LCT_RGB, 8); // Remember to check the last 2 parameters
		if (fail != 0) {
			cerr << "Error: cannot load texture file "
				<< material.colorMapFilename << endl;
			return;
		}

		// Create the texture object
		if (TextureObjects[i] != 0)
			glDeleteTextures(1, &TextureObjects[i]);
		glGenTextures(1, &TextureObjects[i]);

		// Bind it as a 2D texture (note that other types of textures are supported as well)
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureObjects[i]);

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
}

void MultiTextureObject::drawObject(VertexGLLocs vertexGLLocs, MaterialGLLocs materialGLLocs) {

	/* Code for multiple textures */
	for (int i = 0; i < model.getNumberOfMeshes(); i++) {

		ModelOBJ::Mesh mesh = model.getMesh(i);
		ModelOBJ::Material material = *(mesh.pMaterial);

		// Set material parameters for house
		glUniform3f(materialGLLocs.aColorLoc, material.ambient[0], material.ambient[1], material.ambient[2]);
		glUniform3f(materialGLLocs.dColorLoc, material.diffuse[0], material.diffuse[1], material.diffuse[2]);
		glUniform3f(materialGLLocs.sColorLoc, material.specular[0], material.specular[1], material.specular[2]);
		glUniform1f(materialGLLocs.shineLoc, material.shininess);

		drawMesh(model.getMesh(i).triangleCount * 3, model.getMesh(i).startIndex, TextureObjects[i], VBO, IBO, vertexGLLocs);
	}
}


void MultiTextureObject::drawMesh(int numberOfIndices, GLuint startIndex, GLuint texture, GLuint &VBO, GLuint &IBO, VertexGLLocs vertexGLLocs) {

	glBindTexture(GL_TEXTURE_2D, texture);
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
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(startIndex));
}