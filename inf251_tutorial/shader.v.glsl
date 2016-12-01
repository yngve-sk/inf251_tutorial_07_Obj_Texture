#version 330	// GLSL version


// model-view transformation
uniform mat4 worldToProjectionMatrix;
uniform mat4 modelToWorldMatrix;

// vertex position	
layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 normal;

// vertex texture coordinates

// pass the texture coordinates to the fragment shader
out vec3 fragVert;
out vec2 fragTexCoord;
out vec3 fragNormal;

void main() {
    // Pass some variables to the fragment shader
    fragTexCoord = tex_coords;
    fragNormal = vec3(modelToWorldMatrix * vec4(normal,1.));
    fragVert = vec3(modelToWorldMatrix * vec4(position,1.));
    


    // Apply all matrix transformations to vert
	gl_Position = worldToProjectionMatrix * modelToWorldMatrix * vec4(position, 1.); // old version
}