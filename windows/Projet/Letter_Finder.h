#pragma once

#ifndef _H_LETTER_FINDER
#define _H_LETTER_FINDER
#include<SDL.h>
#define THREAD_COUNT 5
struct str {
	char* data;
	size_t size, capacity;
};
struct str_arr {
	struct str* data;
	size_t size, capacity;
};

struct str_arr get_grid(SDL_Surface* image, int classic, int* letter_count_x, struct str* grid);
#endif
