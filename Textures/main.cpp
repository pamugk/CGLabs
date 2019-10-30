#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "MVPMatrix.h"
#include "FreeImage.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

GLFWwindow *g_window;

#pragma region Хэндлеры
	GLuint g_shaderProgram; //Программа
	GLint g_uMVP; //MVP
	GLint g_n; //Матрица нормалей
	GLint g_mph;//Высота пиков модели
	GLint g_mc;//Вогнутость фрагментов модели (concavity)
#pragma endregion
#pragma region Шейдеры
	const GLchar vsh[] =
		"#version 330\n"
		""
		"layout(location = 0) in vec2 a_position;"
		"layout(location = 1) in vec2 a_texCoord;"
		"layout(location = 2) in vec3 a_color;"
		""
		"uniform mat4 u_mvp;"
		"uniform mat3 u_n;"
		"uniform float u_mph;"
		"uniform float u_mc;"
		""
		"out vec2 v_texCoord;"
		"out vec3 v_color;"
		"out vec3 v_normal;"
		"out vec3 v_pos;"
		""
		"void main()"
		"{"
		"    float x = a_position[0];"
		"    float z = a_position[1];"
		"    float y = u_mph * sin((x * x + z * z) * 3.14 * u_mc);"
		"    v_texCoord = a_texCoord;"
		"    v_color = a_color;"
		"    v_pos = vec3(x, y, z);"
		"    v_normal = normalize(u_n * vec3(-u_mph * 2.0 * x * u_mc * 3.14 * cos((x * x + z * z) * 3.14 * u_mc), 1.0, -u_mph * 2.0 * z * u_mc * 3.14 * cos((x * x + z * z) * 3.14 * u_mc)));"
		"    gl_Position = u_mvp * vec4(v_pos, 1.0);"
		"}"
		;
	const GLchar fsh[] =
		"#version 330\n"
		""
		"uniform vec3 u_olpos;"
		"uniform vec3 u_olcol;"
		"uniform vec3 u_oeye;"
		"uniform float u_odmin;"
		"uniform float u_osfoc;"
		""
		"uniform sampler2D u_map1;"
		"uniform sampler2D u_map2;"
		""
		"uniform bool u_lie;"
		""
		"in vec2 v_texCoord;"
		"in vec3 v_color;"
		"in vec3 v_normal;"
		"in vec3 v_pos;"
		""
		"layout(location = 0) out vec4 o_color;"
		""
		"void main()"
		"{"
		"   vec3 l = normalize(v_pos - u_olpos);"
		"   float cosa = dot(l, v_normal);"
		"   float d = max(cosa, u_odmin);"
		"   vec3 r = reflect(l, v_normal);"
		"   vec3 e = normalize(u_oeye - v_pos);"
		"   float s = max(pow(dot(r, e), u_osfoc), 0.0) * (int(cosa >= 0.0));"
		"   vec4 texColor = mix(texture(u_map1, v_texCoord), texture(u_map2, v_texCoord), 0.5);"
		"   o_color = int(u_lie) * vec4(u_olcol * (d * texColor.xyz + s), 1.0) + (int(!u_lie)) * texColor;"
		"}"
		;
	//l-нормализованный вектор падения света
	//d-коэффициент диффузного освещения
	//r-вектор отраженного луча света
	//e-нормализованный вектор обзора
	//s-коэффициент зеркального блика
#pragma endregion
#pragma region Модель
	const int N = 100;
	const int verCoordCount = 2;
	const int texCoordCount = 2;
	const int colorCount = 3;
	const int vertexSize = verCoordCount + texCoordCount + colorCount;
	const int vertexArrayLength = (N + 1) * (N + 1) * vertexSize;
	const int indexArrayLength = N * N * 2 * 3;

	GLfloat vertices[vertexArrayLength];
	GLuint indices[indexArrayLength];

	MVPMatrix m;
	MVPMatrix v;

	struct Model
	{
		GLuint vbo;
		GLuint ibo;
		GLuint vao;
		GLsizei indexCount;
		GLfloat peaksHeight = 1.0f;
		GLfloat concavity = 10.0f;
	} model;
#pragma endregion
#pragma region Освещение
	struct
	{
		GLint g_olpos; //Позиция источника света
		GLint g_olcol; //Цвет источника света
		GLint g_oeye; //Позиция наблюдателя
		GLint g_odmin;//dmin в модели Фонга
		GLint g_osfoc;//dmin в модели Фонга
		GLfloat position[3] = { N, N, N };
		GLfloat color[3] = { 1.0f, 1.0f, 1.0f };
		GLfloat eye[3] = { 0.f, 0.f, 0.f };
		GLfloat dmin = 0.5f;
		GLfloat sfoc = 4.0f;
	} g_optics;

	GLint g_ulie;
	bool u_lie = true;
