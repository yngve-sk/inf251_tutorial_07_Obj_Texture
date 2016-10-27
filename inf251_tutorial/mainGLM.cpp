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
void setDirectionalLight();
void setSpotLight();
void setHeadLight();
void drawText(string, double, double, double);
void drawObject(int, GLuint, GLuint&, GLuint&, GLuint, GLuint, GLuint);
ModelOBJ loadObject(const char*, GLuint&, GLuint&);
void loadGrassObject(GLuint&, GLuint&);
void loadMaterials(const char*, const ModelOBJ&, GLuint&);
void loadMaterial(const char*, GLuint&);


// --- Global variables ---------------------------------------------------------------------------
// 3D model
ModelOBJ Model;		///< A 3D model
ModelOBJ Model2;		///< A 3D model
GLuint VBO = 0;		///< A vertex buffer object
GLuint IBO = 0;		///< An index buffer object
GLuint cubeVBO = 0;		///< A vertex buffer object
GLuint cubeIBO = 0;		///< An index buffer object

						// Model of the grass
const int GRASS_VERTS_NUM = 4;
const int GRASS_TRIS_NUM = 2;
GLuint GrassVBO = 0;
GLuint GrassIBO = 0;

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

										// Texture for the grass
GLuint TexGrassObj = 0;
unsigned int TexGrassWidth = 0;
unsigned int TexGrassHeight = 0;
unsigned char *TexGrassData = nullptr;


// Shaders
GLuint ShaderProgram = 0;	///< A shader program
GLuint HouseShaderProgram = 0;

GLint TrLoc = 0;				///< model-view matrix uniform variable
GLint LocalTrLoc = -1;

GLint SamplerLoc = -1;			///< texture sampler uniform variable
GLint CameraPositionLoc = -1;

// Lighting params (DIRECTIONAL)
GLint DLightDirLoc = -1;

GLint DLightAColorLoc = -1;
GLint DLightDColorLoc = -1;
GLint DLightSColorLoc = -1;

GLint DLightAIntensityLoc = -1;
GLint DLightDIntensityLoc = -1;
GLint DLightSIntensityLoc = -1;

GLint MaterialAColorLoc = -1;
GLint MaterialDColorLoc = -1;
GLint MaterialSColorLoc = -1;
GLint MaterialShineLoc = -1;

// Lighting params (DIRECTIONAL)
GLint SLightDirLoc = -1;
	  
GLint SLightAColorLoc = -1;
GLint SLightDColorLoc = -1;
GLint SLightSColorLoc = -1;
	  
GLint SLightAIntensityLoc = -1;
GLint SLightDIntensityLoc = -1;
GLint SLightSIntensityLoc = -1;

GLint Headlight = -1;
bool HeadlightInt = true;

GLfloat  lightPos[] = { 0.0f, 0.0f, 75.0f, 1.0f };

// Vertex transformation
//glm::fmat4 RotationX, RotationY;		///< Rotation (along X and Y axis)
//glm::fvec3 Translation;	///< Translation
//float Scaling;			///< Scaling

						// Mouse interaction
int MouseX, MouseY;		///< The last position of the mouse
int MouseButton;		///< The last mouse button pressed or released

GLMCamera Cam;
bool USE_CAM = false;

// House transformation

mat4 LocalRotationX, LocalRotationY = mat4(1.), LocalTranslation;
float LocalScaling;

// Non-transformation matrix
mat4 NonTransformation = mat4(1, 0, 0, 0,
							  0, 1, 0, 0,
							  0, 0, 1, 0,
							  0, 0, 0, 1);

float centerX, centerY, centerZ;

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
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_COLOR_MATERIAL);// Enable color tracking

	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// Transformation
	Cam = *(new GLMCamera());


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

	// clean-up before exit
	if (TextureData2 != nullptr)
		free(TextureData2);

	return 0;

}



