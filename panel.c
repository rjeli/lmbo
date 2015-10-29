#include <kazmath/kazmath.h>
#include <stdlib.h>
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
	kmMat4RotationYawPitchRoll(&p->model, p->xrot, p->yrot, p->zrot);
}

void
move_panel(panel *p, float x, float y, float z)
{
	p->x = x;
	p->y = y;
	p->z = z;
	panel_model_update(p);
}
