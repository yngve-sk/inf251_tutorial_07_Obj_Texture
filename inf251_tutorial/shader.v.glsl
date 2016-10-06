#version 130	// GLSL version

// model-view transformation
uniform mat4 transformation;

// camera pos
uniform vec3 camera_position;

// vertex position
in vec3 position; 

// vertex texture coordinates
in vec2 tex_coords;

// pass the texture coordinates to the fragment shader
out vec2 cur_tex_coords;

void main() {
	// transform the vertex
    gl_Position = transformation * vec4(position, 1.);	
	
	// pass the texture coordinates to the fragment shader
	cur_tex_coords = tex_coords;
}