#pragma once

struct letter_pos {
	int x, y, w, h;
	int letter;
};
#define let_pos_t struct letter_pos
struct letter_pos_array {
	let_pos_t* data;
	size_t size, capacity;
};
