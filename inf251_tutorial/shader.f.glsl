#version 130	// GLSL version

// Sampler to access the texture
uniform sampler2D sampler;

// Per fragment texture coordinates
in vec2 cur_tex_coords;
in vec4 f_lighting;

// Per-frgament output color
out vec4 FragColor;

void main() { 
	// Set the output color according to the input
    FragColor = texture2D(sampler, cur_tex_coords.st);
	//texture = texture2D(sampler, cur_tex_coords.st);
	//FragColor = f_lighting * texture;
}