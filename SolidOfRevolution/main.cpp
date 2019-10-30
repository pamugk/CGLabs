#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "MVPMatrix.h"
#include "Model.h"
#include "LED.h"

using namespace std;

GLFWwindow* g_window;
int windowWidth;
int windowHeight;

enum States {
	BasePoints,
	Curve,
	SolidOfRevolution
};

States state;

Model g_model;
vector<GLfloat> vertices;
vector<GLuint> indices;
GLuint g_shaderProgram; //Программа

GLint g_uMVP; //MVP
MVPMatrix v;
GLint g_n; //Матрица нормалей

#pragma region Шейдеры
const GLchar vsh[] =
"#version 330\n"
""
"layout(location = 0) in vec3 a_position;"
"layout(location = 1) in vec3 a_color;"
""
"uniform mat4 u_mvp;"
"uniform mat3 u_n;"
"uniform float u_mph;"
"uniform float u_mc;"
""
"out vec3 v_color;"
"out vec3 v_pos;"
"out vec3 v_normal;"
""
"void main()"
"{"
"    v_color = a_color;"
"    v_pos = v_pos;"
"    v_normal = vec3(0.0,0.0,0.0);"
"    gl_Position = u_mvp * vec4(a_position, 1.0);"
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
"uniform bool u_lie;"
""
"in vec3 v_color;"
"in vec3 v_pos;"
"in vec3 v_normal;"
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
//"   o_color = int(u_lie) * vec4(u_olcol * (d * v_color + s), 1.0) + int(!u_lie) * vec4(v_color, 1.0);"
"   o_color = vec4(v_color, 1.0);"
"}"
;
//l-нормализованный вектор падения света
//d-коэффициент диффузного освещения
//r-вектор отраженного луча света
//e-нормализованный вектор обзора
//s-коэффициент зеркального блика
#pragma endregion
#pragma region Освещение
LED led;

struct Optics
{
	GLint g_oeye; //Позиция наблюдателя
	GLint g_odmin;//dmin в модели Фонга
	GLint g_osfoc;//dmin в модели Фонга
	GLfloat eye[3] = { 0.f, 0.f, 0.f };
	GLfloat dmin = 0.5f;
	GLfloat sfoc = 4.0f;
} g_optics;

#pragma endregion
#pragma region Технические дела с OpenGL
GLuint createShader(const GLchar * code, GLenum type)
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
			char* infoLog = (char*)_malloca(infoLen);
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
			char* infoLog = (char*)_malloca(infoLen);
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

	led.positionHandler = glGetUniformLocation(g_shaderProgram, "u_olpos");
	led.colorHandler = glGetUniformLocation(g_shaderProgram, "u_olcol");
	g_optics.g_oeye = glGetUniformLocation(g_shaderProgram, "u_oeye");
	g_optics.g_odmin = glGetUniformLocation(g_shaderProgram, "u_odmin");
	g_optics.g_osfoc = glGetUniformLocation(g_shaderProgram, "u_osfoc");
	led.stateHandler = glGetUniformLocation(g_shaderProgram, "u_lie");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

bool createModel()
{
	glGenVertexArrays(1, &g_model.vao);
	glBindVertexArray(g_model.vao);
	glGenBuffers(1, &g_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
	glGenBuffers(1, &g_model.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

template <typename T>
T* convert(vector<T> vec)
{
	T* arr = new T[vec.size()];
	for (size_t i = 0; i < vec.size(); i++)
		arr[i] = vec[i];
	return arr;
}

void updateModel()
{
	switch (state)
	{
		case BasePoints:
		{
			vertices = g_model.baseVertices;
			indices = g_model.baseIndices;
			break;
		}
		case Curve:
		{
			vertices = g_model.curveVertices;
			indices = g_model.curveIndices;
			break;
		}
		case SolidOfRevolution:
		{
			vertices = g_model.modelVertices;
			indices = g_model.modelIndices;
			break;
		}
	}

	glBindVertexArray(g_model.vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), convert(vertices), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), convert(indices), GL_STATIC_DRAW);

	glVertexAttribPointer(0, g_model.verCoordCount, GL_FLOAT, GL_FALSE, g_model.vertexSize * sizeof(GLfloat), (const GLvoid*)0);
	glVertexAttribPointer(1, g_model.colorCount, GL_FLOAT, GL_FALSE, g_model.vertexSize * sizeof(GLfloat), (const GLvoid*)(g_model.verCoordCount * sizeof(GLfloat)));
}

bool init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	return createShaderProgram() && createModel();
}

void reshape(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_shaderProgram);
	glBindVertexArray(g_model.vao);

	MVPMatrix mv = v * g_model.m;

	MVPMatrix p = MVPMatrix::getParallelProjectionMatrix(-1.0f, 1.0f, -1.0f, 1.0f, -3.0f, 3.0f);
	glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, (p * mv).getContent());

	float* nMatrix = mv.getNMatrix();
	glUniformMatrix3fv(g_n, 1, GL_TRUE, nMatrix);
	delete[] nMatrix;

	glUniform3fv(led.positionHandler, 1, led.position);
	glUniform3fv(led.colorHandler, 1, led.color);
	glUniform3fv(g_optics.g_oeye, 1, g_optics.eye);
	glUniform1f(g_optics.g_odmin, g_optics.dmin);
	glUniform1f(g_optics.g_osfoc, g_optics.sfoc);
	glUniform1i(led.stateHandler, led.isEnabled);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
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
}

