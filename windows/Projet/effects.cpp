#include <SDL.h>
#include<stdio.h>
#include<stdlib.h>


void desaturate(SDL_Surface* surf) {


	int w = surf->w;
	int h = surf->h;
	int r, g, b,temp;
	int* data =(int*) surf->pixels;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			temp = data[y * w + x];
			r = (temp >> 8) & 0xff;
			g = (temp >> 16) & 0xff;
			b = (temp >> 24) & 0xff;
			temp = ((r << 1) + r + (g << 2) + b) >> 3;
			data[y * w + x] = (temp << 24) | (temp << 16) | (temp << 8) | 0XFF;
		}
	}
}


void contrast(SDL_Surface* surf,int count) {
	const float mul = 1.0f / 255.0f;
	float c=count, factor = mul* float(259.0f * (c + 255.0f))/(259.0f- c);
	int w = surf->w;
	int h = surf->h;
	int r, g, b, temp;
	int* data = (int*)surf->pixels;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			temp = (data[y * w + x]&0xFF00)>>8;
			c = factor *(temp - 128)+128;
			temp = c;
			temp = temp &0xFF;
			data[y * w + x] = (temp << 24) | (temp << 16) | (temp << 8) | 0XFF;
		}
	}
}