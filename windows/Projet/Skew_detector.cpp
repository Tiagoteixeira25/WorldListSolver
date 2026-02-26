#include<SDL.h>
#include"get_rect.h"
#include"letters.h"
#include"pthread.h"
#define ISBLACK(p) ((p&0xFFFFFF00)==0)
/*
struct skew_info{
	int dir,		//direction to which we need to rotate it ,1 is for right -1 for left
		top_x, top_y,
		bottom_x, bottom_y,
		top_end_x, top_end_y,
		bottom_end_x, bottom_end_y
		;
	double line[2];

};
*/
#include<stdio.h>
struct skew_info find_top_bottom(SDL_Surface* img) {
	int w = img->w, h = img->h,
		*data=(int*)img->pixels;
	struct skew_info ret;
	

	//get top black pixel
	for (int y = 0; y != h; y++) {
		for (int x = 0; x != w; x++) {
			if (ISBLACK(data[y * w + x])) {
				ret.top_x = x;
				ret.top_y = y;
				y = h -1;
				break;
			}
		}
	}

	
	if (ret.top_x < w / 2) {
		ret.dir = -1;
		/*
		for (int y = h - 1; y != -1; y--) {
			for (int x = w - 1; x != -1; x--) {
				if (ISBLACK(data[y*w+x])) {
					ret.bottom_x = x;
					ret.bottom_y = y;
					y = 0;
					break;
				}
			}
		}
		*/
	}
	else {
		ret.dir = 1;
		for (int x = ret.top_x+1; x != w; x++) {
			if (ISBLACK(data[ret.top_y * w + x])) {
				ret.top_x = x;
			}
			else {
				break;
			}
		}
		/*
		for (int y = h - 1; y != -1; y--) {
			for (int x = 0; x != w; x++) {

				if (ISBLACK(data[y * w + x])) {
					ret.bottom_x = x;
					ret.bottom_y = y;
					y = 0;
					break;
				}
			}
		}
		*/
	}




	return ret;
}
void find_lines(SDL_Surface* img, struct skew_info * info){
	int w = img->w, h = img->h,
		* data = (int*)img->pixels;
	char* temp_buffer = (char*)calloc(h, w);
	get_rect_special_top(data, temp_buffer,info->top_x, info->top_y,w,h, info);
	//get_rect_special_bot(data, temp_buffer, info->bottom_x, info->bottom_y, w, h, info);
	printf("dir:%i\ninitial top:{%i;%i}->{%i;%i}\n", info->dir,
		info->top_x, info->top_y,
		info->top_end_x, info->top_end_y);

	int temp_x = info->top_end_x, temp_y = info->top_end_y;

	int x_end_y = info->top_end_y+1,x_end=info->top_end_x;
	int iter = 0;
	for(int temp_y= x_end_y; temp_y!=h; temp_y++){
		if (ISBLACK(data[temp_y * w + x_end])) {
			//printf("%i / %i", temp_y, h);
			get_rect_special_top(data, temp_buffer, x_end, temp_y, w, h, info);
			temp_y = info->top_end_y ;
			//printf("; %i / %i\n", temp_y, h);
			x_end_y = temp_y;
			x_end = info->top_end_x;
			iter = 1;
		}
	}
	if (iter == 1) {
		info->top_x = temp_x;
		info->top_y = temp_y;
	}
	/*
	x_end_y = info->bottom_end_y - 1; x_end = info->bottom_end_x;
	for (int temp_y = x_end_y; temp_y != -1; temp_y--) {
		if (ISBLACK(data[temp_y * w + x_end])) {
			//printf("%i / %i", temp_y, h);
			//printf(";; %i / %i\n", temp_y, x_end);
			get_rect_special_bot(data, temp_buffer, x_end, temp_y, w, h, info);
			temp_y = info->bottom_end_y;
			//printf("; %i / %i\n", temp_y, info->bottom_end_x);
			x_end_y = temp_y;
			x_end = info->bottom_end_x;
		}

	}
	*/
	printf("final top:{%i;%i}->{%i;%i}\n",
		info->top_x, info->top_y,
		info->top_end_x, info->top_end_y);


	free(temp_buffer);

}
extern int pll_temp_init;

double find_skew(SDL_Surface* img) {
	pthread_t thr;
	if (pll_temp_init == 0) {
		if (pthread_create(&thr, NULL, init_pll_temp, (void*)pll_temp)) {
			exit(-80);
		}
	}
	struct skew_info info = find_top_bottom(img);
	
	
	if (pll_temp_init == 0) { pthread_join(thr, NULL); pll_temp_init=1; }

	find_lines(img, &info);

	int dist_x = info.top_end_x - info.top_x,
		dist_y = info.top_end_y - info.top_y;

	
	double dist = dist_x * dist_x + dist_y * dist_y;
	dist = 1.0/sqrt(dist);
	double vy = dist_y * dist;
	
	vy = fabs(vy);
	 
	double mul = 180.0 / M_PI * info.dir;

	double ret = acos(vy) * mul,test=fabs(ret);
	
	if (test > 1.0) {
		printf("skew detected : %lf degrees \n", ret);
		return  ret;
	}
	return  0.0;
}