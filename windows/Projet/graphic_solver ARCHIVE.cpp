#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include"Letter_Finder.h"
#include"solver.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
SDL_Texture* letters[26];
SDL_Texture* bg;



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
		letters[i] = SDL_CreateTextureFromSurface(r, temp);
		if (!letters[i]) {
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
	init_letters(r);

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
			SDL_RenderCopy(r, letters[letter], NULL,&temp);
			
		}
	}
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("ROBOTO.ttf",20);
	if (!font)	exit(-80);
	SDL_Color fg = { 0,0,0 };
	pthread_t threads[THREAD_COUNT];
	size_t pthread_pos = 0,cycle=0;
	struct str* cur_str;
	int* colour_choosed;

	for (size_t word_i = 0; word_i != word_list->size; word_i++) {
		//SDL_Surface* texte = TTF_RenderText_Blended(font, word_list->data[word_i].data, fg);
		//SDL_Texture* tex = SDL_CreateTextureFromSurface(r, texte);
		//SDL_FreeSurface(texte);
		
		colour_choosed = colour[word_i % 12];

		SDL_SetRenderDrawColor(r, colour_choosed[0], colour_choosed[1], colour_choosed[2],255);

		cur_str = word_list->data + word_i;

		
		solutions[word_i].x *= letter_w;
		solutions[word_i].tox *= letter_w;
		solutions[word_i].y *= letter_h;
		solutions[word_i].toy *= letter_h;

		//solutions[word_i].x += (letter_w>>1);
		//solutions[word_i].tox += (letter_w >> 1) + (letter_w >> 2);
		solutions[word_i].y += (letter_h >> 2);
		solutions[word_i].toy += (letter_h >> 2);

		//solutions[word_i].y += (letter_h >> 2) ;
		//solutions[word_i].toy -= (letter_h >> 2) ;

		printf("%.*s : {%i;%i -> %i;%i} {%i;%i;%i}\n", cur_str->size, cur_str->data, solutions[word_i].x, solutions[word_i].y, solutions[word_i].tox,
			solutions[word_i].toy, colour_choosed[0], colour_choosed[1], colour_choosed[2]);

		if (solutions[word_i].x < solutions[word_i].tox) {
			puts("1");


			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));



			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));



			SDL_RenderDrawLine(r, solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));





			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3));


			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));



		}
		else if (solutions[word_i].x > solutions[word_i].tox) {
			puts("2");

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >>2),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].tox + (letter_w >> 2), solutions[word_i].toy + (letter_h >> 3),
				solutions[word_i].tox + (letter_w >>2), solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));



			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 3));

			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2));

			SDL_RenderDrawLine(r, solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].x + letter_w, solutions[word_i].y + (letter_h >> 3));




			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 3),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 3));


			SDL_RenderDrawLine(r, solutions[word_i].x + (letter_w >> 2), solutions[word_i].y + (letter_h >> 1) + (letter_h >> 2),
				solutions[word_i].tox + letter_w, solutions[word_i].toy + (letter_h >> 1) + (letter_h >> 2));


		}
		else {
			puts("3");
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
		


		/*
		if (solutions[word_i].y > solutions[word_i].toy) {
			SDL_RenderDrawLine(r, solutions[word_i].x, solutions[word_i].y,
				solutions[word_i].tox, solutions[word_i].y+letter_h);
		}
		else if (solutions[word_i].y < solutions[word_i].toy) {
			SDL_RenderDrawLine(r, solutions[word_i].x, solutions[word_i].toy,
				solutions[word_i].tox , solutions[word_i].toy + letter_h);
		}
		*/

	

		


		/*
		SDL_RenderDrawRect(SDL_Renderer * renderer,
			const SDL_Rect * rect);
		*/

		//SDL_RenderDrawLine(r, solutions[word_i].x, solutions[word_i].y- (letter_h >> 2),
		//	solutions[word_i].tox, solutions[word_i].toy- (letter_h >> 2));


		//SDL_RenderDrawLine(r, solutions[word_i].x, solutions[word_i].y,
		//	solutions[word_i].tox, solutions[word_i].toy);

		//SDL_RenderDrawLine(r, solutions[word_i].x, solutions[word_i].y + (letter_h >> 2),
		//	solutions[word_i].tox, solutions[word_i].toy + (letter_h >> 2));
		
		
		//system("pause");
	}
	SDL_RenderPresent(r);
	TTF_CloseFont(font);
	
	for (size_t i = 0; i != 26; i++) {
		SDL_DestroyTexture(letters[i]);
	}
	TTF_Quit();
}
