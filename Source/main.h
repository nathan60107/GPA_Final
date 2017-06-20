#pragma once
///include
#include "../Externals/Include/Include.h"
#include <math.h>
#include <stdlib.h>

///define
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define FOG 4
#define SHADOW 5
#define BLINNPHONG 6
#define ANIMATION 7
#define NOEFFECT 8
#define SHADOW_MAP_SIZE 14745

///using namespace
using namespace glm;
using namespace std;


///struct
typedef struct Shape
{
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_texcoord;
	GLuint ibo;
	int drawCount;
	int materialID;
} Shape;

typedef struct Material
{
	GLuint diffuse_tex;
	aiColor3D diffuse;
	aiColor3D specular;
	aiColor3D ambient = aiColor3D(0.0f,0.0f,0.0f);
	float shininess = 384.314;
} Material;

typedef struct Model {
	std::vector<Material> materials;
	std::vector<Shape> shapes;
	mat4 model_matrix;
	vec3 center;
	int index;
} Model;

typedef struct _TextureData
{
	_TextureData(void) :
		width(0),
		height(0),
		data(0)
	{
	}

	int width;
	int height;
	unsigned char* data;
} TextureData;

struct
{
	struct
	{
		GLint mvp;
		GLint light_pos;
	} light;
	struct
	{
		GLuint  shadow_tex;
		GLint   shadow_matrix;
	} view;
	struct
	{
		GLint um4v;
		GLint um4mv;
		GLint um4p;
		GLint um4m;
		GLint us2dtex;
		GLint diffuse;
		GLint specular;
		GLint ambient;
		GLint shininess;
	} blinnPhong;
	struct
	{
		GLint fogSwitch;
		GLint shadowSwitch;
		GLint blinnPhongSwitch;
	} parameter;
	struct
	{
		GLint inv_vp_matrix;
		GLint eye;
	} skybox;
} uniforms;

struct
{
	GLuint fbo;
	GLuint depthMap;
} shadowBuffer;

struct
{
	int width;
	int height;
} viewportSize;

typedef struct
{
	vec3 position;
	float horizontalAngle;
	float verticalAngle;
	float currentFov = 0;
} Camera;

/// timer
GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

/// models
Model streets[16];
unsigned int streetCount = 0;
Model grass[225];
unsigned int grassCount = 0;
Model signBox[16];
unsigned int signCount = 0;
Model animals[3];
unsigned int animalCount = 0;
float streetDegree[] = {
	0.0f, 180.0f, 90.0f, 270.0f, 90.0f, 0.0f, 180.0f, 0.0f,
	180.0f, 0.0f, 270.0f, 90.0f, 270.0f, 180.0f, 0.0f, 180.0f
};
vec3 streetPosition[] = {
	vec3(75, 0, 355), vec3(35, 0, -590), vec3(-583, 0, -247), vec3(394, 0, 342),
	vec3(-284, 0, -577), vec3(-570, 0, 72), vec3(381, 0, 23), vec3(-271, 0, -258),
	vec3(82, 0, 353), vec3(28, 0, -588), vec3(-581, 0, -240), vec3(392, 0, 335),
	vec3(-282, 0, -570), vec3(-563, 0, 70), vec3(374, 0, 25), vec3(-264, 0, -260)
};
float signDegree[] = {
	0.0f, 270.0f, 0.0f, 270.0f, 0.0f, 90.0f, 0.0f, 90.0f,
	180.0f, 90.0f, 180.0f, 90.0f, 180.0f, 270.0f, 180.0f, 270.0f
};
vec3 signPosition[] = {
	vec3(20, 0, 470), vec3(310, 0, -170), vec3(315, 0, 140), vec3(-20, 0, -470),
	vec3(620, 0, -175), vec3(340, 0, 460), vec3(290, 0, -480), vec3(635, 0, 125),
	vec3(30, 0, 480), vec3(305, 0, -180), vec3(330, 0, 140), vec3(-25, 0, -470),
	vec3(625, 0, -170), vec3(335, 0, 480), vec3(295, 0, -475), vec3(635, 0, 125)
};
string signImage[] = {
	"1", "2", "3", "4", "5", "6", "7", "8",
	"9", "10", "11", "12", "13", "14", "15", "16"
};
float animalInatialDegree[] = {
	0.0f, 0.0f, 0.0f
};
float animalHeight[] = {
	-10.0f, -12.0f, -13.0f
};
float animalSpeed[] = {
	100.0f, 20.0f, 50.0f
};
float animalTime[] = {
	0, 0, 0
};

/// shader
GLuint program;
GLuint depthProg;
mat4 view_matrix;
mat4 proj_matrix;

/// camera setting
Camera actualCamera;
Camera cameras[2];
unsigned int cameraCount = 2;
unsigned int nowCamera = 0;
int cameraAnimateIndex = 0;
float initialFoV = 45.0f;
float viewportAspect;
float cameraSpeed = 80.0f;

