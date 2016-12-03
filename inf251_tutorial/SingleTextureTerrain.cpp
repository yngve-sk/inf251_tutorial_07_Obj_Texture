#include "SingleTextureTerrain.h"
#include <fstream>

SingleTextureTerrain::SingleTextureTerrain() {
}

void SingleTextureTerrain::init(const char* directory,
	const char* materialDirectory) {
	loadTerrain(directory);
	createVertices();
	initTriangles();
	generateTerrainBuffers();
	//loadTerrain(materialDirectory);
	//loadBumpMap(bumpMap);
}
void SingleTextureTerrain::init(const char* directory,
	const char* materialDirectory, const char* bumpMapDirectory) {
	loadTerrain(directory);
	createVertices();
	initTriangles();
	generateTerrainBuffers();
	//loadTerrain(materialDirectory);
	loadBumpMap(bumpMapDirectory);
}

void SingleTextureTerrain::loadTerrain(const char* directory) {

	// open the file
	fstream fileIn(directory, ifstream::in | ifstream::binary);

	//check if the file has been opened
	if (!fileIn.good()) {
		cout << "Error opening the file." << directory << endl;
		return;
	}

	cout << "Imported model: " << directory << endl;

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

	// close the file
	fileIn.close();
}

// compute the vertices from the heights array
void SingleTextureTerrain::createVertices() {
	vertices = new Vector3f[numberOfPoints];

	for (int r = 0; r < rowsNum - 1; r++) {
		for (int c = 0; c < colsNum - 1; c++) {
			int i = rc2index(r, c);
			float x = (i % rowsNum) * cellSize;
			float y = heights[i];
			float z = -floor(float(i / rowsNum))*cellSize;

			vertices[i].set(x, y, z);
		}
	}
}

