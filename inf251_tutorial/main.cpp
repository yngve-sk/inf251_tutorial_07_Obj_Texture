#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/extend.hpp>

#include "CameraV2.h"
#include "DirectionalLight.h"
#include "Spotlight.h"

#include "FunctionDeclarations.h"

#include "SingleTextureObject.h"
#include "MultiTextureObject.h"
#include "AnimatedTextureSquare.h"
#include "SingleTextureTerrain.h"
#include "GLLocStructs.h"

#include "Path.h"

#define PI 3.14159265

// --- OpenGL callbacks ---------------------------------------------------------------------------
void display();
void idle();
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void motion(int, int);
void specialInput(int, int, int);

//--- Scene objects ---------------------------------------------------------------------------
Camera _cam;
Spotlight _spotlight;
DirectionalLight _directionalLight;

SingleTextureTerrain _terrain;

SingleTextureObject _cat;
SingleTextureObject _house;

AnimatedTextureSquare _canvas;

Path _cameraPath;
Path _cameraLookAtPath;

// TODO MOVE THIS
bool initShaders();
bool initObjects();
bool initLights();
void drawText(string, double, double, double);
string readTextFile(const string&);
void loadUniformLocation(GLuint&, GLint&, char*);
void loadUniformLocationsFromShader(GLuint&);
void loadMatricesToUniform(mat4, mat4, mat4);

// --- GL Shader location ----------------------------------------------
GLuint ShaderProgram = -1;

// --- GL uniform locations --------------------------------------------
GLint  MVPMatrixLoc = -1;
GLint  MVMatrixLoc = -1;
GLint  MMatrixLoc = -1;
GLint  ViewMatrixLoc = -1;

MaterialGLLocs MaterialLocs;

GLint SamplerLoc = -1;

GLint NormalTextureLoc = -1;
GLint BumpMappingLoc = -1;
GLint DisplacementLoc = -1;

GLint ColorByHeightLoc = -1;

// --- GL attrib locations --------/------------------------------------
VertexGLLocs VertexLocs = {0, 1, 2};

// --- MICS-----------------------------
bool SpotlightInt = true;
bool SpotlightIntensity = true;

vec2 displacement = vec2(0,0);
vec2 displacementDelta = vec2(-1, 3);
int time = 0;

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
	glutSpecialFunc(specialInput);
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
	glCullFace(GL_BACK);		// back-faces should be remove
								//glEnable(GL_CULL_FACE);		// enable back-face culling
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_COLOR_MATERIAL);// Enable color tracking

	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// Transformation
	//_cam = *(new GLMCamera());


	// Shaders & mesh
	if (!initShaders() || !initObjects() || !initLights()) {
		cerr << "An error occurred, press Enter to quit ..." << endl;
		getchar();
		return -1;
	}

	// Start the main event loop
	glutMainLoop();
	return 0;
}

