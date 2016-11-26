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
#include "help_structs.h"

using namespace std;

#define PI 3.14159265




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
void setHeaDLight();
void drawText(string, double, double, double);
void drawMesh(int, GLuint, GLuint, GLuint&, GLuint&, GLuint, GLuint, GLuint);
void drawObject(int, GLuint, GLuint&, GLuint&, GLuint, GLuint, GLuint);
void drawObject(int, GLuint, GLuint, GLuint&, GLuint&, GLuint, GLuint, GLuint);
ModelOBJ loadObject(const char*, GLuint&, GLuint&);
void loadGrassObject(GLuint&, GLuint&);
void loadCanvasObject(GLuint&, GLuint&);
void loadMaterials(const char*, const ModelOBJ&, GLuint&);
void loadMaterial(const char*, GLuint&, unsigned int&, unsigned int&);
void loadMaterial(const char*, GLuint&, const ModelOBJ::Material&);
void loadMaterial(const char*, GLuint&);
GLuint LoadTexture(const char*, int, int);

void loadTexture(const char*, GLuint&);
void loadCanvasTextures(GLuint*);

// --- Global variables ---------------------------------------------------------------------------
// 3D model
// Model of the house
ModelOBJ Model;		///< A 3D model
GLuint VBO = 0;		///< A vertex buffer object
GLuint IBO = 0;		///< An index buffer object

// Model of the cube
ModelOBJ Model2;
GLuint cubeVBO = 0;
GLuint cubeIBO = 0;

// Model of the cat
ModelOBJ cat;
GLuint catVBO = 0;
GLuint catIBO = 0;

// Model of the grass
const int GRASS_VERTS_NUM = 4;
const int GRASS_TRIS_NUM = 2;
GLuint GrassVBO = 0;
GLuint GrassIBO = 0;

// Model of the canvas
const int CANVAS_VERTS_NUM = 4;
const int CANVAS_TRIS_NUM = 2;
GLuint CanvasVBO = 0;
GLuint CanvasIBO = 0;

GLuint CanvasTextureArray[256];


// Texture
GLuint TextureObjects[32];
GLuint TextureObject = 0;				///< A texture object


										// Texture for second object
GLuint TextureObject2 = 0;				///< A texture object

										// Texture for the grass
GLuint TexGrassObj = 0;
GLuint normal_texture = 0;

// Texture for the cat
GLuint TexCatObj = 0;
GLuint normal_texture_cat = 0;

// Active texture for canvas
GLuint ActiveTexCanvas = -1;


// Shaders
GLuint ShaderProgram = 0;	///< A shader program
GLuint HouseShaderProgram = 0;

GLint TrLoc = 0;				///< model-view matrix uniform variable
GLint LocalTrLoc = -1;

GLint SamplerLoc = -1;			///< texture sampler uniform variable
GLint CameraPositionLoc = -1;

// Lighting params (DIRECTIONAL)
LightParameters DLight;

GLint DLightDirLoc = -1;

GLint DLightAColorLoc = -1;
GLint DLightDColorLoc = -1;
GLint DLightSColorLoc = -1;

GLint DLightAIntensityLoc = -1;
GLint DLightDIntensityLoc = -1;
GLint DLightSIntensityLoc = -1;

MaterialParameters HouseMaterial;
MaterialParameters GrassMaterial;

GLint MaterialAColorLoc = -1;
GLint MaterialDColorLoc = -1;
GLint MaterialSColorLoc = -1;
GLint MaterialShineLoc = -1;


MaterialGLLoc materialLoc;

// Lighting params (DIRECTIONAL)
LightParameters SLight;

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

mat4 LocalRotationX, LocalRotationY, LocalTranslation;
float LocalScaling;

// Non-transformation matrix
mat4 NonTransformation = mat4(1, 0, 0, 0,
							  0, 1, 0, 0,
							  0, 0, 1, 0,
							  0, 0, 0, 1);
mat4 LocalRotation = mat4(1, 0, 0, 0,
							0, cos(180 * PI / 180.0), -sin(PI), 0,
							0, sin(180 * PI / 180.0), -cos(PI), 0,
							0, 0, 0, 1);
mat4 catTransformation;

GLuint BumpMapping = -1;
GLuint ColorByHeightLoc = -1;
int colorByHeightOnOff = -1;


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
	if (!initShaders() || !initMesh()) {
		cerr << "An error occurred, press Enter to quit ..." << endl;
		getchar();
		return -1;
	}

	// Start the main event loop
	glutMainLoop();
	return 0;
}

