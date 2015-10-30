typedef struct panel
{
	kmMat4 model;
	char *text;
	unsigned int cursor;
	int focused;
	float x, y, z;
	float xrot, yrot, zrot;
	struct panel *next;
} panel;

panel *add_panel(panel *root);
void free_panels(panel *root);
void panel_model_update(panel *p);
void move_panel(panel *p, float x, float y, float z);
void panel_add_char(panel *p, char c);
void panel_add_string(panel *p, char *s);
void panel_clear(panel *p);
