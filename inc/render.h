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
void render_text(char *text, float x, float y, float sx, float sy);

void init_framebuffer();

extern GLuint worldvbo, textvbo;
extern GLuint world_vao, textvao, fbvao;
extern GLuint textShaderProgram;
extern GLuint world_shader_program;
extern GLuint fbProgram;

extern GLuint texColorBuffer;

extern GLint uniTextColor;
extern GLint uniModel;
extern GLint fbModel;

extern GLint uniFBTime;
extern GLuint tex;
extern GLuint frameBuffer;

extern FT_Face face;
extern FT_GlyphSlot g;