#pragma endregion
#pragma region Текстуры
	struct Texture
	{
		GLuint ID;
		GLint mapLocation;
		const GLchar* mapName;
		const GLchar* file;
		int width;
		int height;

		Texture(GLchar* mapName, GLchar* file)
		{
			ID = 0;
			mapLocation = 0;
			this->mapName = mapName;
			this->file = file;
			width = 0;
			height = 0;
		}
	};

	const int countOfTextures = 2;
	GLuint texIDs[countOfTextures];
	Texture textures[countOfTextures] =
	{
		Texture("u_map1", "metal.jpg"),
		Texture("u_map2", "smoke.jpg")
	};
#pragma endregion
#pragma region Формирование модели
	void fillVertices()
	{
		int i = 0;
		for (int z = 0; z <= N; z++)
			for (int x = 0; x <= N; x++)
			{
				vertices[i++] = GLfloat(x);
				vertices[i++] = GLfloat(z);

				vertices[i++] = GLfloat(1.f * x / N);
				vertices[i++] = GLfloat(1.f * z / N); 
				
				vertices[i++] = 0.25f;
				vertices[i++] = 0.5f;
				vertices[i++] = 0.25f;
			}
	}

	void fillIndices()
	{
		int i = 0;
		for (int z = 0; z < N; z++)
		{
			int nextZ = z + 1;
			int zN = z * N;
			int nzN = nextZ * (N + 1);
			for (int x = 0; x < N; x++)
			{
				indices[i++] = zN + x + z;
				indices[i++] = zN + x + nextZ;
				indices[i++] = nzN + x;
				indices[i++] = nzN + x;
				indices[i++] = nzN + x + 1;
				indices[i++] = zN + x + nextZ;
			}
		}
	}
#pragma endregion
#pragma region Работа с текстурами
	GLvoid* loadTextureFromFile(const char* fileName, int* texW, int* texH)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(fileName, 0);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(fileName);
		if (fif == FIF_UNKNOWN)
			return nullptr;

		FIBITMAP *dib = nullptr;
		if (FreeImage_FIFSupportsReading(fif))
			dib = FreeImage_Load(fif, fileName);
		if (dib == nullptr)
			return nullptr;

		BYTE* image = FreeImage_GetBits(dib);
		*texW = FreeImage_GetWidth(dib);
		*texH = FreeImage_GetHeight(dib);
		FreeImage_Unload(dib);
		return image;
	}

	bool initializeTextures()
	{
		for (int i = 0; i < countOfTextures; i++)
		{
			FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(textures[i].file, 0);
			if (fif == FIF_UNKNOWN)
				fif = FreeImage_GetFIFFromFilename(textures[i].file);
			if (fif == FIF_UNKNOWN)
				return nullptr;

			FIBITMAP *dib = nullptr;
			if (FreeImage_FIFSupportsReading(fif))
				dib = FreeImage_Load(fif, textures[i].file);
			if (dib == nullptr)
				return nullptr;

			BYTE* image = FreeImage_GetBits(dib);
			textures[i].width = FreeImage_GetWidth(dib);
			textures[i].height = FreeImage_GetHeight(dib);

			if (!image)
				return false;

			glGenTextures(1, texIDs + i);
			textures[i].ID = texIDs[i];

			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i].ID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textures[i].width, textures[i].height, 0, GL_BGR, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			
			textures[i].mapLocation = glGetUniformLocation(g_shaderProgram, textures[i].mapName);

			FreeImage_Unload(dib);
		}
		return true;
	}