void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;
	switch (state)
	{
		case BasePoints:
		{
			switch (key)
			{
				case GLFW_KEY_SPACE:
				{
					g_model.clearBasePoints();
					updateModel();
					break;
				}
				case GLFW_KEY_E:
				{
					g_model.formCurve();
					state = States::Curve;
					updateModel();
					break;
				}
				default:
					break;
			}
			break;
		}
		case Curve:
			switch (key)
			{
				case GLFW_KEY_Q:
				{
					g_model.clearCurve();
					state = States::BasePoints;
					updateModel();
					break;
				}
				case GLFW_KEY_E:
				{
					g_model.formSolidOfRevolution(15);
					state = States::SolidOfRevolution;
					led.isEnabled = true;
					updateModel();
					break;
				}
				default:
					break;
				}
			break;
		case SolidOfRevolution:
			switch (key)
			{
				case GLFW_KEY_Q:
				{
					g_model.clearSolidOfRevolution();
					led.isEnabled = false;
					state = States::Curve;
					updateModel();
					break;
				}
				default:
					break;
				}
			break;
		default:
			break;
	}
}

void onMouseButtonPress(GLFWwindow* window, int button, int action, int mods)
{
	if (action != GLFW_PRESS || button != GLFW_MOUSE_BUTTON_LEFT || state != States::BasePoints)
		return;
	double x = 0, y = 0;
	glfwGetCursorPos(g_window, &x, &y);
	double w_2 = windowWidth / 2, h_2 = windowHeight / 2;
	g_model.addBasePoint(Point2D((x - w_2) / w_2, ((windowHeight - y) - h_2) / h_2));
	updateModel();
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
	windowWidth = 1024;
	windowHeight = 768;

	glfwMakeContextCurrent(g_window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		return false;
	}
	glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(g_window, onKeyPress);
	glfwSetFramebufferSizeCallback(g_window, reshape);
	glfwSetMouseButtonCallback(g_window, onMouseButtonPress);
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
float currentAngle = 0.0;

void checkInput()
{
		switch (state)
		{
		case BasePoints:
			break;
		case Curve:
			break;
		case SolidOfRevolution:
		{
			for (int i = 0; i < countOfSpeeds; i++)
				if (glfwGetKey(g_window, degreeKeys[i]) == GLFW_PRESS)
					degree = degrees[i];

			if (glfwGetKey(g_window, GLFW_KEY_UP) == GLFW_PRESS)
				g_model.rotateAboutAxis(degree);
			if (glfwGetKey(g_window, GLFW_KEY_DOWN) == GLFW_PRESS)
				g_model.rotateAboutAxis(-degree);
			if (glfwGetKey(g_window, GLFW_KEY_LEFT) == GLFW_PRESS)
				g_model.rotateAboutY(degree);
			if (glfwGetKey(g_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				g_model.rotateAboutY(-degree);
			if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS)
				g_model.rotateAboutX(degree);
			if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS)
				g_model.rotateAboutX(-degree);
			if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS)
				g_model.rotateAboutZ(degree);
			if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS)
				g_model.rotateAboutZ(-degree);

			if (glfwGetKey(g_window, GLFW_KEY_KP_4) == GLFW_PRESS)
				led.moveAlongX(-lightStep);
			if (glfwGetKey(g_window, GLFW_KEY_KP_6) == GLFW_PRESS)
				led.moveAlongX(lightStep);
			if (glfwGetKey(g_window, GLFW_KEY_KP_5) == GLFW_PRESS)
				led.moveAlongY(-lightStep);
			if (glfwGetKey(g_window, GLFW_KEY_KP_8) == GLFW_PRESS)
				led.moveAlongY(lightStep);
			if (glfwGetKey(g_window, GLFW_KEY_KP_7) == GLFW_PRESS)
				led.moveAlongX(-lightStep);
			if (glfwGetKey(g_window, GLFW_KEY_KP_9) == GLFW_PRESS)
				led.moveAlongX(lightStep);

			break;
		}
	}
}
#pragma endregion
int main()
{
	if (!initOpenGL())
		return -1;
	bool isOk = init();
	if (isOk)
	{
		v = MVPMatrix::getIdentityMatrix().move(0.0, 0.0, 2.0);
		g_model.pointRadius = 0.015; 
		led.isEnabled = false;
		g_model.setColor(0.25, 0.5, 0.25);
		state = States::BasePoints;
		for (int i = 0; i < countOfSpeeds; i++)
		{
			degrees[i] = (i + 1) * 0.05f;
			degreeKeys[i] = GLFW_KEY_1 + i;
		}
		degree = degrees[1];
		cout << "Controls:\n\tUp/Down Arrows: rotate about model's axis;\n";
		cout << "\t1-9: speed selection.\n";
		cout << "\t4/6 (Keypad): move light along X axis;\n\t5/8 (Keypad): move light along Y axis;\n\t7/9 (Keypad): move light along Z axis;\n";
		cout << "\t-/+: change model's peaks height;\n\t-/+(Keypad): change model's fragments concavity;\n";
		cout << "Q: disable/enable lightning.\n";
		cout << "Enjoy!\n";
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