void display() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int width = glutGet(GLUT_WINDOW_WIDTH),
		height = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, width, height);

	glEnableVertexAttribArray(VertexLocs.posLoc);
	glEnableVertexAttribArray(VertexLocs.texLoc);
	glEnableVertexAttribArray(VertexLocs.normalLoc);

	// Enable the shader program
	assert(ShaderProgram != 0);
	glUseProgram(ShaderProgram);

	//NEEDS TO BE REFACTORED
	glUniform1i(ColorByHeightLoc, 1);
	glUniform2fv(DisplacementLoc, 1, &displacement[0]);

	// Camera 
	//_cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	
	mat4 VMatrix = _cam.getWorldToViewMatrix();
	mat4 PMatrix = _cam.getViewToProjectionMatrix();

	glUniformMatrix4fv(ViewMatrixLoc, 1, GL_FALSE, &VMatrix[0][0]);

	//Models
	//_house.usingBumpMapping = false;
	//loadMatricesToUniform(_house.transformation.getTransformationMatrix(), VMatrix, PMatrix);
	//_house.drawObject(VertexLocs, MaterialLocs);

	_terrain.usingBumpMapping = true;
	loadMatricesToUniform(_terrain.transformation.getTransformationMatrix(), VMatrix, PMatrix);
	_terrain.drawObject(VertexLocs, MaterialLocs, BumpMappingLoc, NormalTextureLoc);

	_canvas.usingBumpMapping = false;
	loadMatricesToUniform(_canvas.transformation.getTransformationMatrix(), VMatrix, PMatrix);
	_canvas.drawObject(VertexLocs, MaterialLocs);

	_cat.usingBumpMapping = false;
	loadMatricesToUniform(_cat.transformation.getTransformationMatrix(), VMatrix, PMatrix);
	_cat.drawObject(VertexLocs, MaterialLocs);

	// Draw projection text
	string projection;
	if (_cam.isProjectionPerspective()) {
		projection = "You are using perspective projection. Press 'p' for change.";
	}
	else {
		projection = "You are using orthogonal projection. Press 'p' for change.";
	}
	drawText(projection, -0.9, 0.9, 0);

	// Draw camera position text
	vec3 camPosition = _cam.getPosition();
	string position = "Camera position x: ";
	position.append(to_string(camPosition[0]) + ", y: ");
	position.append(to_string(camPosition[1])) + ", z: ";
	position.append(to_string(camPosition[2]));

	drawText(position, -0.7, 0.7, 0);

	// Disable the "position" vertex attribute (not necessary but recommended)
	glDisableVertexAttribArray(VertexLocs.posLoc);
	glDisableVertexAttribArray(VertexLocs.texLoc);
	glDisableVertexAttribArray(VertexLocs.normalLoc);

	// Disable the shader program (not necessary but recommended)
	glUseProgram(0);

	// Swap the frame buffers (off-screen rendering)
	glutSwapBuffers();
}

bool _idle_disable_house_rotation = false;
bool _idle_traverse_camera_movement_path = false;

int _idle_traverse_camera_wait = 20;
int _idle_traverse_camera_timeout = _idle_traverse_camera_wait;

bool _idle_traverse_camera_lookat_path = false;
int _idle_traverse_camera_lookat_wait = 20;
int _idle_traverse_camera_lookat_timeout = _idle_traverse_camera_lookat_wait;


void idle() {
	// rotate around Y-axis
	//LocalRotationY = _idle_disable_house_rotation ? LocalRotationY : LocalRotationY * glm::rotate(0.005f, vec3(0, 1, 0));
	_house.transformation.rotate(0.005f, vec3(0, 1, 0));

	_canvas.stepAnimation();

	if (_idle_traverse_camera_movement_path && (--_idle_traverse_camera_timeout%_idle_traverse_camera_wait)) {
		_cam.setPosition(_cameraPath.getNextCurvePoint());
	}

	if (_idle_traverse_camera_lookat_path && 
		(--_idle_traverse_camera_lookat_timeout % _idle_traverse_camera_lookat_wait)) {
		_cam.setLookAtPoint(_cameraLookAtPath.getNextCurvePoint());
	}

	
	time++;

	if ((time % 5) == 0){
		//_terrain.stepAnimation();
		displacement += displacementDelta;
		displacement.x = ((int)displacement.x % 2324);
		displacement.y = ((int)displacement.y % 2324);
		//cout << displacement << endl;
		time = 0;
	}

	glutPostRedisplay();
}

bool initShader(GLuint& program, string vShaderPath, string fShaderPath) {
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
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	// Check for linking error
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 1024, nullptr, errorLog);
		cerr << "Error: cannot link shader program.\nError log:\n" << errorLog << endl;
		return false;
	}

	// Make sure that the shader program can run
	glValidateProgram(program);

	// Check for validation error
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 1024, nullptr, errorLog);
		cerr << "Error: cannot validate shader program.\nError log:\n" << errorLog << endl;
		return false;
	}
	glUseProgram(ShaderProgram);

	loadUniformLocationsFromShader(ShaderProgram);

	// Shaders can be deleted now
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
}

