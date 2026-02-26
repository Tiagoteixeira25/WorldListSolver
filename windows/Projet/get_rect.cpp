#include"POSLL.h"
#include"LetterPos.h"
#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include"skew_detector.h"
#define ISBLACK(p) ((p&0xFFFFFF00)==0)
#ifdef DFS
struct letter_pos get_rect(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h) {
	pos_ll_t* linked_list = NULL;
	size_t pos = y * pixels_w + x;
	pos_ll_add(&linked_list, x, y);
	struct letter_pos ret;
	ret.letter = 0;
	ret.x = x;
	ret.y = y;
	ret.w = 1;
	ret.h = 1;
	int px, py, temp_x, temp_y = 0;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		//printf("%i %i\n", x, y);
		pos = y * pixels_w + x;
		temp_buffer[pos] = 1;
		if (x < ret.x) {
			ret.x = x;
			ret.w++;
		}
		if (x > ret.x + ret.w - 1) {
			ret.w++;
		}
		if (y < ret.y) {
			ret.y = y;
			ret.h++;
		}
		if (y > ret.y + ret.h - 1) {
			ret.h++;
		}
		temp_x = x + 1;
		pos++;
		//printf("0 %i %i %i %.*x\n", (temp_x < pixels_w), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff) == 0),8, pixels[pos]);
		if ((temp_x < pixels_w) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, temp_x, y);
		}
		temp_x = x - 1;
		pos -= 2;
		//printf("1 %i %i %i\n", (temp_x >= 0), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff) == 0));
		if ((temp_x >= 0) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, temp_x, y);
		}


		temp_y = y + 1;
		pos = temp_y * pixels_w + x;
		//printf("2 %i %i %i\n", (temp_y < pixels_h), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff) == 0));
		if ((temp_y < pixels_h) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, x, temp_y);
		}

		temp_y = y - 1;
		pos = temp_y * pixels_w + x;
		//printf("3 %i %i %i\n", (temp_y >= 0), (temp_buffer[pos] == 0), ((pixels[pos] & 0xffffff00) == 0));
		if ((temp_y >= 0) && (temp_buffer[pos] == 0)
			&& ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, x, temp_y);
		}


	}
	return ret;
}
#endif

struct letter_pos get_rect(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h) {
	pos_ll_t* linked_list = NULL;
	size_t pos;
	pos_ll_add(&linked_list, x, y);
	struct letter_pos ret;
	//ret.letter = 0;
	//ret.x = x;
	//ret.y = y;
	//ret.w = 1;
	//ret.h = 1;
	int first_x = x, first_y = y,
		last_x = x, last_y = y,
		temp_x, temp_y = 0;
	int add_bottom, add_top, cond;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		temp_x = x;
		add_bottom = 1;
		add_top = 1;
		cond = !temp_buffer[y * pixels_w + temp_x]
			&& ISBLACK(pixels[y * pixels_w + temp_x]);
		while (temp_x != -1 && !temp_buffer[y * pixels_w + temp_x]
			&& ISBLACK(pixels[y * pixels_w + temp_x])) {
			pos = y * pixels_w + temp_x;
			first_x = first_x > temp_x ? temp_x : first_x;
			temp_buffer[pos] = 1;
			temp_y = y + 1;
			if (temp_y != pixels_h) {
				pos = temp_y * pixels_w + temp_x;
				if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
					if (add_bottom) {
						last_y = last_y < temp_y ? temp_y : last_y;
						pos_ll_add(&linked_list, temp_x, temp_y);
						add_bottom = 0;
					}
				}
				else {
					add_bottom = 1;
				}
			}
			temp_y = y - 1;
			if (temp_y != -1) {
				pos = temp_y * pixels_w + temp_x;
				if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
					if (add_top) {
						first_y = first_y > temp_y ? temp_y : first_y;
						pos_ll_add(&linked_list, temp_x, temp_y);
						add_top = 0;
					}
				}
				else {
					add_top = 1;
				}
			}
			temp_x--;
		}
		temp_x = x + 1;
		add_bottom = 1;
		add_top = 1;
		if (cond) {
			while (temp_x != pixels_w && !temp_buffer[y * pixels_w + temp_x]
				&& ISBLACK(pixels[y * pixels_w + temp_x])) {
				pos = y * pixels_w + temp_x;
				last_x = last_x < temp_x ? temp_x : last_x;
				temp_buffer[pos] = 1;
				temp_y = y + 1;
				if (temp_y != pixels_h) {
					pos = temp_y * pixels_w + temp_x;
					if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
						if (add_bottom) {
							last_y = last_y < temp_y ? temp_y : last_y;
							pos_ll_add(&linked_list, temp_x, temp_y);
							add_bottom = 0;
						}
					}
					else {
						add_bottom = 1;
					}
				}
				temp_y = y - 1;
				if (temp_y != -1) {
					pos = temp_y * pixels_w + temp_x;
					if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
						if (add_top) {
							first_y = first_y > temp_y ? temp_y : first_y;
							pos_ll_add(&linked_list, temp_x, temp_y);
							add_top = 0;
						}
					}
					else {
						add_top = 1;
					}
				}
				temp_x++;
			}
		}


	}

	ret.x = first_x;
	ret.y = first_y;
	ret.w = last_x - first_x + 1;
	ret.h = last_y - first_y + 1;

	return ret;
}