bool disable_house_rotation = false;


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

	glUniform1i(ColorByHeightLoc, 1);

	Cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	mat4 worldToProjection = Cam.computeCameraTransform();

	glUniform3fv(CameraPositionLoc, 1, &Cam.getPosition()[0]);
	glUniformMatrix4fv(TrLoc, 1, GL_FALSE, &worldToProjection[0][0]);
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);

	setDirectionalLight();
	setSpotLight();
	setHeaDLight();
	
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

	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &LocalRotationY[0][0]);
	// Draw the house

	/*Model.getCenter(centerX, centerY, centerZ);
	GLint centerLoc = glGetAttribLocation(ShaderProgram, "center");

	vec3 centerNone = vec3(0, 0, 0);
	vec3 centerv = vec3(centerX, centerY, centerZ);
	glUniform1fv(centerLoc, sizeof(fvec3), &centerv[0]);
	//
	mat4 translateToCenter = glm::translate(vec3(0.0f, -50, 0.0f));
	//mat4 translateFromCenter = glm::translate(vec3(-centerX, -centerY, -centerZ));
	//
	//mat4 rotateMat = translateFromCenter * LocalRotationY * translateToCenter;
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);*/

	/* Code for multiple textures
	for (int i = 0; i < Model.getNumberOfMeshes(); i++) {
		drawMesh(Model.getMesh(i).triangleCount * 3, Model.getMesh(i).startIndex, TextureObjects[i], VBO, IBO, posLoc, texLoc, normalLoc);
	}*/

	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(5 * sizeof(float)));
	glUniform1i(BumpMapping, 0);

	drawObject(Model.getNumberOfIndices(), TextureObject, VBO, IBO, posLoc, texLoc, normalLoc);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex), reinterpret_cast<const GLvoid*>(5 * sizeof(float)));
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);

	// Draw the cube
	drawObject(Model2.getNumberOfIndices(), TextureObject2, cubeVBO, cubeIBO, posLoc, texLoc, -1);
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &NonTransformation[0][0]);

	// Set material parameters for grass
	glUniform3f(MaterialAColorLoc, 0.9f, 1.0f, 0.9f);
	glUniform3f(MaterialDColorLoc, 0.3f, 1.0f, 0.3f);
	glUniform3f(MaterialSColorLoc, 0.1f, 0.1f, 0.1f);
	glUniform1f(MaterialShineLoc, 10.0f);

	glUniform1i(BumpMapping, 1);
	// Draw the grass
	//drawObject(3 * GRASS_TRIS_NUM, TexGrassObj, GrassVBO, GrassIBO, posLoc, texLoc, -1);
	drawObject(3 * GRASS_TRIS_NUM, TexGrassObj, normal_texture, GrassVBO, GrassIBO, posLoc, texLoc, normalLoc);


	glUniform1i(BumpMapping, 0);
	// Draw the canvas
	drawObject(3 * CANVAS_TRIS_NUM, ActiveTexCanvas, CanvasVBO, CanvasIBO, posLoc, texLoc, -1);

	catTransformation = glm::rotate((float)(180 * PI / 180.0), vec3(1, 0, 0)) * glm::translate(vec3(5, -0.5, 8)) * glm::scale(vec3(5, 5, 5));
	// Draw the cat
	glUniform1i(BumpMapping, 1);
	glUniformMatrix4fv(LocalTrLoc, 1, GL_FALSE, &catTransformation[0][0]);
	drawObject(cat.getNumberOfIndices(), TexCatObj, normal_texture_cat, catVBO, catIBO, posLoc, texLoc, -1);
	//drawObject(cat.getNumberOfIndices(), TexCatObj, normal_texture, catVBO, catIBO, posLoc, texLoc, -1);

	glUniform1i(BumpMapping, 0);
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

