#include "main.h"

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void shaderLog(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete[] errorLog;
	}
}

void freeShaderSource(char** srcp)
{
	delete[] srcp[0];
	delete[] srcp;
}

TextureData loadPNG(const char* const pngFilepath)
{
	TextureData texture;
	int components;

	// load the texture with stb image, force RGBA (4 components required)
	stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);

	// is the image successfully loaded?
	if (data != NULL)
	{
		// copy the raw data
		size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
		texture.data = new unsigned char[dataSize];
		memcpy(texture.data, data, dataSize);

		// mirror the image vertically to comply with OpenGL convention
		for (size_t i = 0; i < texture.width; ++i)
		{
			for (size_t j = 0; j < texture.height / 2; ++j)
			{
				for (size_t k = 0; k < 4; ++k)
				{
					size_t coord1 = (j * texture.width + i) * 4 + k;
					size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
					std::swap(texture.data[coord1], texture.data[coord2]);
				}
			}
		}

		// release the loaded image
		stbi_image_free(data);
	}
	else {
		printf("ERROR!!data==NULL. Image not found. Image address=%s.\n", pngFilepath);
	}

	return texture;
}

void drawCube(float width, float height, float length, vec3 center, Model* models, unsigned int* count)
{
	models[*count].shapes.push_back(Shape());
	glGenVertexArrays(1, &models[*count].shapes[0].vao);
	glBindVertexArray(models[*count].shapes[0].vao);

	// create data
	GLfloat x = center.x;
	GLfloat y = center.y;
	GLfloat z = center.z;
	length /= 2;
	width /= 2;
	height /= 2;
	GLfloat vertex[] =
	{
		x - width, y + height, z - length, 1, 1, 0, 0, -1,
		x - width, y - height, z - length, 1, 0, 0, 0, -1,
		x + width, y - height, z - length, 0, 0, 0, 0, -1,

		x + width, y - height, z - length, 0, 0, 0, 0, -1,
		x + width, y + height, z - length, 0, 1, 0, 0, -1,
		x - width, y + height, z - length, 1, 1, 0, 0, -1,

		x + width, y - height, z - length, 1, 0, 1, 0, 0,
		x + width, y - height, z + length, 0, 0, 1, 0, 0,
		x + width, y + height, z - length, 1, 1, 1, 0, 0,

		x + width, y - height, z + length, 0, 0, 1, 0, 0,
		x + width, y + height, z + length, 0, 1, 1, 0, 0,
		x + width, y + height, z - length, 1, 1, 1, 0, 0,

		x + width, y - height, z + length, 1, 0, 0, 0, 1,
		x - width, y - height, z + length, 0, 0, 0, 0, 1,
		x + width, y + height, z + length, 1, 1, 0, 0, 1,

		x - width, y - height, z + length, 0, 0, 0, 0, 1,
		x - width, y + height, z + length, 0, 1, 0, 0, 1,
		x + width, y + height, z + length, 1, 1, 0, 0, 1,

		x - width, y - height, z + length, 1, 0, -1, 0, 0,
		x - width, y - height, z - length, 0, 0, -1, 0, 0,
		x - width, y + height, z + length, 1, 1, -1, 0, 0,

		x - width, y - height, z - length, 0, 0, -1, 0, 0,
		x - width, y + height, z - length, 0, 1, -1, 0, 0,
		x - width, y + height, z + length, 1, 1, -1, 0, 0,

		x - width, y - height, z + length, 0, 1, 0, -1, 0,
		x + width, y - height, z + length, 1, 1, 0, -1, 0,
		x + width, y - height, z - length, 1, 0, 0, -1, 0,

		x + width, y - height, z - length, 1, 0, 0, -1, 0,
		x - width, y - height, z - length, 0, 0, 0, -1, 0,
		x - width, y - height, z + length, 0, 1, 0, -1, 0,

		x - width, y + height, z - length, 0, 1, 0, 1, 0,
		x + width, y + height, z - length, 1, 1, 0, 1, 0,
		x + width, y + height, z + length, 1, 0, 0, 1, 0,

		x + width, y + height, z + length, 1, 0, 0, 1, 0,
		x - width, y + height, z + length, 0, 0, 0, 1, 0,
		x - width, y + height, z - length, 0, 1, 0, 1, 0
	};

	// put data to buffer
	glGenBuffers(1, &models[*count].shapes[0].vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, models[*count].shapes[0].vbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// set vao format
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (const void*)(sizeof(GLfloat) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (const void*)(sizeof(GLfloat) * 5));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	models[*count].shapes[0].drawCount = 36;
	*count = *count + 1;
}

void drawSquare(float width, float height, vec2 center, Model* models, unsigned int* count)
{
	models[*count].shapes.push_back(Shape());
	glGenVertexArrays(1, &models[*count].shapes[0].vao);
	glBindVertexArray(models[*count].shapes[0].vao);

	// create data
	GLfloat x = center.x;
	GLfloat y = center.y;
	width /= 2;
	height /= 2;
	GLfloat vertex[] =
	{
		x - width, -20, y + height, 0, 0, 0, 1, 0,
		x - width, -20, y - height, 0, 1, 0, 1, 0,
		x + width, -20, y - height, 1, 1, 0, 1, 0,

		x + width, -20, y - height, 1, 1, 0, 1, 0,
		x + width, -20, y + height, 1, 0, 0, 1, 0,
		x - width, -20, y + height, 0, 0, 0, 1, 0
	};

	// put data to buffer
	glGenBuffers(1, &models[*count].shapes[0].vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, models[*count].shapes[0].vbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// set vao format
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (const void*)(sizeof(GLfloat) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (const void*)(sizeof(GLfloat) * 5));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	models[*count].shapes[0].drawCount = 6;
	*count = *count + 1;
}

void changeView()
{
	direction = vec3(cos(actualCamera.verticalAngle) * sin(actualCamera.horizontalAngle), sin(actualCamera.verticalAngle), cos(actualCamera.verticalAngle) * cos(actualCamera.horizontalAngle));
	rightDirection = vec3(sin(actualCamera.horizontalAngle - 3.14f / 2.0f), 0, cos(actualCamera.horizontalAngle - 3.14f / 2.0f));
	up = cross(rightDirection, direction);
	view_matrix = lookAt(actualCamera.position, actualCamera.position + direction, up);
	proj_matrix = perspective(initialFoV - 5 * actualCamera.currentFov, viewportAspect, 0.1f, 2121.0f);
}

GLuint createProgram(std::string vertex, std::string fragment)
{
	GLuint prog = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char ** vertexShaderSource = loadShaderSource(vertex.c_str());
	char ** fragmentShaderSource = loadShaderSource(fragment.c_str());
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	shaderLog(vertexShader);
	shaderLog(fragmentShader);
	glAttachShader(prog, vertexShader);
	glAttachShader(prog, fragmentShader);
	glLinkProgram(prog);
	return prog;
}

void loadSence(char* objPathInput, char* textuerPathInput, Model* models, unsigned int* count, vec3 center, vec3 scale)
{
	printf("----------------------------\nStart to load sence %d.\n", *count);

	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	const aiScene* scene = aiImportFile(objPathInput, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene == NULL) {
		printf("Fail to load scene and print ERROR:");
		printf(aiGetErrorString());
		printf("\n");
		return;
	}
	models[*count].materials.resize(scene->mNumMaterials);
	models[*count].shapes.resize(scene->mNumMeshes);

	// load materials
	for (unsigned int i = 0; i< scene->mNumMaterials; ++i)
	{
		aiMaterial *material = scene->mMaterials[i];
		aiString texturePath;
		string pngPath;
		TextureData textureData;

		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			// load width, height and data from texturePath.C_Str();
			glGenTextures(1, &models[*count].materials[i].diffuse_tex);
			glBindTexture(GL_TEXTURE_2D, models[*count].materials[i].diffuse_tex);
			pngPath.append(textuerPathInput);
			pngPath.append(texturePath.C_Str());
			textureData = loadPNG(pngPath.c_str());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureData.width, textureData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//X軸Y軸的處理方式
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glGenerateMipmap(GL_TEXTURE_2D);
			if (printOrNot) printf("Successly saving material %d. Texture name = %s.\n", i, texturePath.C_Str());
		}
		else
		{
			// load some default image as default_diffuse_tex
			//material.diffuse_tex = default_diffuse_tex;
			printf("Fail to save material %d. Texture name = %s.\n", i, texturePath.C_Str());
		}

		aiColor3D color(0.f, 0.f, 0.f);
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS) {
			models[*count].materials[i].diffuse = color;
			//cout << "DIFFUSE" << " " << color.r << " " << color.g << " " << color.b << endl;
		}
		else {
			//cout << "DIFFUSE NOT FOUND<----------------------------ERROR!!"<< endl;
		}
		if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS) {
			models[*count].materials[i].specular = color;
			//cout << "SPECULAR" << " " << color.r << " " << color.g << " " << color.b << endl;
		}
		else {
			//cout << "SPECULAR NOT FOUND<----------------------------ERROR!!" << endl;
		}
		if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == aiReturn_SUCCESS) {
			models[*count].materials[i].ambient = color;
			//cout << "AMBIENT" << " " << color.r << " " << color.g << " " << color.b << endl;
		}
		else {
			//cout << "AMBIENT NOT FOUND<----------------------------ERROR!!" << endl;
		}
		float c;
		if (material->Get(AI_MATKEY_SHININESS, c) == aiReturn_SUCCESS) {
			models[*count].materials[i].shininess = c;
			//cout << "SHININESS" << " " << c << endl;
		}
		else {
			//cout << "SHININESS NOT FOUND<----------------------------ERROR!!" << endl;
		}
	}

	// calculate center
	int n = 0;
	vec3 c = vec3(0, 0, 0);
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh *mesh = scene->mMeshes[i];
		if (mesh != nullptr) {
			for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
				const aiVector3D *pos = &(mesh->mVertices[v]);
				c.x = (c.x*n + pos->x) / (n + 1);
				c.y = (c.y*n + pos->y) / (n + 1);
				c.z = (c.z*n + pos->z) / (n + 1);
				n++;
			}
		}
	}
	models[*count].center = c;
	
	// load geometry
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh*mesh = scene->mMeshes[i];
		float temp[4][50000];
		int tempInt[50000];
		
		glGenVertexArrays(1, &models[*count].shapes[i].vao);
		glBindVertexArray(models[*count].shapes[i].vao);
		
		if (mesh != nullptr) {
			int errorCount[2] = { 0 };

			// create 3 vbos to hold data
			for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
			{
				// mesh->mVertices[v][0~2] => position
				const aiVector3D *pos = &(mesh->mVertices[v]);
				temp[0][v * 3] = (pos->x - c.x)*scale.x + center.x;
				temp[0][v * 3 + 1] = (pos->y - c.y)*scale.y + center.y;
				temp[0][v * 3 + 2] = (pos->z - c.z)*scale.z + center.z;
				
				// mesh->mNormals[v][0~2] => normal
				if (mesh->HasNormals()) {
					temp[1][v * 3] = mesh->mNormals[v][0];
					temp[1][v * 3 + 1] = mesh->mNormals[v][1];
					temp[1][v * 3 + 2] = mesh->mNormals[v][2];
				}
				else {
					errorCount[0]++;
					temp[1][v * 3] = Zero3D[0];
					temp[1][v * 3 + 1] = Zero3D[1];
					temp[1][v * 3 + 2] = Zero3D[2];
				}
				
				// mesh->mTextureCoords[0][v][0~1] => texcoord
				if (mesh->HasTextureCoords(0)) {
					temp[2][v * 2] = mesh->mTextureCoords[0][v][0];
					temp[2][v * 2 + 1] = mesh->mTextureCoords[0][v][1];
				}
				else {
					errorCount[1]++;
					temp[2][v * 2] = Zero3D[0];
					temp[2][v * 2 + 1] = Zero3D[1];
				}
			}

			if (errorCount[0] != 0)printf("ERROR!! Normals not found %d time(s), replace by vec(0, 0, 0).\n", errorCount[0]);
			if (errorCount[1] != 0)printf("ERROR!! TextureCoords not found %d time(s), replace by vec(0, 0, 0).\n", errorCount[1]);

			glGenBuffers(1, &models[*count].shapes[i].vbo_position);
			glBindBuffer(GL_ARRAY_BUFFER, models[*count].shapes[i].vbo_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, temp[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &models[*count].shapes[i].vbo_normal);
			glBindBuffer(GL_ARRAY_BUFFER, models[*count].shapes[i].vbo_normal);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, temp[1], GL_STATIC_DRAW);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);

			glGenBuffers(1, &models[*count].shapes[i].vbo_texcoord);
			glBindBuffer(GL_ARRAY_BUFFER, models[*count].shapes[i].vbo_texcoord);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, temp[2], GL_STATIC_DRAW);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			// create 1 ibo to hold data
			for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
			{
				// mesh->mFaces[f].mIndices[0~2] => index
				tempInt[f * 3] = mesh->mFaces[f].mIndices[0];
				tempInt[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
				tempInt[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
			}

			glGenBuffers(1, &models[*count].shapes[i].ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, models[*count].shapes[i].ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * mesh->mNumFaces, tempInt, GL_STATIC_DRAW);
			
			// save shape info
			models[*count].shapes[i].materialID = mesh->mMaterialIndex;
			models[*count].shapes[i].drawCount = mesh->mNumFaces * 3;
			if (printOrNot) printf("Successly saving shapes %d.\n", i);
		}
	}
	printf("Loading sence %d completes.\n----------------------------\n", *count);
	aiReleaseImport(scene);
	*count = *count + 1;
}