void initCameraBoundingBox() {
	BoundingBox& b = _cam.bounds;

	b.setYBounding(-4000, -7);
}

bool initObjects() {
	// init bezier path
	vec3 controlpts[10];
	controlpts[0] = vec3(1500, -100, 3000);
	controlpts[1] = vec3(500, -250, 2500);
	controlpts[2] = vec3(1500, -900, 2000);
	controlpts[3] = vec3(3500, -350, 1500);
	controlpts[4] = vec3(1, -350, 1300);
	controlpts[5] = vec3(4000, -3500, 200);
	controlpts[6] = vec3(1, -350, 2500);
	controlpts[7] = vec3(1, -3500, -1500);
	controlpts[8] = vec3(1, 2550, 500);
	controlpts[9] = vec3(1500, -100, 3000);

	_cameraPath.bezier(controlpts, 10, 200);
	
	initCameraBoundingBox();

	_terrain.init("terrain\\bergen_1024x918.bin",
		"terrain\\bergen_terrain_texture.png");
	//_terrain.loadBumpMaps(13,
	//	10,
	//	"Animated-waves\\");
	_terrain.loadBumpMap("terrain\\waves.png");
	_terrain.transformation.rotate(180, vec3(1, 0, 0));
	_terrain.transformation.translate(vec3(-2613, -1, 2010));

	_cat.init("Objects\\cat\\cat.obj",
		"Objects\\cat\\cat_diff.png",
		"Objects\\cat\\cat_norm.png");
	_cat.transformation.rotate(180, vec3(1, 0, 0));
	_cat.transformation.translate(vec3(0, 0, 5));
	_cat.transformation.setScale(2.5);

	//_house.init("House-Model\\House.obj",
	//	"House-Model\\House\\basic_realistic.png",
	//	"Objects\\cat\\cat_norm.png");

	_canvas.init(vec3(0, 50, 10),
		(float)36.f*2.f,
		(float)18.5f*2.f,
		173,
		10,
		10,
		"Animated-textures\\");
	_canvas.transformation.translate(vec3(0, -90, -60));

	initLights();


//
	return true;
}

bool initLights() {

	_directionalLight = *(new DirectionalLight());
	_spotlight = *(new Spotlight());

	_directionalLight.loadToUniformAt(ShaderProgram, "dLight");
	_spotlight.loadToUniformAt(ShaderProgram, "spotlight");

	return true;
}


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

bool initShaders() {
	return initShader(ShaderProgram, "shader.v.glsl", "shader.f.glsl");
}

void loadUniformLocationsFromShader(GLuint& shaderProgram) {
	//loadUniformLocation(shaderProgram, WorldToProjectionMatrixLoc, "worldToProjectionMatrix");
	//loadUniformLocation(shaderProgram, ModelToWorldMatrixLoc, "modelToWorldMatrix");

	loadUniformLocation(ShaderProgram, MMatrixLoc, "MMatrix");
	loadUniformLocation(ShaderProgram, ViewMatrixLoc, "ViewMatrix");
	loadUniformLocation(ShaderProgram, MVMatrixLoc, "MVMatrix");
	loadUniformLocation(ShaderProgram, MVPMatrixLoc, "MVPMatrix");

	loadUniformLocation(shaderProgram, MaterialLocs.aColorLoc, "material.aColor");
	loadUniformLocation(shaderProgram, MaterialLocs.dColorLoc, "material.dColor");
	loadUniformLocation(shaderProgram, MaterialLocs.sColorLoc, "material.sColor");
	loadUniformLocation(shaderProgram, MaterialLocs.shineLoc, "material.shininess");

	loadUniformLocation(shaderProgram, SamplerLoc, "sampler");

	//loadUniformLocation(shaderProgram, CameraPositionLoc, "cameraPosition");
	loadUniformLocation(shaderProgram, NormalTextureLoc, "normalTexture");
	loadUniformLocation(shaderProgram, BumpMappingLoc, "bumpMapping");
	loadUniformLocation(shaderProgram, DisplacementLoc, "displacement");
	loadUniformLocation(shaderProgram, ColorByHeightLoc, "colorByHeight");
}

