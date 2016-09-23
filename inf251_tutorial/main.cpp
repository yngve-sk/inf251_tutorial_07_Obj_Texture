#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "lodepng.h"
#include "model_obj.h"
#include "Vector3.h"
#include "Matrix4.h"

using namespace std;

struct Vertex {
	Vector3f position, normal;
};

struct Camera {
	Vector3f position, target, up;

	float fov; // fieldof view
	float ar; // aspect ratio

	float zNear, zFar; // depthof near/far plane

	float zoom; // extra scaling param
};

// --- OpenGL callbacks ---------------------------------------------------------------------------
void display();
void idle();
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void motion(int, int);

// --- Other methods ------------------------------------------------------------------------------
bool initMesh();
bool initShaders();
string readTextFile(const string&);


// --- Global variables ---------------------------------------------------------------------------
// 3D model
ModelOBJ Model;		///< A 3D model
GLuint VBO = 0;		///< A vertex buffer object
GLuint IBO = 0;		///< An index buffer object

					// Texture
GLuint TextureObject = 0;				///< A texture object
unsigned int TextureWidth = 0;			///< The width of the current texture
unsigned int TextureHeight = 0;			///< The height of the current texture
unsigned char *TextureData = nullptr;	///< the array where the texture image will be stored

										// Shaders
GLuint ShaderProgram = 0;	///< A shader program
GLint TrLoc = -1;				///< model-view matrix uniform variable
GLint SamplerLoc = -1;			///< texture sampler uniform variable

								// Vertex transformation
Matrix4f RotationX, RotationY;		///< Rotation (along X and Y axis)
Vector3f Translation;	///< Translation
float Scaling;			///< Scaling

						// Mouse interaction
int MouseX, MouseY;		///< The last position of the mouse
int MouseButton;		///< The last mouse button pressed or released

Camera Cam;

// --- main() -------------------------------------------------------------------------------------
/// The entry point of the application
int main(int argc, char **argv) {

	// Initialize glut and create a simple window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(300, 50);
	glutCreateWindow("OpenGL Tutorial");

	// Initialize OpenGL callbacks
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	// Initialize glew (must be done after glut is initialized!)
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		cerr << "Error initializing glew: \n"
			<< reinterpret_cast<const char*>(glewGetErrorString(res)) << endl;
		cerr << "Press Enter to quit ..." << endl;
		getchar();
		return -1;
	}

	// Initialize program variables
	// OpenGL
	glClearColor(0.1f, 0.3f, 0.1f, 0.0f); // background color
	glEnable(GL_DEPTH_TEST);	// enable depth ordering
	glFrontFace(GL_CCW);		// Vertex order for the front face
	glCullFace(GL_BACK);		// back-faces should be removed
	glEnable(GL_CULL_FACE);		// enable back-face culling

								// Transformation
	RotationX.identity();
	RotationY.identity();
	Translation.set(0.0f, 0.0f, 0.0f);
	Scaling = 1.0f;

	// Shaders & mesh
	if (!initShaders() || !initMesh()) {
		cerr << "An error occurred, press Enter to quit ..." << endl;
		getchar();
		return -1;
	}

	Cam.position.set(0.f, 0.f, 0.f);
	Cam.target.set(0.f, 0.f, -1.f);
	Cam.up.set(0.f, 0.f, 0.f);
	Cam.fov = 30.0f;
	Cam.ar = 1.f;
	Cam.zNear = 0.1f;
	Cam.zFar = 100.f;
	Cam.zoom = 1.f;

	// Start the main event loop
	glutMainLoop();

	// clean-up before exit
	if (TextureData != nullptr)
		free(TextureData);

	return 0;
}

Matrix4f computeCameraTransform(const Camera& cam) {

	Vector3f t = cam.target.getNormalized();
	Vector3f u = cam.up.getNormalized();
	Vector3f r = t.cross(u);
	Matrix4f camR(r.x(), r.y(), r.z(), 0.f,
		u.x(), u.y(), u.z(), 0.f,
		-t.x(), -t.y(), -t.z(), 0.f,
		0.f, 0.f, 0.f, 1.f);

	Matrix4f camT = Matrix4f::createTranslation(-cam.position);

	Matrix4f prj = Matrix4f::createPerspectivePrj(cam.fov, cam.ar, cam.zNear, cam.zFar);

}