void get_rect_special_top(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h, struct skew_info* info) {
	pos_ll_t* linked_list = NULL;
	size_t pos;
	pos_ll_add(&linked_list, x, y);
	
	//printf("\start:{%i,%i}\n", x, y);
	int first_x = x, first_y = y,
		last_x = x, last_y = y,
		temp_x = 0, temp_y = 0,
		first_x_y=y, last_x_y=y;
	int add_bottom, add_top, cond;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		temp_x = x;
		add_bottom = 1;
		add_top = 1;
		cond = !temp_buffer[y * pixels_w + temp_x]
			&& ISBLACK(pixels[y * pixels_w + temp_x]);
		while (temp_x != -1 && !temp_buffer[y * pixels_w + temp_x]
			&& ISBLACK(pixels[y * pixels_w + temp_x])) {
			pos = y * pixels_w + temp_x;
			//printf("x:%i y:%i %x\n",temp_x,y, pixels[pos]);
			if (first_x > temp_x||(first_x == temp_x && y> first_x_y)) {
				first_x = temp_x;
				first_x_y = y;
			}
			if (last_x == temp_x) {
				last_x_y = y > last_x_y ? y : last_x_y;
			}
			temp_buffer[pos] = 1;
			temp_y = y + 1;
			if (temp_y != pixels_h) {
				pos = temp_y * pixels_w + temp_x;
				if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
					if (add_bottom) {
						last_y = last_y < temp_y ? temp_y : last_y;
						pos_ll_add(&linked_list, temp_x, temp_y);
						add_bottom = 0;
					}
				}
				else {
					add_bottom = 1;
				}
			}
			temp_y = y - 1;
			if (temp_y != -1) {
				pos = temp_y * pixels_w + temp_x;
				if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
					if (add_top) {
						first_y = first_y > temp_y ? temp_y : first_y;
						pos_ll_add(&linked_list, temp_x, temp_y);
						add_top = 0;
					}
				}
				else {
					add_top = 1;
				}
			}
			temp_x--;
		}
		temp_x = x + 1;
		add_bottom = 1;
		add_top = 1;
		if (cond) {
			while (temp_x != pixels_w && !temp_buffer[y * pixels_w + temp_x]
				&& ISBLACK(pixels[y * pixels_w + temp_x])) {
				pos = y * pixels_w + temp_x;
				//printf("2x:%i y:%i\n", temp_x, y);
				if (last_x < temp_x || (last_x == temp_x && y > last_x_y)) {
					//printf("2huj: %i %i ; %i %i %i %i\n", last_x < temp_x, (last_x == temp_x && y > last_x_y),
					//	last_x, temp_x, y, last_x_y 
					//	);
					last_x = temp_x;
					last_x_y = y;
				}
				if (first_x == temp_x) {
					first_x_y = y > first_x_y ? y : first_x_y;
				}
				temp_buffer[pos] = 1;
				temp_y = y + 1;
				if (temp_y != pixels_h) {
					pos = temp_y * pixels_w + temp_x;
					if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
						if (add_bottom) {
							//if (last_y < temp_y) {
							//	last_y = temp_y;
							//	last_y_x = temp_x;
							//}
							last_y = last_y < temp_y ? temp_y : last_y;
							pos_ll_add(&linked_list, temp_x, temp_y);
							add_bottom = 0;
						}
					}
					else {
						add_bottom = 1;
					}
				}
				temp_y = y - 1;
				if (temp_y != -1) {
					pos = temp_y * pixels_w + temp_x;
					if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
						if (add_top) {
							//if (first_y > temp_y) {
							//	first_y = temp_y;
							//	first_y_x = temp_x;
							//}
							first_y = first_y > temp_y ? temp_y : first_y;
							pos_ll_add(&linked_list, temp_x, temp_y);
							add_top = 0;
						}
					}
					else {
						add_top = 1;
					}
				}
				temp_x++;
			}
		}


	}


	if (info->dir==1) {
		info->top_end_y= last_x_y;
		info->top_end_x= last_x;
		//info->bottom_end_y = last_x_y;
		//info->bottom_end_x = last_x;
	}
	else {
		//info->bottom_end_y = first_x_y;
		//info->bottom_end_x = first_x;

		info->top_end_y = first_x_y;
		info->top_end_x = first_x;

	}
	
}
void get_rect_special_bot(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h, struct skew_info* info) {
	pos_ll_t* linked_list = NULL;
	size_t pos;
	pos_ll_add(&linked_list, x, y);

	//printf("\start:{%i,%i}\n", x, y);
	int first_x = x, first_y = y,
		last_x = x, last_y = y,
		temp_x = 0, temp_y = 0,
		first_x_y = y, last_x_y = y;
	int add_bottom, add_top, cond;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		temp_x = x;
		add_bottom = 1;
		add_top = 1;
		cond = !temp_buffer[y * pixels_w + temp_x]
			&& ISBLACK(pixels[y * pixels_w + temp_x]);
		while (temp_x != -1 && !temp_buffer[y * pixels_w + temp_x]
			&& ISBLACK(pixels[y * pixels_w + temp_x])) {
			pos = y * pixels_w + temp_x;
			//printf("x:%i y:%i\n",temp_x,y);
			if (first_x > temp_x || (first_x == temp_x && y < first_x_y)) {
				first_x = temp_x;
				first_x_y = y;
			}
			if (last_x == temp_x) {
				last_x_y = y < last_x_y ? y : last_x_y;
			}
			temp_buffer[pos] = 1;
			temp_y = y + 1;
			if (temp_y != pixels_h) {
				pos = temp_y * pixels_w + temp_x;
				if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
					if (add_bottom) {
						last_y = last_y < temp_y ? temp_y : last_y;
						pos_ll_add(&linked_list, temp_x, temp_y);
						add_bottom = 0;
					}
				}
				else {
					add_bottom = 1;
				}
			}
			temp_y = y - 1;
			if (temp_y != -1) {
				pos = temp_y * pixels_w + temp_x;
				if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
					if (add_top) {
						first_y = first_y > temp_y ? temp_y : first_y;
						pos_ll_add(&linked_list, temp_x, temp_y);
						add_top = 0;
					}
				}
				else {
					add_top = 1;
				}
			}
			temp_x--;
		}
		temp_x = x + 1;
		add_bottom = 1;
		add_top = 1;
		if (cond) {
			while (temp_x != pixels_w && !temp_buffer[y * pixels_w + temp_x]
				&& ISBLACK(pixels[y * pixels_w + temp_x])) {
				pos = y * pixels_w + temp_x;
				//printf("2x:%i y:%i\n", temp_x, y);
				if (last_x < temp_x || (last_x == temp_x && y< last_x_y)) {
					//printf("2huj: %i %i ; %i %i %i %i\n", last_x < temp_x, (last_x == temp_x && y > last_x_y),
					//	last_x, temp_x, y, last_x_y 
					//	);
					last_x = temp_x;
					last_x_y = y;
				}
				if (first_x == temp_x) {
					first_x_y = y < first_x_y ? y : first_x_y;
				}
				temp_buffer[pos] = 1;
				temp_y = y + 1;
				if (temp_y != pixels_h) {
					pos = temp_y * pixels_w + temp_x;
					if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
						if (add_bottom) {
							//if (last_y < temp_y) {
							//	last_y = temp_y;
							//	last_y_x = temp_x;
							//}
							last_y = last_y < temp_y ? temp_y : last_y;
							pos_ll_add(&linked_list, temp_x, temp_y);
							add_bottom = 0;
						}
					}
					else {
						add_bottom = 1;
					}
				}
				temp_y = y - 1;
				if (temp_y != -1) {
					pos = temp_y * pixels_w + temp_x;
					if (ISBLACK(pixels[pos]) && !temp_buffer[pos]) {
						if (add_top) {
							//if (first_y > temp_y) {
							//	first_y = temp_y;
							//	first_y_x = temp_x;
							//}
							first_y = first_y > temp_y ? temp_y : first_y;
							pos_ll_add(&linked_list, temp_x, temp_y);
							add_top = 0;
						}
					}
					else {
						add_top = 1;
					}
				}
				temp_x++;
			}
		}


	}


	if (info->dir == 1) {
		//info->top_end_y = last_x_y;
		//info->top_end_x = last_x;
		info->bottom_end_y = first_x_y;
		info->bottom_end_x = first_x;
	}
	else {
		info->bottom_end_y = last_x_y;
		info->bottom_end_x = last_x;

		//info->top_end_y = first_x_y;
		//info->top_end_x = first_x;

	}

}