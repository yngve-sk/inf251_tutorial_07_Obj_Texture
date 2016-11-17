#include <gl/GL.h>

struct Vertex {
	glm::fvec3 position, normal;
};


struct LightParameters {
	GLint DirLoc = -1;
	
	GLint AColorLoc = -1;
	GLint DColorLoc = -1;
	GLint SColorLoc = -1;
	
	GLint AIntensityLoc = -1;
	GLint DIntensityLoc = -1;
	GLint SIntensityLoc = -1;
};

struct MaterialParameters {
	vec3 MaterialAColor;
	vec3 MaterialDColor;
	vec3 MaterialSColor;
	float MaterialShine;
};

struct ShaderData {
	GLint TrLoc, LocalTrLoc;
	GLint SamplerLoc;
};

struct Transformations {
	mat4 rotateX, rotateY, rotateZ, translate;
	float scale;
};

