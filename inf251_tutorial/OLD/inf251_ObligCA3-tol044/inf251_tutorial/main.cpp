#include <gl/glew.h>
#include <gl/glut.h>
#include <gl/GL.h>

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>

#include "lodepng.h"
#include "model_obj.h"
#include "Vector3.h"
#include "Matrix4.h"

using namespace std;

// ************************************************************************************************
//	 Display can be manipulated with the following keys and mouse gestures:
//
//	 1,2,3..				toggles corresponding lightsource on/off
//	 W,A,S,D				FPS-style camera control (in perspective mode)
//   Keypad +/-				control the movement of current movable lightsource
//   F						Toggles the fog-effect (quite cool with asteroids!)
//	 M						Toggle Auto-Animation mode on/off
//   P						Projection mode toggle
//	 R						Reset camera status and main model
//   T						Toggle simple toon effect
//   Arrow keys				increase/decrease model rotation on X/Y (in AutoAnimation mode only)
//	 Shift L/R arrow		increase/decrease model rotation on Z (in AutoAnimation mode only)
//
//   Q						Terminate the application
//   G						Display the current OpenGL version
//
//   Mouse:
//	 Left Button + Move		Change camera orientation/view direction
//   Middle Btn + Fwd/Back  Camera zoom effect
//   Right Btn + Move		Rotates the car about its own X/Y axis (if AutoAnimation mode OFF)
//
// ************************************************************************************************

// --- Data types ---------------------------------------------------------------------------------

/// A simple structure to handle a moving camera
struct Camera {
	Vector3f position;	///< the position of the camera
	Vector3f target;	///< the direction the camera is looking at
	Vector3f up;		///< the up vector of the camera

	float fov;			///< camera field of view
	float ar;			///< camera aspect ratio

	float zNear, zFar;	///< depth of the near and far plane

	float zoom;			///< an additional scaling parameter
};

/// A simple structure to handle models/object matrices
struct ModelMatrix {
	Matrix4f rotationX;
	Matrix4f rotationY;
	Matrix4f rotationZ;
	Vector3f translation;
	Vector3f defaultPosition;
	Vector3f rotateDelta;
	Vector3f transDelta;
	float scaling;
};

/// Structure to control model animation
struct AnimateModel {
	bool activated;
	float stepsizeX;
	float stepsizeY;
	float stepsizeZ;
	float sphereIncrement;

};

/// Structure with texture information
struct ModelTexture {
	unordered_map <string, GLuint> name2objectMap;
	unsigned int width;
	unsigned int height;
	unsigned char *data;

};

// --- OpenGL callbacks ---------------------------------------------------------------------------
void display();
void idle();
void keyboard(unsigned char, int, int);
void keySpecial(int, int, int);
void mouse(int, int, int, int);
void motion(int, int);

// --- Other methods ------------------------------------------------------------------------------
bool initMesh();
bool initShaders();
void initAsteroids();
void initModelMatrix(ModelMatrix&);
void initAnimateModel(AnimateModel&);
void initModelTextures(const ModelOBJ&);
void initCamera(Camera&);
void drawTexture(const ModelOBJ&);

Matrix4f computeCameraView(const Camera&);
Matrix4f computeCameraZoom(const Camera&);
Matrix4f computePerspectiveProjection(const Camera&);
Matrix4f computeOrthogonalProjection(const Camera& cam, int width, int height);
Matrix4f getActiveProjectionMatrix(int width, int height);

string readTextFile(const string&);


// --- Global variables ---------------------------------------------------------------------------

// Lighting settings
const unsigned int TOTAL_LIGHTS = 3;	// should correspond to shader light count
unsigned int idLight = 0;				// for fiddling with lights
float angle = 90.0f;

// Light 1 (id 0): is the moving positional, Light 2 is the camera headlight, light 3 is the directional light (sun?)
// Set up the initial direction & positions (where applicable) of light sources
Vector3f vLightSource[TOTAL_LIGHTS] = { Vector3f(-0.4f, 0.9f, -0.4f), Vector3f(0.0f, 0.0f, -2.0f), Vector3f(2.0f, -2.0f, -3.0f) };

// Corresponding lightsource w-coordinate -> (1.0f) positional light,  (0.0f) directional light, (-1.0f) spotlight ??
GLfloat fLightType[TOTAL_LIGHTS] = { 1.0f, 1.0f, 0.0f };

Vector3f vLightAcolor[TOTAL_LIGHTS] = { Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f) };
Vector3f vLightDcolor[TOTAL_LIGHTS] = { Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f) };
Vector3f vLightScolor[TOTAL_LIGHTS] = { Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f) };

// Light Intensity  - also used for effectively "switching off" lightsources hopefully 
// light intensity after textures : new version now basically keeps RGB information of lightsource - vLightA/D/S not used.
// temp *** light intensity .x is used for differentiation between stars and bulb lightsource!!! ****
const Vector3f LightIntensity[TOTAL_LIGHTS] = { Vector3f(0.95f, 0.2f, 0.1f), Vector3f(0.6f, 0.6f, 0.5f), Vector3f(0.6f, 0.6f, 0.5f) };
Vector3f LightIads[TOTAL_LIGHTS] = { Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f) };

