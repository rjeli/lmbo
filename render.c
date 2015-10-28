#include <stdlib.h>
#include <stdio.h>

#include <kazmath/kazmath.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

#include "macros.h"

GLuint worldvbo, textvbo;
GLuint world_vao, textvao, fbvao;
GLuint textShaderProgram;
GLuint world_shader_program;
GLuint fbProgram;

GLuint texColorBuffer;

GLint uniTextColor;
GLint uniModel;

GLint fbModel;

GLint uniFBTime;
GLuint tex;

GLuint frameBuffer;

FT_Face face;
FT_GlyphSlot g;

kmMat4 view;
kmMat4 proj;

GLfloat cubeVertices[] = {
	-0.5f, -0.5f, -0.5f, 0.5f, 0.7f, 0.3f, 
	0.5f, -0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 
	0.5f,  0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 
	0.5f,  0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 
	-0.5f,  0.5f, -0.5f, 0.5f, 0.7f, 0.3f,
	-0.5f, -0.5f, -0.5f, 0.5f, 0.7f, 0.3f,

	-0.5f, -0.5f,  0.5f, 0.4f, 0.2f, 0.6f,
	0.5f, -0.5f,  0.5f,  0.4f, 0.2f, 0.6f, 
	0.5f,  0.5f,  0.5f,  0.4f, 0.2f, 0.6f, 
	0.5f,  0.5f,  0.5f,  0.4f, 0.2f, 0.6f, 
	-0.5f,  0.5f,  0.5f, 0.4f, 0.2f, 0.6f,
	-0.5f, -0.5f,  0.5f, 0.4f, 0.2f, 0.6f,

	-0.5f,  0.5f,  0.5f, 0.2f, 0.5f, 0.4f,
	-0.5f,  0.5f, -0.5f, 0.2f, 0.5f, 0.4f,
	-0.5f, -0.5f, -0.5f, 0.2f, 0.5f, 0.4f,
	-0.5f, -0.5f, -0.5f, 0.2f, 0.5f, 0.4f,
	-0.5f, -0.5f,  0.5f, 0.2f, 0.5f, 0.4f,
	-0.5f,  0.5f,  0.5f, 0.2f, 0.5f, 0.4f,

	0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 1.0f, 
	0.5f,  0.5f, -0.5f,  0.2f, 0.8f, 1.0f, 
	0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 1.0f, 
	0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 1.0f, 
	0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 1.0f, 
	0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 1.0f, 

	-0.5f, -0.5f, -0.5f, 0.3f, 0.9f, 0.2f,
	0.5f, -0.5f, -0.5f,  0.3f, 0.9f, 0.2f, 
	0.5f, -0.5f,  0.5f,  0.3f, 0.9f, 0.2f, 
	0.5f, -0.5f,  0.5f,  0.3f, 0.9f, 0.2f, 
	-0.5f, -0.5f,  0.5f, 0.3f, 0.9f, 0.2f,
	-0.5f, -0.5f, -0.5f, 0.3f, 0.9f, 0.2f,

	-0.5f,  0.5f, -0.5f, 0.8f, 1.0f, 0.7f,
	0.5f,  0.5f, -0.5f,  0.8f, 1.0f, 0.7f, 
	0.5f,  0.5f,  0.5f,  0.8f, 1.0f, 0.7f, 
	0.5f,  0.5f,  0.5f,  0.8f, 1.0f, 0.7f, 
	-0.5f,  0.5f,  0.5f, 0.8f, 1.0f, 0.7f, 
	-0.5f,  0.5f, -0.5f, 0.8f, 1.0f, 0.7f, 
};

GLfloat quadVertices[] = {
	-1.66f,  0.5f,  0.0f, 0.0f, 1.0f,
	 1.66f,  0.5f,  0.0f, 1.0f, 1.0f,
	 1.66f, -0.5f,  0.0f, 1.0f, 0.0f,

	 1.66f, -0.5f,  0.0f, 1.0f, 0.0f,
	-1.66f, -0.5f,  0.0f, 0.0f, 0.0f,
	-1.66f,  0.5f,  0.0f, 0.0f, 1.0f
};

char *
loadTextFileIntoString(char *filename)
{
	char *text = malloc(2048 * sizeof(char));
	FILE *fp = fopen(filename, "r");
	int i = 0;

	while (!feof(fp)) {
		text[i++] = fgetc(fp);
	}
	/* Null terminator */
	text[i-1] = 0;

	return text;
}

void 
checkShaderError(GLuint shader)
{
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		puts("compilation failed\n");
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		if (logSize > 512) {
			puts("log too long\n");
		} else {
			char buf[512];
			glGetShaderInfoLog(shader, logSize, NULL, buf);
			printf("%s", buf);
		}
	}
}

void 
loadShaders(char *vertFile, char *fragFile, GLuint *vs, GLuint *fs, GLuint *s)
{
	char *prefixed_file = malloc(512 * sizeof(char));
	char *offset = stpcpy(prefixed_file, "glsl/");

	strcpy(offset, vertFile);
	char *vertexSource = loadTextFileIntoString(prefixed_file);

	strcpy(offset, fragFile);
	char *fragSource = loadTextFileIntoString(prefixed_file);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkShaderError(vertexShader);

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, NULL);
	glCompileShader(fragShader);
	checkShaderError(fragShader);

	GLuint world_shader_program = glCreateProgram();
	glAttachShader(world_shader_program, vertexShader);
	glAttachShader(world_shader_program, fragShader);

	glBindFragDataLocation(world_shader_program, 0, "outColor");
	glLinkProgram(world_shader_program);

	free(vertexSource);
	free(fragSource);
	free(prefixed_file);

	*vs = vertexShader;
	*fs = fragShader;
	*s = world_shader_program;
}

