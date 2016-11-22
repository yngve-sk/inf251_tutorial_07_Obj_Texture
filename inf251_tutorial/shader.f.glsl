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
vec3 generateLightColor(DirectionalLight light);

struct Spotlight {
	vec3 vDirection;
	vec3 vPosition;
	vec3 vColor;

	int bOn;

	float fConeAngle, fConeCosine;
	float fLinearAtt;
}
vec3 generateSpotlightColor(Spotlight spotlight);

struct Material {
	vec3 aColor;
	vec3 dColor;
	vec3 sColor;
	float shininess;
};



// Sampler to access the texture
uniform sampler2D sampler;

// model-view transformation
uniform mat4 worldToProjectionMatrix;
uniform mat4 modelToWorldMatrix;

// camera pos
uniform vec3 camera_position;

// lights
uniform DirectionalLight directionalLight;
uniform Spotlight spotlight;

// material
Material material;

// Per fragment texture coordinates
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragVert;

// Normal texture for bump mapping
uniform sampler2D normal_texture;
uniform int bump_mapping;

// color by height on/off
uniform int colorByHeight;

void main() { 

	//calculate normal in world coordinates
    mat3 normalMatrix = transpose(inverse(mat3(worldToProjectionMatrix)));
    vec3 normal = normalize(normalMatrix * fragNormal);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 fragPosition = vec3(worldToProjectionMatrix * vec4(fragVert, 1));

	//calculate the vector from this pixels surface to the light source
    vec3 surfaceToLight = directionalLight.direction - fragPosition;

	//calculate the cosine of the angle of incidence
    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = 1;//clamp(brightness, 0, 1);

	// calculate, nomalize normals from bump mapping
	normal_bump = normalize(texture2D(normal_texture, fragTexCoord).rgb * 2.0 - 1.0);  

	vec3 color;
	//calculate light color
	if (bump_mapping == 0){
		color = generateLightColor(directionalLight, normal);
	} else if (bump_mapping == 1){
		color = generateLightColor(directionalLight, normalize(vec3(vec4(normal_bump,1.)*modelToWorldMatrix)));
	}

	//create the spotlight
	SpotLight sl;
	sl.vColor = vec3(122, 122, 122); // White Color
	//sl.vPosition = vec3(camera_position[0]-10, camera_position[1]-10, camera_position[2]-5);
	sl.vPosition = vec3(0,0,0);
	sl.bOn = 1;
	sl.fConeCosine = 0.86602540378;
	sl.fLinearAtt = 1.0;
	sl.vDirection = vec3(0,0,1);

	vec4 fWorldPosition = worldToProjectionMatrix * vec4(fragVert, 1.);
	vec4 s_lighting = GetSpotLightColor(splotlight, vec3(fWorldPosition));

	vec4 f_lighting;
	if (headlight == 1){
		//Add headlight to camera
		f_lighting = clamp(s_lighting + vec4(color, 1.0), 0, 255);
	} else {
		f_lighting = vec4(color, 1.0);
	}
	

    //calculate final color of the pixel, based on:
    // 1. The angle of incidence: brightness
    // 2. The color/intensities of the light: light.intensities
    // 3. The texture and texture coord: texture(sampler, fragTexCoord)
    vec4 surfaceColor = f_lighting * texture2D(sampler, fragTexCoord);

	if(colorByHeight == 1){
		float fy = fragVert.y + 0.;
		//brightness = fy/41.;
		FragColor = vec4(brightness * vec3(directionalLight.aIntensity, directionalLight.dIntensity, directionalLight.sIntensity) * surfaceColor.rgb, surfaceColor.a);
	}
	else {
		FragColor = vec4(brightness * vec3(d_light_a_intensity, d_light_s_intensity, d_light_d_intensity) * surfaceColor.rgb, surfaceColor.a);
	}
    
}

vec4 GetSpotLightColor(const Spotlight spotlight, vec3 vWorldPos) 
{ 
  if(spotLight.bOn == 0)return vec4(0.0, 0.0, 0.0, 0.0); 

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

vec3 generateLightColor(DirectionalLight dLight, Material mtl, vec3 normal) {
	//From Sergej
	vec4 fWorldPosition = worldToProjectionMatrix * vec4(fragVert, 1.);	   //WorldPosition
	//vec3 normal_nn = normalize((worldToProjectionMatrix * vec4(normal,0.0)).xyz);	//The normal must be transformed in World coordinates as well
	
	vec3 normal_nn = normalize(normal);	

	// Sophisticated lights
	// 100% lightning in 0-40m distance
	// 100% - 0% lightning in 40-100m distance
	float max_dist = 100;
	float distance = distance(vec3(fWorldPosition), camera_position);
	
	float distance_multiplier = 1 - (distance/max_dist);

	float full_light_treshold = 40;
	float gradial_max_dist = max_dist - full_light_treshold;

	if(distance > full_light_treshold) {
		distance_multiplier = 1;// - (distance/gradial_max_dist);
	}
	else {
		distance_multiplier = 1;
	}

	vec3 camLightDirection = camera_position - fragVert;

	vec3 d_light_dir_nn = normalize(dLight.direction);
	vec3 view_dir_nn = normalize(camera_position - fWorldPosition.xyz /*position*/ );		//Transform into world position (Sergej)
	float dot_d_light_normal = max(dot(-d_light_dir_nn, normal_nn), 0.0);   // notice the minus!   //The minus was missing and I used the transformed normal here (Sergej)
	
	vec3 d_reflected_dir_nn = reflect(d_light_dir_nn,normal_nn);					//There is a reflect function build in (Sergej)
	d_reflected_dir_nn = normalize(d_reflected_dir_nn); // should be already normalized, but we "need" to correct numerical errors

	vec3 color;
	vec3 ambient_color = clamp(
		mtl.aColor * dLight.aColor * dLight.aIntensity, 
		0.0, 1.0);
	vec3 diff_color = clamp(
		mtl.dColor * dot_d_light_normal * dLight.dIntensity,
		0.0,1.0);
	vec3 spec_color = clamp(
		mtl.sColor * 
		pow(dot(d_reflected_dir_nn, view_dir_nn), mtl.shininess),
		0.0,1.0);
	color = distance_multiplier*(ambient_color + diff_color + spec_color); // NOT JUST ONE CHANNEL

	return(color);
}

