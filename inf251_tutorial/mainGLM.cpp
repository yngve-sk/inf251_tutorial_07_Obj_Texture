#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "lodepng.h"
#include "model_obj.h"

#include <glm/glm.hpp>
#include <glm/gtx/extend.hpp>

#include "glm_camera.h"

using namespace std;

struct Vertex {
	glm::fvec3 position, normal;
};

//class Camera {
//	public:
//	Vector3f position, target, up;
//
//	float fov; // fieldof view
//	float ar; // aspect ratio
//
//	float zNear, zFar; // depthof near/far plane
//
//	float zoom; // extra scaling param
//};

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
ModelOBJ Model2;		///< A 3D model
GLuint VBO = 0;		///< A vertex buffer object
GLuint IBO = 0;		///< An index buffer object

					// Texture
GLuint TextureObject = 0;				///< A texture object
unsigned int TextureWidth = 0;			///< The width of the current texture
unsigned int TextureHeight = 0;			///< The height of the current texture
unsigned char *TextureData = nullptr;	///< the array where the texture image will be stored

										// Texture for second object
GLuint TextureObject2 = 0;				///< A texture object
unsigned int TextureWidth2 = 0;			///< The width of the current texture
unsigned int TextureHeight2 = 0;			///< The height of the current texture
unsigned char *TextureData2 = nullptr;	///< the array where the texture image will be stored

										// Shaders
GLuint ShaderProgram = 0;	///< A shader program
GLint TrLoc = -1;				///< model-view matrix uniform variable
GLint SamplerLoc = -1;			///< texture sampler uniform variable
GLint CameraPositionLoc = -1;
								// Vertex transformation
glm::fmat4 RotationX, RotationY;		///< Rotation (along X and Y axis)
glm::fvec3 Translation;	///< Translation
float Scaling;			///< Scaling

						// Mouse interaction
int MouseX, MouseY;		///< The last position of the mouse
int MouseButton;		///< The last mouse button pressed or released

GLMCamera Cam;
bool USE_CAM = false;

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
	//glEnable(GL_CULL_FACE);		// enable back-face culling


								// Transformation
	Cam = *(new GLMCamera());

	if (USE_CAM) {
		// init camera position, rotation, translation
		// ETC!!
	}
	else {
		RotationX = glm::mat4(1.0);
		RotationY = glm::mat4(1.0);
		Translation = glm::fvec3(0.0f, 0.0f, 0.0f);
		Scaling = 1.0f;
		//	}

			// Shaders & mesh
		if (!initShaders() || !initMesh()) {
			cerr << "An error occurred, press Enter to quit ..." << endl;
			getchar();
			return -1;
		}



		// Start the main event loop
		glutMainLoop();

		// clean-up before exit
		if (TextureData != nullptr)
			free(TextureData);

		return 0;
	}

}
// ************************************************************************************************
// *** OpenGL callbacks implementation ************************************************************
void display() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable the shader program
	assert(ShaderProgram != 0);
	glUseProgram(ShaderProgram);

	//Matrix4f camTransformation = computeCameraTransform(Cam),
//	glm::fmat4 camTransformation = Cam.computeCameraTransform(),
	//glm::fmat4	originalTransformation = glm::vec4(Translation, 1) *
	//	RotationX * RotationY *
	//	glm::scale(glm::fmat4(), glm::vec4(1, 1, 1,0));
		//Matrix4f::createScaling(Scaling, Scaling, Scaling);
	//glm::fmat4 translationMatrix = glm::translate(fmat4(), Translation);
	//glm::fmat4 rotationMatrixX = RotationX;
	//glm::fmat4 rotationMatrixY = RotationY;
	//glm::fmat4 scaleMatrix = glm::scale(fmat4(), vec3(Scaling, Scaling, Scaling));
	//
	//glm::fmat4 transformation = USE_CAM ? Cam.computeCameraTransform() : translationMatrix *
	//	rotationMatrixX * rotationMatrixY *
	//	scaleMatrix;

	Cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	mat4 transformation = Cam.computeCameraTransform();

	glUniform3fv(CameraPositionLoc, 1, &Cam.getPosition()[0]);

	//// Set the uniform variable for the vertex transformation
	//Matrix4f transformation = USE_CAM ? computeCameraTransform(Cam) :
	//	Matrix4f::createTranslation(Translation) *
	//	RotationX * RotationY * 
	//	Matrix4f::createScaling(Scaling, Scaling, Scaling);


	glUniformMatrix4fv(TrLoc, 1, GL_FALSE, &transformation[0][0]);

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


	// Enable texture unit 0 and bind the texture to it
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, TextureObject2);

	// Bind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// Draw the elements on the GPU
	glDrawElements(
		GL_TRIANGLES,
		Model.getNumberOfIndices(),
		GL_UNSIGNED_INT,
		0);
	/*glDrawElements(
		GL_TRIANGLES,
		Model2.getNumberOfIndices(),
		GL_UNSIGNED_INT,
		(void*)Model.getNumberOfIndices());*/

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
		break;
	case 'p':
		//cout << "ROTATION X:" << endl;
		//RotationX.print(cout);
		//cout << "ROTATION Y:" << endl;
		//RotationY.print(cout);
		//cout << "TRANSLATE XYZ" << endl;
		//Translation.print(cout);
		break;
	case 't':
		USE_CAM = !USE_CAM;
		glutPostRedisplay();
		break;
	case 'w':
		std::cout << "forkwards" << std::endl;
		Cam.moveForward();
		glutPostRedisplay();
		break;
	case 'a':
		Cam.strafeLeft();
		glutPostRedisplay();
		break;
	case 's':
		std::cout << "backwards" << std::endl;
		Cam.moveBackwards();
		glutPostRedisplay();
		break;
	case 'd':
		Cam.strafeRight();
		glutPostRedisplay();
		break;
	case 'c':
		Cam.moveDown();
		glutPostRedisplay();
		break;
	case ' ':
		Cam.moveUp();
		glutPostRedisplay();
		break;
	}
}

