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
}; // chao chao dobhre ahoooy
//^cau cau dobre ahoj

vec4 generateSpotlightColor(Spotlight spotlight, vec3);

struct Material {
	vec3 aColor;
	vec3 dColor;
	vec3 sColor;
	float shininess;
};

// model-view transformation
uniform mat4 worldToProjectionMatrix;
uniform mat4 modelToWorldMatrix;

// camera pos
uniform vec3 cameraPosition;

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

// Animated bumpTexture
uniform int displacement;

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
	} else if ((bumpMapping == 1) && (belowSeaLevel == 0)){
		normalBump = normalize(texture2D(normalTexture, texCoord).rgb * 2.0 - 1.0); 
		newViewNormal = normalize(vec3(MVMatrix * vec4(normalBump, 0.0)));
	}
	
	//------------------------------------------------------------------------//
	//--------------------------DIRECTIONAL LIGHTS----------------------------//
	//------------------------------------------------------------------------//
	
	float angleX = dot(-dLight.direction, newViewNormal);
	vec3 dirR = -dLight.direction + 2.0*angleX + newViewNormal;

	float RV = dot(dirR, viewDirection);
	float specular = clamp(pow(RV, material.shininess), 0.0, 1.0);

	float diffuse = clamp(angleX, 0.0, 1.0);

	vec3 rgbI = vec3(1.,1.,1.);

	vec3 ambientRes = dLight.aIntensity * material.aColor;
	vec3 diffuseRes = dLight.dIntensity * diffuse * material.dColor;
	vec3 specularRes = dLight.sIntensity * specular * material.sColor;
	
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

	float cosTheta = dot(vLight, vLightToFragment);
	if(cosTheta  < spotlight.fConeCosine) {
		float radialAttentuation = pow(1.05*cosTheta, 18);
		fColor += (radialAttentuation * spotlight.vColor);
	}

	//------------------------------------------------------------------------//
	//----------------------------SPOTLIGHTS END------------------------------//
	//------------------------------------------------------------------------//

	FragColor = texture + vec4(fColor,1.);
//	FragColor = vec4(spotlight.vColor, 1.);
//	FragColor = vec4(spotlight.vColor.xyz, 1.);
	//FragColor = vec4(.5,.1,.2, 1.);
	//FragColor = vec4(dLight.dColor, 1.);

	////calculate normal in world coordinates
    //mat3 normalMatrix = transpose(inverse(mat3(worldToProjectionMatrix)));
    //vec3 normal = normalize(normalMatrix * viewNormal);
	//
	////calculate the location of this fragment (pixel) in world coordinates
    //vec3 fragPosition = vec3(worldToProjectionMatrix * vec4(viewPosition], 1));
	//
	////calculate the vector from this pixels surface to the light source
    //vec3 surfaceToLight = dLight.direction - fragPosition;
	//
	////calculate the cosine of the angle of incidence
    //float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    //brightness = 1;//clamp(brightness, 0, 1);
	//
	//// calculate, nomalize normals from bump mapping
	//normalBump = normalize(texture2D(normalTexture, texCoord).rgb * 2.0 - 1.0);  
	//
	//vec3 color;
	////calculate light color
	//if (bumpMapping == 0){
	//	color = generateLightColor(dLight, normal);
	//} else if (bumpMapping == 1){
	//	color = generateLightColor(dLight, normalize(vec3(vec4(normalBump,1.)*modelToWorldMatrix)));
	//}
	//
	////create the spotlight
	////SpotLight sl;
	////sl.vColor = vec3(122, 122, 122); // White Color
	//////sl.vPosition = vec3(cameraPosition[0]-10, cameraPosition[1]-10, cameraPosition[2]-5);
	////sl.vPosition = vec3(0,0,0);
	////sl.bOn = 1;
	////sl.fConeCosine = 0.86602540378;
	////sl.fLinearAtt = 1.0;
	////sl.vDirection = vec3(0,0,1);
	//
	//vec4 fWorldPosition = worldToProjectionMatrix * vec4(viewPosition, 1.);
	//vec4 spotLighting = generateSpotlightColor(spotlight, vec3(fWorldPosition));
	//
	//vec4 fLighting;
	//if (spotlight.bOn == 1){
	//	//Add headlight to camera
	//	fLighting = clamp(spotLighting + vec4(color, 1.0), 0, 255);
	//} else {
	//	fLighting = vec4(color, 1.0);
	//}
	//
	//fLighting = vec4(1,1,1,1);
	//
	//
    ////calculate final color of the pixel, based on:
    //// 1. The angle of incidence: brightness
    //// 2. The color/intensities of the light: light.intensities
    //// 3. The texture and texture coord: texture(sampler, texCoord)
    //vec4 surfaceColor = fLighting * texture2D(sampler, texCoord);
	//
	//if(colorByHeight == 1){
	//	float fy = viewPosition.y + 0.;
	//	//brightness = fy/41.;
	//	FragColor = vec4(brightness * vec3(dLight.aIntensity, dLight.dIntensity, dLight.sIntensity) * surfaceColor.rgb, surfaceColor.a);
	//}
	//else {
	//	FragColor = vec4(brightness * vec3(dLight.aIntensity, dLight.dIntensity, dLight.sIntensity) * surfaceColor.rgb, surfaceColor.a);
	//}
    //
	//FragColor = vec4(surfaceColor.rgb,surfaceColor.a);
}

