#version 330	// GLSL version


// model-view transformation
uniform mat4 transformation;

// camera pos
uniform vec3 camera_position;

// vertex position
layout (location = 0) in vec3 position; 
layout (location = 2) in vec3 normal;

// vertex texture coordinates
layout (location = 1) in vec2 tex_coords;

// pass the texture coordinates to the fragment shader
out vec2 cur_tex_coords;
out vec4 f_lighting;

// Lights
uniform vec3 d_light_direction;
uniform vec3 d_light_a_color;
uniform float d_light_a_intensity;

uniform vec3 d_light_d_color;
uniform float d_light_d_intensity;

uniform vec3 d_light_s_color;
uniform float d_light_s_intensity;

// Spotlight
uniform vec3 s_light_a_color;
uniform float s_light_a_intensity;

uniform vec3 s_light_d_color;
uniform float s_light_d_intensity;

uniform vec3 s_light_s_color;
uniform float s_light_s_intensity;


// Object material
 uniform vec3 material_a_color;
 uniform vec3 material_d_color;
 uniform vec3 material_s_color;
 uniform float material_shininess;


vec3 generateLightColor(vec3 light_dir);

void main() {
	// transform the vertex
    gl_Position = transformation * vec4(position, 1.);	

	// pass the texture coordinates to the fragment shader
	cur_tex_coords = tex_coords;

	// calculate light color
	vec3 color = generateLightColor(d_light_direction);

	f_lighting = vec4(color, 1.0);
}

vec3 generateLightColor(vec3 light_dir) {
	//From Sergej
	vec4 fWorldPosition = transformation * vec4(position, 1.);	   //WorldPosition
	vec3 normal_nn = normalize((transformation * vec4(normal,0.0)).xyz);	//The normal must be transformed in World coordinates as well
	

	vec3 camLightDirection = camera_position - position;

	vec3 d_light_dir_nn = normalize(light_dir);
	vec3 view_dir_nn = normalize(camera_position - fWorldPosition.xyz /*position*/ );		//Transform into world position (Sergej)
	float dot_d_light_normal = dot(-d_light_dir_nn, normal_nn);   // notice the minus!   //The minus was missing and I used the transformed normal here (Sergej)
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
	color = (ambient_color + diff_color + spec_color); // NOT JUST ONE CHANNEL

	return(color);
}