/// Called whenever a mouse event occur (press or release)
void mouse(int button, int state, int x, int y) {
	// Store the current mouse status
	MouseButton = button;
	MouseX = x;
	MouseY = y;
}

inline void updateMousePosition(int newX, int newY) {
	MouseX = newX;
	MouseY = newY;
}
/// Called whenever the mouse is moving while a button is pressed
void motion(int x, int y) {
	if (MouseButton == GLUT_RIGHT_BUTTON) {
		Cam.translate(vec2(MouseX, MouseY), vec2(x, y));
	
		updateMousePosition(x, y);
	}
	if (MouseButton == GLUT_MIDDLE_BUTTON) {
		Cam.adjustZoom(vec2(MouseX, MouseY), vec2(x, y));

		updateMousePosition(x, y);
	}
	if (MouseButton == GLUT_LEFT_BUTTON) {
		Cam.rotate(vec2(MouseX, MouseY), glm::vec2(x, y));
		
		updateMousePosition(x, y);
	}

	glutPostRedisplay(); // Specify that the scene needs to be updated
}



// ************************************************************************************************
// *** Other methods implementation ***************************************************************
/// Initialize buffer objects
bool initMesh() {
	// Load the OBJ model
	//if (!Model.import("House-Model\\House.obj")) {
	if (!Model.import("Objects\\cube-textured\\cube.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}

	Model.normalize();

	/*if (!Model2.import("Objects\\cube-textured\\cubetextured.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}*/

	Model2.normalize();

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
	
	/* VBO and IBO for two objects */
	// VBO
	/*glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		(Model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex)) + (Model2.getNumberOfVertices() * sizeof(ModelOBJ::Vertex)),
		0,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 
		0, 
		Model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		&Model.getVertex(0));
	glBufferSubData(GL_ARRAY_BUFFER,
		Model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		Model2.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		&Model2.getVertex(0));

	// IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		(Model.getNumberOfIndices() * sizeof(unsigned int)) + (Model2.getNumberOfIndices() * sizeof(unsigned int)),
		0,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
		0,
		Model.getNumberOfIndices() * sizeof(unsigned int),
		&(Model.getIndexBuffer()[0]));
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
		Model.getNumberOfIndices() * sizeof(unsigned int),
		Model2.getNumberOfIndices() * sizeof(unsigned int),
		&(Model2.getIndexBuffer()[0]));
		*/

	cout << "number of materials = " << Model.getNumberOfMaterials() << endl;
	// Check the materials for the texture
	for (int i = 0; i < Model.getNumberOfMaterials(); ++i) {

		// if the current material has a texture
		if (Model.getMaterial(i).colorMapFilename != "") {

			// Load the texture
			if (TextureData != nullptr)
				free(TextureData);

			//cout << "round " << i << " trying to load @ file path " << "House-Model\\" << Model.getMaterial(i).colorMapFilename.c_str() << endl;
			unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
				//("House-Model\\" + Model.getMaterial(i).colorMapFilename).c_str(),
				("Objects\\cube-textured\\" + Model.getMaterial(i).colorMapFilename).c_str(),
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

	/*// Load the Wall OBJ model
	cout << "number of materials = " << Model2.getNumberOfMaterials() << endl;
	// Check the materials for the texture
	for (int i = 0; i < Model2.getNumberOfMaterials(); ++i) {

		// if the current material has a texture
		if (Model2.getMaterial(i).colorMapFilename != "") {

			// Load the texture
			if (TextureData != nullptr)
				free(TextureData);

			cout << "round " << i << " trying to load @ file path " << "Objects\\cube-textured\\" << Model2.getMaterial(i).colorMapFilename.c_str() << endl;
			unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
				("Objects\\cube-textured\\" + Model2.getMaterial(i).colorMapFilename).c_str(),
				LCT_RGB, 24); // Remember to check the last 2 parameters
			if (fail != 0) {
				cerr << "Error: cannot load texture file "
					<< Model2.getMaterial(i).colorMapFilename << endl;
				return false;
			}

			// Create the texture object
			if (TextureObject2 != 0)
				glDeleteTextures(1, &TextureObject2);
			glGenTextures(1, &TextureObject2);

			// Bind it as a 2D texture (note that other types of textures are supported as well)
			glBindTexture(GL_TEXTURE_2D, TextureObject2);

			// Set the texture data
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB,			// remember to check this
				TextureWidth2,
				TextureHeight2,
				0,
				GL_RGB,			// remember to check this
				GL_UNSIGNED_BYTE,
				TextureData2
				);

			// Configure texture parameter
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// For the moment, assumes there is only one texture to be loaded
			// break;
		}
	}*/

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

	CameraPositionLoc = glGetUniformLocation(ShaderProgram, "camera_position");

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