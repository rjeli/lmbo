typedef struct panel
{
	kmMat4 model;
	char text[4096];
	int cursor;
	float x, y, z;
	float rotation;
	struct panel *next;
} panel;

panel *add_panel(panel *root);
void free_panels(panel *root);