void skyboxInit()
{
	skybox_prog = glCreateProgram();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, skybox_fs_glsl, NULL);
	glCompileShader(fs);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, skybox_vs_glsl, NULL);
	glCompileShader(vs);

	glAttachShader(skybox_prog, vs);
	glAttachShader(skybox_prog, fs);

	glLinkProgram(skybox_prog);
	glUseProgram(skybox_prog);

	uniforms.skybox.inv_vp_matrix = glGetUniformLocation(skybox_prog, "inv_vp_matrix");
	uniforms.skybox.eye = glGetUniformLocation(skybox_prog, "eye");

	TextureData envmap_data = loadPNG("../TexturedScene/skyboxes/skycube.png");
	glGenTextures(1, &tex_envmap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, envmap_data.width, envmap_data.height / 6, 0, GL_RGBA, GL_UNSIGNED_BYTE, envmap_data.data + i * (envmap_data.width * (envmap_data.height / 6) * sizeof(unsigned char) * 4));
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	delete[] envmap_data.data;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glGenVertexArrays(1, &skybox_vao);
}

void drawSkybox() {
	static const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	static const GLfloat ones[] = { 1.0f };
	/*float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.0005f;

	vec3 eye = vec3(0.0f, 0.0f, 0.0f);
	mat4 view_matrix = lookAt(eye, vec3(15.0f * sinf(currentTime), 7.0f * sinf(currentTime), 15.0f * cosf(currentTime)), vec3(0.0f, 1.0f, 0.0f));
	mat4 mv_matrix = view_matrix *
		rotate(mat4(), 0.0f, vec3(1.0f, 0.0f, 0.0f)) *
		rotate(mat4(), 0.0f, vec3(0.0f, 1.0f, 0.0f)) *
		translate(mat4(), vec3(0.0f, -4.0f, 0.0f));*/
	mat4 inv_vp_matrix = inverse(proj_matrix * view_matrix);

	glClearBufferfv(GL_COLOR, 0, gray);
	glClearBufferfv(GL_DEPTH, 0, ones);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);

	glUseProgram(skybox_prog);
	glBindVertexArray(skybox_vao);

	glUniformMatrix4fv(uniforms.skybox.inv_vp_matrix, 1, GL_FALSE, &inv_vp_matrix[0][0]);
	glUniform3fv(uniforms.skybox.eye, 1, &actualCamera.position[0]);

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
}

