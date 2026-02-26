#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include"Letter_Finder.h"
#include"solver.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
SDL_Texture* g_letters[26], * button[4] ;
SDL_Texture* bg;
int init = 0;


struct solution __NOSOLUTIONS;
void init_letters(SDL_Renderer* r) {
	char * save = (char*)malloc(23);
	SDL_Surface* temp;
	memset(&__NOSOLUTIONS, 0, sizeof(__NOSOLUTIONS));
	memcpy(save, "res/grid_letter/a.png", 23);
	for (size_t i = 0; i != 26; i++) {
		//puts(save);
		temp = IMG_Load(save);
		if (!temp) {
			puts(IMG_GetError());
			puts("IMG_Load()");
			exit(80);
		}
		g_letters[i] = SDL_CreateTextureFromSurface(r, temp);
		if (!g_letters[i]) {
			puts(SDL_GetError());
			puts("SDL_CreateTextureFromSurface()");
			exit(56);
		}
		SDL_FreeSurface(temp);
		save[16]++;
	}
	free(save);
	temp = IMG_Load("res/bg.png");
	if (!temp) {
		puts(IMG_GetError());
		puts("IMG_Load()");
		exit(80);
	}
	bg = SDL_CreateTextureFromSurface(r, temp);
	if (!bg) {
		puts(SDL_GetError());
		puts("SDL_CreateTextureFromSurface()");
		exit(56);
	}
	SDL_FreeSurface(temp);


	temp =IMG_Load("res/button_off.png");
	if (!temp) {
		puts(IMG_GetError());
		puts("IMG_Load()");
		exit(80);
	}
	SDL_Surface *nu_surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET);
	if (!nu_surface) {
		puts(SDL_GetError());
		puts("SDL_ConvertSurfaceFormat()");
		exit(80);
	}
	SDL_FreeSurface(temp);
	button[0] = SDL_CreateTextureFromSurface(r, nu_surface);
	if (!button[0]) {
		puts(SDL_GetError());
		puts("SDL_CreateTextureFromSurface()");
		exit(56);
	}
	SDL_FreeSurface(nu_surface);
	SDL_SetTextureBlendMode(button[0], SDL_BLENDMODE_BLEND);




	temp = IMG_Load("res/button_hoover.png");
	if (!temp) {
		puts(IMG_GetError());
		puts("IMG_Load()");
		exit(80);
	}
	nu_surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET);
	if (!nu_surface) {
		puts(SDL_GetError());
		puts("SDL_ConvertSurfaceFormat()");
		exit(80);
	}
	SDL_FreeSurface(temp);
	button[1] = SDL_CreateTextureFromSurface(r, nu_surface);
	if (!button[1]) {
		puts(SDL_GetError());
		puts("SDL_CreateTextureFromSurface()");
		exit(56);
	}
	SDL_FreeSurface(nu_surface);

	SDL_SetTextureBlendMode(button[1], SDL_BLENDMODE_BLEND);


	temp = IMG_Load("res/button_on.png");
	if (!temp) {
		puts(IMG_GetError());
		puts("IMG_Load()");
		exit(80);
	}
	nu_surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET);
	if (!nu_surface) {
		puts(SDL_GetError());
		puts("SDL_ConvertSurfaceFormat()");
		exit(80);
	}
	SDL_FreeSurface(temp);
	button[2] = SDL_CreateTextureFromSurface(r, nu_surface);
	if (!button[2]) {
		puts(SDL_GetError());
		puts("SDL_CreateTextureFromSurface()");
		exit(56);
	}
	SDL_FreeSurface(nu_surface);
	SDL_SetTextureBlendMode(button[2], SDL_BLENDMODE_BLEND);



	temp = IMG_Load("res/button_bg.png");
	if (!temp) {
		puts(IMG_GetError());
		puts("IMG_Load()");
		exit(80);
	}
	
	button[3] = SDL_CreateTextureFromSurface(r, temp);
	if (!button[3]) {
		puts(SDL_GetError());
		puts("SDL_CreateTextureFromSurface()");
		exit(56);
	}
	SDL_FreeSurface(temp);

}

void treat_letter(SDL_Rect* r,size_t letter) {


}


struct dw_mt_args {
	TTF_Font* font;
	SDL_Renderer* renderer;
	char* text;
	struct solution* solution;
	size_t word_i;
};
int colour[12][3] = {
	255,0,0,
	255,127,0,
	255,255,0,
	127,255,0,

	0,255,0,
	0,255,127,
	0,255,255,
	0,127,255,

	0,0,255,
	127,0,255,
	255,0,255,
	255,0,127
};


