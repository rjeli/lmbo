#include <stdio.h>
#include <stdlib.h>
#include <chicken.h>
#include <assert.h>
#include <poll.h>
#include <math.h>
#include <stdbool.h>
#include <kazmath/kazmath.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "macros.h"

extern void keyCallback(GLFWwindow *, int, int, int, int);

GLuint worldvbo, textvbo;
GLuint worldvao, textvao, fbvao;
GLuint textShaderProgram;
GLuint worldShaderProgram;
GLuint fbProgram;

GLuint texColorBuffer;

GLint uniTextColor;
GLint uniModel;

GLint uniFBTime;

kmMat4 model;

FT_Face face;
FT_GlyphSlot g;

GLuint tex;

void checkShaderError(shader)
{
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) 
	{
		puts("compilation failed\n");
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		if(logSize > 512)
		{
			puts("log too long\n");
		} else {
			char buf[512];
			glGetShaderInfoLog(shader, logSize, NULL, buf);
			printf("%s", buf);
		}
	}
}

char *loadTextFileIntoString(const char *filename)
{
	char *text = malloc(sizeof(char) * 2048);
	FILE *fp = fopen(filename, "r");
	int i = 0;
	while(!feof(fp)){
		text[i++] = fgetc(fp);
	}
	text[i-1] = 0;
	return text;
}

void loadShaders( 
		const char *vertexSourceFilename, 
		const char *fragSourceFilename, 
		GLuint *vs, GLuint *fs, GLuint *s)
{
	const char *vertexSource = loadTextFileIntoString(vertexSourceFilename);
	const char *fragSource = loadTextFileIntoString(fragSourceFilename);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkShaderError(vertexShader);

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, NULL);
	glCompileShader(fragShader);
	checkShaderError(fragShader);

	GLuint worldShaderProgram = glCreateProgram();
	glAttachShader(worldShaderProgram, vertexShader);
	glAttachShader(worldShaderProgram, fragShader);

	glBindFragDataLocation(worldShaderProgram, 0, "outColor");
	glLinkProgram(worldShaderProgram);

	free((void *)vertexSource);
	free((void *)fragSource);

	*vs = vertexShader;
	*fs = fragShader;
	*s = worldShaderProgram;
}