void My_Init()
{
	glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	skyboxInit();

	// ----- Begin Initialize Depth Shader Program -----
	GLuint shadow_vs;
	GLuint shadow_fs;
	shadow_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shadow_vs, 1, depth_vs, 0);
	glCompileShader(shadow_vs);
	shadow_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shadow_fs, 1, depth_fs, 0);
	glCompileShader(shadow_fs);
	depthProg = glCreateProgram();
	glAttachShader(depthProg, shadow_vs);
	glAttachShader(depthProg, shadow_fs);
	glLinkProgram(depthProg);
	uniforms.light.mvp = glGetUniformLocation(depthProg, "mvp");
	// ----- End Initialize Depth Shader Program -----


	// ----- Begin Initialize Blinn-Phong Shader Program -----
	// create shader
	program = createProgram("vertex.vs.glsl", "fragment.fs.glsl");
	glUseProgram(program);

	// get uniform location
	uniforms.blinnPhong.um4v = glGetUniformLocation(program, "um4v");
	uniforms.blinnPhong.um4mv = glGetUniformLocation(program, "um4mv");
	uniforms.blinnPhong.um4p = glGetUniformLocation(program, "um4p");
	uniforms.blinnPhong.um4m = glGetUniformLocation(program, "um4m");
	uniforms.blinnPhong.us2dtex = glGetUniformLocation(program, "tex");
	uniforms.view.shadow_matrix = glGetUniformLocation(program, "shadow_matrix");
	uniforms.view.shadow_tex = glGetUniformLocation(program, "shadow_tex");
	uniforms.blinnPhong.diffuse = glGetUniformLocation(program, "diffuse_albedo");
	uniforms.blinnPhong.specular = glGetUniformLocation(program, "specular_albedo");
	uniforms.blinnPhong.ambient = glGetUniformLocation(program, "ambient");
	uniforms.blinnPhong.shininess = glGetUniformLocation(program, "specular_power");
	uniforms.light.light_pos = glGetUniformLocation(program, "light_pos");
	uniforms.parameter.fogSwitch = glGetUniformLocation(program, "fogSwitch");
	uniforms.parameter.shadowSwitch = glGetUniformLocation(program, "shadowSwitch");
	uniforms.parameter.blinnPhongSwitch = glGetUniformLocation(program, "blinnPhongSwitch");
	// ----- End Initialize Blinn-Phong Shader Program -----

	// ----- Begin Initialize Scene Model -----
	// street
	loadSence("../TexturedScene/scene/old fashion town/old town block.obj", "../TexturedScene/scene/old fashion town/", streets, &streetCount, vec3(0, 0, 0), vec3(0.5));
	streets[0].model_matrix = translate(mat4(), streetPosition[0]);
	for (int i = 1; i < 16; i++) {
		streets[streetCount] = streets[streetCount - 1];
		mat4 matrix = rotate(mat4(), float(deg2rad(streetDegree[i]), vec3(0, 1, 0));
		streets[streetCount++].model_matrix = translate(matrix, streetPosition[i]);
	}

	// grass
	for (int i = -7; i <= 7; i++) {
		for (int j = -7; j <= 7; j++) {
			drawSquare(100, 100, vec2(i * 100, j * 100), grass, &grassCount);
			grass[grassCount - 1].model_matrix = mat4();
		}
	}
	grass[0].materials.resize(sizeof(Material));
	TextureData textureData = loadPNG("../TexturedScene/scene/grass.jpg");
	glGenTextures(1, &grass[0].materials[0].diffuse_tex);
	glBindTexture(GL_TEXTURE_2D, grass[0].materials[0].diffuse_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureData.width, textureData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// sign
	for (int i = 0; i < 16; i++) {
		drawCube(5, 5, 5, signPosition[i], signBox, &signCount);
		signBox[i].model_matrix = rotate(mat4(), float(deg2rad(signDegree[i]), vec3(0, 1, 0));
		string path = "../TexturedScene/scene/number box/";
		path.append(signImage[i]);
		path.append(".png");
		TextureData textureData = loadPNG(path.c_str());
		signBox[i].materials.resize(sizeof(Material));
		glGenTextures(1, &signBox[i].materials[0].diffuse_tex);
		glBindTexture(GL_TEXTURE_2D, signBox[i].materials[0].diffuse_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureData.width, textureData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	// draw animals
	loadSence("../TexturedScene/chimp/chimp.obj", "../TexturedScene/chimp/", animals, &animalCount, vec3(0, 0, 0), vec3(10));
	loadSence("../TexturedScene/Wolf/Wolf.obj", "../TexturedScene/Wolf/", animals, &animalCount, vec3(0, 0, 0), vec3(6));
	loadSence("../TexturedScene/Bear_Brown/Bear_Brown.obj", "../TexturedScene/Bear_Brown/", animals, &animalCount, vec3(0, 0, 0), vec3(5));
	// ----- End Initialize Scene Model -----

	// calculate camara bezier curve
	int controlPointNum = sizeof(controlPoints) / sizeof(controlPoints[0]);
	for (int i = 0; i < (controlPointNum - 1) / 3; ++i) {
		float controlGroup[4][3] = {
			{ controlPoints[i * 3].x, controlPoints[i * 3].y , controlPoints[i * 3].z },
			{ controlPoints[i * 3 + 1].x, controlPoints[i * 3 + 1].y , controlPoints[i * 3 + 1].z },
			{ controlPoints[i * 3 + 2].x, controlPoints[i * 3 + 2].y , controlPoints[i * 3 + 2].z },
			{ controlPoints[i * 3 + 3].x, controlPoints[i * 3 + 3].y , controlPoints[i * 3 + 3].z }
		};
		float matrix[4][3];
		for (int a = 0; a < 4; ++a) {
			for (int b = 0; b < 3; ++b) {
				float value = 0;
				for (int c = 0; c < 4; ++c) {
					value += basis_matrix[a][c] * controlGroup[c][b];
				}
				matrix[a][b] = value;
			}
		}
		for (int j = 0; j < detailOfLevel; ++j) {
			vec3 pos;
			float t = float(j) / detailOfLevel;
			float T[4] = { pow(t, 3), pow(t, 2), t, 1 };
			for (int a = 0; a < 3; ++a) {
				float value = 0;
				for (int b = 0; b < 4; ++b) {
					value += T[b] * matrix[b][a];
				}
				pos[a] = value;
			}
			curve.push_back(pos);
		}
	}
	for (int i = 0; i < animalCount; i++) {
		animals[i].index = rand() % curve.size();
	}
	
	// setting shadow frame buffer
	glGenFramebuffers(1, &shadowBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	glGenTextures(1, &shadowBuffer.depthMap);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBuffer.depthMap, 0);

	// setting camera
	cameras[0].position = vec3(600, 300, 600);
	cameras[0].horizontalAngle = 3.93f;
	cameras[0].verticalAngle = -0.39f;
	cameras[1].position = vec3(20, -10, 470);
	cameras[1].horizontalAngle = 3.14f;
	cameras[1].verticalAngle = 0.0f;
	actualCamera = cameras[0];

	if (printOrNot) printf("finish My_Init\n");
}

void My_Display()
{
	mat4 scale_bias_matrix = mat4(
		vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f)
	);
	
	// ----- Begin Shadow Map Pass -----
	vec3 lightPosition = vec3(75.0f, 75.0f, 20.0f);
	mat4 light_proj_matrix = ortho<float>(-1000, 1000, -750, 750, -750, 750);
	mat4 light_view_matrix = lookAt(lightPosition, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix * mat4(1.0);

	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	glUseProgram(depthProg);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	
	for (int m = 0; m < streetCount; ++m) {
		// transmit uniform variable
		glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix *streets[m].model_matrix));

		// draw
		for (int i = 0; i < streets[m].shapes.size(); ++i)
		{
			glBindVertexArray(streets[m].shapes[i].vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, streets[m].shapes[i].ibo);
			glDrawElements(GL_TRIANGLES, streets[m].shapes[i].drawCount, GL_UNSIGNED_INT, 0);
		}
	}

	for (int m = 0; m < animalCount; ++m) {
		// transmit uniform variable
		glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix *animals[m].model_matrix));

		// draw
		for (int i = 0; i < animals[m].shapes.size(); ++i)
		{
			glBindVertexArray(animals[m].shapes[i].vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, animals[m].shapes[i].ibo);
			glDrawElements(GL_TRIANGLES, animals[m].shapes[i].drawCount, GL_UNSIGNED_INT, 0);
		}
	}

	// draw grass
	for (int i = 0; i < grassCount; i++) {
		glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix *grass[i].model_matrix));
		glBindVertexArray(grass[i].shapes[0].vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	for (int i = 0; i < signCount; i++) {
		glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix *signBox[i].model_matrix));
		glBindVertexArray(signBox[i].shapes[0].vao);
		glDrawArrays(GL_TRIANGLES, 0, signBox[i].shapes[0].drawCount);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	// ----- End Shadow Map Pass -----

	// ----- Begin Blinn-Phong Shading Pass -----
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, viewportSize.width, viewportSize.height);
	drawSkybox();
	glUseProgram(program);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glUniform1i(uniforms.view.shadow_tex, 1);
	glUniform3fv(uniforms.light.light_pos, 3, value_ptr(lightPosition));
	glUniformMatrix4fv(uniforms.blinnPhong.um4p, 1, GL_FALSE, value_ptr(proj_matrix));

	// calculate elapsed time
	static double lastTime = glutGet(GLUT_ELAPSED_TIME);
	double currentTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = float(currentTime - lastTime);
	pastTime += deltaTime;
	for (int i = 0; i < animalCount; i++) {
		animalTime[i] += deltaTime;
	}

	// reset mouse position and calculate angle
	if (mousePressOrNot && !animateStart) {
		glutWarpPointer(start.x, start.y);
		actualCamera.horizontalAngle += mouseSpeed * float(start.x - xpos);
		actualCamera.verticalAngle += mouseSpeed * float(start.y - ypos);
	}

	// animate
	if (pastTime > cameraSpeed && animateStart) {
		actualCamera.position = curve[cameraAnimateIndex];
		vec3 vec = curve[(cameraAnimateIndex + 1) % curve.size()] - curve[cameraAnimateIndex];
		actualCamera.horizontalAngle = atan(vec.x / vec.z);
		if (vec.z < 0) actualCamera.horizontalAngle += 3.14;
		actualCamera.verticalAngle = 0.0f;
		cameraAnimateIndex = (cameraAnimateIndex + 1) % curve.size();
		pastTime = 0;
	}

	for (int i = 0; i < animalCount; i++) {
		if (animalTime[i] > animalSpeed[i]) {
			// real time dir
			vec3 vec = curve[(animals[i].index + 1) % curve.size()] - curve[animals[i].index];
			float horizontalAngle = atan(vec.x / vec.z);
			if (vec.z < 0) horizontalAngle += 3.14;

			// polar coord
			vec3 pos = curve[animals[i].index];
			float theta = atan(pos.x / pos.z);
			if (pos.z < 0) theta += 3.14;
			theta = (theta - horizontalAngle) * 180 / 3.14 - animalInatialDegree[i];
			float r = distance(pos, vec3(0, pos.y, 0));

			// model matrix
			mat4 matrix = rotate(mat4(), float(deg2rad(animalInatialDegree[i]) + horizontalAngle, vec3(0, 1, 0));
			animals[i].model_matrix = translate(matrix, vec3(r*sin(deg2rad(theta), animalHeight[i], r*cos(deg2rad(theta)));
			animals[i].index = (animals[i].index + 1) % curve.size();
			animalTime[i] = 0;
		}
	}

	// change view and record time
	changeView();
	lastTime = currentTime;

	// draw streets
	for (int m = 0; m < streetCount; ++m) {

		// transmit uniform variable
		mat4 mv_matrix = view_matrix * streets[m].model_matrix;
		mat4 shadow_matrix = shadow_sbpv_matrix * streets[m].model_matrix;
		glUniformMatrix4fv(uniforms.view.shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix));
		glUniformMatrix4fv(uniforms.blinnPhong.um4v, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4mv, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4p, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4m, 1, GL_FALSE, value_ptr(streets[m].model_matrix));

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(uniforms.blinnPhong.us2dtex, 0);

		// draw
		for (int i = 0; i < streets[m].shapes.size(); ++i)
		{
			glBindVertexArray(streets[m].shapes[i].vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, streets[m].shapes[i].ibo);
			int materialID = streets[m].shapes[i].materialID;

			glBindTexture(GL_TEXTURE_2D, streets[m].materials[materialID].diffuse_tex);
			glDrawElements(GL_TRIANGLES, streets[m].shapes[i].drawCount, GL_UNSIGNED_INT, 0);
		}
	}

	// draw grass
	glBindTexture(GL_TEXTURE_2D, grass[0].materials[0].diffuse_tex);
	for (int i = 0; i < grassCount; i++) {
		mat4 mv_matrix = view_matrix * grass[i].model_matrix;
		mat4 shadow_matrix = shadow_sbpv_matrix * grass[i].model_matrix;
		glUniformMatrix4fv(uniforms.view.shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix));
		glUniformMatrix4fv(uniforms.blinnPhong.um4v, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4mv, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4p, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4m, 1, GL_FALSE, value_ptr(grass[i].model_matrix));
		glBindVertexArray(grass[i].shapes[0].vao);
		glDrawArrays(GL_TRIANGLES, 0, grass[i].shapes[0].drawCount);
	}

	// draw animals
	for (int m = 0; m < animalCount; ++m) {

		// transmit uniform variable
		mat4 mv_matrix = view_matrix * animals[m].model_matrix;
		mat4 shadow_matrix = shadow_sbpv_matrix * animals[m].model_matrix;
		glUniformMatrix4fv(uniforms.view.shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix));
		glUniformMatrix4fv(uniforms.blinnPhong.um4v, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4mv, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4p, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4m, 1, GL_FALSE, value_ptr(streets[m].model_matrix));

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(uniforms.blinnPhong.us2dtex, 0);

		// draw
		for (int i = 0; i < animals[m].shapes.size(); ++i)
		{
			glBindVertexArray(animals[m].shapes[i].vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, animals[m].shapes[i].ibo);
			int materialID = animals[m].shapes[i].materialID;
			glBindTexture(GL_TEXTURE_2D, animals[m].materials[materialID].diffuse_tex);
			glDrawElements(GL_TRIANGLES, animals[m].shapes[i].drawCount, GL_UNSIGNED_INT, 0);
		}
	}

	// draw sign
	for (int i = 0; i < signCount; i++) {
		mat4 mv_matrix = view_matrix * signBox[i].model_matrix;
		mat4 shadow_matrix = shadow_sbpv_matrix * animals[i].model_matrix;
		glUniformMatrix4fv(uniforms.view.shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix));
		glUniformMatrix4fv(uniforms.blinnPhong.um4v, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4mv, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4p, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(uniforms.blinnPhong.um4m, 1, GL_FALSE, value_ptr(animals[i].model_matrix));
		glBindVertexArray(signBox[i].shapes[0].vao);
		glBindTexture(GL_TEXTURE_2D, signBox[i].materials[0].diffuse_tex);
		glDrawArrays(GL_TRIANGLES, 0, signBox[i].shapes[0].drawCount);
	}

	// ----- End Begin Blinn-Phong Shading Pass -----
	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	viewportSize.width = width;
	viewportSize.height = height;
	glViewport(0, 0, width, height);
	viewportAspect = (float)width / (float)height;
	changeView();
}

