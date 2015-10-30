#include <kazmath/kazmath.h>
#include <stdlib.h>
#include <string.h>
#include "panel.h"

panel *
add_panel(panel *root)
{
	panel *p = malloc(sizeof(panel));
	p->text = malloc(4096 * sizeof(char));
	p->text[0] = '\0';
	p->next = NULL;
	while(root->next) root = root->next;
	root->next = p;
	return p;
}

void
free_panels(panel *root)
{
	panel *next;
	while(root) {
		next = root->next;
		free(root);
		root = next;
	}
}

void
panel_model_update(panel *p)
{
	kmMat4 rot_mat;
	kmMat4RotationYawPitchRoll(&rot_mat, p->xrot, p->yrot, p->zrot);
	kmMat4Translation(&p->model, p->x, p->y, p->z);
	kmMat4Multiply(&p->model, &p->model, &rot_mat);
}

void
move_panel(panel *p, float x, float y, float z)
{
	p->x = x;
	p->y = y;
	p->z = z;
	panel_model_update(p);
}

void
panel_add_char(panel *p, char c)
{
	p->text[p->cursor] = c;
	p->text[p->cursor+1] = '\0';
	p->cursor++;
}

void
panel_add_string(panel *p, char *s)
{
	strcat(p->text, s);
	p->cursor += strlen(s);
}

void
panel_clear(panel *p)
{
	p->text[0] = '\0';
}