int canvas_frame = 0;
int steps = 10;
int canvas_frame_wait = 10;
/// Called at regular intervals (can be used for animations)
void idle() {
	// rotate around Y-axis
	LocalRotationY = disable_house_rotation ? LocalRotationY : LocalRotationY * glm::rotate(0.005f, vec3(0, 1, 0));

	if (--steps == 0) {
		canvas_frame = (++canvas_frame) % 173;
		steps = canvas_frame_wait;
	}
	ActiveTexCanvas = CanvasTextureArray[canvas_frame];

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
		disable_house_rotation = !disable_house_rotation;
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
	case '9':
		Cam.flip();
		glutPostRedisplay();
		break;
	case 'b': 
		colorByHeightOnOff *= -1;
		//glUniform1i(ColorByHeightLoc, colorByHeightOnOff);
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
	/*for (int i = 0; i < Model.getNumberOfMeshes(); i++) {
		ModelOBJ::Mesh m = Model.getMesh(i);
		cout << "mesh material: " << m.pMaterial << endl;
		//ModelOBJ::Material mat = *(m.pMaterial);

		loadMaterial("House-Model\\", TextureObjects[i], *m.pMaterial);
	}*/
	loadMaterials("House-Model\\", Model, TextureObject);

	Model2 = loadObject("Objects\\cube-textured\\cube.obj", cubeVBO, cubeIBO);
	loadMaterials("Objects\\cube-textured\\", Model2, TextureObject2);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	cat = loadObject("Objects\\cat\\cat.obj", catVBO, catIBO);
	//loadMaterials("Objects\\cat\\", cat, TexCatObj);
	loadMaterial("Objects\\cat\\cat_diff.png", TexCatObj);
	loadMaterial("Objects\\cat\\cat_norm.png", normal_texture);
	

	loadGrassObject(GrassVBO, GrassIBO);
	loadMaterial("pngs\\grass.png", TexGrassObj);
	loadMaterial("pngs\\normalMap3.png", normal_texture);

	loadCanvasObject(CanvasVBO, CanvasIBO);
	loadCanvasTextures(&CanvasTextureArray[0]);

	ActiveTexCanvas = CanvasTextureArray[0];

	return true;
} /* initBuffers() */

void loadCanvasTextures(GLuint* start) {
	int num_frames = 173;

	GLuint* offset = start;
	int step = sizeof(GLuint);

	for (int i = 1; i < num_frames + 1; i++) {
		string path = "Animated-textures\\" + std::to_string(i) + ".png";
		loadTexture(&path[0], start[i-1]);
		//cout << "loading texture " << i << " filepath: " << path << endl;
	}

	for (int i = 0; i < num_frames; i++) {
		//cout << "arr[" << std::to_string(i) << "] = " << start[i] << endl;
	}
}

bool initShader(GLuint& program, 
				string vShaderPath, string fShaderPath, 
				GLint& worldToProjectionMatrixLoc, GLint& modelToWorldMatrixLoc,
				GLint& samplerLoc,
				GLint& cameraPositionLoc,
				LightParameters& DLight,
				GLint& materialAColorLoc, GLint& materialDColorLoc, GLint& materialSColorLoc,
			//	vec3& materialADSColorLoc,
				GLint& materialShineLoc, GLint& headlightLoc, GLuint& bumpMapping, 
			    GLuint& colorByHeightLoc) {
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
	worldToProjectionMatrixLoc = glGetUniformLocation(ShaderProgram, "worldToProjectionMatrix");
	// normal transformation (not camera)
	modelToWorldMatrixLoc = glGetUniformLocation(ShaderProgram, "modelToWorldMatrix");

	// bump mapping (0-false, 1-true)
	bumpMapping = glGetUniformLocation(ShaderProgram, "bump_mapping");


	samplerLoc = glGetUniformLocation(ShaderProgram, "sampler");
	assert(TrLoc != -1
		&& samplerLoc != -1
	);

	cameraPositionLoc = glGetUniformLocation(ShaderProgram, "camera_position");

	DLight.DirLoc = glGetUniformLocation(ShaderProgram, "d_light_direction");

	DLight.AColorLoc = glGetUniformLocation(ShaderProgram, "d_light_a_color");
	DLight.DColorLoc = glGetUniformLocation(ShaderProgram, "d_light_d_color");
	DLight.SColorLoc = glGetUniformLocation(ShaderProgram, "d_light_s_color");
		  
	DLight.AIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_a_intensity");
	DLight.DIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_d_intensity");
	DLight.SIntensityLoc = glGetUniformLocation(ShaderProgram, "d_light_s_intensity");

	materialAColorLoc = glGetUniformLocation(ShaderProgram, "material_a_color");
	materialDColorLoc = glGetUniformLocation(ShaderProgram, "material_d_color");
	materialSColorLoc = glGetUniformLocation(ShaderProgram, "material_s_color");

	materialShineLoc = glGetUniformLocation(ShaderProgram, "material_shininess");

	headlightLoc = glGetUniformLocation(ShaderProgram, "headlight");

	colorByHeightLoc = glGetUniformLocation(ShaderProgram, "colorByHeight");

	// Shaders can be deleted now
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;

}

  /// Initialize shaders. Return false if initialization fail
