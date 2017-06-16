#version 410

layout(location = 0) out vec4 fragColor;
uniform sampler2DShadow shadow_tex;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform vec3 diffuse_albedo = vec3(0.9, 0.8, 1.0);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 300.0;
uniform bool full_shading = true;	


in VertexData
{
	vec4 shadow_coord;
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
	vec3 V;
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;

void main()
{	
	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 V = normalize(vertexData.V);
	vec3 H = normalize(L + V);
	
	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
	vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;
	
    vec3 texColor = texture(tex,vertexData.texcoord).rgb;
    fragColor = vec4(texColor, 1.0) * vec4(diffuse + specular, 1.0);
	
	fragColor += textureProj(shadow_tex,vertexData.shadow_coord) * vec4(diffuse + specular, 1.0);
}