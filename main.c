/*
 * A 3D Workspace
 * Written by Eli Riggs 2014
 * Released under the 3-clause BSD License
 */

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include <chicken.h>
#include <kazmath/kazmath.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "macros.h"
#include "render.h"
#include "panel.h"

char *FBBUFFER;
extern void key_callback(GLFWwindow *, int, int, int, int);

static kmMat4 model;

int 
main()
{
	/* variables for rendering */
	GLFWwindow *window;
	float now, sx, sy;

	/* panel linked list */
	panel *root_panel = malloc(sizeof(panel));
	root_panel->next = NULL;

	/* for the scheme REPL */
	char repl_buf[512];
	char repl_result_buf[512];
	int status;
	struct pollfd stdin_poll = { 
		.fd = STDIN_FILENO,
		.events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI 
	};

	/* font library */
	FT_Library ft;

	panel *first_panel = add_panel(root_panel);
	first_panel->x = 0.0;
	first_panel->y = 0.0;
	first_panel->z = 0.0;
	first_panel->rotation = 20.0;
	strcpy(first_panel->text, "it's me!");
	FBBUFFER = first_panel->text;

	/* chicken scheme */
	CHICKEN_run((void *)C_toplevel);

	/* initialize FreeType library */
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

	/* initialize GLFW */
	window = init_context();
	glfwSetKeyCallback(window, key_callback);

	/* using globals here... yuck */
	init_world_renderer();
	init_text_renderer();
	init_framebuffer();

	// kmMat4RotationAxisAngle(&first_panel->model, &(kmVec3){1.0f, 0.0f, 0.0f}, first_panel->rotation);
	kmMat4Translation(&first_panel->model, first_panel->x, first_panel->y, first_panel->z);

	printf("> ");
	fflush(stdout);

	while (!glfwWindowShouldClose(window)) {
		now = glfwGetTime();
		glfwPollEvents();

		/* check for REPL input */
		if (poll(&stdin_poll, 1, 0) == 1) {
			fgets(repl_buf, 511, stdin);
			status = CHICKEN_eval_string_to_string(repl_buf, repl_result_buf, 511);
			if (status) {
				printf("%s\n> ", repl_result_buf);
			} else {
				CHICKEN_get_error_message(repl_result_buf, 511);
				printf("err: %s", repl_result_buf);
			}
			fflush(stdout);
		}

		/* clear the screen */
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* draw the world */
		glUseProgram(world_shader_program);
		glBindVertexArray(world_vao);
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

		render_text(output, -1 + 8 * sx, 0.5 - 50 * sy, sx, sy);

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sx = 2.0 / 2300.0;
		sy = 2.0 / 600.0;

		render_text(first_panel->text, -1, 1 - 50.0 * sy, sx, sy);
		render_text(first_panel->text, -1, 1 - 100.0 * sy, sx, sy);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* Draw framebuffer */
		glUseProgram(fbProgram);
		glBindVertexArray(fbvao);
		glDisable(GL_DEPTH_TEST);

		glUniform1f(uniFBTime, now);

		GL_CHECK_ERROR();

		// kmMat4 fbmodelmat;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);

		/* render each panel */
		for(panel *p = root_panel->next; p; p = p->next) {
			glUniformMatrix4fv(fbModel, 1, GL_FALSE, p->model.mat);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		//kmMat4RotationAxisAngle(&fbmodelmat, &(kmVec3){0.0f, 0.0f, 1.0f}, now + 0.5);

		//glUniformMatrix4fv(fbModel, 1, GL_FALSE, fbmodelmat.mat);

		//glDrawArrays(GL_TRIANGLES, 0, 6);

		GL_CHECK_ERROR();
		glUseProgram(0);
		glfwSwapBuffers(window);
	}

	free_panels(root_panel);
	glfwTerminate();
	return 0;
}
