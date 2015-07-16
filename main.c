/*
 * Written by Eli Riggs 2014
 * Released under the 3-clause BSD License
 */

#include <assert.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char text[4096];
	int cursor;
	float x, y, z;
	float rotation;
} Panel;

typedef struct 
{
	Panel *array;
	int used;
	int size;
} PanelArray;

PanelArray 
initPanelArray()
{
	PanelArray pa;
	pa.array = malloc(4 * sizeof(Panel));
	pa.used = 0;
	pa.size = 4;
	return pa;
}

Panel *
addPanel(PanelArray *pa)
{
	if (pa->used == pa->size) {
		int newsize = pa->size * 2;
		pa->array = realloc(pa->array, newsize);
		pa->size = newsize;
	}
	return pa->array + (pa->used)++;
}

void
removePanel(PanelArray *pa, int index)
{
	for (; index < (int)pa->used - 1; index++)
		pa->array[index] = pa->array[index + 1];
	(pa->used)--;
}

void 
keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		printf("pressed key %c\n", key);
	else if (action == GLFW_RELEASE)
		printf("released key %c\n", key);
}

char *FBBUFFER;

static kmMat4 model;

int 
main()
{
	/* variables for rendering */
	GLFWwindow *window;
	float now, sx, sy;
	PanelArray PANELS = initPanelArray();

	FT_Library ft;

	Panel *firstPanel = addPanel(&PANELS);
	firstPanel->x = 0.0;
	firstPanel->y = 0.0;
	firstPanel->z = 0.0;
	firstPanel->rotation = 0.0;
	strcpy(firstPanel->text, "it's me!");
	FBBUFFER = firstPanel->text;

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

	//kmMat4RotationAxisAngle(&firstPanel->model, &(kmVec3){0.0f, 0.0f, 1.0f}, firstPanel->rotation);
	kmMat4Translation(&firstPanel->model, firstPanel->x, firstPanel->y, firstPanel->z);

	while (!glfwWindowShouldClose(window)) {
		now = glfwGetTime();
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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

		char *output;
		if((int)now % 2)
			output = "A";
		else
			output = "B";

		renderText(output, -1 + 8 * sx, 0.5 - 50 * sy, sx, sy);

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sx = 2.0 / 2300.0;
		sy = 2.0 / 600.0;

		renderText(firstPanel->text, -1, 1 - 50.0 * sy, sx, sy);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* Draw framebuffer */
		glUseProgram(fbProgram);
		glBindVertexArray(fbvao);
		glDisable(GL_DEPTH_TEST);

		glUniform1f(uniFBTime, now);

		GL_CHECK_ERROR();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);

		int i;
		for (i = 0; i < PANELS.used; i++) {
			glUniformMatrix4fv(fbModel, 1, GL_FALSE, PANELS.array[i].model.mat);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		GL_CHECK_ERROR();
		glUseProgram(0);
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