// ************************************************************************************************
// *** OpenGL callbacks implementation ************************************************************
void display() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int width = glutGet(GLUT_WINDOW_WIDTH),
		height = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, width, height);

	
	// Enable the shader program
	assert(ShaderProgram != 0);
	glUseProgram(ShaderProgram);

	Cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	mat4 transformation = Cam.computeCameraTransform();

	glUniform3fv(CameraPositionLoc, 1, &Cam.getPosition()[0]);
	glUniformMatrix4fv(TrLoc, 1, GL_FALSE, &transformation[0][0]);
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);

	setDirectionalLight();
	setSpotLight();
	setHeadLight();
	
	// Set the uniform variable for the texture unit (texture unit 0)
	//glUniform1i(SamplerLoc, 0);	

	// Enable the vertex attributes and set their format
	GLint posLoc = glGetAttribLocation(ShaderProgram, "position");
	glEnableVertexAttribArray(posLoc);
	GLint texLoc = glGetAttribLocation(ShaderProgram, "tex_coords");
	glEnableVertexAttribArray(texLoc);
	GLint normalLoc = glGetAttribLocation(ShaderProgram, "normal");
	glEnableVertexAttribArray(normalLoc);

	glActiveTexture(GL_TEXTURE0);

	// Draw the house

	Model.getCenter(centerX, centerY, centerZ);
	GLint centerLoc = glGetAttribLocation(ShaderProgram, "center");

	vec3 centerNone = vec3(0, 0, 0);
	vec3 centerv = vec3(centerX, centerY, centerZ);
	glUniform1fv(centerLoc, sizeof(fvec3), &centerv[0]);
	//
	mat4 translateToCenter = glm::translate(vec3(0.0f, -50, 0.0f));
	//mat4 translateFromCenter = glm::translate(vec3(-centerX, -centerY, -centerZ));
	//
	//mat4 rotateMat = translateFromCenter * LocalRotationY * translateToCenter;

	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);
	drawObject(Model.getNumberOfIndices(), TextureObject, VBO, IBO, posLoc, texLoc, normalLoc);

	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(5*sizeof(float)));

	
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);
	float x, y, z;
	Model2.getCenter(x, y, z);
	centerv = vec3(x, y, z);
	glUniform1fv(centerLoc, sizeof(fvec3), &centerv[0]);

	//mat4 translateToCenter = glm::translate(vec3(centerX, centerY, centerZ));
	//glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &LocalRotationY[0][0]);
	// Draw the cube
	drawObject(Model2.getNumberOfIndices(), TextureObject2, cubeVBO, cubeIBO, posLoc, texLoc, -1);

	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);
	glUniform1fv(centerLoc, sizeof(fvec3), &centerNone[0]);

	// Set material parameters for grass
	glUniform3f(MaterialAColorLoc, 0.9f, 1.0f, 0.9f);
	glUniform3f(MaterialDColorLoc, 0.3f, 1.0f, 0.3f);
	glUniform3f(MaterialSColorLoc, 0.1f, 0.1f, 0.1f);
	glUniform1f(MaterialShineLoc, 10.0f);

	// Draw the grass
	drawObject(3 * GRASS_TRIS_NUM, TexGrassObj, GrassVBO, GrassIBO, posLoc, texLoc, -1);



	// Draw projection text
	string projection;
	if (Cam.isProjectionPerspective()) {
		projection = "You are using perspective projection. Press 'p' for change.";
	}
	else {
		projection = "You are using orthogonal projection. Press 'p' for change.";
	}
	drawText(projection, -0.9, 0.9, 0);

	// Draw camera position text
	vec3 CamPosition = Cam.getPosition();
	string position = "Camera position x: ";
	position.append(to_string(CamPosition[0]) + ", y: ");
	position.append(to_string(CamPosition[1])) + ", z: ";
	position.append(to_string(CamPosition[2]));
	
	drawText(position, -0.7, 0.7, 0);


	// Disable the "position" vertex attribute (not necessary but recommended)
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(texLoc);
	glDisableVertexAttribArray(normalLoc);

	// Disable the shader program (not necessary but recommended)
	glUseProgram(0);

	// Swap the frame buffers (off-screen rendering)
	glutSwapBuffers();
}

/// Called at regular intervals (can be used for animations)
void idle() {
	// rotate around Y-axis
	//cout << "idle()" << endl;
	//LocalRotationY *= glm::rotate(0.05f, vec3(0, 1, 0));

	//LocalRotationY *= translate(vec3(0.05f, 0.0f, 0.0f)); //subtract origin position
//	LocalRotationY = translate(vec3(centerX, centerY, centerZ)) * rotate(LocalRotationY, 0.005f, vec3(0.0f, 1.0f, 0.0f)) * translate(vec3(-centerX, -centerY, -centerZ)); //rotate
	//LocalRotationY = rotate(LocalRotationY, 0.005f, vec3(0.0f, 1.0f, 0.0f)); //rotate	
//LocalRotationY *= translate(vec3(centerX, centerY, centerZ)); //return object on it's place

	LocalRotationY;

	glutPostRedisplay();
}

