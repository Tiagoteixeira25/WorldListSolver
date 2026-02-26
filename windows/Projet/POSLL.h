#pragma once
struct pos_linked_list {
	int x, y;
	struct pos_linked_list* next;
};
#define pos_ll_t struct pos_linked_list
void pos_ll_add(pos_ll_t** linked_list, int x, int y);
void pos_ll_pop(pos_ll_t** linked_list, int* x, int* y);

#define PLL_SIZE 8
pos_ll_t* pll_temp[PLL_SIZE];

void* init_pll_temp(void* args);