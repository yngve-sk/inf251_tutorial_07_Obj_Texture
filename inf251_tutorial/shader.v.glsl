#version 330	// GLSL version

// vertex position	
layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec3 normal;

// Constant Inputs from Client
uniform mat4 MVPMatrix;         // MVP = Projection x View x Model matrix
uniform mat4 MVMatrix;          // MV = View x Model matrix
uniform mat4 ViewMatrix;        // Just the view matrix (position of camera in the world, looking at some point, usually 0,0,0)

// pass the texture coordinates to the fragment shader
out vec3 viewPosition;
out vec3 viewNormal;
out vec2 texCoord;

void main() {
    // Pass some variables to the fragment shader
    //fragTexCoord = tex_coords;
    //fragNormal = vec3(modelToWorldMatrix * vec4(normal,1.));
    //fragVert = vec3(modelToWorldMatrix * vec4(position,1.));
    
	viewPosition = vec3(MVMatrix * vec4(position, 1.0));
	viewNormal = normalize(vec3(MVMatrix * vec4(normal, 0.0)));
		
	texCoord = tex_coords;

    // Apply all matrix transformations to vert
	//gl_Position = worldToProjectionMatrix * modelToWorldMatrix * vec4(position, 1.); // old version
	gl_Position = MVPMatrix * vec4(position, 1.0);	
}