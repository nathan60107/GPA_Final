#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform mat4 shadow_matrix;

out VertexData
{
	vec4 shadow_coord;
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
	vec3 V;
    vec2 texcoord;
} vertexData;

uniform vec3 light_pos = vec3(20.0, 20.0, 20.0);

void main()
{
	// Calculate view-space coordinate
	vec4 P = um4mv * vec4(iv3vertex, 1.0);
	// Calculate normal in view-space
	vertexData.N = mat3(um4mv) * iv3normal;
	// Calculate light vector
	vertexData.L = light_pos - P.xyz;
	// Calculate view vector
	vertexData.V = -P.xyz;
	// Light-space coordinates
	vertexData.shadow_coord = shadow_matrix * vec4(iv3vertex, 1.0);
	
	gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
}