#pragma endregion
#pragma region Технические дела с OpenGL
	GLuint createShader(const GLchar *code, GLenum type)
	{
		GLuint result = glCreateShader(type);
		glShaderSource(result, 1, &code, NULL);
		glCompileShader(result);
		GLint compiled;
		glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLint infoLen = 0;
			glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen > 0)
			{
				char *infoLog = (char *)alloca(infoLen);
				glGetProgramInfoLog(result, infoLen, NULL, infoLog);
				cout << "Shader compilation error" << endl << infoLog << endl;
			}
			glDeleteShader(result);
			return 0;
		}
		return result;
	}

	GLuint createProgram(GLuint vsh, GLuint fsh)
	{
		GLuint result = glCreateProgram();
		glAttachShader(result, vsh);
		glAttachShader(result, fsh);
		glLinkProgram(result);
		GLint linked;
		glGetProgramiv(result, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLint infoLen = 0;
			glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen > 0)
			{
				char *infoLog = (char *)alloca(infoLen);
				glGetProgramInfoLog(result, infoLen, NULL, infoLog);
				cout << "Shader program linking error" << endl << infoLog << endl;
			}
			glDeleteProgram(result);
			return 0;
		}
		return result;
	}

	bool createShaderProgram()
	{
		g_shaderProgram = 0;
		GLuint vertexShader, fragmentShader;
		vertexShader = createShader(vsh, GL_VERTEX_SHADER);
		fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);
		g_shaderProgram = createProgram(vertexShader, fragmentShader);

		if (g_shaderProgram == 0)
			return false;

		g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");
		g_n = glGetUniformLocation(g_shaderProgram, "u_n");

		g_optics.g_olpos = glGetUniformLocation(g_shaderProgram, "u_olpos");
		g_optics.g_olcol = glGetUniformLocation(g_shaderProgram, "u_olcol");
		g_optics.g_oeye = glGetUniformLocation(g_shaderProgram, "u_oeye");
		g_optics.g_odmin = glGetUniformLocation(g_shaderProgram, "u_odmin");
		g_optics.g_osfoc = glGetUniformLocation(g_shaderProgram, "u_osfoc");
		g_ulie = glGetUniformLocation(g_shaderProgram, "u_lie");

		g_mph = glGetUniformLocation(g_shaderProgram, "u_mph");
		g_mc = glGetUniformLocation(g_shaderProgram, "u_mc");

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		if (!initializeTextures())
		{
			cout << "Failed to load textures" << endl;
			return false;
		}

		return true;
	}

	bool createModel()
	{
		m = MVPMatrix::getIdentityMatrix().rotateAboutX(45);
		v = MVPMatrix::getIdentityMatrix().move(0.f, 0.f, -3.f * N);

		glGenVertexArrays(1, &model.vao);
		glBindVertexArray(model.vao);
		glGenBuffers(1, &model.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
		fillVertices();
		glBufferData(GL_ARRAY_BUFFER, vertexArrayLength * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
		glGenBuffers(1, &model.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);
		fillIndices();
		model.indexCount = indexArrayLength;

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, verCoordCount, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (const GLvoid *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, texCoordCount, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (const GLvoid *)(verCoordCount * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, colorCount, GL_FLOAT, GL_FALSE, vertexSize * sizeof(GLfloat), (const GLvoid *)((verCoordCount + texCoordCount) * sizeof(GLfloat)));
		return model.vbo != 0 && model.ibo != 0 && model.vao != 0;
	}

	bool init()
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		return createShaderProgram() && createModel();
	}

	void reshape(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(g_shaderProgram);
		glBindVertexArray(g_model.vao);
		MVPMatrix mv = v * m;
		int w, h = 0;
		glfwGetWindowSize(g_window, &w, &h);
		MVPMatrix p = MVPMatrix::getPerspectiveProjectionMatrix(N, -N, w, h, 45.0f);
		glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, (p * mv).getContent());

		float * nMatrix = mv.getNMatrix();
		glUniformMatrix3fv(g_n, 1, GL_TRUE, nMatrix);
		delete[] nMatrix;

		glUniform3fv(g_optics.g_olpos, 1, g_optics.position);
		glUniform3fv(g_optics.g_olcol, 1, g_optics.color);
		glUniform3fv(g_optics.g_oeye, 1, g_optics.eye);
		glUniform1f(g_optics.g_odmin, g_optics.dmin);
		glUniform1f(g_optics.g_osfoc, g_optics.sfoc);
		glUniform1i(g_ulie, u_lie);

		glUniform1f(g_mph, g_model.peaksHeight);
		glUniform1f(g_mc, g_model.concavity);

		for (int i = 0; i < countOfTextures; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, texIDs[i]);
			glUniform1i(textures[i].mapLocation, i);
		}

		glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
	}

	void cleanup()
	{
		if (g_shaderProgram != 0)
			glDeleteProgram(g_shaderProgram);
		if (g_model.vbo != 0)
			glDeleteBuffers(1, &g_model.vbo);
		if (g_model.ibo != 0)
			glDeleteBuffers(1, &g_model.ibo);
		if (g_model.vao != 0)
			glDeleteVertexArrays(1, &g_model.vao);
		glDeleteTextures(countOfTextures, texIDs);
	}

	void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
			u_lie = !u_lie;
	}

	bool initOpenGL()
	{
		if (!glfwInit())
		{
			cout << "Failed to initialize GLFW" << endl;
			return false;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		g_window = glfwCreateWindow(1024, 768, "OpenGL Test", NULL, NULL);
		if (g_window == NULL)
		{
			cout << "Failed to open GLFW window" << endl;
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(g_window);
		glewExperimental = true;
		if (glewInit() != GLEW_OK)
		{
			cout << "Failed to initialize GLEW" << endl;
			return false;
		}
		glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);
		glfwSetFramebufferSizeCallback(g_window, reshape);
		glfwSetKeyCallback(g_window, onKeyPress);
		return true;
	}

	void tearDownOpenGL()
	{
		glfwTerminate();
	}
#pragma endregion
#pragma region Интерактив
	const int countOfSpeeds = 9;
	float degrees[countOfSpeeds];
	int degreeKeys[countOfSpeeds];
	float degree;

	const float lightMovementBorder = 3.f;
	const float lightStep = 0.05f;

	void checkInput()
	{
		for (int i = 0; i < countOfSpeeds; i++)
			if (glfwGetKey(g_window, degreeKeys[i]) == GLFW_PRESS)
				degree = degrees[i];

		if (glfwGetKey(g_window, GLFW_KEY_LEFT) == GLFW_PRESS)
			m = m.rotateAboutY(degree);
		if (glfwGetKey(g_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			m = m.rotateAboutY(-degree);
		if (glfwGetKey(g_window, GLFW_KEY_UP) == GLFW_PRESS)
			m = m.rotateAboutX(degree);
		if (glfwGetKey(g_window, GLFW_KEY_DOWN) == GLFW_PRESS)
			m = m.rotateAboutX(-degree);
		if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS)
			m = m.rotateAboutZ(degree);
		if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS)
			m = m.rotateAboutZ(-degree); 
		
		if (glfwGetKey(g_window, GLFW_KEY_KP_4) == GLFW_PRESS)
			g_optics.position[0] -= lightStep;
		if (glfwGetKey(g_window, GLFW_KEY_KP_6) == GLFW_PRESS)
			g_optics.position[0] += lightStep;
		if (glfwGetKey(g_window, GLFW_KEY_KP_5) == GLFW_PRESS)
			g_optics.position[1] -= lightStep;
		if (glfwGetKey(g_window, GLFW_KEY_KP_8) == GLFW_PRESS)
			g_optics.position[1] += lightStep;
		if (glfwGetKey(g_window, GLFW_KEY_KP_7) == GLFW_PRESS)
			g_optics.position[2] -= lightStep;
		if (glfwGetKey(g_window, GLFW_KEY_KP_9) == GLFW_PRESS)
			g_optics.position[2] += lightStep;

		if (glfwGetKey(g_window, GLFW_KEY_MINUS) == GLFW_PRESS && model.peaksHeight > 1.f)
			model.peaksHeight -= 0.01f;
		if (glfwGetKey(g_window, GLFW_KEY_EQUAL) == GLFW_PRESS)
			model.peaksHeight += 0.01f;
		if (glfwGetKey(g_window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS && model.concavity > 1.f)
			model.concavity -= 0.01f;
		if (glfwGetKey(g_window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
			model.concavity += 0.01f;
	}
#pragma endregion
int main()
{
	if (!initOpenGL())
		return -1;
	bool isOk = init();
	if (isOk)
	{
		for (int i = 0; i < countOfSpeeds; i++)
		{
			degrees[i] = (i + 1) * 0.05f;
			degreeKeys[i] = GLFW_KEY_1 + i;
		}
		degree = degrees[1];
		cout << "Controls:\n\tLeft/Right Arrows: rotate about Y axis;\n\tUp/Down Arrows: rotate about X axis;\n\tW/S Keys: rotate about Z axis;\n";
		cout << "\t1-9: speed selection.\n";
		cout << "\t4/6 (Keypad): move light along X axis;\n\t5/8 (Keypad): move light along Y axis;\n\t7/9 (Keypad): move light along Z axis;\n";
		cout << "\t-/+: change model's peaks height;\n\t-/+(Keypad): change model's fragments concavity;\n";
		cout << "Q: disable/enable lightning.\n";
		cout << "Enjoy!";
		while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
		{
			checkInput();
			draw();
			glfwSwapBuffers(g_window);
			glfwPollEvents();
		}
	}
	cleanup();
	tearDownOpenGL();
	return isOk ? 0 : -1;
}