void graphical_solver(SDL_Renderer* r, struct str* grid, size_t grid_w, size_t grid_h,
	struct str_arr* word_list,struct solution *solutions) {
	if (init == 0) {
		init_letters(r);
		init = 1;
	}






	size_t letter_w=940/ grid_w, letter_h= 940 / grid_h;
	SDL_Rect rect,temp;
	rect.w = letter_w;
	rect.h = letter_h;
	size_t letter;
	SDL_RenderCopy(r, bg, NULL, NULL);
	for (size_t y = 0; y != grid_h; y++) {
		rect.y = y * letter_h;
		for (size_t x = 0; x != grid_w; x++) {
			rect.x = x * letter_w;
			letter = grid->data[y * grid_w + x]-0x41;
			//printf("letter: %zu ; rect{%i;%i;%i;%i}\n", letter,
			//	rect.x, rect.y, rect.w, rect.h);
			memcpy(&temp, &rect, sizeof(rect));
			temp.w >>= 1;
			temp.x += temp.w;
			temp.h >>= 1;
			temp.y += temp.w;
			if (letter == 8) {		//If it is equal to 8
				temp.w = temp.w-(temp.w>>1)- (temp.w>>3);
				temp.x += temp.w>>1;
				
			}
			else {
				temp.w = temp.w - (temp.w >> 2);
			}
			SDL_RenderCopy(r, g_letters[letter], NULL,&temp);
			
		}
	}
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("ROBOTO.ttf",100);
	if (!font)	exit(-80);
	SDL_Color fg = { 0,0,0 };

	size_t pthread_pos = 0,cycle=0;
	struct str* cur_str;
	int* colour_choosed;

	SDL_Rect draw_rect,temp_rect;
	draw_rect.x = 994;
	draw_rect.y = 146;

	draw_rect.w = 1150-994+1;
	draw_rect.h = 190 - 146 + 1;
	SDL_Surface* texte;
	SDL_Texture* tex;




	for (size_t word_i = 0; word_i != word_list->size; word_i++) {
		

		

		
		colour_choosed = colour[word_i % 12];

		SDL_SetRenderDrawColor(r, colour_choosed[0], colour_choosed[1], colour_choosed[2],255);

		fg = { (unsigned char)colour_choosed[0],
			(unsigned char)colour_choosed[1],
			(unsigned char)colour_choosed[2] };


		SDL_Surface* texte = TTF_RenderText_Blended(font, word_list->data[word_i].data, fg);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(r, texte);
		SDL_FreeSurface(texte);

		cur_str = word_list->data + word_i;


		memcpy(&temp_rect, &draw_rect, sizeof(draw_rect));
		temp_rect.x = word_list->data[word_i].size > 15 ?
			temp_rect.x : temp_rect.x + temp_rect.w / 2;


		SDL_RenderCopy(r, tex,NULL,&temp_rect);
		draw_rect.y += draw_rect.h;

		SDL_DestroyTexture(tex);
		
		solutions[word_i].x *= letter_w;
		solutions[word_i].tox *= letter_w;
		solutions[word_i].y *= letter_h;
		solutions[word_i].toy *= letter_h;

		solutions[word_i].y += (letter_h >> 2);
		solutions[word_i].toy += (letter_h >> 2);


		//printf("%.*s : {%i;%i -> %i;%i} {%i;%i;%i}\n", cur_str->size, cur_str->data, solutions[word_i].x, solutions[word_i].y, solutions[word_i].tox,
		//	solutions[word_i].toy, colour_choosed[0], colour_choosed[1], colour_choosed[2]);

		if (solutions[word_i].x < solutions[word_i].tox) {


			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) , solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].x + letter_w - (letter_w >> 3), solutions[word_i].y + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w - (letter_w >> 3), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));



			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));



			SDL_RenderDrawLine(r, solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));





			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w - (letter_w >> 3), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].tox + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy + (letter_h >> 3));


			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w - (letter_w >> 3), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].tox + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));



		}
		else if (solutions[word_i].x > solutions[word_i].tox) {

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + letter_w - (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >>2),
				solutions[word_i].tox + letter_w - (letter_w >> 2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + (letter_w >>2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));



			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 1), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 1), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w , solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 3));




			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 1), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].tox + letter_w - (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3));


			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 1), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].tox + letter_w - (letter_w >> 2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));


		}
		else {
			if(solutions[word_i].toy> solutions[word_i].y){
			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].y,
				solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w, solutions[word_i].y,
				solutions[word_i].x + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));


			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].y,
				solutions[word_i].x + letter_w, solutions[word_i].y);


			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));
			}
			else {
				SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy,
					solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));

				SDL_RenderDrawLine(r, solutions[word_i].x + letter_w, solutions[word_i].toy,
					solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));


				SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].toy,
					solutions[word_i].x + letter_w, solutions[word_i].toy);


				SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2) + (letter_w >> 3), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
					solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));
			}
		}
		

	}
	SDL_RenderPresent(r);
	TTF_CloseFont(font);
	
	

	TTF_Quit();
}
