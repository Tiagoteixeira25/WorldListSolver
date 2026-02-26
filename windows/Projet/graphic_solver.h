#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include"Letter_Finder.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
void graphical_solver(SDL_Renderer* r, struct str* grid, size_t grid_w, size_t grid_h,
	struct str_arr* word_list, struct solution* solutions);