// Shader program
GLuint ShaderProgram = 0;

// used to access uniform variables Model, View, Projection matrices
GLint M_Loc = -1;
GLint V_Loc = -1;
GLint MV_Loc = -1;
GLint MVP_Loc = -1;

GLint LightEmLoc = -1;
GLint LightSourceLoc = -1;
GLint LightTypeLoc = -1;

GLint LightIadsLoc = -1;

GLint MaterialKaLoc = -1;
GLint MaterialKdLoc = -1;
GLint MaterialKsLoc = -1;
GLint MaterialShineLoc = -1;


// 3D models
ModelOBJ Model;								///< A 3D model
ModelOBJ Sphere;
ModelOBJ Banner;
ModelOBJ Boulder;

GLuint carVBO = 0;							///< A vertex buffer object
GLuint carIBO = 0;							///< An index buffer object
GLuint sphereVBO = 0;						///< A vertex buffer object
GLuint sphereIBO = 0;
GLuint bannerVBO = 0;						///< A vertex buffer object
GLuint bannerIBO = 0;

// Stars setup
const unsigned int STARS_COUNT = 300;		///< The number of stars displayed
GLuint starVBO[STARS_COUNT];
GLuint starIBO[STARS_COUNT];

// Textures
unordered_map <string, int> TextureMap;		/// using unordered map (hashmap) for texture objects
ModelTexture texture;

GLint SamplerLoc = -1;						/// texture sampler uniform variable
GLint IndexLoc = -1;						/// Index uniform (1 of 4 textures)

// Mouse interaction
int MouseX, MouseY;							///< The last position of the mouse
int MouseButton;							///< The last mouse button pressed or released

// Camera
Camera Cam;
BOOL ProjSwitch = true;						/// Switch between projection modes

BOOL FogToggle = false;						/// Toggle fog effect on/off
BOOL ToonToggle = false;					/// Toggle toon effect on/off
GLint FogToggleLoc = -1;					/// uniform for FogToggle
GLint ToonToggleLoc = -1;					/// uniform for ToonToggle

// Model matrix for the car
ModelMatrix car;
ModelMatrix sphere;
ModelMatrix banner;
ModelMatrix star[STARS_COUNT];

// Animation for car
AnimateModel animation;

// --- main() -------------------------------------------------------------------------------------
/// The entry point of the application
int main(int argc, char **argv) {

	// Initialize glut and create a simple window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(300, 50);
	glutCreateWindow("OpenGL INF251 Assignment CA3");

	// Initialize OpenGL callbacks
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keySpecial);
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
	glClearColor(0.1f, 0.15f, 0.15f, 0.0f);			// background color
	glEnable(GL_DEPTH_TEST);						// enable depth ordering
	glEnable(GL_BLEND);								// enable blend transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW);							// Vertex order for the front face
	glCullFace(GL_BACK);							// back-faces should be removed
	//glEnable(GL_CULL_FACE);							// enable back-face culling
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);			// hide the cursor

	// Model matrix initialization
	initModelMatrix(car);
	initModelMatrix(sphere);
	initModelMatrix(banner);

	sphere.translation = vLightSource[0];				// Lightsource 1 (index 0)  == sphere position :)
	sphere.scaling = 1.0f;

	banner.scaling = 3.0f;
	banner.translation = Vector3f(0.0f, -2.0f, -2.0f);	// Banner in the background slightly below
	banner.rotationX.rotate(90.0f, Vector3f(1, 0, 0));

	// Asteroid belt initialization
	initAsteroids();


	// Camera initialization
	initCamera(Cam);

	// Animation model init
	initAnimateModel(animation);


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
/// Initialize the asteroid/starfield positions and movement
void initAsteroids(){
	// set up the stars: initial positions & scaling
	{
		srand(99);
		float randX;
		float randY;
		float randZ;
		int randSign;

		for (int i = 0; i < STARS_COUNT; i++){


			randSign = (rand() % 2 == 0 ? -1 : 1);		// will need a 1 or -1 cout << randSign << "  Random value " << endl;
			randX = ((rand() % 12) + .7f) * randSign;	// +.6f is an offset so the stars dont "hit" the viewer :)

			randSign = (rand() % 2 == 0 ? -1 : 1);
			randY = ((rand() % 12) + .7f) * randSign;

			randZ = (rand() % 120) - 220.0f;			// we start beyond zFar limit...
			initModelMatrix(star[i]);


			star[i].translation = Vector3f(randX, randY, randZ);
			star[i].defaultPosition = Vector3f(randX, randY, -110.0f);


			randSign = (rand() % 2 == 0 ? -1 : 1);
			randX = ((float)rand() / RAND_MAX) * randSign / 20.0f;
			randSign = (rand() % 2 == 0 ? -1 : 1);
			randY = ((float)rand() / RAND_MAX) * randSign / 20.0f;
			randZ = ((float)rand() / RAND_MAX) / 3.0f;
			star[i].transDelta = Vector3f(0.0f, 0.0f, randZ);


			star[i].rotateDelta = Vector3f(/*randX*/0.6f, randX * 16, randY * 40);
			randSign = (rand() % 6) + 1;
			randX = ((float)rand() / RAND_MAX);
			star[i].scaling = randX * randSign;					// an idea is to vary speed and direction (asteroids!)


		}
	}

}