void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{

	if (state == GLUT_DOWN)
	{
		//if (printOrNot) printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		mousePressOrNot = 1;
		start = vec2(x, y);
	}
	else if (state == GLUT_UP)
	{
		//if (printOrNot) printf("Mouse %d is released at (%d, %d)\n", button, x, y);
		mousePressOrNot = 0;
	}
}

void My_MouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		actualCamera.currentFov += 0.01; // Zoom in
	}
	else
	{
		actualCamera.currentFov -= 0.01; // Zoom out
	}
}

void My_PressedMotionMouse(int x, int y)
{
	xpos = x;
	ypos = y;
}

void My_MotionMouse(int x, int y)
{
	xpos = x;
	ypos = y;
}

void My_Keyboard(unsigned char key, int x, int y)
{
	//if (printOrNot) printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	if (key == 'd' || key == 'D')
	{
		actualCamera.position += rightDirection * deltaTime * speed;
	}
	else if (key == 'a' || key == 'A')
	{
		actualCamera.position -= rightDirection * deltaTime * speed;
	}
	else if (key == 'w' || key == 'W')
	{
		actualCamera.position += direction * deltaTime * speed;
	}
	else if (key == 's' || key == 'S')
	{
		actualCamera.position -= direction * deltaTime * speed;
	}
	else if (key == 'z' || key == 'Z')
	{
		actualCamera.position += up * deltaTime * speed;
	}
	else if (key == 'x' || key == 'X')
	{
		actualCamera.position -= up * deltaTime * speed;
	}
	else if (key == 'c' || key == 'C')
	{
		Camera temp = actualCamera;
		actualCamera = cameras[(nowCamera + 1) % cameraCount];
		cameras[nowCamera] = temp;
		nowCamera = (nowCamera + 1) % cameraCount;
	}
	else if (key == 27) //ESC
	{
		exit(0);
	}
}