float deltaDefault = 10;
/// Called whenever a keyboard button is pressed (only ASCII characters)
void keyboard(unsigned char key, int x, int y) {
	switch (tolower(key)) {
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
	case '+':
		Cam.adjustFov(deltaDefault);
		glutPostRedisplay();
		break;
	case '-':
		Cam.adjustFov(-deltaDefault);
		glutPostRedisplay();
		break;
	case 'f':
		Cam.adjustZFar(deltaDefault);
		glutPostRedisplay();
		break;
	case 'g':
		Cam.adjustZFar(-deltaDefault);
		glutPostRedisplay();
		break;
	case 'n':
		Cam.adjustZNear(deltaDefault);
		glutPostRedisplay();
		break;
	case 'm':
		Cam.adjustZNear(-deltaDefault);
		glutPostRedisplay();
		break;
	case 'p':
		Cam.switchPerspective();
		glutPostRedisplay();
		break;
	case 'l':
		HeadlightInt = !HeadlightInt;
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

	Model = loadObject("House-Model\\House.obj", VBO, IBO);
	loadMaterials("House-Model\\", Model, TextureObject);

	Model2 = loadObject("Objects\\cube-textured\\cube.obj", cubeVBO, cubeIBO);
	loadMaterials("Objects\\cube-textured\\", Model2, TextureObject2);

	loadGrassObject(GrassVBO, GrassIBO);
	loadMaterial("grass.png", TexGrassObj);

	/*
	// Load the texture image for the grass
	if (TexGrassData != nullptr)
		free(TexGrassData);
	unsigned int fail = lodepng_decode_file(&TexGrassData, &TexGrassWidth, &TexGrassHeight,
		"grass.png", LCT_RGB, 8);
	if (fail != 0) {
		cerr << "Error: cannot load the texture file for the grass. " << endl;
		return false;
	}

	// Create the texture object
	if (TexGrassObj != 0)
		glDeleteTextures(1, &TexGrassObj);
	glGenTextures(1, &TexGrassObj);

	// Bind it as a 2D texture (note that other types of textures are supported as well)
	glBindTexture(GL_TEXTURE_2D, TexGrassObj);

	// Set the texture data
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGB,		// remember to check this
		TexGrassWidth, TexGrassHeight, 0,
		GL_RGB,		// remember to check this
		GL_UNSIGNED_BYTE, TexGrassData);

	// Configure texture parameter
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

	return true;
} /* initBuffers() */

bool initShader(GLuint& program, 
				string vShaderPath, string fShaderPath, 
				GLint& globalTransformationLoc, GLint& localTransformationLoc, 
				GLint& samplerLoc,
				GLint& cameraPositionLoc,
				GLint& dLightDirectionLoc,
				GLint& dLightAColorLoc, GLint& dLightDColorLoc, GLint& dLightSColorLoc,
				GLint& dLightAIntensityLoc, GLint& dLightDIntensityLoc, GLint& dLightSIntensityLoc,
				GLint& materialAColorLoc, GLint& materialDColorLoc, GLint& materialSColorLoc,
			//	vec3& materialADSColorLoc,
				GLint& materialShineLoc, GLint& headlightLoc) {
	if (program != 0)
		glDeleteProgram(program);

	program = glCreateProgram();
	if (program == 0) {
		cerr << "Error: cannot create shaderprogram." << endl;
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
	string text = readTextFile(vShaderPath);
	const char* code = text.c_str();
	int length = static_cast<int>(text.length());
	if (length == 0)
		return false;
	glShaderSource(vertShader, 1, &code, &length);

	// Read and set the source code for the fragment shader
	string text2 = readTextFile(fShaderPath);
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
	globalTransformationLoc = glGetUniformLocation(ShaderProgram, "transformation");
	// normal transformation (not camera)
	localTransformationLoc = glGetUniformLocation(ShaderProgram, "transformationLocal");


	samplerLoc = glGetUniformLocation(ShaderProgram, "sampler");
	assert(TrLoc != -1
		&& samplerLoc != -1
	);

	cameraPositionLoc = glGetUniformLocation(ShaderProgram, "camera_position");

	dLightDirectionLoc = glGetUniformLocation(ShaderProgram, "d_light_direction");

	dLightAColorLoc = glGetUniformLocation(ShaderProgram, "d_light_a_color");
	dLightDColorLoc = glGetUniformLocation(ShaderProgram, "d_light_d_color");
	dLightSColorLoc = glGetUniformLocation(ShaderProgram, "d_light_s_color");

	dLightAIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_a_intensity");
	dLightDIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_d_intensity");
	dLightSIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_s_intensity");

	materialAColorLoc = glGetUniformLocation(ShaderProgram, "material_a_color");
	materialDColorLoc = glGetUniformLocation(ShaderProgram, "material_d_color");
	materialSColorLoc = glGetUniformLocation(ShaderProgram, "material_s_color");

	materialShineLoc = glGetUniformLocation(ShaderProgram, "material_shininess");

	headlightLoc = glGetUniformLocation(ShaderProgram, "headlight");


	// Shaders can be deleted now
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;

}

  /// Initialize shaders. Return false if initialization fail
bool initShaders() {
	if(1)
	return initShader(ShaderProgram, "shader.v.glsl", "shader.f.glsl", 
						TrLoc, LocalTrLoc, SamplerLoc,
						CameraPositionLoc, 
						DLightDirLoc, 
						DLightAColorLoc, DLightDColorLoc, DLightSColorLoc,
						DLightAIntensityLoc, DLightDIntensityLoc, DLightSIntensityLoc,
						MaterialAColorLoc, MaterialDColorLoc, MaterialSColorLoc,
						MaterialShineLoc, 
						Headlight);

//	// Create the shader program and check for errors
//	if (ShaderProgram != 0)
//		glDeleteProgram(ShaderProgram);
//
////	if (HouseShaderProgram != 0)
////		glDeleteProgram(HouseShaderProgram);
////
////	HouseShaderProgram = glCreateProgram();
//
//	ShaderProgram = glCreateProgram();
//	if (ShaderProgram == 0) {
//		cerr << "Error: cannot create shader program." << endl;
//		return false;
//	}
//
//	// Create the shader objects and check for errors
//	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
//	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
//	if (vertShader == 0 || fragShader == 0) {
//		cerr << "Error: cannot create shader objects." << endl;
//		return false;
//	}
//
//	// Read and set the source code for the vertex shader
//	string text = readTextFile("shader.v.glsl");
//	const char* code = text.c_str();
//	int length = static_cast<int>(text.length());
//	if (length == 0)
//		return false;
//	glShaderSource(vertShader, 1, &code, &length);
//
//	// Read and set the source code for the fragment shader
//	string text2 = readTextFile("shader.f.glsl");
//	const char *code2 = text2.c_str();
//	length = static_cast<int>(text2.length());
//	if (length == 0)
//		return false;
//	glShaderSource(fragShader, 1, &code2, &length);
//
//	// Compile the shaders
//	glCompileShader(vertShader);
//	glCompileShader(fragShader);
//
//	// Check for compilation error
//	GLint success;
//	GLchar errorLog[1024];
//	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(vertShader, 1024, nullptr, errorLog);
//		cerr << "Error: cannot compile vertex shader.\nError log:\n" << errorLog << endl;
//		return false;
//	}
//	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(fragShader, 1024, nullptr, errorLog);
//		cerr << "Error: cannot compile fragment shader.\nError log:\n" << errorLog << endl;
//		return false;
//	}
//
//	// Attach the shader to the program and link it
//	glAttachShader(ShaderProgram, vertShader);
//	glAttachShader(ShaderProgram, fragShader);
//	glLinkProgram(ShaderProgram);
//
//	// Check for linking error
//	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
//	if (!success) {
//		glGetProgramInfoLog(ShaderProgram, 1024, nullptr, errorLog);
//		cerr << "Error: cannot link shader program.\nError log:\n" << errorLog << endl;
//		return false;
//	}
//
//	// Make sure that the shader program can run
//	glValidateProgram(ShaderProgram);
//
//	// Check for validation error
//	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
//	if (!success) {
//		glGetProgramInfoLog(ShaderProgram, 1024, nullptr, errorLog);
//		cerr << "Error: cannot validate shader program.\nError log:\n" << errorLog << endl;
//		return false;
//	}
//
//	// Get the location of the uniform variables
//	TrLoc = glGetUniformLocation(ShaderProgram, "transformation");
//	// normal transformation (not camera)
//	LocalTrLoc = glGetUniformLocation(ShaderProgram, "transformationLocal");
//
//
//	SamplerLoc = glGetUniformLocation(ShaderProgram, "sampler");
//	assert(TrLoc != -1
//		&& SamplerLoc != -1
//	);
//
//	CameraPositionLoc = glGetUniformLocation(ShaderProgram, "camera_position");
//
//	DLightDirLoc = glGetUniformLocation(ShaderProgram, "d_light_direction");
//
//	DLightAColorLoc = glGetUniformLocation(ShaderProgram, "d_light_a_color");
//	DLightDColorLoc = glGetUniformLocation(ShaderProgram, "d_light_d_color");
//	DLightSColorLoc = glGetUniformLocation(ShaderProgram, "d_light_s_color");
//
//	DLightAIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_a_intensity");
//	DLightDIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_d_intensity");
//	DLightSIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_s_intensity");
//
//	MaterialAColorLoc = glGetUniformLocation(ShaderProgram, "material_a_color");
//	MaterialDColorLoc = glGetUniformLocation(ShaderProgram, "material_d_color");
//	MaterialSColorLoc = glGetUniformLocation(ShaderProgram, "material_s_color");
//
//	MaterialShineLoc = glGetUniformLocation(ShaderProgram, "material_shininess");
//
//	Headlight = glGetUniformLocation(ShaderProgram, "headlight");
//
//
//	// Shaders can be deleted now
//	glDeleteShader(vertShader);
//	glDeleteShader(fragShader);
//
//	return true;
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

	/// Draw string in specific position on window
void drawText(string s, double x, double y, double z) {
	glRasterPos3f(x, y, z);
	for (char& c : s) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}
}

	/// PS: TEXTURE COORDINATE PRINTING:
	/// PROBLEM WITH SKETCHUP IS IT DOESNT NORMALIZE TEXTURE
	/// COORDINATES 
	/// :(
void printTextureCoordinates(ModelOBJ model) {
	
	for (int i = 0; i < model.getNumberOfVertices(); i++) {
		ModelOBJ::Vertex v = model.getVertex(i);
		cout << "Vertex with index " << i << " has texture coordinates (" << v.texCoord[0] << ", " << v.texCoord[1] << ")" << endl;
	}
}

	/// Set up directional light for openGL
void setDirectionalLight() {
	
	glUniform3f(DLightDirLoc, 0.5f, -0.5f, -1.0f);
	glUniform3f(DLightAColorLoc, 0.5f, 0.5f, 0.5f);
	glUniform3f(DLightDColorLoc, 0.f, 0.4f, 0.3f);
	glUniform3f(DLightSColorLoc, 0.6f, 0.6f, 0.7f);
	glUniform1f(DLightAIntensityLoc, 1.0f);
	glUniform1f(DLightDIntensityLoc, 1.0f);
	glUniform1f(DLightSIntensityLoc, 1.0f);
}

	/// Set up spot light for openGL
void setSpotLight() {
	
	glUniform3f(SLightDirLoc, 0.5f, -0.5f, -1.0f);
	glUniform3f(SLightAColorLoc, 0.5f, 0.5f, 0.5f);
	glUniform3f(SLightDColorLoc, 0.f, 0.4f, 0.3f);
	glUniform3f(SLightSColorLoc, 0.6f, 0.6f, 0.7f);
	glUniform1f(SLightAIntensityLoc, 1.0f);
	glUniform1f(SLightDIntensityLoc, 1.0f);
	glUniform1f(SLightSIntensityLoc, 1.0f);
}

	/// Set up head light for openGL
void setHeadLight() {
	
	glUniform1i(Headlight, HeadlightInt ? 1 : 0);

	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	GLfloat _light_position[] = { Cam.getPosition()[0], Cam.getPosition()[1], Cam.getPosition()[2], 1.0 }; // Last argument 0.0 for directional ligt, non-zero (1.0) for spotlight
	float _spotlight_position[] = { Cam.getPosition()[0], Cam.getPosition()[1], Cam.getPosition()[2] };

	glLightfv(GL_LIGHT1, GL_POSITION, _light_position);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 10.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, _spotlight_position);
}

/// Draw an object with normals
void drawObject(int numberOfIndices, GLuint texture, GLuint &VBO, GLuint &IBO, GLuint posLoc, GLuint texLoc, GLuint normalLoc) {

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
	if (normalLoc != -1) {
		glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(5 * sizeof(float)));
	}
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, 0);
}