// ************************************************************************************************
// *** OpenGL callbacks implementation ************************************************************
/// Called whenever the scene has to be drawn
void display() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable the shader program
	assert(ShaderProgram != 0);
	glUseProgram(ShaderProgram);

	// Set the uniform variable for the vertex transformation
	Matrix4f transformation =
		Matrix4f::createTranslation(Translation) *
		RotationX * RotationY *
		Matrix4f::createScaling(Scaling, Scaling, Scaling);
	glUniformMatrix4fv(TrLoc, 1, GL_FALSE, transformation.get());

	// Set the uniform variable for the texture unit (texture unit 0)
	glUniform1i(SamplerLoc, 0);

	// Enable the vertex attributes and set their format
	GLint posLoc = glGetAttribLocation(ShaderProgram, "position");
	glEnableVertexAttribArray(posLoc);
	GLint texLoc = glGetAttribLocation(ShaderProgram, "tex_coords");
	glEnableVertexAttribArray(texLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(3 * sizeof(float)));

	// Enable texture unit 0 and bind the texture to it
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureObject);

	// Bind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// Draw the elements on the GPU
	glDrawElements(
		GL_TRIANGLES,
		Model.getNumberOfIndices(),
		GL_UNSIGNED_INT,
		0);

	// Disable the "position" vertex attribute (not necessary but recommended)
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(texLoc);

	// Disable the shader program (not necessary but recommended)
	glUseProgram(0);

	// Swap the frame buffers (off-screen rendering)
	glutSwapBuffers();
}

/// Called at regular intervals (can be used for animations)
void idle() {
}

/// Called whenever a keyboard button is pressed (only ASCII characters)
void keyboard(unsigned char key, int x, int y) {
	switch (tolower(key)) {
	case 'g': // show the current OpenGL version
		cout << "OpenGL version " << glGetString(GL_VERSION) << endl;
		break;
	case 'q':  // terminate the application
		exit(0);
		break;
	case 'r':
		cout << "Re-loading shaders..." << endl;
		if (initShaders()) {
			cout << "> done." << endl;
			glutPostRedisplay();
		}
	}
}

/// Called whenever a mouse event occur (press or release)
void mouse(int button, int state, int x, int y) {
	// Store the current mouse status
	MouseButton = button;
	MouseX = x;
	MouseY = y;
}

/// Called whenever the mouse is moving while a button is pressed
void motion(int x, int y) {
	if (MouseButton == GLUT_RIGHT_BUTTON) {
		Translation.x() += 0.003f * (x - MouseX); // Accumulate translation amount
		Translation.y() += 0.003f * (MouseY - y);
		MouseX = x; // Store the current mouse position
		MouseY = y;
	}
	if (MouseButton == GLUT_MIDDLE_BUTTON) {
		Scaling += 0.003f * (MouseY - y); // Accumulate scaling amount
		MouseX = x; // Store the current mouse position
		MouseY = y;
	}
	if (MouseButton == GLUT_LEFT_BUTTON) {
		Matrix4f rx, ry;	// compute the rotation matrices
		rx.rotate(-0.1f * (MouseY - y), Vector3f(1, 0, 0));
		ry.rotate(0.1f * (x - MouseX), Vector3f(0, 1, 0));
		RotationX *= rx;	// accumulate the rotation
		RotationY *= ry;
		MouseX = x; // Store the current mouse position
		MouseY = y;
	}

	glutPostRedisplay(); // Specify that the scene needs to be updated
}

// ************************************************************************************************
// *** Other methods implementation ***************************************************************
/// Initialize buffer objects
bool initMesh() {
	// Load the OBJ model
	if (!Model.import("House-Model\\House.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}

	Model.normalize();

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		Model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		Model.getVertexBuffer(),
		GL_STATIC_DRAW);

	// IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		Model.getNumberOfIndices() * sizeof(unsigned int),
		Model.getIndexBuffer(),
		GL_STATIC_DRAW);


	cout << "number of materials = " << Model.getNumberOfMaterials() << endl;
	// Check the materials for the texture
	for (int i = 0; i < Model.getNumberOfMaterials(); ++i) {

		// if the current material has a texture
		if (Model.getMaterial(i).colorMapFilename != "") {

			// Load the texture
			if (TextureData != nullptr)
				free(TextureData);

			cout << "round " << i << " trying to load @ file path " << "House-Model\\House\\" << Model.getMaterial(i).colorMapFilename.c_str() << endl;
			unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
				("House-Model\\" + Model.getMaterial(i).colorMapFilename).c_str(),
				LCT_RGB, 24); // Remember to check the last 2 parameters
			if (fail != 0) {
				cerr << "Error: cannot load texture file "
					<< Model.getMaterial(i).colorMapFilename << endl;
				return false;
			}

			// Create the texture object
			if (TextureObject != 0)
				glDeleteTextures(1, &TextureObject);
			glGenTextures(1, &TextureObject);

			// Bind it as a 2D texture (note that other types of textures are supported as well)
			glBindTexture(GL_TEXTURE_2D, TextureObject);

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

			// For the moment, assumes there is only one texture to be loaded
			// break;
		}
	}

	return true;
} /* initBuffers() */


  /// Initialize shaders. Return false if initialization fail
