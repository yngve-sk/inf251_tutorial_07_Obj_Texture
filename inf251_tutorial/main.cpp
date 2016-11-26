#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/extend.hpp>

#include "glm_camera.h"
#include "DirectionalLight.h"
#include "Spotlight.h"

#include "FunctionDeclarations.h"
#include "GLLocStructs.h"

#include "SingleTextureObject.h"
#include "MultiTextureObject.h"
#include "AnimatedTextureSquare.h"

// --- OpenGL callbacks ---------------------------------------------------------------------------
void display();
void idle();
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void motion(int, int);

// --- Scene objects ---------------------------------------------------------------------------
GLMCamera Cam;
Spotlight theSpotlight;
DirectionalLight theDirectionalLight;

SingleTextureObject terrain;

SingleTextureObject cat;
SingleTextureObject house;
AnimatedTextureSquare canvas;


// --- GL Shader location ----------------------------------------------
GLuint ShaderProgram = -1;

// --- GL uniform locations --------------------------------------------
GLint ModelToWorldMatrixLoc = -1,
	  WorldToProjectionMatrixLoc = -1;

GLint MaterialAColorLoc = -1, 
	  MaterialDColorLoc = -1,
	  MaterialSColorLoc = -1,
	  MaterialShineLoc = -1;

GLint SamplerLoc = -1;

GLint CameraPositionLoc = -1;

GLint NormalTextureLoc = -1;

// --- GL attrib locations --------------------------------------------
GLint PosLoc = 0,
	  NormalLoc = 1,
	  TexCoordsLoc = 2; 

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
	glCullFace(GL_BACK);		// back-faces should be remove
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
	if (!initShaders() || !initObjects()) {
		cerr << "An error occurred, press Enter to quit ..." << endl;
		getchar();
		return -1;
	}

	// Start the main event loop
	glutMainLoop();
	return 0;
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

	// Shaders can be deleted now
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
}

void initObjects() {
	
	terrain = new SingleTextureObject();
	terrain.loadObject("Objects\\cat\\cat.obj");

	cat = new SingleTextureObject();
}

//void bindUniformLocs(GLint shader) {
//	GeneralGLLocs.transformations.modelToWorldMatrixLoc
//}


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
	loadUniformLocation(shaderProgram, WorldToProjectionMatrixLoc, "worldToProjectionMatrix");
	loadUniformLocation(shaderProgram, ModelToWorldMatrixLoc, "modelToWorldMatrix");

	loadUniformLocation(shaderProgram, MaterialAColorLoc, "material.aColor");
	loadUniformLocation(shaderProgram, MaterialDColorLoc, "material.dColor");
	loadUniformLocation(shaderProgram, MaterialSColorLoc, "material.sColor");
	loadUniformLocation(shaderProgram, MaterialShineLoc, "material.shininess");

	loadUniformLocation(shaderProgram, SamplerLoc, "sampler");

	loadUniformLocation(shaderProgram, CameraPositionLoc, "cameraPosition");
	loadUniformLocation(shaderProgram, NormalTextureLoc, "normal_texture");
}

void loadAttribPointersFromShader(GLuint& shaderProgram) {

}

void loadUniformLocation(GLuint& ShaderProgram, GLint& loc, char* name) {
	loc = glGetUniformLocation(ShaderProgram, name);
	assert(loc!= -1);
}