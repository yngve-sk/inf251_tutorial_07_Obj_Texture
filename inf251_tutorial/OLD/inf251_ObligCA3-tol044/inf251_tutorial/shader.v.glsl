#version 330	// GLSL version

const unsigned int LIGHTS = 3u;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;
layout (location = 2) in vec3 normal;


// Constant Inputs from Client
uniform    mat4  MVPMatrix;         // MVP = Projection x View x Model matrix
uniform    mat4  MVMatrix;          // MV = View x Model matrix
uniform    mat4  MMatrix;           // M  = model matrix
uniform    mat4  ViewMatrix;        // Just the view matrix (position of camera in the world, looking at some point, usually 0,0,0)


uniform vec3 vLightSource[LIGHTS];			// Lightsource coordinate
uniform float fLightType[LIGHTS];			// positional or directional (w-coordinate)


// out variables passed to the Fragment shader
out  vec3   ViewPosition;				// Pass to fragment shader in View space
out  vec3   ViewNormal;					// Pass to fragment shader in View space
out  vec3	vLightVector[LIGHTS];		// for each lightsource a vector from Vertex -> Light source
out  vec2	texCoord;

// ==========================================================================================================================
// Attenuation calculations are now done in the vertex shader. Earlier version was done in
// fragment shader ONLY because we have reached the max attribute slots limitation.

const float	attConst = 1.0;
const float	attLinear = 0.01;
const float	attSquare = 0.01;

out vec3 attVal;		// the attenuation values will be used in f-shader final calculation	
out float dist;			// needed for fog calculations
// ===========================================================================================================================


void main()
{
 
    ViewPosition = vec3(MVMatrix * vec4(position, 1.0));					// Calculate position in view space
    ViewNormal = normalize(vec3(MVMatrix * vec4(normal, 0.0)));				// Calculate Normal using ModelView matrix
																			// normalization not needed here?
	
	for (int id = 0; id < 3; id++){
		vec4 LightPos = vec4(vLightSource[id], fLightType[id]);				// create vec4 with appriopriate w-coordinate
		vec3 ViewLightPos = vec3(ViewMatrix * LightPos);					// Transform lightPos into View space
																			// multiply only by View since LightPos is already in world space
		
		if (fLightType[id] == 1.0) {										// If we have a positional vector for the light source:
			vLightVector[id] = normalize(ViewLightPos - ViewPosition);		// (-L) : Normalized vector from Vertex -> Light source
		}else{																	
			vLightVector[id] = normalize(-ViewLightPos);					// If we have a directional vector for the light source: 	
		}
			
	}

	// ==================================================================================================================================
	// The attenuated pointlight will be hardcoded to headlight. Calculate the pointlight attenuation for the chosen lightsource
	// (see comment above!). We assume lightsource 2 (index 1) is the one we want attenuated.

	dist = length(-ViewPosition);										// distance to light source for attenuated pointlight
		
	attVal[0] = attConst / ((1.0 + attLinear * dist) * (1.0 +attSquare * dist * dist));
	attVal[1] = attConst / ((1.0 +attSquare * dist * dist));
	attVal[2] = 1.0;

	// ===================================================================================================================================


	// Transform the vertex into view space, with projection.
    gl_Position = MVPMatrix * vec4(position, 1.0);						// Model-View-Perspective matrix, with W=1

	texCoord = tex_coord;

}
