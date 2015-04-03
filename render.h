void checkShaderError(GLuint shader);

char *loadTextFileIntoString(const char *filename);

void loadShaders( 
		const char *vertexSourceFilename, 
		const char *fragSourceFilename, 
		GLuint *vs, GLuint *fs, GLuint *s);

GLFWwindow *initializeContext();

void initializeTextRenderer();

void initializeWorldRenderer();

void renderText(const char *text, float x, float y, float sx, float sy);

void initializeFramebuffer();

extern GLuint worldvbo, textvbo;
extern GLuint worldvao, textvao, fbvao;
extern GLuint textShaderProgram;
extern GLuint worldShaderProgram;
extern GLuint fbProgram;

extern GLuint texColorBuffer;

extern GLint uniTextColor;
extern GLint uniModel;

extern GLint uniFBTime;
extern GLuint tex;

extern FT_Face face;
extern FT_GlyphSlot g;
