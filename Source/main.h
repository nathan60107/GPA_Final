#pragma once
///include
#include "../Externals/Include/Include.h"
#include "CTexture.h"
#include "CVertexBufferObject.h"
#include "CSkybox.h"
#include <math.h>

///define
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define SHADOW_MAP_SIZE 4096

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
} Model;

struct
{
	struct
	{
		GLint mv_matrix;
		GLint proj_matrix;
	} render;
	struct
	{
		GLint view_matrix;
		GLint eye_position;
	} skybox;
} uniforms;

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
		GLint diffuse;
		GLint specular;
		GLint ambient;
		GLint shininess;
		GLint light_pos;
	} light;
	struct
	{
		GLuint  shadow_tex;
		GLint   mv_matrix;
		GLint   proj_matrix;
		GLint   shadow_matrix;
		GLint   full_shading;
		GLint   light_matrix;
	} view;
} uniforms_shadow;

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

/// timer
GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

/// models
Model models[50];
unsigned int materialsCount = 0;
unsigned int shapeIndex = 0;
unsigned int shapeIndexCount = 0;

/// shader
GLuint program;
GLuint depthProg;
GLuint skyBoxProgram;
GLint um4v;
GLint um4mv;
GLint um4p;
GLint us2dtex;
mat4 view_matrix;
mat4 proj_matrix;

/// sky box
CSkybox skybox;
struct
{
	GLint vColor;
	GLint fAmbientIntensity;
	GLint vDirection;
	GLint projectionMatrix;
	GLint gSampler;
} skyboxUniform;


/// camera setting
vec3 camera_position = vec3(0, 30, 50);		// Initial position : on +Z
float horizontalAngle = 3.14f;				// Initial horizontal angle : toward -Z
float verticalAngle = 0.0f;					// Initial vertical angle : none
float initialFoV = 45.0f;					// Initial Field of View
float currentFov = 0;
float viewportAspect;
int detailOfLevel = 100;
int index = 0;
float pastTime = 0;
vector<vec3> curve;
const float basis_matrix[4][4] = {
	{-1, 3, -3, 1},
	{3, -6, 3, 0},
	{-3, 3, 0, 0},
	{1, 0, 0, 0}
};
vec3 controlPoints[] = {
	vec3(0, 30 ,50), vec3(0, -10, 10), vec3(0, -10, -10),
	vec3(0, 30,-50)
};

/// track ball setting
float deltaTime;
float speed = 0.012f;
float mouseSpeed = 0.005f;
vec3 direction;
vec3 rightDirection;
vec3 up;
vec2 start;

/// mouse info
int xpos, ypos;
bool mousePressOrNot = false;

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
	"#version 410 core                                         \n"
	"                                                          \n"
	"out VS_OUT                                                \n"
	"{                                                         \n"
	"    vec3    tc;                                           \n"
	"} vs_out;                                                 \n"
	"                                                          \n"
	"uniform mat4 view_matrix;                                 \n"
	"                                                          \n"
	"void main(void)                                           \n"
	"{                                                         \n"
	"    vec3[4] vertices = vec3[4](vec3(-1.0, -1.0, 1.0),     \n"
	"                               vec3( 1.0, -1.0, 1.0),     \n"
	"                               vec3(-1.0,  1.0, 1.0),     \n"
	"                               vec3( 1.0,  1.0, 1.0));    \n"
	"                                                          \n"
	"    vs_out.tc = mat3(view_matrix) * vertices[gl_VertexID];\n"
	"                                                          \n"
	"    gl_Position = vec4(vertices[gl_VertexID], 1.0);       \n"
	"}                                                         \n"
	"                                                          \n"
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