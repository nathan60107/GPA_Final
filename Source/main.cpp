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

void cameraPositionChecker()
{
	if (camera_position.x > 50) camera_position.x = 50.0f;
	if (camera_position.x < -50) camera_position.x = -50.0f;
	if (camera_position.y > 50) camera_position.y = 50.0f;
	if (camera_position.y < -50) camera_position.y = -50.0f;
	if (camera_position.z > 50) camera_position.z = 50.0f;
	if (camera_position.z < -50) camera_position.z = -50.0f;
}

void changeView()
{
	direction = vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle));
	rightDirection = vec3(sin(horizontalAngle - 3.14f / 2.0f), 0, cos(horizontalAngle - 3.14f / 2.0f));
	up = cross(rightDirection, direction);
	view_matrix = lookAt(camera_position, camera_position + direction, up);
	proj_matrix = perspective(initialFoV - 5 * currentFov, viewportAspect, 0.1f, 200.0f);
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

void loadSence(char* objPathInput, char* textuerPathInput, unsigned int senceIndexInput, vec3 center, vec3 scale)
{
	printf("----------------------------\nStart to load sence %d.\n", senceIndexInput);

	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	const aiScene* scene = aiImportFile(objPathInput, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene == NULL) {
		printf("Fail to load scene and print ERROR:");
		printf(aiGetErrorString());
		printf("\n");
		return;
	}
	models[senceIndexInput].materials.resize(scene->mNumMaterials);
	models[senceIndexInput].shapes.resize(scene->mNumMeshes);

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
			glGenTextures(1, &models[senceIndexInput].materials[i].diffuse_tex);
			glBindTexture(GL_TEXTURE_2D, models[senceIndexInput].materials[i].diffuse_tex);
			pngPath.append(textuerPathInput);
			pngPath.append(texturePath.C_Str());
			textureData = loadPNG(pngPath.c_str());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureData.width, textureData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//X軸Y軸的處理方式
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glGenerateMipmap(GL_TEXTURE_2D);
			if (printOrNot) printf("Successly saving material %d. Texture name = %s.\n", i, texturePath.C_Str());
		}
		else
		{
			// load some default image as default_diffuse_tex
			//material.diffuse_tex = default_diffuse_tex;
			printf("Fail to save material %d. Texture name = %s.\n", i, texturePath.C_Str());
		}
		materialsCount++;
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
	models[senceIndexInput].center = c;
	
	// load geometry
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh*mesh = scene->mMeshes[i];
		float temp[4][50000];
		int tempInt[50000];
		
		glGenVertexArrays(1, &models[senceIndexInput].shapes[i].vao);
		glBindVertexArray(models[senceIndexInput].shapes[i].vao);
		
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

			glGenBuffers(1, &models[senceIndexInput].shapes[i].vbo_position);
			glBindBuffer(GL_ARRAY_BUFFER, models[senceIndexInput].shapes[i].vbo_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, temp[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &models[senceIndexInput].shapes[i].vbo_normal);
			glBindBuffer(GL_ARRAY_BUFFER, models[senceIndexInput].shapes[i].vbo_normal);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, temp[1], GL_STATIC_DRAW);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);

			glGenBuffers(1, &models[senceIndexInput].shapes[i].vbo_texcoord);
			glBindBuffer(GL_ARRAY_BUFFER, models[senceIndexInput].shapes[i].vbo_texcoord);
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

			glGenBuffers(1, &models[senceIndexInput].shapes[i].ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, models[senceIndexInput].shapes[i].ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * mesh->mNumFaces, tempInt, GL_STATIC_DRAW);
			
			// save shape info
			models[senceIndexInput].shapes[i].materialID = mesh->mMaterialIndex;
			models[senceIndexInput].shapes[i].drawCount = mesh->mNumFaces * 3;
			if (printOrNot) printf("Successly saving shapes %d.\n", i);
		}
	}

	shapeIndexCount++;
	printf("Loading sence %d completes.\n----------------------------\n", senceIndexInput);
	aiReleaseImport(scene);
}

