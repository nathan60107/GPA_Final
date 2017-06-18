#version 420

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4v;
uniform mat4 um4mv;
uniform mat4 um4p;
uniform mat4 um4m;
uniform mat4 shadow_matrix;

out VertexData
{
	vec4 shadow_coord;
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
	vec3 V;
    vec2 texcoord;
	vec4 viewSpace_coord;
} vertexData;

uniform vec3 light_pos = vec3(20.0, 20.0, 20.0);

void main()
{
	// Calculate view-space coordinate
	vec4 P = um4mv * vec4(iv3vertex, 1.0);
	// Calculate normal in view-space
	vertexData.N = normalize(mat3(um4mv) * iv3normal);
	// Calculate light vector
	vertexData.L = vec3(normalize(um4v*vec4(light_pos, 0.0)));
	// Calculate view vector
	vertexData.V = normalize(-P.xyz);
	vertexData.H = normalize(vertexData.V+vertexData.L);
	// Light-space coordinates
	vertexData.shadow_coord = shadow_matrix * vec4(iv3vertex, 1.0);
	
	gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
	
	vertexData.viewSpace_coord=um4v*um4m*vec4(iv3vertex, 1.0);
}