bool initShaders() {
	if (1)
	return initShader(ShaderProgram, "shader.v.glsl", "shader.f.glsl", 
						TrLoc, LocalTrLoc, SamplerLoc,
						CameraPositionLoc, 
						DLight,
						MaterialAColorLoc, MaterialDColorLoc, MaterialSColorLoc,
						MaterialShineLoc, 
						Headlight,
						BumpMapping, 
						ColorByHeightLoc);

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
	
	glUniform3f(DLight.DirLoc,			0.5f, -0.5f, -1.0f);
	glUniform3f(DLight.AColorLoc,		0.5f, 0.5f, 0.5f);
	glUniform3f(DLight.DColorLoc,		0.f, 0.4f, 0.3f);
	glUniform3f(DLight.SColorLoc,		0.6f, 0.6f, 0.7f);
	glUniform1f(DLight.AIntensityLoc, 1.0f);
	glUniform1f(DLight.DIntensityLoc, 1.0f);
	glUniform1f(DLight.SIntensityLoc, 1.0f);
}

	/// Set up spot light for openGL
void setSpotLight() {
	
	glUniform3f(SLight.DirLoc, 0.5f, -0.5f, -1.0f);
	glUniform3f(SLight.AColorLoc, 0.5f, 0.5f, 0.5f);
	glUniform3f(SLight.DColorLoc, 0.f, 0.4f, 0.3f);
	glUniform3f(SLight.SColorLoc, 0.6f, 0.6f, 0.7f);
	glUniform1f(SLight.AIntensityLoc, 1.0f);
	glUniform1f(SLight.DIntensityLoc, 1.0f);
	glUniform1f(SLight.SIntensityLoc, 1.0f);
}

	/// Set up head light for openGL
void setHeaDLight() {
	
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
void drawMesh(int numberOfIndices, GLuint startIndex, GLuint texture, GLuint &VBO, GLuint &IBO, GLuint posLoc, GLuint texLoc, GLuint normalLoc) {

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
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(startIndex));
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

/// Draw an object with bump mapping
void drawObject(int numberOfIndices, GLuint texture, GLuint normalTexture, GLuint &VBO, GLuint &IBO, GLuint posLoc, GLuint texLoc, GLuint normalLoc) {

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	int normal_location = glGetUniformLocation(ShaderProgram, "normal_texture");
	glUniform1i(normal_location, 1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);

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

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

ModelOBJ loadObject(const char* directory, GLuint &VBO, GLuint &IBO) {

	ModelOBJ model;

	if (!model.import(directory)) {
		cerr << "Error: cannot load model." << endl;
		return model;
	}
	cout << "Imported model..." << endl;

	// VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		model.getVertexBuffer(),
		GL_STATIC_DRAW);

	//cout << "starting ibo" << endl;

	// IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		model.getNumberOfIndices() * sizeof(unsigned int),
		model.getIndexBuffer(),
		GL_STATIC_DRAW);

	//cout << "ibo end" << endl;

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

	GrassMaterial.MaterialAColor = vec3(0.9f, 1.0f, 0.9f);
	GrassMaterial.MaterialDColor = vec3(0.3f, 1.0f, 0.3f);
	GrassMaterial.MaterialSColor = vec3(0.1f, 0.1f, 0.1f);
	GrassMaterial.MaterialShine = 10.f;


	// Create an IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		3 * GRASS_TRIS_NUM * sizeof(unsigned int),
		grassTris,
		GL_DYNAMIC_DRAW);

	return;
}

MaterialParameters* activeMaterial;
void activateMaterial(MaterialParameters* active) {
	activeMaterial = active;
}

