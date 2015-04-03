/*
 * A 3D Workspace
 * Written by Eli Riggs 2014
 * Released under the 3-clause BSD License
 */

#include <assert.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include <chicken.h>
#include <stdbool.h>
#include <kazmath/kazmath.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "macros.h"
#include "render.h"

typedef struct 
{
	kmMat4 model;
	char text[1024];
} Panel;

typedef struct 
{
	int *array;
	size_t used;
	size_t size;
} PanelArray;

PanelArray initPanelArray()
{
	PanelArray a;
	a.array = malloc(4 * sizeof(Panel));
	a.used = 0;
	a.size = 4;
	return a;
}

extern void keyCallback(GLFWwindow *, int, int, int, int);

static kmMat4 model;

int
main()
{
	/* variables for rendering */
	GLFWwindow *window;
	float now, sx, sy;
	PanelArray PANELS = initPanelArray();

	/* Buffers for the scheme REPL */
	char replbuf[512];
	char replresultbuf[512];
	int status;

	FT_Library ft;

	struct pollfd stdin_poll = { 
		.fd = STDIN_FILENO,
		.events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI 
	};

	CHICKEN_run((void *)C_toplevel);

	/* Initialize FreeType library */
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "could not init freetype\n");
		return 1;
	}
	if (FT_New_Face(ft, "/Library/Fonts/Courier New.ttf", 0, &face)) {
		fprintf(stderr, "could not load font\n");
		return 1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);
	g = face->glyph;

	window = initializeContext();
	glfwSetKeyCallback(window, keyCallback);

	initializeWorldRenderer();
	initializeTextRenderer();
	initializeFramebuffer();

	printf("> ");
	fflush(stdout);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		now = glfwGetTime();
		glfwPollEvents();

		/* Check for REPL input */
		if (poll(&stdin_poll, 1, 0) == 1) {
			fgets(replbuf, 511, stdin);
			status = CHICKEN_eval_string_to_string(replbuf, replresultbuf, 511);
			if (status) {
				printf("%s\n> ", replresultbuf);
			} else {
				CHICKEN_get_error_message(replresultbuf, 511);
				printf("err: %s", replresultbuf);
			}
			fflush(stdout);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Draw the world */
		glUseProgram(worldShaderProgram);
		glBindVertexArray(worldvao);
		glEnable(GL_DEPTH_TEST);

		kmMat4RotationAxisAngle(&model, &(kmVec3){0.0f, 0.0f, 1.0f}, now);

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, model.mat);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* Draw text */
		sx = 2.0 / 800.0;
		sy = 2.0 / 600.0;

		char output;
		if((int)now % 2)
			output = 'A';
		else
			output = 'B';

		renderText(&output, -1 + 8 * sx, 0.5 - 50 * sy, sx, sy);

		/* Draw framebuffer */
		glUseProgram(fbProgram);
		glBindVertexArray(fbvao);
		glDisable(GL_DEPTH_TEST);

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