void loadAttribPointersFromShader(GLuint& shaderProgram) {

}

void loadUniformLocation(GLuint& ShaderProgram, GLint& loc, char* name) {
	loc = glGetUniformLocation(ShaderProgram, name);
	assert(name, loc!= -1);
}

/// Draw string in specific position on window
void drawText(string s, double x, double y, double z) {
	glRasterPos3f(x, y, z);
	for (char& c : s) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}
}

void specialInput(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
		_cam.moveForward();
		break;
	case GLUT_KEY_DOWN:
		_cam.moveBackward();
		break;
	case GLUT_KEY_LEFT:
		_cam.strafeLeft();
		break;
	case GLUT_KEY_RIGHT:
		_cam.strafeRight();
		break;
	}

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
		_idle_disable_house_rotation = !_idle_disable_house_rotation;
		glutPostRedisplay();
		break;
	case 'w':
		std::cout << "forkwards" << std::endl;
		_cam.moveForward();
		glutPostRedisplay();
		break;
	case 'a':
		_cam.strafeLeft();
		glutPostRedisplay();
		break;
	case 's':
		std::cout << "backwards" << std::endl;
		_cam.moveBackward();
		glutPostRedisplay();
		break;
	case 'd':
		_cam.strafeRight();
		glutPostRedisplay();
		break;
	case 'c':
		_cam.moveDown();
		glutPostRedisplay();
		break;
	case ' ':
		_cam.moveUp();
		glutPostRedisplay();
		break;
	case 'p':
		_cam.switchPerspective();
		glutPostRedisplay();
		break;
	case 'l':
		_spotlight.toggleOnOff();
		glutPostRedisplay();
		break;
	case 'k':
		_spotlight.increaseIntensity();
		glutPostRedisplay();
		break;
	case 'j':
		_spotlight.decreaseIntensity();
		glutPostRedisplay();
		break;
	case 'b':
		//colorByHeightOnOff *= -1;
		//glUniform1i(ColorByHeightLoc, colorByHeightOnOff);
		break;
	case 'h':
		_idle_traverse_camera_movement_path = !_idle_traverse_camera_movement_path;
		glutPostRedisplay();
		break;
	case 't':
		_idle_traverse_camera_lookat_path = !_idle_traverse_camera_lookat_path;
		glutPostRedisplay();
		break;
	case 'y':
		_cam.generateCircularBezierAroundCurrentPosition(_cameraLookAtPath, 40, -10);
		glutPostRedisplay();
		break;
	}
}

int MouseX, MouseY;		///< The last position of the mouse
int MouseButton;		///< The last mouse button pressed or released

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
		_cam.mouseUpdate(vec2(x, y));

		updateMousePosition(x, y);
	}
	if (MouseButton == GLUT_MIDDLE_BUTTON) {
		//_cam.adjustZoom(vec2(MouseX, MouseY), vec2(x, y));

		updateMousePosition(x, y);
	}
	if (MouseButton == GLUT_LEFT_BUTTON) {
		_cam.mouseUpdate(vec2(x, y));

		updateMousePosition(x, y);
	}

	glutPostRedisplay(); // Specify that the scene needs to be updated
}

void loadMatricesToUniform(mat4 MMatrix, mat4 VMatrix, mat4 PMatrix) {

	mat4 MVMatrix = VMatrix * MMatrix;
	mat4 MVPMatrix = PMatrix * MVMatrix;

	glUniformMatrix4fv(MMatrixLoc, 1, GL_FALSE, &MMatrix[0][0]);
	glUniformMatrix4fv(MVMatrixLoc, 1, GL_FALSE, &MVMatrix[0][0]);
	glUniformMatrix4fv(MVPMatrixLoc, 1, GL_FALSE, &MVPMatrix[0][0]);
}