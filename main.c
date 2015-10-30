/*
 * A 3D Workspace
 * Written by Eli Riggs 2014
 * Released under the 3-clause BSD License
 */

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

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

#define PORT 5005
#define SRV_IP "0.0.0.0"

panel *root_panel;
extern void key_callback(GLFWwindow *, int, int, int, int);

float CHAR_WIDTH = 25.0;
float CHAR_HEIGHT = 35.0;

int tracking = 0;

struct sensor_data
{
	char d1[4];
	float accel[3];
	char d2[36];
	double dir;
	char d3[51];
} __attribute__((packed));

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

	int s;
	struct pollfd fd;
	struct sockaddr_in si_other;
	unsigned int dlen, slen=sizeof(si_other);
	char buf[128];
	struct sensor_data data;
	struct sockaddr_in si_me;
	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		errn("socket");

	memset((char *)&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
		errn("bind");

	fd.fd = s;
	fd.events = POLLIN;

	/* using globals here... yuck */
	init_world_renderer();
	init_text_renderer();
	init_framebuffer();

	CHICKEN_run((void *)C_toplevel);

	printf("> ");
	fflush(stdout);

	kmMat4 fb_view, fb_view_rot;
	float x, y, z;
	double dir = 0.0f;

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

		while (poll(&fd, 1, 0) == 1) {
			dlen=recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&si_other, &slen);
			if(dlen != sizeof(struct sensor_data))
				errn("data incorrect size");
			memcpy(&data, buf, dlen);
			x = data.accel[0];
			y = data.accel[1];
			z = data.accel[2];
			dir = data.dir;
		}

		if(!tracking)
		{
			x = 0;
			y = 0;
			z = 0;
			dir = 157;
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

		glUseProgram(framebuffer_program);
		kmMat4RotationYawPitchRoll(&fb_view_rot, y, (dir-157)*3.14/100, 0);
		kmMat4Translation(&fb_view, 0, 0, -3);
		kmMat4Multiply(&fb_view, &fb_view_rot, &fb_view);
		glUniformMatrix4fv(uni_fb_view, 1, GL_FALSE, fb_view.mat);
		glUseProgram(0);

		for(panel *p = root_panel->next; p; p = p->next) {
			/* draw text to framebuffer */
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

			if(p->focused)
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			else
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

		GL_CHECK_ERROR();
			/* draw framebuffer */
			glBindVertexArray(framebuffer_vao);
			glDisable(GL_DEPTH_TEST);

		GL_CHECK_ERROR();
			glUseProgram(framebuffer_program);
			glUniform1f(uniform_framebuffer_time, now);

		GL_CHECK_ERROR();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_color_buffer);

		GL_CHECK_ERROR();
			/* render each panel */
			glUniformMatrix4fv(framebuffer_model, 1, GL_FALSE, p->model.mat);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		GL_CHECK_ERROR();
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