// ************************************************************************************************
// *** OpenGL callbacks implementation ************************************************************
/// Called whenever the scene has to be drawn
void display() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);											// Enable depth test

	Matrix4f P_Matrix = computeCameraZoom(Cam) *						// add "camera zoom"-effect (view scaling actually)
		getActiveProjectionMatrix(width, height);						// find active proj matrix based on ProjSwitch

	// Enable the shader program
	assert(ShaderProgram != 0);
	glUseProgram(ShaderProgram);

	glUniform1i(FogToggleLoc, FogToggle);								// inform shader if fog effect is activated
	glUniform1i(ToonToggleLoc, ToonToggle);								// inform shader if fog effect is activated


	// Model car transformation matrix calculation
	Matrix4f M_Matrix =
		Matrix4f::createTranslation(car.translation) * car.rotationX * car.rotationY *
		Matrix4f::createScaling(car.scaling, car.scaling, car.scaling);

	// Set the camera position, ModelView and  MVP matrices
	Cam.ar = (1.0f * width) / height;
	Matrix4f MV_Matrix = computeCameraView(Cam) * M_Matrix;
	Matrix4f MVP_Matrix = P_Matrix * MV_Matrix;

	// update the shader uniforms with transformation matrices
	glUniformMatrix4fv(M_Loc, 1, GL_FALSE, M_Matrix.get());
	glUniformMatrix4fv(MV_Loc, 1, GL_FALSE, MV_Matrix.get());
	glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, MVP_Matrix.get());
	glUniformMatrix4fv(V_Loc, 1, GL_FALSE, computeCameraView(Cam).get());

	// Set the light parameters for each lightsource
	vLightSource[1] = Cam.position.get();								// set the camera position as 2nd light source position
	glUniform3fv(LightSourceLoc, TOTAL_LIGHTS, vLightSource[0].get());	// light source position/direction vectors	
	glUniform1fv(LightTypeLoc, TOTAL_LIGHTS, &fLightType[0]);			// w-parameters 
	glUniform3fv(LightIadsLoc, TOTAL_LIGHTS, LightIads[0].get());		// Light intensity settings, ADS-vector per lightsource

	// Enable the vertex attributes and set their format
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	// make sure its neither emissive nor transparent
	float carEm[3] = { 0.0f, 0.0f, 0.0f };
	glUniform3fv(LightEmLoc, 1, &carEm[0]);

	// Bind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, carVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, carIBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(5 * sizeof(float)));


	// draw the model with appriopriate textures and materials
	drawTexture(Model);



	// =====================================================================================================================

	// =====================================================================================================================
	// draw the banner

	// Model transformation matrix calculation 
	Matrix4f M_Matrix2 =
		Matrix4f::createTranslation(banner.translation) * banner.rotationX *
		Matrix4f::createScaling(banner.scaling, banner.scaling, banner.scaling);

	Matrix4f MV_Matrix2 = computeCameraView(Cam) * M_Matrix2;
	Matrix4f MVP_Matrix2 = P_Matrix * MV_Matrix2;

	// update the shader uniforms with transformation matrices
	glUniformMatrix4fv(M_Loc, 1, GL_FALSE, M_Matrix2.get());
	glUniformMatrix4fv(MV_Loc, 1, GL_FALSE, MV_Matrix2.get());
	glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, MVP_Matrix2.get());

	// Set the material parameters 
	float BKa[3] = { 0.05f, 0.05f, 0.05f };
	float BKd[3] = { 0.8f, 0.8f, 0.8f };					// used in later objects as well!
	float BKs[3] = { 0.9f, 0.9f, 0.9f };

	float bannerEm[3] = { 0.0f, 0.0f, 0.0f };
	glUniform3fv(LightEmLoc, 1, &bannerEm[0]);

	glUniform3fv(MaterialKaLoc, 1, &BKa[0]);
	glUniform3fv(MaterialKdLoc, 1, &BKd[0]);
	glUniform3fv(MaterialKsLoc, 1, &BKs[0]);
	glUniform1f(MaterialShineLoc, 40.0f);

	// Bind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, bannerVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bannerIBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(5 * sizeof(float)));


	// Draw the elements on the GPU
	glDrawElements(
		GL_TRIANGLES,
		Banner.getNumberOfIndices(),
		GL_UNSIGNED_INT,
		0);


	// =====================================================================================================================

	// =====================================================================================================================

	// draw the stars 

		//// Set the material parameters 
		//float Sa[3] = { 0.4f, 0.4f, 0.4f };
		//float Sd[3] = { 0.9f, 0.9f, 0.1f };
		//float Ss[3] = { 0.8f, 0.8f, 0.5f };

		////glUniform3fv(LightEmLoc, 1, &Ss[0]);				// emissive color for the stars (uses specular setting)

		//glUniform3fv(MaterialKaLoc, 1, &Sa[0]);
		//glUniform3fv(MaterialKdLoc, 1, &Sd[0]);
		//glUniform3fv(MaterialKsLoc, 1, &Ss[0]);
		//glUniform1f(MaterialShineLoc, 16.0f);

	
	Matrix4f starM_Matrix[STARS_COUNT];
	Matrix4f starMV_Matrix[STARS_COUNT];
	Matrix4f starMVP_Matrix[STARS_COUNT];


	for (int i = 0; i < STARS_COUNT; i++){

		// Model transformation matrix calculation 
		starM_Matrix[i] =
			Matrix4f::createTranslation(star[i].translation) * star[i].rotationX * star[i].rotationY * star[i].rotationZ;
		Matrix4f::createScaling(star[i].scaling, star[i].scaling, star[i].scaling);

		starMV_Matrix[i] = computeCameraView(Cam) * starM_Matrix[i];
		starMVP_Matrix[i] = P_Matrix * starMV_Matrix[i];

		// update the shader uniforms with transformation matrices
		glUniformMatrix4fv(M_Loc, 1, GL_FALSE, starM_Matrix[i].get());
		glUniformMatrix4fv(MV_Loc, 1, GL_FALSE, starMV_Matrix[i].get());
		glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, starMVP_Matrix[i].get());

		// Bind the buffers
		glBindBuffer(GL_ARRAY_BUFFER, starVBO[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starIBO[i]);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			sizeof(ModelOBJ::Vertex),
			reinterpret_cast<const GLvoid*>(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
			sizeof(ModelOBJ::Vertex),
			reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
			sizeof(ModelOBJ::Vertex),
			reinterpret_cast<const GLvoid*>(5 * sizeof(float)));


		// draw the model with appriopriate textures and materials
		drawTexture(Boulder);

		// Draw the elements on the GPU
		//glDrawElements(
		//	GL_TRIANGLES,
		//	Sphere.getNumberOfIndices(),
		//	GL_UNSIGNED_INT,
		//	0);

	}

	// =====================================================================================================================


	// =====================================================================================================================
	// draw the sphere 


	// **** to control swicthing on/off bulb effect (without destroying the whole "feel")
	// we need to scale the unlit bulb (sphere) to the size of the halo-centre sphere, which we cant resize easily.
	sphere.scaling = (LightIads[0].get(0) > 0.0f ? 1.0f : 0.06f);	// either default size or attempt to make actual bulb size

	// Model transformation matrix calculation 
	Matrix4f M_Matrix1 =
		Matrix4f::createTranslation(vLightSource[0]) *								// hardcoded sphere with vLightSource[0]
		Matrix4f::createScaling(sphere.scaling, sphere.scaling, sphere.scaling);

	Matrix4f MV_Matrix1 = computeCameraView(Cam) * M_Matrix1;
	Matrix4f MVP_Matrix1 = P_Matrix * MV_Matrix1;

	// update the shader uniforms with transformation matrices
	glUniformMatrix4fv(M_Loc, 1, GL_FALSE, M_Matrix1.get());
	glUniformMatrix4fv(MV_Loc, 1, GL_FALSE, MV_Matrix1.get());
	glUniformMatrix4fv(MVP_Loc, 1, GL_FALSE, MVP_Matrix1.get());

	// Set the material parameters 
	float Ka[3] = { 0.1f, 0.05f, 0.05f };
	float Kd[3] = { 0.8f, 0.1f, 0.1f };					// used in later objects as well!
	float Ks[3] = { 1.0f, 0.9f, 0.9f };					// emissive value .x is used to differentiate between stars & bulb!!

	glUniform3fv(LightEmLoc, 1, LightIads[0].get());	// emissive color for the sphere == light colour

	glUniform3fv(MaterialKaLoc, 1, &Ka[0]);
	glUniform3fv(MaterialKdLoc, 1, &Kd[0]);
	glUniform3fv(MaterialKsLoc, 1, &Ks[0]);
	glUniform1f(MaterialShineLoc, 64.0f);

	// Bind the buffers
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		sizeof(ModelOBJ::Vertex),
		reinterpret_cast<const GLvoid*>(5 * sizeof(float)));

	// Draw the elements on the GPU
	glDrawElements(
		GL_TRIANGLES,
		Sphere.getNumberOfIndices(),
		GL_UNSIGNED_INT,
		0);


	// ========================================================================================================

	// Disable the "position" vertex attribute (not necessary but recommended)
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// Disable the shader program (not necessary but recommended)
	glUseProgram(0);

	// Lock the mouse at the center of the screen
	glutWarpPointer(MouseX, MouseY);

	// Swap the frame buffers (off-screen rendering)
	glutSwapBuffers();
}

