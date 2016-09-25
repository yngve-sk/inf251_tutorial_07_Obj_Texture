#version 130	// GLSL version

// Sampler to access the texture
uniform sampler2D sampler;

// Per fragment texture coordinates
in vec2 cur_tex_coords;

// Per-frgament output color
out vec4 FragColor;

void main() { 
	// Set the output color according to the input
    FragColor = texture2D(sampler, cur_tex_coords.st);
	
}