/**
* Load canvas object...
*/
void loadCanvasObject(GLuint &VBO, GLuint &IBO) {

	float width = 36.f*2.f,
		height = 18.5f*2.f,
		z = 5.f,
		x = 20.f,
		y = -40.f;

	ModelOBJ::Vertex canvasVerts[CANVAS_VERTS_NUM];

	canvasVerts[0].position[0] = x;
	canvasVerts[0].position[1] = y;
	canvasVerts[0].position[2] = z;

	canvasVerts[1].position[0] = x + width;
	canvasVerts[1].position[1] = y;
	canvasVerts[1].position[2] = z;

	canvasVerts[2].position[0] = x + width;
	canvasVerts[2].position[1] = y + height;
	canvasVerts[2].position[2] = z;

	canvasVerts[3].position[0] = x;
	canvasVerts[3].position[1] = y + height;
	canvasVerts[3].position[2] = z;


	canvasVerts[0].texCoord[0] = 0.f;
	canvasVerts[0].texCoord[1] = 0.f;

	canvasVerts[1].texCoord[0] = 1.f;
	canvasVerts[1].texCoord[1] = 0.f;

	canvasVerts[2].texCoord[0] = 1.f;
	canvasVerts[2].texCoord[1] = 1.f;

	canvasVerts[3].texCoord[0] = 0.f;
	canvasVerts[3].texCoord[1] = 1.f;

	// Generate a VBO
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		CANVAS_VERTS_NUM * sizeof(ModelOBJ::Vertex),
		canvasVerts,
		GL_STATIC_DRAW);

	// Create an array of indices representing the triangles (faces of the cube)
	unsigned int canvasTris[3 * CANVAS_TRIS_NUM] = {
		0, 1, 2,
		2, 3, 0
	};

	// Create an IBO
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		3 * CANVAS_TRIS_NUM * sizeof(unsigned int),
		canvasTris,
		GL_DYNAMIC_DRAW);

	return;
}

void loadMaterials(const char* TextureDirectory, const ModelOBJ &model, GLuint &TextureObject) {

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0;
	unsigned int TextureHeight = 0;

	cout << "number of materials = " << model.getNumberOfMaterials() << endl;
	
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

void loadMaterial(const char* TextureDirectory, GLuint &TextureObject, unsigned int &width, unsigned int &height) {

	unsigned char* TextureData = nullptr;

	// Load the texture
	if (TextureData != nullptr)
		free(TextureData);

	unsigned int fail = lodepng_decode_file(&TextureData, &width, &height,
		TextureDirectory,
		LCT_RGB, 8); // Remember to check the last 2 parameters
	if (fail != 0) {
		cerr << "Error: cannot load texture file "
			<< TextureDirectory << endl;
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
		width,
		height,
		0,
		GL_RGB,			// remember to check this
		GL_UNSIGNED_BYTE,
		TextureData
		);

	// Configure texture parameter
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void loadMaterial(const char* TextureDirectory, GLuint &TextureObject, const ModelOBJ::Material &material) {

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
		if (TextureObject != 0)
			glDeleteTextures(1, &TextureObject);
		glGenTextures(1, &TextureObject);

		// Bind it as a 2D texture (note that other types of textures are supported as well)
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureObject);

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
		//glUniform3f(MaterialAColorLoc, material.ambient[0], material.ambient[1], material.ambient[2]);
		//glUniform3f(MaterialDColorLoc, material.diffuse[0], material.diffuse[1], material.diffuse[2]);
		//glUniform3f(MaterialSColorLoc, material.specular[0], material.specular[1], material.specular[2]);
		//glUniform1f(MaterialShineLoc, material.shininess);

	}
}

/**
* Loads one texture and puts it @ name
*/
void loadTexture(const char* loc, GLuint& name) {

	unsigned char* TextureData = nullptr;
	unsigned int TextureWidth = 0,
				 TextureHeight = 0;

	if (TextureData != nullptr)
		free(TextureData);
	unsigned int fail = lodepng_decode_file(&TextureData, &TextureWidth, &TextureHeight,
		loc, LCT_RGB, 8);

	if (fail != 0) {
		cerr << "Error: cant load the texture at loc " << loc << endl;
	}

	if (name != 0)
		glDeleteTextures(1, &name);
	glGenTextures(1, &name);

	// Bind it as a 2D texture (note that other types of textures are supported as well)
	glBindTexture(GL_TEXTURE_2D, name);

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

GLuint LoadTexture(const char * filename, int width, int height)
{
	GLuint texture;
	unsigned char * data;
	FILE * file;

	//The following code will read in our RAW file  
	file = fopen(filename, "rb");

	if (file == NULL) return 0;
	data = (unsigned char *)malloc(width * height * 3);
	fread(data, width * height * 3, 1, file);

	fclose(file);

	glGenTextures(1, &texture); //generate the texture with the loaded data  
	glBindTexture(GL_TEXTURE_2D, texture); //bind the texture to it’s array  

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //set texture environment parameters  

																 //And if you go and use extensions, you can use Anisotropic filtering textures which are of an  
																 //even better quality, but this will do for now.  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Here we are setting the parameter to repeat the texture instead of clamping the texture  
	//to the edge of our shape.  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Generate the texture  
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	free(data); //free the texture  

	return texture; //return whether it was successfull  
}

  /* --- eof main.cpp --- */