/// Called at regular intervals (can be used for animations)
void idle() {

	if (animation.activated){
		// force model to rotate on given axis
		Matrix4f rx, ry, rz;							// compute the rotation matrices
		rx.rotate(-0.15f * animation.stepsizeX, Vector3f(1, 0, 0));
		ry.rotate(0.15f * animation.stepsizeY, Vector3f(0, 1, 0));
		ry.rotate(0.15f * animation.stepsizeZ, Vector3f(0, 0, 1));
		car.rotationX *= rx;							// accumulate the rotation
		car.rotationY *= ry;
		car.rotationZ *= rz;

		// just move the lightsource automatically as well
		angle = (angle > 359.96f ? 0.0f : angle + animation.sphereIncrement);
		vLightSource[idLight] = Vector3f(cosf(angle), sinf(angle), vLightSource[idLight].get(0) * 2);

		// add the stars movement....
		for (int i = 0; i < STARS_COUNT; i++){
			if (star[i].translation.get(2) > 20.0f){
				star[i].translation = Vector3f(star[i].defaultPosition.get(0), star[i].defaultPosition.get(1), star[i].defaultPosition.get(2));
			}
			else{
				star[i].translation += star[i].transDelta;

				Matrix4f sx, sy, sz;
				sx.rotate(star[i].rotateDelta.get(0), Vector3f(1, 0, 0));
				sy.rotate(star[i].rotateDelta.get(1), Vector3f(0, 1, 0));
				sz.rotate(star[i].rotateDelta.get(2), Vector3f(0, 0, 1));

				star[i].rotationX *= sx;
				star[i].rotationY *= sy;
				star[i].rotationZ *= sz;
			}
		}


		glutPostRedisplay();;
	}

}

