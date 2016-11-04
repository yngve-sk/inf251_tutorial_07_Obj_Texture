#version 330	// GLSL version

// Sampler to access the texture
uniform sampler2D sampler;

// model-view transformation
uniform mat4 transformation;
uniform mat4 transformationLocal;

// camera pos
uniform vec3 camera_position;

// Per fragment texture coordinates
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragVert;

// Normal texture for bump mapping
uniform sampler2D normal_texture;

// Lights
uniform vec3 d_light_direction;
uniform vec3 d_light_a_color;
uniform float d_light_a_intensity;

uniform vec3 d_light_d_color;
uniform float d_light_d_intensity;

uniform vec3 d_light_s_color;
uniform float d_light_s_intensity;


// Object material
uniform vec3 material_a_color;
uniform vec3 material_d_color;
uniform vec3 material_s_color;
uniform float material_shininess;

//Headlight
uniform int headlight;

// Per-frgament output color
out vec4 FragColor;

// Spotlight structure
struct SpotLight 
{ 
  vec3 vColor; 
  vec3 vPosition; 
  vec3 vDirection; 

  int bOn; 
   
  float fConeAngle, fConeCosine; 
  float fLinearAtt; 
};

// Extract the normal from the normal map (Bump Mapping)
vec3 normal_bump;

vec3 generateLightColor(vec3 light_dir, vec3 normal);
vec4 GetSpotLightColor(const SpotLight spotLight, vec3 vWorldPos); 

void main() { 

	//calculate normal in world coordinates
    mat3 normalMatrix = transpose(inverse(mat3(transformation)));
    vec3 normal = normalize(normalMatrix * fragNormal);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 fragPosition = vec3(transformation * vec4(fragVert, 1));

	//calculate the vector from this pixels surface to the light source
    vec3 surfaceToLight = d_light_direction - fragPosition;

	//calculate the cosine of the angle of incidence
    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = clamp(brightness, 0, 1);

	// calculate, nomalize normals from bump mapping
	normal_bump = normalize(texture2D(normal_texture, fragTexCoord).rgb * 2.0 - 1.0);  

	//calculate light color
	vec3 color = generateLightColor(d_light_direction, normal_bump);

	//create the spotlight
	SpotLight sl;
	sl.vColor = vec3(200, 200, 200); // White Color
	sl.vPosition = vec3(camera_position[0]-10, camera_position[1]-10, camera_position[2]-5);
	sl.bOn = 1;
	sl.fConeCosine = 0.86602540378;
	sl.fLinearAtt = 1.0;
	sl.vDirection = vec3(0,0,1);

	vec4 fWorldPosition = transformation * transformationLocal * vec4(fragVert, 1.);
	vec4 s_lighting = GetSpotLightColor(sl, vec3(fWorldPosition));

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


    FragColor = vec4(brightness * vec3(d_light_a_intensity, d_light_s_intensity, d_light_d_intensity) * surfaceColor.rgb, surfaceColor.a);
}

vec4 GetSpotLightColor(const SpotLight spotLight, vec3 vWorldPos) 
{ 
  if(spotLight.bOn == 0)return vec4(0.0, 0.0, 0.0, 0.0); 

  float fDistance = distance(vWorldPos, spotLight.vPosition); 

  vec3 vDir = vWorldPos-spotLight.vPosition; 
  vDir = normalize(vDir); 
   
  float fCosine = dot(spotLight.vDirection, vDir); 
  float fDif = 1.0-spotLight.fConeCosine; 
  float fFactor = clamp((fCosine-spotLight.fConeCosine)/fDif, 0.0, 1.0); 

  if(fCosine > spotLight.fConeCosine) 
    return vec4(spotLight.vColor, 1.0)*fFactor/(fDistance*spotLight.fLinearAtt); 

  return vec4(0.0, 0.0, 0.0, 0.0); 
}

vec3 generateLightColor(vec3 light_dir, vec3 normal) {
	//From Sergej
	vec4 fWorldPosition = transformation * vec4(fragVert, 1.);	   //WorldPosition
	vec3 normal_nn = normalize((transformation * vec4(normal,0.0)).xyz);	//The normal must be transformed in World coordinates as well
	
	// Sophisticated lights
	// 100% lightning in 0-40m distance
	// 100% - 0% lightning in 40-100m distance
	float max_dist = 100;
	float distance = distance(vec3(fWorldPosition), camera_position);
	
	float distance_multiplier = 1 - (distance/max_dist);

	float full_light_treshold = 40;
	float gradial_max_dist = max_dist - full_light_treshold;

	if(distance > full_light_treshold) {
		distance_multiplier = 1 - (distance/gradial_max_dist);
	}
	else {
		distance_multiplier = 1;
	}

	vec3 camLightDirection = camera_position - fragVert;

	vec3 d_light_dir_nn = normalize(light_dir);
	vec3 view_dir_nn = normalize(camera_position - fWorldPosition.xyz /*position*/ );		//Transform into world position (Sergej)
	float dot_d_light_normal = max(dot(-d_light_dir_nn, normal_nn), 0.0);   // notice the minus!   //The minus was missing and I used the transformed normal here (Sergej)
	
	vec3 d_reflected_dir_nn = reflect(d_light_dir_nn,normal_nn);					//There is a reflect function build in (Sergej)
	d_reflected_dir_nn = normalize(d_reflected_dir_nn); // should be already normalized, but we "need" to correct numerical errors

	vec3 color;
	vec3 ambient_color = clamp(
		material_a_color * d_light_a_color * d_light_a_intensity, 
		0.0, 1.0);
	vec3 diff_color = clamp(
		material_d_color * dot_d_light_normal * d_light_d_intensity,
		0.0,1.0);
	vec3 spec_color = clamp(
		material_s_color * 
		pow(dot(d_reflected_dir_nn, view_dir_nn), material_shininess),
		0.0,1.0);
	color = distance_multiplier*(ambient_color + diff_color + spec_color); // NOT JUST ONE CHANNEL

	return(color);
}

