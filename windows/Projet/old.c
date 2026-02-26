#ifdef DFS
int remove_grid_lines(SDL_Surface* image, SDL_Rect* grid_rect, int x, int y) {
	int h = image->h, w = image->w;
	int* pixels = (int*)image->pixels;
	pos_ll_t* linked_list = NULL;
	if (x < grid_rect->x || y < grid_rect->y
		|| x >= grid_rect->x + grid_rect->w
		|| y >= grid_rect->y + grid_rect->h) {
		return -1;
	}
	size_t pos;
	pos_ll_add(&linked_list, x, y);
	int tx = grid_rect->x
		, ty = grid_rect->y;
	pos_ll_add(&linked_list, tx, ty);

	tx += grid_rect->w - 1;
	pos_ll_add(&linked_list, tx, ty);

	ty += grid_rect->h - 1;
	pos_ll_add(&linked_list, tx, ty);

	tx = grid_rect->x;
	pos_ll_add(&linked_list, tx, ty);
	tx = x; ty = y;
	SDL_Rect rect;
	char* temp_buffer = (char*)calloc(h, w);
	if (temp_buffer == NULL) {
		return -1;
	}
	rect.x = 0x7FFFFFFF;
	rect.y = 0x7FFFFFFF;
	rect.w = 0;
	rect.h = 0;
	int px, py, temp_x, temp_y;
	//printf("linked_list %p\n", linked_list);
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		//printf("x: %i y: %i\n", x, y);
		pos = y * w + x;
		temp_buffer[pos] = 1;
		if (x < rect.x) {
			rect.x = x;
			rect.w++;
		}
		if (x > rect.x + rect.w - 1) {
			rect.w++;
		}
		if (y < rect.y) {
			rect.y = y;
			rect.h++;
		}
		if (y > rect.y + rect.h - 1) {
			rect.h++;
		}

		//RIGHT
		temp_x = x + 1;
		pos++;

		//printf("%i %i %i\n", (temp_x < w),(temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff00) != 0xffffff00));
		if ((temp_x < w) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			//printf("x: %i y: %i\n", temp_x, y);
			pos_ll_add(&linked_list, temp_x, y);
		}
		//LEFT
		temp_x = x - 1;
		pos -= 2;
		//printf("%i %i %i\n", (temp_x < w), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff00) != 0xffffff00));
		if ((temp_x >= 0) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			//printf("x: %i y: %i\n", temp_x, y);
			pos_ll_add(&linked_list, temp_x, y);
		}

		//DOWN
		temp_y = y + 1;
		pos = temp_y * w + x;
		//printf("%i %i %i\n", (temp_x < w), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff00) != 0xffffff00));
		if ((temp_y < h) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			//printf("x: %i y: %i\n", x, temp_y);
			pos_ll_add(&linked_list, x, temp_y);
		}

		//UP
		temp_y = y - 1;
		pos = temp_y * w + x;
		//printf("%i %i %i\n", (temp_x < w), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff00) != 0xffffff00));
		if ((temp_y >= 0) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			//printf("x: %i y: %i\n", x, temp_y);
			pos_ll_add(&linked_list, x, temp_y);
		}
	}

	free(temp_buffer);
	//printf("rect x: %i y: %i w: %i h: %i\n", rect.x, rect.y, rect.w, rect.h);

	if (memcmp(grid_rect, &rect, sizeof(rect)) == 0) {
		pos_ll_add(&linked_list, tx, ty);
		tx = grid_rect->x
			, ty = grid_rect->y;
		pos_ll_add(&linked_list, tx, ty);

		tx += grid_rect->w - 1;
		pos_ll_add(&linked_list, tx, ty);

		ty += grid_rect->h - 1;
		pos_ll_add(&linked_list, tx, ty);

		tx = grid_rect->x;
		pos_ll_add(&linked_list, tx, ty);

		while (linked_list != NULL) {
			pos_ll_pop(&linked_list, &x, &y);
			pos = y * w + x;
			pixels[pos] = -1;//Sets it to white
			temp_x = x + 1;
			pos++;
			if ((temp_x < w) && ISBLACK(pixels[pos])) {
				pos_ll_add(&linked_list, temp_x, y);
			}
			temp_x = x - 1;
			pos -= 2;
			if ((temp_x >= 0) && ISBLACK(pixels[pos])) {
				pos_ll_add(&linked_list, temp_x, y);
			}


			temp_y = y + 1;
			pos = temp_y * w + x;
			if ((temp_y < h) && ISBLACK(pixels[pos])) {
				pos_ll_add(&linked_list, x, temp_y);
			}
			temp_y = y - 1;
			pos = temp_y * w + x;
			if ((temp_y >= 0) && ISBLACK(pixels[pos])) {
				pos_ll_add(&linked_list, x, temp_y);
			}


		}
		return 1;
	}
	else {
		return 0;
	}
}
#endif
#ifdef LEGACY_LINKED_LIST
struct pos_linked_list {
	int x, y;
	struct pos_linked_list* next;
};
#define pos_ll_t struct pos_linked_list
void pos_ll_add(pos_ll_t** linked_list, int x, int y) {
	pos_ll_t* new_cell = (pos_ll_t*)malloc(sizeof(pos_ll_t));
	if (new_cell == NULL) { exit(-1); }
	new_cell->next = *linked_list;
	new_cell->x = x;
	new_cell->y = y;
	*linked_list = new_cell;
}
void pos_ll_pop(pos_ll_t** linked_list, int* x, int* y) {
	pos_ll_t* cell = *linked_list;
	//if (cell == NULL) { exit(-2); }
	*x = cell->x;
	*y = cell->y;
	*linked_list = cell->next;
	free(cell);
}
#endif