GLFWwindow *initializeContext()
{
	// gl
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
	if(err == GLEW_OK)
	{
		printf("gl version: %s\n", glGetString(GL_VERSION));
	} else {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	while(glGetError() != GL_NO_ERROR) {}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

const GLfloat cubeVertices[] = {
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
	-0.5f,  0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f,   1.0f, 1.0f,
	 0.5f, -0.5f,   1.0f, 0.0f,

	 0.5f, -0.5f,   1.0f, 0.0f,
	-0.5f, -0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.0f, 1.0f
};

void initializeTextRenderer()
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

void initializeWorldRenderer()
{
	glGenVertexArrays(1, &worldvao);
	glGenBuffers(1, &worldvbo);

	GLuint vertexShader, fragShader;
	loadShaders("world.vert", "world.frag", &vertexShader, &fragShader, &worldShaderProgram);
	glUseProgram(worldShaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, worldvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindVertexArray(worldvao);

	GLint posAttrib = glGetAttribLocation(worldShaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

	GLint colorAttrib = glGetAttribLocation(worldShaderProgram, "color");
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)(3 * sizeof(float)));

	uniModel = glGetUniformLocation(worldShaderProgram, "model");
	GLint uniView = glGetUniformLocation(worldShaderProgram, "view");
	GLint uniProj = glGetUniformLocation(worldShaderProgram, "proj");

	kmMat4 view;
	kmMat4LookAt(&view, 
			// camera
			&(kmVec3){1.2f, 1.2f, 1.2f}, 
			// center
			&(kmVec3){0.0f, 0.0f, 0.0f}, 
			// up
			&(kmVec3){0.0f, 0.0f, 1.0f});
	glUniformMatrix4fv(uniView, 1, GL_FALSE, view.mat);
	kmMat4 proj;
	kmMat4PerspectiveProjection(&proj, 45.0f, 800.0f / 600.0f, 1.0f, 10.0f);
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, proj.mat);
}

void renderText(const char *text, float x, float y, float sx, float sy)
{
	const char *p;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glUseProgram(textShaderProgram);
	glBindVertexArray(textvao);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for(p = text; *p; p++)
	{
		if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
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
		GL_CHECK_ERROR();

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
		GL_CHECK_ERROR();
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		GL_CHECK_ERROR();
	  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GL_CHECK_ERROR();

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}
	glUseProgram(0);
	GL_CHECK_ERROR();
}

void initializeFramebuffer()
{
	float sx, sy;
	GLuint fbVertShader, fbFragShader;
	GLuint fbvbo;

	glGenBuffers(1, &fbvbo);
	glBindBuffer(GL_ARRAY_BUFFER, fbvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &fbvao);
	GL_CHECK_ERROR();

	loadShaders("fb.vert", "fb.frag", &fbVertShader, &fbFragShader, &fbProgram);
	glUseProgram(fbProgram);
	GL_CHECK_ERROR();

	glBindVertexArray(fbvao);
	GL_CHECK_ERROR();

	GLint fbposAttrib = glGetAttribLocation(fbProgram, "position");
	GL_CHECK_ERROR();
	glEnableVertexAttribArray(fbposAttrib);
	GL_CHECK_ERROR();
	glVertexAttribPointer(fbposAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GL_CHECK_ERROR();
	GLint fbtexAttrib = glGetAttribLocation(fbProgram, "texcoord");
	glEnableVertexAttribArray(fbtexAttrib);
	glVertexAttribPointer(fbtexAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	uniFBTime = glGetUniformLocation(fbProgram, "time");

	GL_CHECK_ERROR();

	glUniform1i(glGetUniformLocation(fbProgram, "texFramebuffer"), 0);

	// Create frame buffer
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// Create texture to hold color buffer
	glGenTextures(1, &texColorBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	// Create Renderbuffer Object to hold depth and stencil buffers
	//GLuint rboDepthStencil;
	//glGenRenderbuffers(1, &rboDepthStencil);
	//glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

	sx = 2.0 / 800.0;
	sy = 2.0 / 600.0;

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	renderText("Hello! I'm on a Framebuffer.", -1, 1 - 50.0 * sy, sx, sy);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

int main()
{
	float now, sx, sy;

	/* Buffers for the scheme REPL */
	char replbuf[512];
	char replresultbuf[512];
	int status;
	CHICKEN_run(C_toplevel);

	/* Initialize FreeType library */
	FT_Library ft;

	if(FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "could not init freetype\n");
		return 1;
	}

	if(FT_New_Face(ft, "/Library/Fonts/Courier New.ttf", 0, &face))
	{
		fprintf(stderr, "could not load font\n");
		return 1;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);
	g = face->glyph;

	GLFWwindow *window = initializeContext();
	glfwSetKeyCallback(window, keyCallback);

	initializeWorldRenderer();
	initializeTextRenderer();

	/* FRAME BUFFER STUFF */
	initializeFramebuffer();

	/* END FRAME BUFFER STUFF */

	struct pollfd stdin_poll = { 
		.fd = STDIN_FILENO,
		.events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI 
	};

	printf("> ");
	fflush(stdout);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	while(!glfwWindowShouldClose(window))
	{
		now = glfwGetTime();
		glfwPollEvents();

		/* Check for REPL input */
		if(poll(&stdin_poll, 1, 0) == 1)
		{
			fgets(replbuf, 511, stdin);
			status = CHICKEN_eval_string_to_string(replbuf, replresultbuf, 511);
			if(status)
			{
				printf("%s\n> ", replresultbuf);
			} else {
				CHICKEN_get_error_message(replresultbuf, 511);
				printf("err: %s", replresultbuf);
			}
			fflush(stdout);
		}

		/* Update GL */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL_CHECK_ERROR();

		glUseProgram(worldShaderProgram);
		GL_CHECK_ERROR();
		glBindVertexArray(worldvao);
		glEnable(GL_DEPTH_TEST);
		GL_CHECK_ERROR();

		kmMat4RotationAxisAngle(&model, &(kmVec3){0.0f, 0.0f, 1.0f}, now);

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, model.mat);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glUseProgram(0);
		glBindVertexArray(0);

		GL_CHECK_ERROR();

		glDisable(GL_DEPTH_TEST);

		sx = 2.0 / 800.0;
		sy = 2.0 / 600.0;

		char output;
		if((int)now % 2)
			output = 'A';
		else
			output = 'B';
		renderText(&output, -1 + 8 * sx, 0.5 - 50 * sy, sx, sy);

		glBindVertexArray(fbvao);
		glUseProgram(fbProgram);

		glUniform1f(uniFBTime, now);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		GL_CHECK_ERROR();
		glUseProgram(0);
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