vec4 generateSpotlightColor(Spotlight spotlight, vec3 vWorldPos) 
{ 
	if(1==1) return vec4(0.0, 0.0, 0.0, 0.0);

  if(spotlight.bOn == 0)return vec4(0.0, 0.0, 0.0, 0.0); 

  float fDistance = distance(vWorldPos, spotlight.vPosition); 

  vec3 vDir = vWorldPos-spotlight.vPosition; 
  vDir = normalize(vDir); 
   
  float fCosine = dot(spotlight.vDirection, vDir); 
  float fDif = 1.0-spotlight.fConeCosine; 
  float fFactor = clamp((fCosine-spotlight.fConeCosine)/fDif, 0.0, 1.0); 

  if(fCosine > spotlight.fConeCosine) 
    return vec4(spotlight.vColor, 1.0)*fFactor/(fDistance*spotlight.fLinearAtt); 

  return vec4(0.0, 0.0, 0.0, 0.0); 
}

vec3 generateLightColor(DirectionalLight dLight, vec3 normal) {
	//From Sergej
	vec4 fWorldPosition = worldToProjectionMatrix * vec4(viewPosition, 1.);	   //WorldPosition
	//vec3 normal_nn = normalize((worldToProjectionMatrix * vec4(normal,0.0)).xyz);	//The normal must be transformed in World coordinates as well
	
	vec3 normal_nn = normalize(normal);	

	// Sophisticated lights
	// 100% lightning in 0-40m distance
	// 100% - 0% lightning in 40-100m distance
	float maxDist = 100;
	float distance = distance(vec3(fWorldPosition), cameraPosition);
	
	float distanceMultiplier = 1 - (distance/maxDist);

	float fullLightTreshold = 40;
	float gradialMaxDist = maxDist - fullLightTreshold;

	if(distance > fullLightTreshold) {
		distanceMultiplier = 1;// - (distance/gradialMaxDist);
	}
	else {
		distanceMultiplier = 1;
	}

	vec3 camLightDirection = cameraPosition - viewPosition;

	vec3 dLightDirNN = normalize(dLight.direction);
	vec3 viewDirNN = normalize(cameraPosition - fWorldPosition.xyz /*position*/ );		//Transform into world position (Sergej)
	float dot_d_light_normal = max(dot(-dLightDirNN, normal_nn), 0.0);   // notice the minus!   //The minus was missing and I used the transformed normal here (Sergej)
	
	vec3 d_reflected_dir_nn = reflect(dLightDirNN,normal_nn);					//There is a reflect function build in (Sergej)
	d_reflected_dir_nn = normalize(d_reflected_dir_nn); // should be already normalized, but we "need" to correct numerical errors

	vec3 color;
	vec3 ambient_color = clamp(
		material.aColor * dLight.aColor * dLight.aIntensity, 
		0.0, 1.0);
	vec3 diff_color = clamp(
		material.dColor * dot_d_light_normal * dLight.dIntensity,
		0.0,1.0);
	vec3 spec_color = clamp(
		material.sColor * 
		pow(dot(d_reflected_dir_nn, viewDirNN), material.shininess),
		0.0,1.0);
	color = distanceMultiplier*(ambient_color + diff_color + spec_color); // NOT JUST ONE CHANNEL

	return(color);
}