ModelOBJ loadObject(const char* directory, GLuint &VBO, GLuint &IBO) {

	ModelOBJ model;

	if (!model.import(directory)) {
		cerr << "Error: cannot load model." << endl;
		return model;
	}
	model.getCenter(centerX, centerY, centerZ);
	cout << "x: " << centerX << ", y: " << centerY << ", z: " <<  centerZ << endl;

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

	return model;
}

void loadGrassObject(GLuint &VBO, GLuint &IBO) {

	// Prepare the vertices of the grass
	ModelOBJ::Vertex grassVerts[GRASS_VERTS_NUM];
	grassVerts[0].position[0] = -100.f;
	grassVerts[0].position[1] = 0.f;
	grassVerts[0].position[2] = -100.f;
	grassVerts[1].position[0] = 100.f;
	grassVerts[1].position[1] = 0.5f;
	grassVerts[1].position[2] = -100.f;
	grassVerts[2].position[0] = -100.f;
	grassVerts[2].position[1] = 0.5f;
	grassVerts[2].position[2] = 100.f;
	grassVerts[3].position[0] = 100.f;
	grassVerts[3].position[1] = 0.5f;
	grassVerts[3].position[2] = 100.f;
	grassVerts[0].texCoord[0] = 0.f;
	grassVerts[0].texCoord[1] = 0.f;
	grassVerts[1].texCoord[0] = 20.f;
	grassVerts[1].texCoord[1] = 0.f;
	grassVerts[2].texCoord[0] = 0.f;
	grassVerts[2].texCoord[1] = 20.f;
	grassVerts[3].texCoord[0] = 20.f;
	grassVerts[3].texCoord[1] = 20.f;

	// Generate a VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		GRASS_VERTS_NUM * sizeof(ModelOBJ::Vertex),
		grassVerts,
		GL_STATIC_DRAW);

	// Create an array of indices representing the triangles (faces of the cube)
	unsigned int grassTris[3 * GRASS_TRIS_NUM] = {
		0, 2, 1,
		1, 2, 3
	};

	// Create an IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		3 * GRASS_TRIS_NUM * sizeof(unsigned int),
		grassTris,
		GL_DYNAMIC_DRAW);

	return;
}

