#pragma once
///include
#include "../Externals/Include/Include.h"
#include "CTexture.h"
#include "CVertexBufferObject.h"
#include "CSkybox.h"

///define
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define PPOPamount 16

///using namespace
using namespace glm;
using namespace std;

///struct
struct Shape
{
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_texcoord;
	GLuint ibo;
	int drawCount;
	int materialID;
};

struct Material
{
	GLuint diffuse_tex;
};

struct uniforms_block
{
	glm::mat4 mv_matrix;
	mat4 view_matrix;
	mat4 proj_matrix;
};

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

// define a simple data structure for storing texture image raw data
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

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

Shape shapes[3][200];//vao
Material materials[3][200]; //text
unsigned int materialsCount = 0;
unsigned int shapesCount[3] = { 0 };
unsigned int shapeIndex = 0;
unsigned int shapeIndexCount = 0;
bool printOrNot = 1;

mat4 mvp;
GLint um4mv;		// id of mvp
GLint um4p;
GLint us2dtex;		// id of the texture
GLuint program;

GLuint program2;
GLuint window_vao;
GLuint window_buffer;
//以Location表示是glUniform的位置變數
int PPOPLocation;//postProcessOperationParameter簡稱PPOP
int PPOP = 0;//目前使用的濾鏡編號
int comparisonBarBorderLocation;
int comparisonBarBorder = 0;//comparisonBar分界位置
int timeLocation;
int magnifierCenterLocation;
float magnifierCenter[2] = { 300, 300 };//放大鏡中心位置
GLuint noiseTexture;

// FBO parameter
GLuint			FBO;
GLuint			depthRBO;
GLuint			FBODataTexture;

vec3 view_position;
mat4 view_matrix;
mat4 model_matrix;
mat4 proj_matrix;

int index_count;
GLuint uniforms_buffer;

// Initial position : on +Z
vec3 position = vec3(0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 0.012f; //
float mouseSpeed = 0.005f;

// Get mouse position
int xpos, ypos;

vec3 direction;
vec3 rightDirection;
vec3 up;

float deltaTime;

float currentFov = 0;

CSkybox skybox;
GLuint skybox_prog;
GLuint skybox_vao;
GLuint tex_envmap;

void My_Reshape(int width, int height);
static const GLfloat window_positions[] =
{
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};

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

///本次新增
typedef struct {
	std::vector<Material> materials;
	std::vector<Shape> shapes;
	vec3 camera_position;
	vec2 camera_angle;
} Model;

Model models[50];

bool mousePressOrNot = 0;

///