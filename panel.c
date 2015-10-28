#include <kazmath/kazmath.h>
#include <stdlib.h>
#include "panel.h"

panel *
add_panel(panel *root)
{
	panel *p = malloc(sizeof(panel));
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