void SingleTextureTerrain::generateTerrainBuffers() {
	terrainVertices = new TerrainVertex[numberOfPoints];
	for (int i = 0; i < numberOfPoints; i++) {
		terrainVertices[i].position = vertices[i];
		terrainVertices[i].tex_coords[0] = texture_coords[i * 2 + 0];
		terrainVertices[i].tex_coords[1] = 1.0 - texture_coords[i * 2 + 1];
		terrainVertices[i].normals = vertexNormals[i];
	}

	// VBO 
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		numberOfPoints * sizeof(TerrainVertex),
		terrainVertices,
		GL_STATIC_DRAW);


	//numberOfTriangles -= 100;
	// IBO 
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		3 * numberOfTriangles * sizeof(unsigned int),
		trIndices,
		GL_STATIC_DRAW);

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0;
	unsigned int TextureHeight = 0;

	// Textures
	if (TextureData != nullptr)
		free(TextureData);
	unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
		"terrain\\bergen_terrain_texture.png",
		LCT_RGBA, 8);
	if (fail != 0) {
		cout << fail << endl;
		cerr << "Error: cannot load texture file: bergen_terrain_texture.png " << endl;
	}

	// Create the texture object
	if (textureObject != 0)
		glDeleteTextures(1, &textureObject);
	glGenTextures(1, &textureObject);

	// Bind it as a 2D texture 
	glBindTexture(GL_TEXTURE_2D, textureObject);

	// Set the texture data
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		TextureWidth,
		TextureHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		TextureData
	);

	// Configure texture parameter
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Configure texture parameters for minification and magnification
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void SingleTextureTerrain::initTriangles() {

	numberOfTriangles = 2 * (rowsNum - 1) * (colsNum - 1);
	trIndices = new unsigned int[3 * numberOfTriangles];
	int currentTriangle = 0;
	for (int r = 0; r < rowsNum - 1; ++r) {
		for (int c = 0; c < colsNum - 1; ++c) {
			int i0 = rc2index(r, c); int i1 = rc2index(r + 1, c);

			int i2 = rc2index(r, c + 1); int i3 = rc2index(r + 1, c + 1);

			vertices[i0] = vertices[i0];
			vertices[i1] = vertices[i1];
			vertices[i2] = vertices[i2];
			vertices[i3] = vertices[i3];
			numberOfTriangles = numberOfTriangles;

			double diagonal0 = sqrt(pow(vertices[i2].x() - vertices[i1].x(), 2) + pow(vertices[i2].y() - vertices[i1].y(), 2) + pow(vertices[i2].z() - vertices[i1].z(), 2));

			double diagonal1 = (sqrt(pow(vertices[i3].x() - vertices[i0].x(), 2) + pow(vertices[i3].y() - vertices[i0].y(), 2) + pow(vertices[i3].z() - vertices[i0].z(), 2)));

			if (diagonal0 < diagonal1) {
				trIndices[3 * currentTriangle + 0] = i0;
				trIndices[3 * currentTriangle + 1] = i2;
				trIndices[3 * currentTriangle + 2] = i1;

				trIndices[3 * (currentTriangle + 1) + 0] = i1;
				trIndices[3 * (currentTriangle + 1) + 1] = i2;
				trIndices[3 * (currentTriangle + 1) + 2] = i3;
			}
			else {
				trIndices[3 * currentTriangle + 0] = i0;
				trIndices[3 * currentTriangle + 1] = i2;
				trIndices[3 * currentTriangle + 2] = i3;

				trIndices[3 * (currentTriangle + 1) + 0] = i3;
				trIndices[3 * (currentTriangle + 1) + 1] = i1;
				trIndices[3 * (currentTriangle + 1) + 2] = i0;
			}
			currentTriangle += 2;
		}
	}

	Vector3f *triangleNormals = new Vector3f[numberOfTriangles];
	vertexNormals = new Vector3f[numberOfPoints];

	for (int i = 0; i < numberOfPoints; i++)
		vertexNormals[i] = Vector3f(0.0, 0.0, 0.0);

	for (int i = 0; i < numberOfTriangles - 1; i++) {
		int v0_index = trIndices[3 * i + 0];
		int v1_index = trIndices[3 * i + 1];
		int v2_index = trIndices[3 * i + 2];

		Vector3f v0 = vertices[v0_index];
		Vector3f v1 = vertices[v1_index];
		Vector3f v2 = vertices[v2_index];

		Vector3f a = v1 - v0;
		Vector3f b = v2 - v0;
		triangleNormals[i] = a.cross(b);
		triangleNormals[i].normalize();

		if (v0.y() >= 0 && v1.y() >= 0 && v2.y() >= 0) {
			vertexNormals[trIndices[3 * i]] -= triangleNormals[i];
			vertexNormals[trIndices[(3 * i) + 1]] -= triangleNormals[i];
			vertexNormals[trIndices[(3 * i) + 2]] -= triangleNormals[i];
		}
	}

	// inittextures
	texture_coords = new float[numberOfPoints * 2];

	for (int r = 0; r < rowsNum; r++) {
		for (int c = 0; c < colsNum; c++) {
			int i = rc2index(r, c);
			texture_coords[i * 2 + 0] = float(double(c) / (double(colsNum) - 1.0) + 0.003);
			texture_coords[i * 2 + 1] = float(double(r) / (double(rowsNum) - 1.0));
		}
	}


	vector<Vector3f> trIndicesTmp;
	// Remove incorrect triangles.
	for (int i = 0; i < numberOfTriangles; i++) {
		if (vertices[trIndices[i * 3 + 0]].y() < 0 ||
			vertices[trIndices[i * 3 + 1]].y() < 0 ||
			vertices[trIndices[i * 3 + 2]].y() < 0 ||
			vertices[trIndices[i * 3 + 0]].z() > -200 ||
			vertices[trIndices[i * 3 + 1]].z() > -200 ||
			vertices[trIndices[i * 3 + 2]].z() > -200)
			;// Do nothing cull the triangle!
		else
			trIndicesTmp.insert(trIndicesTmp.end(), Vector3f(trIndices[i * 3 + 0], trIndices[i * 3 + 1], trIndices[i * 3 + 2]));
	}

	delete[] trIndices;
	numberOfTriangles = trIndicesTmp.size();
	trIndices = new unsigned int[3 * numberOfTriangles];

	for (int i = 0; i < numberOfTriangles; i++) {
		trIndices[i * 3 + 0] = trIndicesTmp[i].x();
		trIndices[i * 3 + 1] = trIndicesTmp[i].y();
		trIndices[i * 3 + 2] = trIndicesTmp[i].z();
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
	glBindTexture(GL_TEXTURE_2D, bumpMapObject);

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

void SingleTextureTerrain::drawObject(VertexGLLocs vertexGLLocs, MaterialGLLocs materialGLLocs, GLint bumpMappingLoc, GLint normalTextureLoc) {
	
	// Set material parameters for grass
	glUniform3f(materialGLLocs.aColorLoc, 0.9f, 1.0f, 0.9f);
	glUniform3f(materialGLLocs.dColorLoc, 0.3f, 1.0f, 0.3f);
	glUniform3f(materialGLLocs.sColorLoc, 0.1f, 0.1f, 0.1f);
	glUniform1f(materialGLLocs.shineLoc, 10.0f);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBindTexture(GL_TEXTURE_2D, textureObject);

	glUniform1i(bumpMappingLoc, 0);

	if (usingBumpMapping == 1) {
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glUniform1i(bumpMappingLoc, 1);
		glUniform1i(normalTextureLoc, 1);
		glBindTexture(GL_TEXTURE_2D, bumpMapObject);
	}

	glVertexAttribPointer(vertexGLLocs.posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(vertexGLLocs.texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
	glVertexAttribPointer(vertexGLLocs.normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), reinterpret_cast<const GLvoid*>(5 * sizeof(float)));

	// Draw the elements on the GPU
	glDrawElements(
		GL_TRIANGLES,
		3 * numberOfTriangles,
		GL_UNSIGNED_INT,
		0);

	if (usingBumpMapping == 1) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

}