void My_Init()
{
	glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

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
	uniforms_shadow.light.mvp = glGetUniformLocation(depthProg, "mvp");
	// ----- End Initialize Depth Shader Program -----



	// create shader
	program = createProgram("vertex.vs.glsl", "fragment.fs.glsl");
	glUseProgram(program);

	// get uniform location
	um4v = glGetUniformLocation(program, "um4v");
	um4mv = glGetUniformLocation(program, "um4mv");
	um4p = glGetUniformLocation(program, "um4p");
	us2dtex = glGetUniformLocation(program, "tex");
	uniforms_shadow.view.shadow_matrix = glGetUniformLocation(program, "shadow_matrix");
	uniforms_shadow.view.shadow_tex = glGetUniformLocation(program, "shadow_tex");
	
	// load sky box
	string dir = "../TexturedScene/skyboxes/jajlands1/";
	string front = "jajlands1_ft.jpg";
	string back = "jajlands1_bk.jpg";
	string left = "jajlands1_lf.jpg";
	string right = "jajlands1_rt.jpg";
	string top = "jajlands1_up.jpg";
	string bottom = "jajlands1_dn.jpg";
	skybox.loadSkybox(dir, front, back, left, right, top, bottom);

	// load scene model

	//loadSence("../TexturedScene/scene/house 2MS/house2.obj", "../TexturedScene/scene/house 2MS/", shapeIndexCount, vec3(0, 0, 0), vec3(1));
	//models[shapeIndexCount - 1].model_matrix = rotate(mat4(), float(deg2rad(90.0f), vec3(0.0f, 1.0f, 0.0f));
	//loadSence("../TexturedScene/scene/Wall/wall.obj", "../TexturedScene/scene/Wall/", shapeIndexCount, vec3(5, -2, 38.5), vec3(0.57, 0.5, 0.5));
	//loadSence("../TexturedScene/scene/Wall/wall.obj", "../TexturedScene/scene/Wall/", shapeIndexCount, vec3(11.75, -2, 38.5), vec3(0.57, 0.5, 0.5));
	//loadSence("../TexturedScene/scene/Wall/wall.obj", "../TexturedScene/scene/Wall/", shapeIndexCount, vec3(18.5, -2, 38.5), vec3(0.57, 0.5, 0.5));
	//loadSence("../TexturedScene/scene/Wall/wall.obj", "../TexturedScene/scene/Wall/", shapeIndexCount, vec3(25.25, -2, 38.5), vec3(0.57, 0.5, 0.5));
	//loadSence("../TexturedScene/scene/Wall/wall.obj", "../TexturedScene/scene/Wall/", shapeIndexCount, vec3(32, -2, 38.5), vec3(0.57, 0.5, 0.5));

	//loadSence("../TexturedScene/Farmhouse Maya/farmhouse_obj.obj", "../TexturedScene/Farmhouse Maya/", 0, vec3(0, 0, 0), 1);
	//loadSence("../TexturedScene/Old_Warehouse/OBJ/Warehouse.obj", "../TexturedScene/Old_Warehouse/", 1);
	//loadSence("../TexturedScene/dabrovic-sponza/sponza.obj", "../TexturedScene/dabrovic-sponza/", 2);
	//loadSence("../TexturedScene/horse/horse.obj", "../TexturedScene/horse/", 0, vec3(0,0,0), 0.1);
	//loadSence("../TexturedScene/Tiger/Tiger.obj", "../TexturedScene/Tiger/", 1);
	
	loadSence("../TexturedScene/chimp/chimp.obj", "../TexturedScene/chimp/", shapeIndexCount, vec3(0, 0, 0), vec3(10));
	/*loadSence("../TexturedScene/Cat2/cat.obj", "../TexturedScene/Cat2/", shapeIndexCount, vec3(0, 0, 0), 0.01);
	loadSence("../TexturedScene/Horse2/Horse.obj", "../TexturedScene/Horse2/", shapeIndexCount, vec3(0, 0, 0), 0.01);
	loadSence("../TexturedScene/The_Dog/The_Dog.obj", "../TexturedScene/The_Dog/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/pig/pig.obj", "../TexturedScene/pig/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/goat/goat.obj", "../TexturedScene/goat/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/horse/LD_HorseRtime02.obj", "../TexturedScene/horse/", shapeIndexCount, vec3(2, 2, 2), 1);
	loadSence("../TexturedScene/Cat/Cat.obj", "../TexturedScene/Cat/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Giraffe/Giraffe.OBJ", "../TexturedScene/Giraffe/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Gorilla/Gorilla.obj", "../TexturedScene/Gorilla/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Great_White_A/GreatWhite.obj", "../TexturedScene/Great_White_A/", shapeIndexCount, vec3(0, 0, 0), 0.01);
	loadSence("../TexturedScene/Sand_Tiger/SandTiger.obj", "../TexturedScene/Sand_Tiger/", shapeIndexCount, vec3(0, 0, 0), 0.01);
	loadSence("../TexturedScene/Killer_Whale/Killer_Whale.obj", "../TexturedScene/Killer_Whale/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Wolf/Wolf.obj", "../TexturedScene/Wolf/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Bear_Brown/Bear_Brown.obj", "../TexturedScene/Bear_Brown/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Chickdee/CHICKDEE.3DS", "../TexturedScene/Chickdee/", shapeIndexCount, vec3(0, 0, 0), 10);
	loadSence("../TexturedScene/1pui1qkawg-Alsatian (Dog)/3ds file.3DS", "../TexturedScene/1pui1qkawg-Alsatian (Dog)/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/aqfj72cgmv-Sheep/3ds file.3DS", "../TexturedScene/aqfj72cgmv-Sheep/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/black bear/BEAR_BLK.3DS", "../TexturedScene/black bear/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Crow/CROW.3DS", "../TexturedScene/Crow/", shapeIndexCount, vec3(0, 0, 0), 3);
	loadSence("../TexturedScene/Duck/DUCK.3DS", "../TexturedScene/Duck/", shapeIndexCount, vec3(0, 0, 0), 5);
	loadSence("../TexturedScene/eagle 3/EAGLE_3.3DS", "../TexturedScene/eagle 3/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Flacon/FALCON_2.3DS", "../TexturedScene/Flacon/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/frog/FROG.3DS", "../TexturedScene/frog/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/Goldfish/GOLDFISH.3DS", "../TexturedScene/Goldfish/", shapeIndexCount, vec3(0, 0, 0), 10);
	loadSence("../TexturedScene/LM bas/LM_BASS.3DS", "../TexturedScene/LM bas/", shapeIndexCount, vec3(0, 0, 0), 3);
	loadSence("../TexturedScene/MONARCH/MONARCH.3DS", "../TexturedScene/MONARCH/", shapeIndexCount, vec3(0, 0, 0), 3);
	loadSence("../TexturedScene/orca/ORCA.3DS", "../TexturedScene/orca/", shapeIndexCount, vec3(0, 0, 0), 1);
	loadSence("../TexturedScene/turtoise/TORTOISE.3DS", "../TexturedScene/turtoise/", shapeIndexCount, vec3(0, 0, 0), 1);
	*/
	//以下待確認
	//loadSence("../TexturedScene/Wolf Rigged and Game Ready/Wolf_3ds.3ds", "../TexturedScene/Wolf Rigged and Game Ready/", shapeIndexCount, vec3(0, 0, 0), 1);


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
	mat4 light_proj_matrix = frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
	mat4 light_view_matrix = lookAt(vec3(20.0f, 20.0f, 20.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	glUseProgram(depthProg);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	
	for (int m = 0; m < shapeIndexCount; ++m) {

		// transmit uniform variable
		mat4 mv_matrix = view_matrix * models[m].model_matrix;
		glUniformMatrix4fv(uniforms_shadow.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix *models[m].model_matrix));
		glActiveTexture(GL_TEXTURE0);

		// draw
		for (int i = 0; i < models[m].shapes.size(); ++i)
		{
			glBindVertexArray(models[m].shapes[i].vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, models[m].shapes[i].ibo);
			glDrawElements(GL_TRIANGLES, models[m].shapes[i].drawCount, GL_UNSIGNED_INT, 0);
		}
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
	// ----- End Shadow Map Pass -----

	// ----- Begin Blinn-Phong Shading Pass -----
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, viewportSize.width, viewportSize.height);
	glUseProgram(program);
	

	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(proj_matrix));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glUniform1i(uniforms_shadow.view.shadow_tex, 1);
	
	// draw sky box
	skybox.renderSkybox();
	glClear(GL_DEPTH_BUFFER_BIT);

	// calculate elapsed time
	static double lastTime = glutGet(GLUT_ELAPSED_TIME);
	double currentTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = float(currentTime - lastTime);
	pastTime += deltaTime;

	// reset mouse position and calculate angle
	if (mousePressOrNot) {
		glutWarpPointer(start.x, start.y);
		horizontalAngle += mouseSpeed * float(start.x - xpos);
		verticalAngle += mouseSpeed * float(start.y - ypos);
	}

	// change view and record time
	/*if (pastTime > 80) {
		camera_position = curve[index];
		index = (index + 1) % 100;
		pastTime = 0;
	}*/
	changeView();
	lastTime = currentTime;

	// draw models
	for (int m = 0; m < shapeIndexCount; ++m) {
		
		// transmit uniform variable
		mat4 mv_matrix = view_matrix * models[m].model_matrix;
		mat4 shadow_matrix = shadow_sbpv_matrix * models[m].model_matrix;
		glUniformMatrix4fv(uniforms_shadow.view.shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix));
		glUniformMatrix4fv(um4v, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(um4mv, 1, GL_FALSE, &mv_matrix[0][0]);
		glUniformMatrix4fv(um4p, 1, GL_FALSE, &proj_matrix[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(us2dtex, 0);
		
		// draw
		for (int i = 0; i < models[m].shapes.size(); ++i)
		{
			glBindVertexArray(models[m].shapes[i].vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, models[m].shapes[i].ibo);
			int materialID = models[m].shapes[i].materialID;
			glBindTexture(GL_TEXTURE_2D, models[m].materials[materialID].diffuse_tex);
			glDrawElements(GL_TRIANGLES, models[m].shapes[i].drawCount, GL_UNSIGNED_INT, 0);
		}
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
		if (printOrNot) printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		mousePressOrNot = 1;
		start = vec2(x, y);
	}
	else if (state == GLUT_UP)
	{
		if (printOrNot) printf("Mouse %d is released at (%d, %d)\n", button, x, y);
		mousePressOrNot = 0;
	}
}

void My_MouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		currentFov += 0.01; // Zoom in
	}
	else
	{
		currentFov -= 0.01; // Zoom out
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
	if (printOrNot) printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	if (key == 'd' || key == 'D')
	{
		camera_position += rightDirection * deltaTime * speed;
		cameraPositionChecker();
	}
	else if (key == 'a' || key == 'A')
	{
		camera_position -= rightDirection * deltaTime * speed;
		cameraPositionChecker();
	}
	else if (key == 'w' || key == 'W')
	{
		camera_position += direction * deltaTime * speed;
		cameraPositionChecker();
	}
	else if (key == 's' || key == 'S')
	{
		camera_position -= direction * deltaTime * speed;
		cameraPositionChecker();
	}
	else if (key == 'z' || key == 'Z')
	{
		camera_position += up * deltaTime * speed;
		cameraPositionChecker();
	}
	else if (key == 'x' || key == 'X')
	{
		camera_position -= up * deltaTime * speed;
		cameraPositionChecker();
	}
	else if (key == 'c' || key == 'C')
	{
		shapeIndex = (shapeIndex + 1) % shapeIndexCount;
	}
	else if (key == 27) //ESC
	{
		exit(0);
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		
		break;
	case GLUT_KEY_RIGHT:
		
		break;
	case GLUT_KEY_UP:

		break;
	case GLUT_KEY_DOWN:

		break;
	default:
		if (printOrNot) printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch (id)
	{
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
	glutCreateWindow("AS2_Framework");
#ifdef _MSC_VER
	glewInit();
#endif
	glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	/*int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);*/

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(My_PressedMotionMouse);
	glutPassiveMotionFunc(My_MotionMouse);
	glutMouseWheelFunc(My_MouseWheel);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
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