void drawTexture(const ModelOBJ& object){
	// Draw all meshes	

	for (int m = 0; m < object.getNumberOfMeshes(); m++)
	{
		const ModelOBJ::Mesh *pMesh = &object.getMesh(m);
		const std::string color_map = pMesh->pMaterial->colorMapFilename;
		unordered_map<string, GLuint>::const_iterator got = texture.name2objectMap.find(color_map);
		int activeTextureId = (got == texture.name2objectMap.end() ? -1 : got->second);	// textureId = -1 if no texture on current mesh

		int startIndex = pMesh->startIndex;								// select parts of mesh to render
		int numIndices = pMesh->triangleCount * 3;

		// Set the material parameters (read materials/textures attributes from the current mesh)
		const float ambient[3] = { pMesh->pMaterial->ambient[0], pMesh->pMaterial->ambient[1], pMesh->pMaterial->ambient[2] };
		const float diffuse[3] = { pMesh->pMaterial->diffuse[0], pMesh->pMaterial->diffuse[1], pMesh->pMaterial->diffuse[2] };
		//auto& diffuse = pMesh->pMaterial->diffuse;   **OR** const float(&diffuse)[4] = pMesh->pMaterial->diffuse;
		const float specular[3] = { pMesh->pMaterial->specular[0], pMesh->pMaterial->specular[1], pMesh->pMaterial->specular[2] };

		glUniform3fv(MaterialKaLoc, 1, &ambient[0]);
		glUniform3fv(MaterialKdLoc, 1, &diffuse[0]);
		glUniform3fv(MaterialKsLoc, 1, &specular[0]);
		glUniform1f(MaterialShineLoc, 120/*pMesh->pMaterial->shininess*/);			// better value than from sketchup-model

		glUniform1i(IndexLoc, (activeTextureId < 0 ? 0 : 1));						// texture index sent to shaders

		if (activeTextureId != -1){

			glActiveTexture(GL_TEXTURE0);								// set the active texture unit
			glBindTexture(GL_TEXTURE_2D, activeTextureId);
			glUniform1i(SamplerLoc, 0);

		}

		// Draw the elements on the GPU
		glDrawElements(
			GL_TRIANGLES,
			numIndices,
			GL_UNSIGNED_INT,
			reinterpret_cast<const GLvoid*>(startIndex* sizeof(GL_UNSIGNED_INT)));	// SIZE IN BYTES!


	}

}

