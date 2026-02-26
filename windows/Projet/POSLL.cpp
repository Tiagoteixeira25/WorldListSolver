#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>




#define ULL_ARR_SIZE 1024
struct pos {
	int x, y;
};

struct pos_linked_list {
	struct pos arr[ULL_ARR_SIZE];
	size_t size;
	struct pos_linked_list* next;
};
#define pos_ll_t struct pos_linked_list

int pll_temp_init = 0;
#define PLL_SIZE 8
pos_ll_t* pll_temp[PLL_SIZE];
size_t pll_temp_size = PLL_SIZE;

void pos_ll_add(pos_ll_t** linked_list, int x, int y) {
	pos_ll_t* ll = *linked_list;
	if (ll == NULL || ll->size == ULL_ARR_SIZE) {
		if (pll_temp_size) {
			pos_ll_t* t = pll_temp[--pll_temp_size];
			t->size = 1;
			t->arr[0].x = x;
			t->arr[0].y = y;
			t->next = ll;
			*linked_list = t;
			return;
		}
		pos_ll_t* new_cell = (pos_ll_t*)malloc(sizeof(pos_ll_t));
		if (new_cell == NULL) { exit(-1); }
		*linked_list = new_cell;
		new_cell->arr[0].x = x;
		new_cell->arr[0].y = y;
		new_cell->size = 1;
		//new_cell->next = ll;
		return;
	}
	struct pos* p = ll->arr + ll->size++;
	p->x = x;
	p->y = y;

}
void pos_ll_pop(pos_ll_t** linked_list, int* x, int* y) {
	pos_ll_t* ll = *linked_list;
	struct pos* p = ll->arr + --ll->size;
	*x = p->x;
	*y = p->y;
	if (ll->size == 0) {
		*linked_list = ll->next;
		if (pll_temp_size == PLL_SIZE) {
			free(ll);
		}
		else {
			pll_temp[pll_temp_size++] = ll;
		}
	}
}


void* init_pll_temp(void* args) {
	pos_ll_t** p = (pos_ll_t**)args;
	for (size_t i = 0; i != PLL_SIZE; i++) {
		p[i] = (pos_ll_t*)malloc(sizeof(pos_ll_t));
		if (!p[i])	exit(-1);
	}
	return NULL;
}