/// bezier curve
bool animateStart = false;
int detailOfLevel = 100;
float pastTime = 0;
vector<vec3> curve;
const float basis_matrix[4][4] = {
	{ -1, 3, -3, 1 },
	{ 3, -6, 3, 0 },
	{ -3, 3, 0, 0 },
	{ 1, 0, 0, 0 }
};
vec3 controlPoints[] = {
	vec3(20, -10 , 470), vec3(10, -10, 380), vec3(80, -10, 300),
	vec3(170, -10, 310), vec3(260, -10, 320), vec3(325, -10, 230),
	vec3(315, -10, 140), vec3(305, -10, 50), vec3(380, -10, -30),
	vec3(470, -10, -20), vec3(560, -10, -10), vec3(630, -10, -85),
	vec3(630, -10, -175), vec3(630, -10, -265), vec3(550, -10, -350),
	vec3(460, -10, -340), vec3(370, -10, -330), vec3(280, -10, -390),
	vec3(290, -10, -480), vec3(300, -10, -570), vec3(215, -10, -645),
	vec3(125, -10, -645), vec3(35, -10, -645), vec3(-40, -10, -570),
	vec3(-30, -10, -480), vec3(-20, -10, -390), vec3(-90, -10, -295),
	vec3(-180, -10, -305), vec3(-270, -10, -315), vec3(-340, -10, -230),
	vec3(-330, -10, -140), vec3(-320, -10, -50), vec3(-380, -10, 35),
	vec3(-470, -10, 25), vec3(-560, -10, 15), vec3(-635, -10, 80),
	vec3(-635, -10, 170), vec3(-635, -10, 260), vec3(-570, -10, 345),
	vec3(-480, -10, 335), vec3(-390, -10, 325), vec3(-285, -10, 385),
	vec3(-295, -10, 475), vec3(-305, -10, 565), vec3(-215, -10, 645),
	vec3(-125, -10, 645), vec3(-35, -10, 645), vec3(30, -10, 560),
	vec3(20, -10, 470)
};

/// track ball setting
float deltaTime;
float speed = 0.3f;
float mouseSpeed = 0.005f;
vec3 direction;
vec3 rightDirection;
vec3 up;
vec2 start;

/// mouse info
int xpos, ypos;
bool mousePressOrNot = false;

///Special effects switch parameters(not location)
int fogSwitch = 0;
int shadowSwitch = 0;
int blinnPhongSwitch = 0;

///skybox
GLuint skybox_prog;
GLuint tex_envmap;
GLuint skybox_vao;

/// others
bool printOrNot = true;


/// sky box shader
const char *skybox_fs_glsl[] =
{
	"#version 410 core                          \n"
	"                                           \n"
	"uniform samplerCube tex_cubemap;           \n"
	"                                           \n"
	"in VS_OUT                                  \n"
	"{                                          \n"
	"    vec3    tc;                            \n"
	"} fs_in;                                   \n"
	"                                           \n"
	"layout (location = 0) out vec4 color;      \n"
	"                                           \n"
	"void main(void)                            \n"
	"{                                          \n"
	"    color = texture(tex_cubemap, fs_in.tc);\n"
	"}                                          \n"
	"                                           \n"
};

const char *skybox_vs_glsl[] =
{
	"#version 410 core                                              \n"
	"                                                               \n"
	"out VS_OUT                                                     \n"
	"{                                                              \n"
	"    vec3    tc;                                                \n"
	"} vs_out;                                                      \n"
	"                                                               \n"
	"uniform mat4 inv_vp_matrix;                                    \n"
	"uniform vec3 eye;                                              \n"
	"                                                               \n"
	"void main(void)                                                \n"
	"{                                                              \n"
	"    vec4[4] vertices = vec4[4](vec4(-1.0, -1.0, 1.0, 1.0),     \n"
	"                               vec4( 1.0, -1.0, 1.0, 1.0),     \n"
	"                               vec4(-1.0,  1.0, 1.0, 1.0),     \n"
	"                               vec4( 1.0,  1.0, 1.0, 1.0));    \n"
	"                                                               \n"
	"    vec4 p = inv_vp_matrix * vertices[gl_VertexID];            \n"
	"    p /= p.w;                                                  \n"
	"    vs_out.tc = normalize(p.xyz - eye);                        \n"
	"                                                               \n"
	"    gl_Position = vertices[gl_VertexID];                       \n"
	"}                                                              \n"
};

///////////////////////////////////////////////////////////////
const char *depth_vs[] =
{
	"#version 410 core                         \n"
	"                                          \n"
	"uniform mat4 mvp;                         \n"
	"                                          \n"
	"layout (location = 0) in vec4 position;   \n"
	"                                          \n"
	"void main(void)                           \n"
	"{                                         \n"
	"    gl_Position = mvp * position;         \n"
	"}                                         \n"
};

const char *depth_fs[] =
{
	"#version 410 core                                \n"
	"                                                 \n"
	"out vec4 fragColor;                              \n"
	"                                                 \n"
	"void main()                                      \n"
	"{                                                \n"
	"    fragColor = vec4(vec3(gl_FragCoord.z), 1.0); \n"
	"}                                                \n"
};