/// Called whenever a keyboard button is pressed (only ASCII characters)
void keyboard(unsigned char key, int x, int y) {

	Vector3f right;

	switch (tolower(key)) {
		// --- camera movements ---
	case 'w':
		Cam.position += Cam.target * 0.1f;
		break;
	case 'a':
		right = Cam.target.cross(Cam.up);
		Cam.position -= right * 0.05f;
		break;
	case 's':
		Cam.position -= Cam.target * 0.1f;
		break;
	case 'd':
		right = Cam.target.cross(Cam.up);
		Cam.position += right * 0.05f;
		break;
	case 'c':
		Cam.position -= Cam.up * 0.05f;
		break;
	case ' ':
		Cam.position += Cam.up * 0.05f;
		break;

	case 'r': // Reset camera status and stop any animations
		initAnimateModel(animation);
		initCamera(Cam);
		break;

	case '+':	// move lightsource in positive direction
		if (animation.activated){
			animation.sphereIncrement += 0.002f;
		}
		angle = (angle > 359.96f ? 0.0f : angle + animation.sphereIncrement);
		vLightSource[idLight] = Vector3f(cosf(angle), sinf(angle), vLightSource[idLight].get(0) * 2);
		break;
	case '-':	// move lightsource in (neg) direction
		if (animation.activated){
			animation.sphereIncrement -= 0.002f;
		}
		angle = (angle < animation.sphereIncrement * 2 ? 360.0f : angle - animation.sphereIncrement);
		vLightSource[idLight] = Vector3f(cosf(angle), sinf(angle), vLightSource[idLight].get(0) * 2);
		break;


	case '1':	// toggle lightsource 1 (index 0) 
		LightIads[0] = (LightIads[0].x() == 0. ? LightIntensity[0] : Vector3f());
		cout << "Light 1 toggle." << endl;
		//idLight = 0;
		break;
	case '2':	// toggle lightsource 2 (index 1)
		LightIads[1] = (LightIads[1].x() == 0. ? LightIntensity[1] : Vector3f());
		cout << "Light 2 toggle." << endl;
		// idLight = 1; no point - controlled by camera position
		break;
	case '3':	// toggle lightsource 3 (index 2)
		LightIads[2] = (LightIads[2].x() == 0. ? LightIntensity[2] : Vector3f());
		cout << "Light 3 toggle." << endl;
		//idLight = 2;
		break;

	case 'f':	// toggle fog effects
		FogToggle = !FogToggle;
		cout << "Fog effect: " << FogToggle << endl;
		break;

	case 't':	// toggle old starfield effects
		ToonToggle = !ToonToggle;
		cout << "Toon effect: " << ToonToggle << endl;
		break;

	case 'm':	// toggle animation for model  
		animation.activated = !animation.activated;
		cout << (animation.activated ? "Autoanimate ON" : "Autoanimate OFF") << endl;
		break;

		// --- utilities ---
	case 'p': // projection type switch
		ProjSwitch = !ProjSwitch;
		cout << (ProjSwitch ? "Proj: Perspective" : "Proj: Orthogonal") << endl;
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;

	case 'o': // change to polygon rendering
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'g': // show the current OpenGL version
		cout << "OpenGL version " << glGetString(GL_VERSION) << endl;
		break;
	case 'ø': // reload shaders
		cout << "Re-loading shaders..." << endl;
		if (initShaders()) {
			cout << "> done." << endl;
			glutPostRedisplay();
		}
		break;
	case 'q':  // terminate the application
		exit(0);
		break;
	}
	// redraw
	glutPostRedisplay();
}

/// Called when special keys are pressed
void keySpecial(int key, int x, int y)
{
	if (animation.activated){

		switch (key) {
		case GLUT_KEY_RIGHT:
			if (glutGetModifiers() == GLUT_ACTIVE_SHIFT){
				animation.stepsizeZ++;
			}
			else{
				animation.stepsizeY++;
			}

			break;

		case GLUT_KEY_LEFT:
			if (glutGetModifiers() == GLUT_ACTIVE_SHIFT){
				animation.stepsizeZ--;
			}
			else{
				animation.stepsizeY--;
			}

			break;

		case GLUT_KEY_UP:
			animation.stepsizeX++;
			break;

		case GLUT_KEY_DOWN:
			animation.stepsizeX--;
			break;
		}
	}
}

/// Called whenever a mouse event occur (press or release)
void mouse(int button, int state, int x, int y) {

	// Store the current mouse status
	MouseButton = button;

	// Instead of updating the mouse position, lock it at the center of the screen
	MouseX = glutGet(GLUT_WINDOW_WIDTH) / 2;
	MouseY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	glutWarpPointer(MouseX, MouseY);

}

/// Called whenever the mouse is moving while a button is pressed
void motion(int x, int y) {

	bool redisplay = false;

	if (MouseButton == GLUT_RIGHT_BUTTON && !animation.activated) {

		Matrix4f rx, ry;	// compute the rotation matrices
		rx.rotate(-0.15f * (MouseY - y), Vector3f(1, 0, 0));
		ry.rotate(0.15f * (x - MouseX), Vector3f(0, 1, 0));
		car.rotationX *= rx;	// accumulate the rotation
		car.rotationY *= ry;
		MouseX = x; // Store the current mouse position
		MouseY = y;
		redisplay = true;
	}
	if (MouseButton == GLUT_MIDDLE_BUTTON) {
		Cam.zoom = std::max(0.001f, Cam.zoom + 0.003f * (y - MouseY));
		redisplay = true;
	}
	if (MouseButton == GLUT_LEFT_BUTTON) {
		Matrix4f ry, rr;

		// "horizontal" rotation
		ry.rotate(0.1f * (MouseX - x), Vector3f(0, 1, 0));
		Cam.target = ry * Cam.target;
		Cam.up = ry * Cam.up;

		// "vertical" rotation
		rr.rotate(0.1f * (MouseY - y), Cam.target.cross(Cam.up));
		Cam.up = rr * Cam.up;
		Cam.target = rr * Cam.target;

		redisplay = true;
	}

	if (redisplay){
		glutPostRedisplay(); // Specify that the scene needs to be updated
	}
}
/// Called when the mouse wheel moves

