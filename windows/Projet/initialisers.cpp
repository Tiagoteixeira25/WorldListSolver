#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
void render_init(SDL_Window** window, SDL_Renderer** renderer,int program_width, int program_height) {
	*window = SDL_CreateWindow("Projet", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, program_width, program_height, 0);
	
	if (*window == NULL) {
		//errx(EXIT_FAILURE, "SDL_CreateWindow()");
	}
	*renderer = SDL_CreateRenderer(*window, 0, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
	if (!*renderer) {
		//errx(EXIT_FAILURE, "SDL_CreateRenderer()");
	}
}

void initialize(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture, char* file) {
	
	if (file != NULL) {
		SDL_Surface* surf = SDL_LoadBMP(file), * surf2;
		if (surf == NULL) {
			//errx(EXIT_FAILURE, "SDL_LoadBMP()");
		}
		surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET);
		if (surf2 == NULL) {
			///errx(EXIT_FAILURE, "SDL_ConvertSurfaceFormat()");
		}
		SDL_FreeSurface(surf);
		SDL_Texture* tex;
		tex = SDL_CreateTextureFromSurface(*renderer, surf2);
		if (tex == NULL) {
			//errx(EXIT_FAILURE, "SDL_CreateTextureFromSurface()");
		}
	}
}