void My_Menu(int id)
{
	switch (id)
	{
	case ANIMATION:
		if (animateStart) {
			actualCamera = cameras[nowCamera];
			animateStart = false;
		}
		else {
			pastTime = 0;
			animateStart = true;
		}
		break;
	case FOG:
		glUniform1i(uniforms.parameter.fogSwitch, (++fogSwitch)%2);
		break;
	case SHADOW:
		glUniform1i(uniforms.parameter.shadowSwitch, 0);
		break;
	case BLINNPHONG:
		glUniform1i(uniforms.parameter.shadowSwitch, 1);
		break;
	case NOEFFECT:
		glUniform1i(uniforms.parameter.shadowSwitch, 2);
		break;
	case MENU_TIMER_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("GPA_Final_Framework");
#ifdef _MSC_VER
	glewInit();
#endif
	glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddMenuEntry("Fog effect", FOG);
	glutAddSubMenu("Shadow effect", menu_timer);
	glutAddMenuEntry("Animation", ANIMATION);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Shadow + Blinn Phong", SHADOW);
	glutAddMenuEntry("BlinnPhong only", BLINNPHONG);
	glutAddMenuEntry("no effect", NOEFFECT);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(My_PressedMotionMouse);
	glutPassiveMotionFunc(My_MotionMouse);
	glutMouseWheelFunc(My_MouseWheel);
	glutKeyboardFunc(My_Keyboard);
	glutTimerFunc(timer_speed, My_Timer, 0);

	// Enter main event loop.
	glutMainLoop();

	return 0;
}

/*reference
http://axdc11239.blogspot.com/2012/03/opengl.html

視角移動
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
與其原始碼
https://github.com/huamulan/OpenGL-tutorial/blob/master/common/controls.cpp

關於滑鼠位置、滑鼠位置控制
https://www.opengl.org/discussion_boards/showthread.php/137133-get-windows-size-in-opengl
https://www.opengl.org/discussion_boards/showthread.php/139067-Get-mouse-cursor-with-GLUT
https://www.opengl.org/discussion_boards/showthread.php/161808-Set-Mouse-Position-with-GLUT
滑鼠滾輪func
http://stackoverflow.com/questions/14378/using-the-mouse-scrollwheel-in-glut
ESC代號
https://www.opengl.org/discussion_boards/showthread.php/169746-How-to-quit-opengl-by-a-key

glfw用法
http://www.cnblogs.com/graphics/p/3661500.html
http://www.glfw.org/docs/latest/group__window.html

VS解惑
http://zalin.logdown.com/posts/166409-note-visual-studio-unresolved-externals-error-solution

SKYBOX
http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=13

assimp tutorial
http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

exe執行所需檔案:
.glsl .dll .obj .png
拿到另一台電腦執行需要以下檔案
msvcp140.dll
msvcp140d.dll
ucrtbased.dll
vcruntime140.dll
vcruntime140d.dll
位於C:\WINDOWS\SysWOW64

以上是上次的reference
以下是新reference

方向鍵key代號
http://blog.csdn.net/rocky_shared_image/article/details/7726566

查詢gl相關function
http://docs.gl/
*/