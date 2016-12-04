#version 330	// GLSL version

struct DirectionalLight {
	vec3 direction;

	vec3 aColor;
	float aIntensity;

	vec3 dColor;
	float dIntensity;

	vec3 sColor;
	float sIntensity;

	int bOn; // on/off switch
};
vec3 generateLightColor(DirectionalLight light,vec3);

struct Spotlight {
	vec3 vDirection;
	vec3 vPosition;
	vec3 vColor;

	int bOn;

	float fConeAngle, fConeCosine;
	float fLinearAtt;
	float range;
	float vIntensity;

	int vRadialAttenuation;
}; // chao chao dobhre ahoooy
//^cau cau dobre ahoj

vec4 generateSpotlightColor(Spotlight spotlight, vec3);

struct Material {
	vec3 aColor;
	vec3 dColor;
	vec3 sColor;
	float shininess;
};

// lights
uniform DirectionalLight dLight;
uniform Spotlight spotlight;

// Sampler to access the texture
uniform sampler2D sampler;

// material
uniform Material material;

// Per fragment texture coordinates
in vec2 texCoord;	
in vec3 viewNormal;
in vec3 viewPosition;
in float belowSeaLevel;

out vec4 FragColor;

// Normal texture for bump mapping
uniform sampler2D normalTexture;
uniform int bumpMapping;
vec3 normalBump;

uniform mat4 MVMatrix;          
uniform mat4 MMatrix;

// Animated bumpTexture
uniform vec2 displacement;

// color by height on/off
uniform int colorByHeight;

uniform mat4 ViewMatrix;        // Just the view matrix (position of camera in the world, looking at some point, usually 0,0,0)

void main() { 

	vec4 texture = texture(sampler, texCoord);

	//vec3 fcolor = vec3(0.0,0.0,0.0);
	float transparency = 1.0;

	vec3 newViewNormal;

	vec3 viewDirection = normalize(-viewPosition); // from camera at (0,0,0)

	if (bumpMapping == 0){
		newViewNormal = normalize(viewNormal);

	//------------------------------------------------------------------------//
	//-----------------------------BUMP MAPPING-------------------------------//
	//------------------------------------------------------------------------//
	} else if ((bumpMapping == 1) && (belowSeaLevel == 0)){

		// Normalize the texture coordinate 	
	//	vec2 normalizedDisplacement = displacement / 2324;
		vec2 displacedTexCoord = displacement + vec2(texCoord.x, texCoord.y)*2324;
		
		displacedTexCoord.x = mod((displacedTexCoord.x), 2324) / 2324;
		displacedTexCoord.y = mod((displacedTexCoord.y), 2324) / 2324;	
		
		//normalBump = normalize(texture2D(normalTexture, texCoord).rgb * 2.0 - 1.0); 
		normalBump = normalize(texture2D(normalTexture, displacedTexCoord).rgb * 2.0 - 1.0); 
		newViewNormal = normalize(vec3(MVMatrix * vec4(normalBump, 0.0)));
	}
	//------------------------------------------------------------------------//
	//--------------------------BUMP MAPPING ENDS-----------------------------//
	//------------------------------------------------------------------------//
	
	//------------------------------------------------------------------------//
	//--------------------------DIRECTIONAL LIGHTS----------------------------//
	//------------------------------------------------------------------------//
	
	float angleX = dot(-dLight.direction, newViewNormal);
	vec3 dirR = -dLight.direction + 2.0*angleX + newViewNormal;

	float RV = dot(dirR, viewDirection);

	float specular;

	if (bumpMapping == 0){
		specular = clamp(pow(RV, material.shininess), 0.0, 1.0);
	} else if ((bumpMapping == 1) && (belowSeaLevel == 0)){
		specular = clamp(pow(RV, 0.2), 0.0, 1.0);
	}
	

	float diffuse = clamp(angleX, 0.0, 1.0);

	vec3 rgbI = vec3(1.,1.,1.);

	vec3 ambientRes;
	vec3 diffuseRes;
	vec3 specularRes;
	//if (bumpMapping == 0){
		ambientRes = dLight.aIntensity * dLight.aColor * material.aColor;
		diffuseRes = dLight.dIntensity * dLight.dColor * diffuse * material.dColor;
		specularRes = dLight.sIntensity * dLight.sColor * specular * material.sColor;
	 if (bumpMapping == 1 && (belowSeaLevel == 0)){
		vec3 oceanColor = vec3(0.11,0.56,1);

		ambientRes = dLight.aIntensity * oceanColor;
		diffuseRes = dLight.dIntensity * diffuse * oceanColor;
		specularRes = dLight.sIntensity * specular * oceanColor;
	}
	
	
	//------------------------------------------------------------------------//
	//----------------------DIRECTIONAL LIGHTS END----------------------------//
	//------------------------------------------------------------------------//
	
	vec3 fColor = ambientRes + diffuseRes + specularRes; // add dLight to fragment color

	//------------------------------------------------------------------------//
	//-----------------------------SPOTLIGHTS---------------------------------//
	//------------------------------------------------------------------------//

	vec3 SpotlightViewPosition = vec3(ViewMatrix * vec4(spotlight.vPosition,1.));
	vec3 vLight = normalize(spotlight.vDirection);
	vec3 vLightToFragment = normalize(spotlight.vPosition - viewPosition);

	float vDistance = distance(viewPosition, spotlight.vPosition); 

	float cosTheta = dot(vLight, vLightToFragment);


	if(cosTheta  < spotlight.fConeCosine) {
		float radialAttentuation = pow(1.05*cosTheta, spotlight.vRadialAttenuation);
		fColor += ((radialAttentuation * spotlight.vColor) / (vDistance * spotlight.vIntensity) );
	}

	//------------------------------------------------------------------------//
	//----------------------------SPOTLIGHTS END------------------------------//
	//------------------------------------------------------------------------//

	FragColor = texture * vec4(fColor,1.);

}

	