GLFWwindow *
init_context()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Autismacs", NULL, NULL); 
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	GLenum err = glewInit();
	if (err == GLEW_OK) {
		printf("gl version: %s\n", glGetString(GL_VERSION));
	} else {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	/* OSX gives some incorrect errors, get rid of them */
	while (glGetError() != GL_NO_ERROR) {}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

void 
init_text_renderer()
{
	glGenVertexArrays(1, &textvao);
	glGenBuffers(1, &textvbo);

	GLuint textVertShader, textFragShader;
	loadShaders("text.vert", "text.frag", &textVertShader, &textFragShader, &textShaderProgram);
	glUseProgram(textShaderProgram);

	glBindVertexArray(textvao);
	glBindBuffer(GL_ARRAY_BUFFER, textvbo);

	GLint uniTex = glGetUniformLocation(textShaderProgram, "tex");
	uniTextColor = glGetUniformLocation(textShaderProgram, "color");

	GLint coordAttrib = glGetAttribLocation(textShaderProgram, "coord");
	glEnableVertexAttribArray(coordAttrib);
	glVertexAttribPointer(coordAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glGenTextures(1, &tex);


	glUniform1i(uniTex, 1);

	GLfloat white[4] = {1, 0, 0, 1};
	glUniform4fv(uniTextColor, 1, white);

	GL_CHECK_ERROR();
}

void 
init_world_renderer()
{
	glGenVertexArrays(1, &world_vao);
	glGenBuffers(1, &worldvbo);

	GLuint vertexShader, fragShader;
	loadShaders("world.vert", "world.frag", &vertexShader, &fragShader, &world_shader_program);
	glUseProgram(world_shader_program);

	glBindBuffer(GL_ARRAY_BUFFER, worldvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindVertexArray(world_vao);

	GLint posAttrib = glGetAttribLocation(world_shader_program, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

	GLint colorAttrib = glGetAttribLocation(world_shader_program, "color");
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)(3 * sizeof(float)));

	uniModel = glGetUniformLocation(world_shader_program, "model");
	GLint uniView = glGetUniformLocation(world_shader_program, "view");
	GLint uniProj = glGetUniformLocation(world_shader_program, "proj");

	kmMat4LookAt(&view, 
			/* Camera */
			&(kmVec3){0.0f, -0.5f, 3.0f}, 
			/* Center */
			&(kmVec3){0.0f, 0.0f, 0.0f}, 
			/* Up */
			&(kmVec3){0.0f, 0.0f, 1.0f});

	glUniformMatrix4fv(uniView, 1, GL_FALSE, view.mat);
	kmMat4PerspectiveProjection(&proj, 45.0f, 800.0f / 600.0f, 1.0f, 10.0f);

	glUniformMatrix4fv(uniProj, 1, GL_FALSE, proj.mat);
}

void 
render_text(char *text, float x, float y, float sx, float sy)
{
	char *p;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glUseProgram(textShaderProgram);
	glBindVertexArray(textvao);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (p = text; *p; p++) {
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;

		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				g->bitmap.width,
				g->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer
				);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] = {
			{x2, -y2, 0, 0},
			{x2 + w, -y2, 1, 0},
			{x2, -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};

		glBindBuffer(GL_ARRAY_BUFFER, textvbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
	  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}
	glUseProgram(0);
	GL_CHECK_ERROR();
}

void 
init_framebuffer()
{
	float sx, sy;
	GLuint fbVertShader, fbFragShader;
	GLuint fbvbo;

	glGenBuffers(1, &fbvbo);
	glBindBuffer(GL_ARRAY_BUFFER, fbvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &fbvao);

	loadShaders("fb.vert", "fb.frag", &fbVertShader, &fbFragShader, &fbProgram);
	glUseProgram(fbProgram);

	glBindVertexArray(fbvao);

	GLint fbposAttrib = glGetAttribLocation(fbProgram, "position");
	glEnableVertexAttribArray(fbposAttrib);
	glVertexAttribPointer(fbposAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);

	GLint fbtexAttrib = glGetAttribLocation(fbProgram, "texcoord");
	glEnableVertexAttribArray(fbtexAttrib);
	glVertexAttribPointer(fbtexAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));

	GLint uniProj = glGetUniformLocation(fbProgram, "proj");
	GLint uniView = glGetUniformLocation(fbProgram, "view");
	fbModel = glGetUniformLocation(fbProgram, "model");
	uniFBTime = glGetUniformLocation(fbProgram, "time");

	glUniformMatrix4fv(uniView, 1, GL_FALSE, view.mat);
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, proj.mat);
	glUniform1i(glGetUniformLocation(fbProgram, "texFramebuffer"), 0);

	/* Create frame buffer */
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	/* Create texture to hold color buffer */
	glGenTextures(1, &texColorBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	/* Create Renderbuffer Object to hold depth and stencil buffers */
	/* Commented out because it causes texture corruption
	GLuint rboDepthStencil;
	glGenRenderbuffers(1, &rboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);
	*/

	sx = 2.0 / 800.0;
	sy = 2.0 / 600.0;

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