void loadMaterials(const char* TextureDirectory, const ModelOBJ &model, GLuint &TextureObject) {

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0;
	unsigned int TextureHeight = 0;

	//cout << "number of materials = " << model.getNumberOfMaterials() << endl;
	
	// Check the materials for the texture
	for (int i = 0; i < model.getNumberOfMaterials(); ++i) {

		// if the current material has a texture
		if (model.getMaterial(i).colorMapFilename != "") {

			// Load the texture
			if (TextureData != nullptr)
				free(TextureData);

			//cout << "round " << i << " trying to load @ file path " << "House-Model\\" << model.getMaterial(i).colorMapFilename.c_str() << endl;
			unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
				(TextureDirectory + model.getMaterial(i).colorMapFilename).c_str(),
				LCT_RGB, 8); // Remember to check the last 2 parameters
			if (fail != 0) {
				cerr << "Error: cannot load texture file "
					<< model.getMaterial(i).colorMapFilename << endl;
				return;
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

			// Set material parameters for house
			glUniform3f(MaterialAColorLoc, model.getMaterial(i).ambient[0], model.getMaterial(i).ambient[1], model.getMaterial(i).ambient[2]);
			glUniform3f(MaterialDColorLoc, model.getMaterial(i).diffuse[0], model.getMaterial(i).diffuse[1], model.getMaterial(i).diffuse[2]);
			glUniform3f(MaterialSColorLoc, model.getMaterial(i).specular[0], model.getMaterial(i).specular[1], model.getMaterial(i).specular[2]);
			glUniform1f(MaterialShineLoc, model.getMaterial(i).shininess);
		}
	}
}

void loadMaterial(const char* textureDirectory, GLuint &TextureObject) {

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0;
	unsigned int TextureHeight = 0;

	// Load the texture image for the object
	if (TextureData != nullptr)
		free(TextureData);
	unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
		textureDirectory, LCT_RGB, 8);
	if (fail != 0) {
		cerr << "Error: cannot load the texture file for the grass. " << endl;
		return;
	}

	// Create the texture object
	if (TextureObject != 0)
		glDeleteTextures(1, &TextureObject);
	glGenTextures(1, &TextureObject);

	// Bind it as a 2D texture (note that other types of textures are supported as well)
	glBindTexture(GL_TEXTURE_2D, TextureObject);

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

  /* --- eof main.cpp --- */