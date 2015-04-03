#ifndef __macros_h
#define __macros_h

#include <stdlib.h>

/* debugging macros from Maciej A. Czyzewski */

#define note(S, ...) fprintf(stderr,                                     \
		  "\x1b[1m(%s:%d, %s)\x1b[0m\n  \x1b[1m\x1b[90mnote:\x1b[0m " S "\n",    \
		  __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define warn(S, ...) fprintf(stderr,                                     \
		  "\x1b[1m(%s:%d, %s)\x1b[0m\n  \x1b[1m\x1b[33mwarning:\x1b[0m " S "\n", \
		  __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define errn(S, ...) do { fprintf(stderr,                                \
		  "\x1b[1m(%s:%d, %s)\x1b[0m\n  \x1b[1m\x1b[31merror:\x1b[0m " S "\n",   \
		  __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); exit(1); } while (0) \

/* OpenGL Macros */

#define GL_CHECK_ERROR()                  \
	do                                      \
{                                         \
	GLenum error = glGetError();            \
	if (error != GL_NO_ERROR)               \
	{																				\
		fprintf(stderr, "E: %s(%d): %s 0x%X\n", \
				__FILE__, __LINE__,                 \
				__PRETTY_FUNCTION__, error);        \
		switch(error){ \
			case GL_INVALID_ENUM: printf("invalid enum\n"); break; \
			case GL_INVALID_VALUE: printf("invalid value\n"); break; \
			case GL_INVALID_OPERATION: printf("invalid operation\n"); break; \
			case GL_INVALID_FRAMEBUFFER_OPERATION: printf("invalid framebuffer operation\n"); break; \
			default: printf("default error\n");\
		}\
	}\
} while(0)

#endif
