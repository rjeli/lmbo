/* does stuff */
void checkShaderError(GLuint shader);

char *loadTextFileIntoString(char *filename);

void loadShaders( 
		char *vertexSourceFilename, 
		char *fragSourceFilename, 
		GLuint *vs, GLuint *fs, GLuint *s);

GLFWwindow *init_context();

void init_text_renderer();

void init_world_renderer();

/*
 * render text
 * x,y on [-1,1]
 *
 */
void render_char(char ch, float x, float y, float sx, float sy);

void init_framebuffer();

extern GLuint worldvbo, textvbo;
extern GLuint world_vao, textvao, framebuffer_vao;
extern GLuint textShaderProgram;
extern GLuint world_shader_program;
extern GLuint framebuffer_program;

extern GLuint tex_color_buffer;

extern GLint uniTextColor;
extern GLint uniModel;

extern GLint uni_fb_view;
extern GLint framebuffer_model;

extern GLint uniform_framebuffer_time;
extern GLuint tex;
extern GLuint framebuffer;

extern FT_Face face;
extern FT_GlyphSlot g;
