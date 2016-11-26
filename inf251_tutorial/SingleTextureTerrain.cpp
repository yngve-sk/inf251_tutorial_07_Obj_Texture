#include "SingleTextureTerrain.h"
#include <fstream>

SingleTextureTerrain::SingleTextureTerrain(const char* directory,
	const char* materialDirectory,
	const char* bumpMap) {
	loadObject(directory);
	loadTerrain(materialDirectory);
	loadBumpMap(bumpMap);
}

void SingleTextureTerrain::loadObject(const char* directory) {

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

void SingleTextureTerrain::loadTerrain(const char* textureDirectory) {

	// open the file
	fstream fileIn(textureDirectory, ifstream::in | ifstream::binary);

	//check if the file has been opened
	if (!fileIn.good()) {
		cout << "Error opening the file." << endl;
		return;
	}

	// struct to read values from the file
	union {
		char cVals[4];
		int iVal;
	} buffer4;

	union {
		char cVals[8];
		double dVal;
	} buffer8;

	union {
		char *cVals;
		float *fVals;
	} bufferX;

	// read the header
	fileIn.read(buffer4.cVals, 4);
	colsNum = buffer4.iVal;
	fileIn.read(buffer4.cVals, 4);
	rowsNum = buffer4.iVal;

	fileIn.read(buffer8.cVals, 8);
	xLowLeft = buffer8.dVal;
	fileIn.read(buffer8.cVals, 8);
	yLowLeft = buffer8.dVal;
	fileIn.read(buffer8.cVals, 8);
	cellSize = buffer8.dVal;

	fileIn.read(buffer4.cVals, 4);
	NO_DATA = buffer4.iVal;

	// read the height value
	numberOfPoints = rowsNum * colsNum;
	bufferX.fVals = new float[numberOfPoints];
	fileIn.read(bufferX.cVals, 4 * numberOfPoints);

	// copy the height values in the global array
	heights = new float[numberOfPoints];
	for (int i = 0; i < numberOfPoints; i++)
		heights[i] = bufferX.fVals[i];


	vertices = new vec3[numberOfPoints];
	createVertices();
	numberOfTriangles = 2 * (rowsNum - 1) * (colsNum - 1);
	trIndices = new unsigned int[3 * numberOfTriangles];
	int currentTriangle = 0;
	for (int r = 0; r < rowsNum - 1; ++r) {
		for (int c = 0; c < colsNum - 1; ++c) {
			int i0 = rc2index(r, c);
			int i1 = rc2index(r + 1, c);
			int i2 = rc2index(r, c + 1);
			int i3 = rc2index(r + 1, c + 1);

			createTriangles(i0, i1, i2, i3, currentTriangle);
			currentTriangle += 2;
		}
	}

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfPoints * sizeof(vec3),
		vertices,
		GL_STATIC_DRAW);

	// IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		numberOfTriangles * 3 * sizeof(unsigned int),
		trIndices,
		GL_STATIC_DRAW);

	// close the file
	fileIn.close();
}

// compute the vertices from the heights array
void SingleTextureTerrain::createVertices() {
	for (int i = 0; i < numberOfPoints; i++) {
		vertices[i].x = (i % rowsNum) * cellSize + xLowLeft;
		if (heights[i] < 0) {
			vertices[i].y = 0;
		}
		else {
			vertices[i].y = heights[i];
		}
		vertices[i].z = (i / rowsNum) * cellSize + yLowLeft;
	}
}

void SingleTextureTerrain::createTriangles(int i0, int i1, int i2, int i3, int currentTriange) {
	//i1 to i2 --diagonals
	//i0 to i3 --diagonals

	// check which is the option that leads to the shortest diagonal
	if (abs(heights[i0] - heights[i3]) <= abs(heights[i1] - heights[i2])) {
		trIndices[3 * currentTriange] = i0, i3, i1;
		trIndices[3 * currentTriange + 1] = i0, i2, i3;
	}
	else if (abs(heights[i0] - heights[i3]) > abs(heights[i1] - heights[i2])) {
		// store the indices for both the ttriangles
		trIndices[3 * currentTriange] = i0, i2, i1;
		trIndices[3 * currentTriange + 1] = i1, i2, i3;
	}
}

int SingleTextureTerrain::rc2index(int row, int col) {
	return col * rowsNum + row;
}

void SingleTextureTerrain::loadBumpMap(const char* textureDirectory) {

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

void SingleTextureTerrain::drawObject(VertexGLLocs vertexGLLocs, MaterialGLLocs materialGLLocs) {

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


