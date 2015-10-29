/*
 * A 3D Workspace
 * Written by Eli Riggs 2014
 * Released under the 3-clause BSD License
 */

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

panel *root_panel;
extern void key_callback(GLFWwindow *, int, int, int, int);

float CHAR_WIDTH = 25.0;
float CHAR_HEIGHT = 35.0;

//static kmMat4 model;

int 
main()
{
	GLFWwindow *window;
	float now, sx, sy;
	sx = 2.0 / 1200.0;
	sy = 2.0 / 600.0;

	root_panel = malloc(sizeof(panel));
	root_panel->next = NULL;

	char repl_buf[512];
	char repl_result_buf[512];
	int status;
	struct pollfd stdin_poll = { 
		.fd = STDIN_FILENO,
		.events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI 
	};

	FT_Library ft;

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

	CHICKEN_run((void *)C_toplevel);

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

		// this is the rotating quad in background
		//kmMat4RotationAxisAngle(&model, &(kmVec3){0.0f, 0.0f, 1.0f}, now);
		//glUniformMatrix4fv(uniModel, 1, GL_FALSE, model.mat);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		for(panel *p = root_panel->next; p; p = p->next) {
			/* draw text to framebuffer */
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			int r = 1;
			int c = 0;
			for(unsigned int i = 0; i < strlen(p->text); i++) {
				if(p->text[i] == '\n') {
					r++;
					c = 0;
					continue;
				}
				render_char(p->text[i], -1 + c * CHAR_WIDTH * sx, 1 - r * CHAR_HEIGHT * sy, sx, sy);
				c++;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			/* draw framebuffer */
			glUseProgram(framebuffer_program);
			glBindVertexArray(framebuffer_vao);
			glDisable(GL_DEPTH_TEST);

			glUniform1f(uniform_framebuffer_time, now);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_color_buffer);

			/* render each panel */
			glUniformMatrix4fv(framebuffer_model, 1, GL_FALSE, p->model.mat);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		/* ** How to move the framebuffer (panel) **
		kmMat4 fbmodelmat;
		kmMat4RotationAxisAngle(&fbmodelmat, &(kmVec3){0.0f, 0.0f, 1.0f}, now + 0.5);
		glUniformMatrix4fv(framebuffer_model, 1, GL_FALSE, fbmodelmat.mat);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		*/

		GL_CHECK_ERROR();
		glUseProgram(0);
		glfwSwapBuffers(window);
	}

	free_panels(root_panel);
	glfwTerminate();
	return 0;
}
