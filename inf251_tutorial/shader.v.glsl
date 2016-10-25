#version 330	// GLSL version


// model-view transformation
uniform mat4 transformation;
uniform mat4 transformationLocal;
uniform vec3 center;

// vertex position
layout (location = 0) in vec3 position; 
layout (location = 2) in vec3 normal;

// vertex texture coordinates
layout (location = 1) in vec2 tex_coords;

// pass the texture coordinates to the fragment shader
out vec3 fragVert;
out vec2 fragTexCoord;
out vec3 fragNormal;

void main() {
    // Pass some variables to the fragment shader
    fragTexCoord = tex_coords;
    fragNormal = normal;
    fragVert = position;
    
    // Apply all matrix transformations to vert

	vec4 centerWorld = vec4(center, 1);
	mat4 translateToCenter = mat4(1, 0, 0, centerWorld.x,
								  0, 1, 0, centerWorld.y,
								  0, 0, 1, centerWorld.z,
								  0, 0, 0,				1);
	
	mat4 translateFromCenter = mat4(1, 0, 0, -centerWorld.x,
									0, 1, 0, -centerWorld.y,
									0, 0, 1, -centerWorld.z,
									0, 0, 0,			  1);

	vec4 position_rotated = translateFromCenter * transformationLocal * translateToCenter * vec4(position, 1.); 

    gl_Position = transformation * transformationLocal * position_rotated;
//	gl_Position = transformation * transformationLocal * vec4(position, 1.); // old version
}