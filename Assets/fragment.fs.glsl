#version 420

layout(location = 0) out vec4 fragColor;
layout (binding = 1)uniform sampler2DShadow shadow_tex;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform vec3 diffuse_albedo = vec3(0.9, 0.8, 1.0);//顏色鮮明度
uniform vec3 specular_albedo = vec3(0.7);//光點會不會亮
uniform float specular_power = 300.0;//光點範圍
uniform vec3 ambient;//環境光
uniform bool full_shading = true;
uniform int fogSwitch = 0;
uniform int shadowSwitch = 0;
uniform int blinnPhongSwitch = 0;//blinn和Phong是兩個人 blinn改良Phong做出來的模型

uniform int fog_type = 2;
const vec4 fogColor= vec4(0.5, 0.5,0.5,1.0);
float fogFactor= 0;
float fogDensity= 0.008f;
float fog_start= 1;
float fog_end= 6.0f;

in VertexData
{
	vec4 shadow_coord;
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
	vec3 V;
    vec2 texcoord;
	vec4 viewSpace_coord;
} vertexData;

layout (binding = 0) uniform sampler2D tex;

void main()
{	
	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 H = normalize(vertexData.H);
	vec3 V = normalize(vertexData.V);
	vec3 texColor = texture(tex,vertexData.texcoord).rgb;
	vec3 diffuse = texColor * max(dot(N, L), 0.0) * diffuse_albedo;//擴散:光照上去在物質表面反射出的漫射光
	vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;//鏡面:光照上去在物質表面的反射光(有方向性
	
    if(shadowSwitch==1){
		fragColor = vec4(texColor, 1.0) * vec4(max(dot(N, L), 0.0) * diffuse_albedo + specular, 1.0);//original
			//=texColor * max(dot(N, L), 0.0) * diffuse_albedo + texColor * max(dot(N, L), 0.0) * specular	
	}else if(shadowSwitch == 2){
		fragColor = textureProj(shadow_tex,vertexData.shadow_coord) * vec4(max(dot(N, L), 0.0) * diffuse_albedo + specular, 1.0);//shadow only
			//=
	}else if(shadowSwitch == 3){
		fragColor = textureProj(shadow_tex,vertexData.shadow_coord) * vec4(diffuse + specular, 1.0) + vec4(ambient, 1.0);//by TA
	}else if(shadowSwitch == 4){
		fragColor = textureProj(shadow_tex,vertexData.shadow_coord) * vec4(diffuse + specular, 1.0) + vec4(ambient, 1.0);
		fragColor *= vec4(0.6);
	}else{
		fragColor = vec4(diffuse, 1.0);
			//=texColor * max(dot(N, L), 0.0) * diffuse_albedo
	}
	
	float dist= length(vertexData.viewSpace_coord);
	switch(fog_type)
	{
	case 0: //Linear
		fogFactor= (fog_end-dist)/(fog_end-fog_start);
		break;
	case 1: //Exp
		fogFactor= 1.0 /exp(dist* fogDensity);
		break;
	case 2: //Expsqare
		fogFactor= 1.0 /exp( (dist* fogDensity)* (dist* fogDensity));
		break;
	}
	fogFactor = clamp( fogFactor, 0.0, 1.0 );
	if(fogSwitch==1)fragColor= mix(fogColor,fragColor,fogFactor);
}