bool initShaders() {

	//NEW GREAT COMMENT FROM ZUZU, I AM COMMENTING BECAUSE I WAS FORCED TO DO THAT

	// Create the shader program and check for errors
	if (ShaderProgram != 0)
		glDeleteProgram(ShaderProgram);
	ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		cerr << "Error: cannot create shader program." << endl;
		return false;
	}

	// Create the shader objects and check for errors
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (vertShader == 0 || fragShader == 0) {
		cerr << "Error: cannot create shader objects." << endl;
		return false;
	}

	// Read and set the source code for the vertex shader
	string text = readTextFile("shader.v.glsl");
	const char* code = text.c_str();
	int length = static_cast<int>(text.length());
	if (length == 0)
		return false;
	glShaderSource(vertShader, 1, &code, &length);

	// Read and set the source code for the fragment shader
	string text2 = readTextFile("shader.f.glsl");
	const char *code2 = text2.c_str();
	length = static_cast<int>(text2.length());
	if (length == 0)
		return false;
	glShaderSource(fragShader, 1, &code2, &length);

	// Compile the shaders
	glCompileShader(vertShader);
	glCompileShader(fragShader);

	// Check for compilation error
	GLint success;
	GLchar errorLog[1024];
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertShader, 1024, nullptr, errorLog);
		cerr << "Error: cannot compile vertex shader.\nError log:\n" << errorLog << endl;
		return false;
	}
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragShader, 1024, nullptr, errorLog);
		cerr << "Error: cannot compile fragment shader.\nError log:\n" << errorLog << endl;
		return false;
	}

	// Attach the shader to the program and link it
	glAttachShader(ShaderProgram, vertShader);
	glAttachShader(ShaderProgram, fragShader);
	glLinkProgram(ShaderProgram);

	// Check for linking error
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ShaderProgram, 1024, nullptr, errorLog);
		cerr << "Error: cannot link shader program.\nError log:\n" << errorLog << endl;
		return false;
	}

	// Make sure that the shader program can run
	glValidateProgram(ShaderProgram);

	// Check for validation error
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ShaderProgram, 1024, nullptr, errorLog);
		cerr << "Error: cannot validate shader program.\nError log:\n" << errorLog << endl;
		return false;
	}

	// Get the location of the uniform variables
	TrLoc = glGetUniformLocation(ShaderProgram, "transformation");
	SamplerLoc = glGetUniformLocation(ShaderProgram, "sampler");
	assert(TrLoc != -1
		&& SamplerLoc != -1
	);

	// Shaders can be deleted now
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
} /* initShaders() */


  /// Read the specified file and return its content
string readTextFile(const string& pathAndFileName) {
	// Try to open the file
	ifstream fileIn(pathAndFileName);
	if (!fileIn.is_open()) {
		cerr << "Error: cannot open file " << pathAndFileName.c_str();
		return "";
	}

	// Read the file
	string text = "";
	string line;
	while (!fileIn.eof()) {
		getline(fileIn, line);
		text += line + "\n";
		bool a = fileIn.bad();
		bool b = fileIn.fail();
		if (fileIn.bad() || (fileIn.fail() && !fileIn.eof())) {
			cerr << "Warning: problems reading file " << pathAndFileName.c_str()
				<< "\nText read: \n" << text.c_str();
			fileIn.close();
			return text;
		}
	}
	// finalize
	fileIn.close();

	return text;
} /* readTextFile() */

  /* --- eof main.cpp --- */