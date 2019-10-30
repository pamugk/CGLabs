#include <iostream>
#include <map>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

GLFWwindow *g_window;
GLuint g_shaderProgram;

enum Variants
{
	lines,
	waves,
	circles
};

Variants variant = Variants::waves;

#pragma region Вершины
	const GLfloat vertices[] =
	{
		-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, //0
		 1.0f, -1.0f, -1.0f,  1.0f, 0.0f, //1
		 1.0f,  1.0f,  1.0f,  1.0f, 0.0f, //2
		-1.0f,  1.0f,  1.0f, -1.0f, 0.0f //3
	};

	const GLuint indices[] =
	{
		0, 1, 2, 2, 3, 0
	};
#pragma endregion
#pragma region Шейдеры
	const GLchar vsh[] =
	"#version 330\n"
	""
	"layout(location = 0) in vec2 a_position;"
	"layout(location = 1) in vec3 a_color;"
	""
	"out vec3 v_color;"
	""
	"void main()"
	"{"
	"    v_color = a_color;"
	"    gl_Position = vec4(a_position, 0.0, 1.0);"
	"}"
	;
	
	map<Variants, const GLchar*> fragmentShaders = map<Variants, const GLchar*>
	{
		{
			Variants::lines, 
			"#version 330\n"
			""
			"in vec3 v_color;"
			""
			"layout(location = 0) out vec4 o_color;"
			""
			"void main()"
			"{"
			"   o_color = vec4(sin(v_color[0] * 10 * 3.1415926) * 0.5 + 0.5, 0.0, 0.0, 1.0);"
			"}"
		},
		{
			Variants::waves,
			"#version 330\n"
			""
			"in vec3 v_color;"
			""
			"layout(location = 0) out vec4 o_color;"
			""
			"void main()"
			"{"
			"   o_color = vec4(sin(20 * v_color[0] * 3.1415926 + 5 * sin(10 * 3.1415926 * v_color[1])), 0.0, 0.0, 1.0);"
			"}"
		},
		{
			Variants::circles,
			"#version 330\n"
			""
			"in vec3 v_color;"
			""
			"layout(location = 0) out vec4 o_color;"
			""
			"void main()"
			"{"
			"   o_color = vec4(sin(100 * length(v_color) * 3.1415926), 0.0, 0.0, 1.0);"
			"}"
		}
	};
#pragma endregion

class Model
{
public:
	GLuint vbo; //VertexBufferObject - буфер вершин
	GLuint ibo; //IndexBufferObject - буфер индексов
	GLuint vao; //VertexArrayObject - настройки модели
	GLsizei indexCount; //Число индексов
};

Model g_model;

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
			glGetShaderInfoLog(result, infoLen, NULL, infoLog);
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

bool createShaderProgram(const GLchar vsh[], const GLchar fsh[])
{
	g_shaderProgram = 0;
	GLuint vertexShader, fragmentShader;
	vertexShader = createShader(vsh, GL_VERTEX_SHADER);
	fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);
	g_shaderProgram = createProgram(vertexShader, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return g_shaderProgram != 0;
}

bool createModel()
{
	glGenVertexArrays(1, &g_model.vao);
	glBindVertexArray(g_model.vao);
	glGenBuffers(1, &g_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glGenBuffers(1, &g_model.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
	g_model.indexCount = 6;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_model.indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const GLvoid *)(2 * sizeof(GLfloat)));
	return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	return createShaderProgram(vsh, fragmentShaders[variant]) && createModel();
}

void reshape(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(g_shaderProgram);
	glBindVertexArray(g_model.vao);
	glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, (const GLvoid *)0);
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
	return true;
}

void tearDownOpenGL()
{
	glfwTerminate();
}

int main()
{
	if (!initOpenGL())
		return -1;
	bool isOk = init();
	if (isOk)
	{
		while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
		{
			draw();
			glfwSwapBuffers(g_window);
			glfwPollEvents();
		}
	}
	cleanup();
	tearDownOpenGL();
	return isOk ? 0 : -1;
}