// ************************************************************************************************
// *** Other methods implementation ***************************************************************

/// Initialize buffer objects
bool initMesh() {
	// Load the OBJ model
	if (!Model.import("model\\fiat126.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}

	Model.normalize();

	// VBO
	glGenBuffers(1, &carVBO);
	glBindBuffer(GL_ARRAY_BUFFER, carVBO);
	glBufferData(GL_ARRAY_BUFFER,
		Model.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		Model.getVertexBuffer(),
		GL_STATIC_DRAW);

	// IBO
	glGenBuffers(1, &carIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, carIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		Model.getNumberOfIndices() * sizeof(unsigned int),
		Model.getIndexBuffer(),
		GL_STATIC_DRAW);

	// load the textures
	initModelTextures(Model);

	// ========================================================================================================================

	// Load the sphere OBJ model
	if (!Sphere.import("model\\sphere.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}

	Sphere.normalize();

	// VBO
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER,
		Sphere.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		Sphere.getVertexBuffer(),
		GL_STATIC_DRAW);

	// IBO
	glGenBuffers(1, &sphereIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		Sphere.getNumberOfIndices() * sizeof(unsigned int),
		Sphere.getIndexBuffer(),
		GL_STATIC_DRAW);


	// ****************************************************************************
	// Asteroid objects

	// Load the asteroid rock OBJ model
	if (!Boulder.import("model\\boulder1.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}

	Boulder.normalize();


	for (int i = 0; i < STARS_COUNT; i++){

		// VBO
		glGenBuffers(1, &starVBO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, starVBO[i]);
		glBufferData(GL_ARRAY_BUFFER,
			Boulder.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
			Boulder.getVertexBuffer(),
			GL_STATIC_DRAW);

		// IBO
		glGenBuffers(1, &starIBO[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starIBO[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			Boulder.getNumberOfIndices() * sizeof(unsigned int),
			Boulder.getIndexBuffer(),
			GL_STATIC_DRAW);

	}

	// load the textures
	initModelTextures(Boulder);


	// ******************************************************************************

	// Load the OpenGL OBJ model
	if (!Banner.import("model\\OpenGL.obj")) {
		cerr << "Error: cannot load model." << endl;
		return false;
	}

	Banner.normalize();

	// VBO
	glGenBuffers(1, &bannerVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bannerVBO);
	glBufferData(GL_ARRAY_BUFFER,
		Banner.getNumberOfVertices() * sizeof(ModelOBJ::Vertex),
		Banner.getVertexBuffer(),
		GL_STATIC_DRAW);

	// IBO
	glGenBuffers(1, &bannerIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bannerIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		Banner.getNumberOfIndices() * sizeof(unsigned int),
		Banner.getIndexBuffer(),
		GL_STATIC_DRAW);







	return true;
} /* initBuffers() */

/// Loads textures if textures exist for specified object 
void initModelTextures(const ModelOBJ& object) {

	//init texture variables
	texture.width = 0;							///< The width of the current texture
	texture.height = 0;							///< The height of the current texture
	texture.data = nullptr;						///< the array where the texture image will be stored

	// Check the materials for the texture
	for (int i = 0; i < object.getNumberOfMaterials(); ++i) {

		// if the current material has a texture
		string texName = object.getMaterial(i).colorMapFilename;
		if (texName != "") {

			// Load the texture
			if (texture.data != nullptr) free(texture.data);

			unsigned int fail = lodepng_decode_file(&texture.data, &texture.width, &texture.height,
				("model\\" + texName).c_str(), LCT_RGB, 8);  // Remember to check the last 2 parameters

			//cout << (texName).c_str() << "  Texture txtID:" << txtID << endl;
			if (fail != 0) {
				cerr << "Error: cannot load texture file " << texName << endl;
			}

			// Create the texture object use texture.idMap[texName] as key in .objectMap 
			if (texture.name2objectMap[texName] != 0){ glDeleteTextures(1, &texture.name2objectMap[texName]); }

			// Bind it as a 2D texture (note that other types of textures are supported as well)
			glGenTextures(1, &texture.name2objectMap[texName]);				// texture name is key, textureId is value
			glBindTexture(GL_TEXTURE_2D, texture.name2objectMap[texName]);

			// Set the texture data
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB,			// remember to check this
				texture.width,
				texture.height,
				0,
				GL_RGB,			// remember to check this
				GL_UNSIGNED_BYTE,
				texture.data
				);

			// Configure texture parameter
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		}
	}

	// clean up - we dont need this anymore
	free(texture.data);
}

/// Initialize shaders. Return false if initialization fail
bool initShaders() {
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
	M_Loc = glGetUniformLocation(ShaderProgram, "MMatrix");
	V_Loc = glGetUniformLocation(ShaderProgram, "ViewMatrix");
	MV_Loc = glGetUniformLocation(ShaderProgram, "MVMatrix");
	MVP_Loc = glGetUniformLocation(ShaderProgram, "MVPMatrix");

	SamplerLoc = glGetUniformLocation(ShaderProgram, "sampler");
	IndexLoc = glGetUniformLocation(ShaderProgram, "colormapId");
	FogToggleLoc = glGetUniformLocation(ShaderProgram, "fogToggle");
	ToonToggleLoc = glGetUniformLocation(ShaderProgram, "toonToggle");

	// lighting uniforms 
	LightEmLoc = glGetUniformLocation(ShaderProgram, "vEmissive");

	LightSourceLoc = glGetUniformLocation(ShaderProgram, "vLightSource");
	LightTypeLoc = glGetUniformLocation(ShaderProgram, "fLightType");

	//DLightAColorLoc = glGetUniformLocation(ShaderProgram, "vLightAmbient");
	//DLightDColorLoc = glGetUniformLocation(ShaderProgram, "vLightDiffuse");
	//DLightSColorLoc = glGetUniformLocation(ShaderProgram, "vLightSpecular");
	LightIadsLoc = glGetUniformLocation(ShaderProgram, "LightIads");

	MaterialKaLoc = glGetUniformLocation(ShaderProgram, "vMaterialAmbient");
	MaterialKdLoc = glGetUniformLocation(ShaderProgram, "vMaterialDiffuse");
	MaterialKsLoc = glGetUniformLocation(ShaderProgram, "vMaterialSpecular");
	MaterialShineLoc = glGetUniformLocation(ShaderProgram, "MaterialShine");


	// Shaders can be deleted now
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
} /* initShaders() */

/// Initialize a  Model Matrix for specified object 
void initModelMatrix(ModelMatrix& model) {
	model.rotationX.identity();
	model.rotationY.identity();
	model.rotationZ.identity();
	model.translation.set(0.0f, 0.0f, 0.0f);
	model.defaultPosition.set(0.0f, 0.0f, 0.0f);
	model.rotateDelta.set(0.0f, 0.0f, 0.0f);
	model.transDelta.set(0.0f, 0.0f, 0.0f);
	model.scaling = 1.0f;
}

/// Initialize a  AnimateModel  
void initAnimateModel(AnimateModel& model) {
	model.activated = false;
	model.stepsizeX = 0.0f;
	model.stepsizeY = 0.0f;
	model.stepsizeZ = 0.0f;

	model.sphereIncrement = 0.01f;
}

/// Camera initial values
void initCamera(Camera& cam) {
	cam.position.set(0.f, 0.f, 5.f);
	cam.target.set(0.f, 0.f, -1.f);
	cam.up.set(0.f, 1.f, 0.f);
	cam.fov = 50.f;
	cam.ar = 1.f; // will be correctly initialized in the "display()" method
	cam.zNear = 0.1f;
	cam.zFar = 100.f;
	cam.zoom = 1.f;

}

/// Return the transformation matrix corresponding to the specified camera
Matrix4f computeCameraView(const Camera& cam) {
	// camera rotation
	Vector3f t = cam.target.getNormalized();
	Vector3f u = cam.up.getNormalized();
	Vector3f r = t.cross(u);
	Matrix4f camR(r.x(), r.y(), r.z(), 0.f,
		u.x(), u.y(), u.z(), 0.f,
		-t.x(), -t.y(), -t.z(), 0.f,
		0.f, 0.f, 0.f, 1.f);

	// camera translation
	Matrix4f camT = Matrix4f::createTranslation(-cam.position);
	return camR  * camT;

}

/// Return the zoom scaling matrix corresponding to the specified camera
Matrix4f computeCameraZoom(const Camera& cam) {
	// scaling due to zooming
	Matrix4f camZoom = Matrix4f::createScaling(cam.zoom, cam.zoom, 1.f);
	return camZoom;
}

/// Return the projection matrix corresponding to the specified camera
Matrix4f computePerspectiveProjection(const Camera& cam) {
	// perspective projection
	Matrix4f prj = Matrix4f::createPerspectivePrj(cam.fov, cam.ar, cam.zNear, cam.zFar);
	return prj;
}

/// Return the orthogonal matrix corresponding to the specified camera
Matrix4f computeOrthogonalProjection(const Camera& cam, int width, int height) {
	// simple 1:1 orthogonal projection
	Matrix4f prj = Matrix4f::createOrthoPrj(-1.0f, 1.0f, -1.0f, 1.0f, cam.zNear, cam.zFar);
	return prj;
}

/// Get active projection Matrix based on BOOL ProjSwitch
Matrix4f getActiveProjectionMatrix(int width, int height){
	Matrix4f prj;
	if (ProjSwitch) {
		prj = computePerspectiveProjection(Cam);
	}
	else{
		prj = computeOrthogonalProjection(Cam, width, height);
	}
	return prj;
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

/* --- eof main.cpp --- */