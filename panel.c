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
	kmMat4 rot_mat;

	kmMat4Translation(&p->model, p->x, p->y, p->z);

	kmMat4RotationAxisAngle(&rot_mat,
			 &(kmVec3){1.0f, 0.0f, 0.0f}, p->xrot);
	kmMat4Multiply(&p->model, &p->model, &rot_mat);

	kmMat4RotationAxisAngle(&rot_mat,
			 &(kmVec3){0.0f, 1.0f, 0.0f}, p->yrot);
	kmMat4Multiply(&p->model, &p->model, &rot_mat);

	kmMat4RotationAxisAngle(&rot_mat,
			 &(kmVec3){0.0f, 0.0f, 1.0f}, p->zrot);
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