#pragma once

struct skew_info {
	int dir,		//direction to which we need to rotate it ,1 is for right -1 for left
		top_x, top_y,
		bottom_x, bottom_y,
		top_end_x, top_end_y,
		bottom_end_x, bottom_end_y
		;
	double line[2];

};

double find_skew(SDL_Surface* img);