#version 330	// GLSL version

#define LIGHTS 3

in	vec2	texCoord;	
in	vec3	ViewNormal;					// Passed from vertex shader in View space
in	vec3	ViewPosition;				// current vertex position in View space

in vec3  vLightVector[LIGHTS];			// (-L) : Normalized vector from Vertex -> Light source 
in vec3	 attVal;						// using a vec3 as we only have 3 lights atm - more lights
										// will require a vec4 or a float array	
					

// for the results of the light calculations (earlier done in v-shader)
vec3	AmbientResult[LIGHTS];
vec3	DiffuseResult[LIGHTS];
vec3	SpecularResult[LIGHTS];

float	ambFactor	= 0.05f;			// set a global ambient coefficent for all light

uniform vec3 LightIads[LIGHTS];			// Light intensity (color) for each of these (RGB) . 

// Material - how much light is re-emitted by the surface for each channel (usually: Ka,Kd,Ks)
uniform vec3 vMaterialAmbient;
uniform vec3 vMaterialDiffuse;
uniform vec3 vMaterialSpecular;

uniform float MaterialShine;			// material shininess exponent (s_exp - see CA2 Tutorial)
uniform float MaterialAlpha;

uniform sampler2D sampler;				// Sampler to access the texture
uniform bool colormapId;				// 1 if using texture color, 0 otherwise
uniform vec3 vEmissive;					// emissive factor for the point light sphere


out vec4 FragColor;						// Per-frgament output color from fragment-shader

// ===============================================================================================================================
// Fog 

in float dist;									// distance to viewer
uniform bool fogToggle;							// turns fog on/off

const vec3 fogColor = vec3(0.1, 0.15, 0.15);	// fog color some work better than others - depends on background
const float FogDensity = 0.07;					// in practice, gives the viewing distance through the fog

uniform bool toonToggle;


// ===============================================================================================================================


void main()
{

	vec3 fcolor = vec3(0.0, 0.0, 0.0);								// sums contributions from all light sources
	float transparency = 1.0f;										// used for transparency and halo-effect
	
	// Fragment shader will interpolate the ViewNormal between the one vertex and the other.
	// Since the interpolation of the normal may result in the normal not actually being a normal, we
	// normalize it again. 
	vec3 NewViewNormal = normalize(ViewNormal);

	// Since we "move" the world, the camera position never actually changes. 
	vec3 viewDirection = normalize(-ViewPosition);					// supposed to be the target vector (view dir) 

	
	// ****************************************************************************************************************************
	//				                       Phong shading as described in the CA2 tutorial
	//	N	surface normal, normalized vector
	//	L   light direction i.e. from light -> surface
	// -L	the computed light vector (from surface -> light position or just reversed light direction)
	//  V	camera position (or view direction from surface -> camera position)
	//  R   ...to be computed
	// ****************************************************************************************************************************
	
		
	vec3 surfaceColor = vMaterialDiffuse;								// default surface(color) is diffuse material color
	
	if (colormapId){													// texture sampled as vec3 for material if texture active
		surfaceColor = vec3( texture(sampler, vec2(texCoord.s , -texCoord.t)).xyz);
	}

	
	// emissive colors will not be treated with external lighting effects this is a cheap way to make it look slightly more realistic

	if (vEmissive.x > 0.9){
		float fdf = 80.0;												// not necessary but makes lightsphere look better in fog
		if (fogToggle) fdf = 32.0;										

		float level = dot(NewViewNormal, viewDirection);				// attempt to make a halo-light effect..
		if (level < .998){
		  transparency = clamp((pow(level, fdf)), 0.0, 1.0);			// its on if emissive is on, exp regulates edge-softness
		}



	}else if (vEmissive.x < 0.9f){
	
	  for (int id = 0; id < LIGHTS; id++){
				
																		// reflection angle calculated manually (Phong):
																		// cosTheta = dot(-L,N)  *** reflectDir = L+2*cosTheta*N
		
		float angleX = dot(vLightVector[id], NewViewNormal);			// angle between -L and N, needed for R-calculation
		vec3 dirR = -vLightVector[id] + 2.0 * angleX * NewViewNormal;	// R = L+2*N*(L dot -N) == L-2*N*(L dot N)

		float RV = dot(dirR, viewDirection);							// RV is our reflection angle between R & V
		float specular = clamp(pow( RV, MaterialShine), 0.0, 1.0);		// specular contribution for this lightsource

		float diffuse = clamp(angleX, 0.0, 1.0);						// diffuse contribution: (-L dot N)  is angle between
																		// vLightVector (opposite light direction) and normal

		
		// ===========================================================================================================================
		// Calculate part of Ambient, Diffuse, Specular and Intensity information.
		
		AmbientResult[id]	= ambFactor * vMaterialAmbient * LightIads[id];		// the 'uniform' light of the enviroment
		DiffuseResult[id]	= diffuse * surfaceColor * LightIads[id];			// actual surface color, usually set by texture color
		SpecularResult[id]	= specular * vMaterialSpecular * LightIads[id];		// gives a 'shine' at specific viewing angles


		fcolor += attVal[id] * (DiffuseResult[id] + SpecularResult[id]);		// Calculate the total color
		fcolor += AmbientResult[id];				

	  }
	}

	fcolor += vEmissive;														// adjust for emissive colours

	// ============================================================================================================================
	// simple toon effect  
	
	if(toonToggle){		
		float angleX = dot(vLightVector[1], NewViewNormal);				// calculated using the head-light
		if( angleX < 0.3)
			fcolor = vec3(0.25, 0.25, 0.25);
		if( (angleX > 0.3) && (angleX < 0.8) )
			fcolor = vec3(0.5, 0.5, 0.5);
		if( angleX > 0.8 )
			fcolor = vec3(0.75, 0.75, 0.75);
	}

	// ===========================================================================================================================
	// fog formula additional in CA3 
			
	vec3 finalColor = fcolor;											// finalColor variable used for optional fog effect
	
	if (fogToggle){
		float fogFactor = 1.0 /exp( (dist * FogDensity) * (dist * FogDensity));
		fogFactor = clamp( fogFactor, 0.0, 1.0 );
   		finalColor = mix(fogColor, fcolor, fogFactor);					// TODO: increase emissive factor weight for brighter effect?
	}




	FragColor = vec4(finalColor, transparency);
	
	//FragColor = vec4(vEmissive + fcolor, transparency);				// emissive value dependent on current mesh/object
	
}
