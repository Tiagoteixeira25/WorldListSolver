#pragma warning(disable : 4996)
#include <SDL.h>
//#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include<pthread.h>
#include"letters.h"
#include"neural.h"
#include"train.h"


pthread_mutex_t xor_net_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t and_net_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sim_net_mutex = PTHREAD_MUTEX_INITIALIZER;



#define ISBLACK(p) ((p&0xFFFFFF00)==0)
#ifdef LEGACY
#define LEGACY_LINKED_LIST
#define DFS
#else
#define UNROLLED_LINKED_LIST
#define SCANLINE
#endif
#include"get_rect.h"
#include"POSLL.h"
#include"LetterPos.h"
#include"array.h"



int lpa_add(struct letter_pos_array* lpa, let_pos_t* elem) {
	if (lpa->size == lpa->capacity) {
		lpa->capacity = lpa->capacity * 2 + 1;
		lpa->data = (let_pos_t*)realloc(lpa->data, sizeof(let_pos_t) * lpa->capacity);
		if (lpa->data == NULL) {
			return -1;
		}
	}
	memcpy(lpa->data + lpa->size, elem, sizeof(let_pos_t));
	lpa->size++;
	return 0;
}


//#define LEGACY

#define MULTITHREADED


SDL_Surface* letters[LETTER_COUNT];

#define UINT64 unsigned long long
#define UINT unsigned int
#define UINT16 unsigned short
#define BYTE unsigned char
#define PTRSIZE sizeof(void*)


#define THREAD_COUNT 15
pthread_mutex_t fl_mutex[THREAD_COUNT];
Network xor_net[THREAD_COUNT];
SDL_Surface * thread_surf[THREAD_COUNT];
char* thread_args[THREAD_COUNT];
char* temp_buffer;
#include<immintrin.h>

inline int horizontal_sum(__m256i v)
{
	__m128i lo = _mm256_castsi256_si128(v);
	__m128i hi = _mm256_extracti128_si256(v, 1);

	__m128i sum = _mm_add_epi32(lo, hi);     // 4 values
	sum = _mm_hadd_epi32(sum, sum);           // 2 values
	sum = _mm_hadd_epi32(sum, sum);           // 1 value

	return _mm_cvtsi128_si32(sum);
}

size_t __fl_xor_line(int* a_line, int* b_line, size_t w) {
	size_t YIMM_size = w >> 3, res = 0;
	__m256i YIMM_a, YIMM_b, YIMM_ZERO = _mm256_setzero_si256(), YIMM_res = _mm256_setzero_si256(),
		YIMM_one = _mm256_set1_epi32(1), YIMM_temp, YIMM_FULL_ONE = _mm256_set1_epi8(0xFF);
	__m128i XIMM_a, XIMM_b, XIMM_ZERO = _mm_setzero_si128(), XIMM_res = _mm_setzero_si128(),
		XIMM_one = _mm_set1_epi32(1), XIMM_temp, XIMM_FULL_ONE = _mm_set1_epi8(0xFF);
	int* int_ptr;
	UINT64* ui64_ptr;
	if (YIMM_size) {
		while (YIMM_size--) {
			YIMM_a = _mm256_loadu_si256((const __m256i*)a_line);
			YIMM_b = _mm256_loadu_si256((const __m256i*)b_line);

			YIMM_temp = _mm256_xor_si256(YIMM_a, YIMM_b);
			YIMM_temp = _mm256_cmpeq_epi32(YIMM_temp, YIMM_ZERO);
			YIMM_temp = _mm256_xor_si256(YIMM_temp, YIMM_FULL_ONE);
			YIMM_temp = _mm256_and_si256(YIMM_temp, YIMM_one);
			YIMM_res = _mm256_add_epi32(YIMM_res, YIMM_temp);
			a_line += 8;
			b_line += 8;
		}
		res += horizontal_sum(YIMM_res);
	}
	if (w & 4) {
		XIMM_a = _mm_loadu_si128((const __m128i*)a_line);
		XIMM_b = _mm_loadu_si128((const __m128i*)b_line);

		XIMM_temp = _mm_xor_si128(XIMM_a, XIMM_b);
		XIMM_temp = _mm_cmpeq_epi32(XIMM_temp, XIMM_ZERO);
		XIMM_temp = _mm_xor_si128(XIMM_temp, XIMM_FULL_ONE);
		XIMM_temp = _mm_and_si128(XIMM_temp, XIMM_one);
		XIMM_res = _mm_add_epi32(XIMM_res, XIMM_temp);
		int_ptr = (int*)&XIMM_res;
		ui64_ptr= (UINT64*)&XIMM_res;
		*ui64_ptr += ui64_ptr[1];
		res += int_ptr[0] + int_ptr[1] ;
		a_line += 4;
		b_line += 4;
	}

	w = w & 3;
	while (w--) {
		res += (*a_line ^ *b_line) != 0;
		a_line++;
		b_line++;
	}
	return res;
}




size_t __fl_xor_line_padded(int* a_line, int* b_line, size_t w) {
	size_t YIMM_size = w >> 3, res = 0;
	__m256i YIMM_a, YIMM_b, YIMM_ZERO = _mm256_setzero_si256(), YIMM_res = _mm256_setzero_si256(),
		YIMM_one = _mm256_set1_epi32(1), YIMM_temp, YIMM_FULL_ONE = _mm256_set1_epi8(0xFF);
	int* int_ptr;
	while (YIMM_size--) {
		YIMM_a = _mm256_loadu_si256((const __m256i*)a_line);
		YIMM_b = _mm256_loadu_si256((const __m256i*)b_line);
		YIMM_temp = _mm256_xor_si256(YIMM_a, YIMM_b);
		YIMM_temp = _mm256_cmpeq_epi32(YIMM_temp, YIMM_ZERO);
		YIMM_temp = _mm256_xor_si256(YIMM_temp, YIMM_FULL_ONE);
		YIMM_temp = _mm256_and_si256(YIMM_temp, YIMM_one);
		YIMM_res = _mm256_add_epi32(YIMM_res, YIMM_temp);
		a_line += 8;
		b_line += 8;
	}
	res += horizontal_sum(YIMM_res);
	return res;
}

size_t __fl_xor_line_old(int* a_line, int* b_line, size_t w) {
	size_t YIMM_size = w >> 3, res = 0;
	__m256i YIMM_a, YIMM_b, YIMM_ZERO = _mm256_setzero_si256(), YIMM_res = _mm256_setzero_si256(),
		YIMM_one = _mm256_set1_epi32(1), YIMM_temp,YIMM_FULL_ONE= _mm256_set1_epi8(0xFF);
	__m128i XIMM_a, XIMM_b, XIMM_ZERO = _mm_setzero_si128(), XIMM_res = _mm_setzero_si128(),
		XIMM_one = _mm_set1_epi32(1), XIMM_temp,XIMM_FULL_ONE = _mm_set1_epi8(0xFF);
	int* int_ptr;
	if (YIMM_size) {
		while (YIMM_size--) {
			YIMM_a = _mm256_loadu_si256((const __m256i*)a_line);
			YIMM_b = _mm256_loadu_si256((const __m256i*)b_line);

			YIMM_temp = _mm256_xor_si256(YIMM_a, YIMM_b);
			YIMM_temp = _mm256_cmpeq_epi32(YIMM_temp, YIMM_ZERO);
			YIMM_temp = _mm256_xor_si256(YIMM_temp, YIMM_FULL_ONE);
			YIMM_temp = _mm256_and_si256(YIMM_temp, YIMM_one);
			YIMM_res = _mm256_add_epi32(YIMM_res, YIMM_temp);
			a_line += 8;
			b_line += 8;
		}
		int_ptr = (int*)&YIMM_res;
		res += int_ptr[0] + int_ptr[1] + int_ptr[2] + int_ptr[3] + int_ptr[4] + int_ptr[5] + int_ptr[6] + int_ptr[7];
	}
	if (w & 4) {
		XIMM_a = _mm_loadu_si128((const __m128i*)a_line);
		XIMM_b = _mm_loadu_si128((const __m128i*)b_line);

		XIMM_temp = _mm_xor_si128(XIMM_a, XIMM_b);
		XIMM_temp =_mm_cmpeq_epi32(XIMM_temp, XIMM_ZERO);
		XIMM_temp = _mm_xor_si128(XIMM_temp, XIMM_FULL_ONE);
		XIMM_temp = _mm_and_si128(XIMM_temp, XIMM_one);
		XIMM_res = _mm_add_epi32(XIMM_res, XIMM_temp);
		int_ptr = (int*)&XIMM_res;
		res += int_ptr[0] + int_ptr[1] + int_ptr[2] + int_ptr[3];
		a_line += 4;
		b_line += 4;
	}

	w = w & 3;
	while (w--) {
		res += (*a_line ^ *b_line)!=0;
		a_line++;
		b_line++;
	}
	return res;
}





struct SDL_SurfaceDouble{
	SDL_Surface* data[2];

};



int find_letter(SDL_Surface* surf, SDL_Surface* temp,struct letter_pos* letter, int grid_mode) {

	if ((letter->w < 5 || letter->h < 8)) {
		if (letter->w < 3 || letter->h < letter->w * 4 ||
			(letter->w < 10 && letter->h < 15 && grid_mode)) {
			letter->letter = '!';
			//puts("No letter found special:cond");
			return 0;
		}


		//check if it is near to i.bmp
		SDL_Rect dst_rect, src_rect;
		float min_percent = 3.40E38;
		dst_rect.x = 0;
		dst_rect.y = 0;

		src_rect.x = letter->x;
		src_rect.y = letter->y;
		src_rect.w = letter->w;
		src_rect.h = letter->h;

		//SDL_Surface* temp = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
		//	0xFF0000, 0xFF00, 0xFF);
		int* surf_pixels,
			* dst_pixels = (int*)temp->pixels;

		int last_chance[6] = { 8,48,67 ,69,70,71 };
		for (int i = 0; i != 6; i++) {
			int p = last_chance[i];
			dst_rect.w = letters[p]->w;
			dst_rect.h = letters[p]->h;
			SDL_UpperBlitScaled(surf, &src_rect, temp, &dst_rect);
			size_t xor_sum = 0;
			size_t w = letters[p]->w;
			surf_pixels = (int*)letters[p]->pixels;
			for (size_t y = 0; y != letters[p]->h; y++) {
				for (size_t x = 0; x != w; x++) {
					dst_pixels[y * 256 + x] ^= surf_pixels[y * w + x];
					xor_sum += dst_pixels[y * 256 + x] != 0;
				}
			}

			size_t size = dst_rect.w * dst_rect.h;
			float percent = size;
			percent = xor_sum / percent;
			if (min_percent > percent) {
				min_percent = percent;
			}

		}
		//SDL_FreeSurface(temp);
		if (min_percent < 0.1f) {
			letter->letter = 'I';
			//puts("letter->letter: I");
			return 1;
		}
		letter->letter = '!';
		//puts("No letter found");
		return 0;

	}



	//SDL_Surface* temp = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
	//	0xFF0000, 0xFF00, 0xFF);

	if (temp == NULL) {
		return -1;
	}
	SDL_Rect dst_rect, src_rect;

	dst_rect.x = 0;
	dst_rect.y = 0;

	size_t xor_sum, index;
	float percent, min_percent = 3.40E38;

	src_rect.x = letter->x;
	src_rect.y = letter->y;
	src_rect.w = letter->w;
	src_rect.h = letter->h;
	size_t w, px, py, h;

	int* dst_pixels = (int*)temp->pixels, * surf_pixels = (int*)surf->pixels;


	for (size_t i = 0; i != LETTER_COUNT; i++) {
		w = letters[i]->w;
		h = letters[i]->h;
		dst_rect.w = w;
		dst_rect.h = h;
		SDL_UpperBlitScaled(surf, &src_rect, temp, &dst_rect);
		surf_pixels = (int*)letters[i]->pixels;
		//xor_sum = sum_xor(temp, letters[i]);
		xor_sum = 0;
		for (size_t y = 0; y != letters[i]->h; y++) {
			for (size_t x = 0; x != w; x++) {
				dst_pixels[y * 256 + x] ^= surf_pixels[y * w + x];
				xor_sum += dst_pixels[y * 256 + x] != 0;
			}
			//printf("xor_sum: %zu  AVX2: %zu\n", xor_sum, __fl_xor_line(dst_pixels+y*256, surf_pixels+y*w,w));
		}
		
		size_t size = dst_rect.w * dst_rect.h;
		percent = size;
		percent = xor_sum / percent;
		if (percent < min_percent) {
			min_percent = percent;
			index = i;
		}
	}


	//printf("percent: %f\n", min_percent * 100.0);
	//SDL_FreeSurface(temp);
	if (min_percent < 0.25f) {
		char c = letters_index(index);
		letter->letter = c;
		if (c == '|') {
			return 2;
		}
		return 1;
	}
	else {
		letter->letter = '!';
		//puts("No letter found");
		return 0;
	}

}
/*
void* find_letter_mt(void* args) {
	char *carg = (char*)args;
	SDL_Surface* surf =*(SDL_Surface**) args;
	int* ret = (int*)(carg + PTRSIZE*2 );
	int surf_w = surf->w, surf_h = surf->h;
	int grid_mode = *ret;
	if ((surf_w < 5 || surf_h < 8)) {
		if (surf_w < 3 || surf_h < surf_w * 4 ||
			(surf_w < 10 && surf_h < 15 && grid_mode)) {
			*ret = '!';
			//puts("No letter found special:cond");
			return NULL;
		}


		//check if it is near to i.bmp
		SDL_Rect dst_rect, src_rect;
		float min_percent = 3.40E38;
		dst_rect.x = 0;
		dst_rect.y = 0;

		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.w = surf_w;
		src_rect.h = surf_h;

		SDL_Surface* temp = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
			0xFF0000, 0xFF00, 0xFF);
		int* surf_pixels,
			* dst_pixels = (int*)temp->pixels;

		int last_chance[6] = { 8,48,67 ,69,70,71 };
		for (int i = 0; i != 6; i++) {
			int p = last_chance[i];
			dst_rect.w = letters[p]->w;
			dst_rect.h = letters[p]->h;
			SDL_UpperBlitScaled(surf, &src_rect, temp, &dst_rect);
			size_t xor_sum = 0;
			size_t w = letters[p]->w;
			surf_pixels = (int*)letters[p]->pixels;
			for (size_t y = 0; y != letters[p]->h; y++) {
				for (size_t x = 0; x != w; x++) {
					dst_pixels[y * 256 + x] ^= surf_pixels[y * w + x];
					xor_sum += dst_pixels[y * 256 + x] != 0;
				}
			}

			size_t size = dst_rect.w * dst_rect.h;
			float percent = size;
			percent = xor_sum / percent;
			if (min_percent > percent) {
				min_percent = percent;
			}

		}
		SDL_FreeSurface(temp);
		if (min_percent < 0.1f) {
			*ret = 'I';
			//puts("letter->letter: I");
			return NULL;
		}
		*ret = '!';
		//puts("No letter found");
		return NULL;
	}



	SDL_Surface* temp = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
		0xFF0000, 0xFF00, 0xFF);

	if (temp == NULL) {
		exit(-1);
	}
	SDL_Rect dst_rect, src_rect;

	dst_rect.x = 0;
	dst_rect.y = 0;

	size_t xor_sum, index;
	float percent, min_percent = 3.40E38;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = surf_w;
	src_rect.h = surf_h;
	size_t w, px, py, h;

	int* dst_pixels = (int*)temp->pixels, * surf_pixels = (int*)surf->pixels;


	for (size_t i = 0; i != LETTER_COUNT; i++) {
		w = letters[i]->w;
		h = letters[i]->h;
		dst_rect.w = w;
		dst_rect.h = h;
		SDL_UpperBlitScaled(surf, &src_rect, temp, &dst_rect);

		surf_pixels = (int*)letters[i]->pixels;
		//xor_sum = sum_xor(temp, letters[i]);
		xor_sum = 0;
		for (size_t y = 0; y != letters[i]->h; y++) {
			for (size_t x = 0; x != w; x++) {
				dst_pixels[y * 256 + x] ^= surf_pixels[y * w + x];
				xor_sum += dst_pixels[y * 256 + x] != 0;
			}
		}
		size_t size = dst_rect.w * dst_rect.h;
		percent = size;
		percent = xor_sum / percent;
		if (percent < min_percent) {
			min_percent = percent;
			index = i;
		}
	}



	//printf("percent: %f\n", min_percent * 100.0);
	SDL_FreeSurface(temp);
	if (min_percent < 0.25f) {
		switch (index) {
		case 26:
			*ret  = 'I';
			break;
		case 27:
			*ret  = 'M';
			break;
		case 28:
			*ret  = 'Q';
			break;
		case 29:
			*ret  = 'R';
			break;
		case 30:
			*ret  = 'U';
			break;
		case 31:
			*ret  = 'H';
			break;
		case 32:
			*ret  = 'O';
			break;
		case 33:
			*ret  = 'B';
			break;
		case 34:
			*ret  = 'H';
			break;
		case 35:
			*ret  = 'O';
			break;
		case 36:
			*ret  = 'E';
			break;
		case 37:
			*ret  = 'N';
			break;
		case 38:
			*ret  = 'D';
			break;
		case 39:
			*ret  = 'L';
			break;
		case 40:
			*ret  = 'M';
			break;
		case 41:
			*ret  = 'M';
			break;
		case 42:
			*ret  = 'C';
			break;
		case 43:
			*ret  = 'G';
			break;
		case 44:
			*ret  = 'S';
			break;
		case 45:
			*ret  = 'R';
			break;
		case 46:
			*ret  = 'P';
			break;
		case 47:
			*ret  = 'W';
			break;
		case 48:
			*ret  = 'I';
			break;
		case 49:
			*ret  = 'A';
			break;
		case 50:
			*ret  = 'F';
			break;
		case 51:
			*ret  = 'M';
			break;
		case 52:
			*ret  = 'E';
			break;
		case 53:
			*ret  = 'M';
			break;
		case 54:
			*ret  = 'I';
			break;
		case 55:
			*ret  = 'J';
			break;
		case 56:
			*ret  = 'M';
			break;
		case 57:
			*ret  = 'T';
			break;
		case 58:
			*ret  = 'E';
			break;
		case 59:
			*ret  = 0xFF00;
			break;
		case 60:
			*ret  = 'M';
			break;
		case 61:
			*ret  = 'Z';
			break;
		case 62:
			*ret  = 'S';
			break;
		case 63:
			*ret  = 'X';
			break;
		case 64:
			*ret  = 'L';
			break;
		case 65:
			*ret  = 'A';
			break;
		case 66:
			*ret  = 'D';
			break;
		case 67:
			*ret  = 'I';
			break;
		case 68:
			*ret  = 'O';
			break;
		case 69:
			*ret  = 'I';
			break;
		case 70:
			*ret  = 'I';
			break;
		case 71:
			*ret  = 'I';
			break;
		case 72:
			*ret  = 'M';
			break;
		case 73:
			*ret  = 'N';
			break;
		case 74:
			*ret  = 'B';
			break;
		case 75:
			*ret  = 'H';
			break;
		case 76:
			*ret  = 'O';
			break;
		case 77:
			*ret  = 'N';
			break;
		case 78:
			*ret  = 'N';
			break;
		case 79:
			*ret  = 'M';
			break;
		case 80:
			*ret  = 'D';
			break;
		case 81:
			*ret  = 'M';
			break;
		case 82:
			*ret  = 'D';
			break;
		default:
			*ret  = 'A' + index;
			break;
		}
		//printf("letter->letter: %c\n", *ret);
		return NULL;
	}
	else {
		*ret = '!';
		//puts("No letter found");
	}
	return NULL;
}
*/

void *(*th_func_comp)(void*);


void* find_letter_mt2_classic(void* args) {
	char* carg = (char*)args;
	SDL_Surface* surf = *(SDL_Surface**)args;
	struct letter_pos* letter = *(struct letter_pos**)(carg + PTRSIZE);
	SDL_Surface* temp = *(SDL_Surface**)(carg + PTRSIZE * 2);
	int grid_mode = *(int*)(carg + PTRSIZE * 3);
	//printf("temp: %p\n", temp);
	//printf(";;x: %i y: %i w: %i h: %i\n", letter->x,letter->y, 
	//	letter->w,letter->h);
	size_t(*line_comp)(int*, int*, size_t);
	//printf("\nsurf: %p : %p ; letter: %p : %p \n", surf, args, letter, carg + PTRSIZE);
	if ((letter->w < 5 || letter->h < 8)) {
		if (letter->w < 3 || letter->h < letter->w * 4 ||
			(letter->w < 10 && letter->h < 15 && grid_mode)) {
			letter->letter = '!';
			//puts("No letter found special:cond");
			return (void*)0;
		}


		//check if it is near to i.bmp
		SDL_Rect dst_rect, src_rect;
		float min_percent = 3.40E38;
		dst_rect.x = 0;
		dst_rect.y = 0;

		src_rect.x = letter->x;
		src_rect.y = letter->y;
		src_rect.w = letter->w;
		src_rect.h = letter->h;


		int* surf_pixels,
			* dst_pixels = (int*)temp->pixels;

		int last_chance[6] = { 8,48,67 ,69,70,71 };
		for (int i = 0; i != 6; i++) {
			int p = last_chance[i];
			dst_rect.w = letters[p]->w;
			dst_rect.h = letters[p]->h;
			SDL_SoftStretch(surf, &src_rect, temp, &dst_rect);
			size_t xor_sum = 0;
			size_t w = letters[p]->w;
			surf_pixels = (int*)letters[p]->pixels;
			line_comp = w & 7 ? __fl_xor_line : __fl_xor_line_padded;
			for (size_t y = 0; y != letters[p]->h; y++) {
				xor_sum += line_comp(dst_pixels + y * 256, surf_pixels + y * w, w);
			}
			size_t size = dst_rect.w * dst_rect.h;
			float percent = size;
			percent = xor_sum / percent;
			if (min_percent > percent) {
				min_percent = percent;
			}

		}
		if (min_percent < 0.1f) {
			letter->letter = 'I';
			//puts("letter->letter: I");
			return (void*)1;
		}
		letter->letter = '!';
		//puts("No letter found");
		return 0;

	}



	//SDL_Surface* temp = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
	//	0xFF0000, 0xFF00, 0xFF);

	//if (temp == NULL) {
	//	return (void*)-1;
	//}
	SDL_Rect dst_rect, src_rect;

	dst_rect.x = 0;
	dst_rect.y = 0;

	size_t xor_sum, index;
	float percent, min_percent = 3.40E38;

	src_rect.x = letter->x;
	src_rect.y = letter->y;
	src_rect.w = letter->w;
	src_rect.h = letter->h;
	size_t w, px, py, h;
	int* dst_pixels = (int*)temp->pixels, * surf_pixels = (int*)surf->pixels;
	//printf("dst_pixels: %p\n", dst_pixels);

	for (size_t i = 0; i != LETTER_COUNT; i++) {
		w = letters[i]->w;
		h = letters[i]->h;
		surf_pixels =(int*) letters[i]->pixels;
		dst_rect.w = w;
		dst_rect.h = h;
		//printf("%lu huj %zu ; src_rect : {%i;%i;%i;%i} ; dst_rect : {%i;%i;%i;%i} ; surf %p ; temp %p\n", pthread_self(), i,
		//	src_rect.x, src_rect.y, src_rect.w, src_rect.h,
		//	dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h,
		//	surf, temp);
		// 
		//SDL_LockSurface(surf);
		//SDL_UnlockSurface(surf);
		//SDL_BlitScaled  SDL_LowerBlitScaled
		
		line_comp = w & 7 ? __fl_xor_line : __fl_xor_line_padded;


		SDL_SoftStretch(surf, &src_rect, temp, &dst_rect);
		
		 xor_sum = 0;
		for (size_t y = 0; y != h; y++) {
			xor_sum += line_comp(dst_pixels + y * 256, surf_pixels + y * w ,w);
			
		}
		//printf("%zu %zu\n", AVX2_sum, xor_sum);
		size_t size = dst_rect.w * dst_rect.h;
		percent = size;
		percent = xor_sum / percent;
		if (percent < min_percent) {
			min_percent = percent;
			index = i;
		}
	}
	
	if (min_percent < 0.25f) {
		char c = letters_index(index);
		letter->letter = c;
		if (c == '|') {
			return (void*)2;
		}
		return (void*)1;
	}
	else {
		letter->letter = '!';
		//puts("No letter found");
		return (void*)0;
	}

}

void* find_letter_mt2_neuronal(void* args) {
	char* carg = (char*)args;
	SDL_Surface* surf = *(SDL_Surface**)args;
	struct letter_pos* letter = *(struct letter_pos**)(carg + PTRSIZE);
	SDL_Surface* temp = *(SDL_Surface**)(carg + PTRSIZE * 2);
	int grid_mode = *(int*)(carg + PTRSIZE * 3);
	Network* nt = *(Network**)(carg + PTRSIZE * 3 + 4);

	//time_t start, end;
	//start = clock();

	if ((letter->w < 5 || letter->h < 8)) {
		if (letter->w < 3 || letter->h < letter->w * 4 ||
			(letter->w < 10 && letter->h < 15 && grid_mode)) {
			letter->letter = '!';
			//puts("No letter found special:cond");
			return (void*)0;
		}


		//check if it is near to i.bmp
		SDL_Rect dst_rect, src_rect;
		float min_percent = 3.40E38;
		dst_rect.x = 0;
		dst_rect.y = 0;

		src_rect.x = letter->x;
		src_rect.y = letter->y;
		src_rect.w = letter->w;
		src_rect.h = letter->h;


		int* surf_pixels,
			* dst_pixels = (int*)temp->pixels;

		int last_chance[6] = { 8,48,67 ,69,70,71 };
		for (int i = 0; i != 6; i++) {
			int p = last_chance[i];
			dst_rect.w = letters[p]->w;
			dst_rect.h = letters[p]->h;
			if (SDL_SoftStretch(surf, &src_rect, temp, &dst_rect)) {
				puts("SDL_SoftStretch()");
				puts(SDL_GetError());
				exit(80);
			}
			size_t w = letters[p]->w;
			size_t xor_sum = generate_xor_output_diff_SDL(nt, temp, letters[p]);

			size_t size = dst_rect.w * dst_rect.h;
			float percent = size;
			percent = xor_sum / percent;
			if (min_percent > percent) {
				min_percent = percent;
			}

		}
		if (min_percent < 0.1f) {
			letter->letter = 'I';
			
			return (void*)1;
		}
		letter->letter = '!';
		return 0;

	}
	SDL_Rect dst_rect, src_rect;

	dst_rect.x = 0;
	dst_rect.y = 0;

	size_t xor_sum, index;
	float percent, min_percent = 3.40E38;

	src_rect.x = letter->x;
	src_rect.y = letter->y;
	src_rect.w = letter->w;
	src_rect.h = letter->h;
	size_t w, px, py, h;
	index = 64564545;
	int* dst_pixels = (int*)temp->pixels, * surf_pixels = (int*)surf->pixels;

	for (size_t i = 0; i != LETTER_COUNT; i++) {
		w = letters[i]->w;
		h = letters[i]->h;
		surf_pixels = (int*)letters[i]->pixels;
		dst_rect.w = w;
		dst_rect.h = h;




		if (SDL_SoftStretch(surf, &src_rect, temp, &dst_rect)) {
			puts("SDL_SoftStretch()");
			puts(SDL_GetError());
			exit(80);
		}
		size_t xor_sum = generate_xor_output_diff_SDL(nt, temp, letters[i]);
		size_t size = dst_rect.w * dst_rect.h;
		percent = size;
		percent = xor_sum / percent;
		if (percent < min_percent) {
			min_percent = percent;
			index = i;
		}
	}

	//end = clock();
	//printf("time: %lums\n", end - start);

	if (min_percent < 0.25f) {
		char c = letters_index(index);
		letter->letter = c;
		if (c == '|') {
			return (void*)2;
		}
		return (void*)1;
	}
	else {
		letter->letter = '!';

		return (void*)0;
	}

}

void fill_white(SDL_Surface* image, int x, int y) {

	int h = image->h, w = image->w;
	int* pixels = (int*)image->pixels;
	pos_ll_t* linked_list = NULL;
	pos_ll_add(&linked_list, x, y);
	size_t pos;
	int temp_x, temp_y;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		pos = y * w + x;
		pixels[pos] = -1;
		//RIGHT
		temp_x = x + 1;
		pos++;
		if ((temp_x < w) && ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, temp_x, y);
		}
		//LEFT
		temp_x = x - 1;
		pos -= 2;
		if ((temp_x >= 0) && ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, temp_x, y);
		}

		//DOWN
		temp_y = y + 1;
		pos = temp_y * w + x;
		if ((temp_y < h) && ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, x, temp_y);
		}

		//UP
		temp_y = y - 1;
		pos = temp_y * w + x; if ((temp_y >= 0) && ISBLACK(pixels[pos])) {
			pos_ll_add(&linked_list, x, temp_y);
		}


	}
}


void bucket_fill(SDL_Surface* image, int x, int y, int dst_clr) {
	int h = image->h, w = image->w;
	int* pixels = (int*)image->pixels;
	pos_ll_t* linked_list = NULL;
	int src_clr = pixels[y * w + x];
	pos_ll_add(&linked_list, x, y);
	size_t pos;
	int temp_x, temp_y;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		temp_x = x;
		while (temp_x != -1 && pixels[y * w + temp_x] == src_clr) {
			pixels[y * w + temp_x] = dst_clr;
			temp_y = y + 1;
			if (temp_y != h) {
				pos = temp_y * w + temp_x;
				if (pixels[pos] == src_clr) {
					pos_ll_add(&linked_list, temp_x, temp_y);
				}
			}
			temp_y = y - 1;
			if (temp_y != -1) {
				pos = temp_y * w + temp_x;
				if (pixels[pos] == src_clr) {
					pos_ll_add(&linked_list, temp_x, temp_y);
				}
			}
			temp_x--;
		}
		temp_x = x + 1;
		while (temp_x != w && pixels[y * w + temp_x] == src_clr) {
			pixels[y * w + temp_x] = dst_clr;
			temp_y = y + 1;
			if (temp_y != h) {
				pos = temp_y * w + temp_x;
				if (pixels[pos] == src_clr) {
					pos_ll_add(&linked_list, temp_x, temp_y);
				}
			}
			temp_y = y - 1;
			if (temp_y != -1) {
				pos = temp_y * w + temp_x;
				if (pixels[pos] == src_clr) {
					pos_ll_add(&linked_list, temp_x, temp_y);
				}
			}
			temp_x++;
		}

	}

}


struct letter_pos_array find_all_letters(SDL_Surface* surf,
	SDL_Rect* rect, int* letter_count_x, int* letter_count_y) {
	size_t h = surf->h, w = surf->w;
	int* pixels = (int*)surf->pixels, * data, temp;
	struct letter_pos_array letters_posistion_array;
	memset(&letters_posistion_array, 0, sizeof(letters_posistion_array));
	struct letter_pos letter_position_rect;
	memset(&letter_position_rect, 0, sizeof(letter_position_rect));
	size_t pos;
	SDL_Surface* temp_surf = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
		0xFF0000, 0xFF00, 0xFF);
	if (!temp_surf)	exit(-1);
	char* temp_buffer = (char*)calloc(h, w);
	if (temp_buffer == NULL) {
		exit(-1);
	}
	int count_x, letter_present;

	int endy = rect->y + rect->h,
		endx = rect->x + rect->w;
	size_t length, end_pos;

	int max_y;
	for (int y = rect->y; y < endy; y++) {
		letter_present = 0;
		count_x = 0;
		for (int x = rect->x; x < endx; x++) {
			pos = y * w + x;
			data = pixels + pos;
			temp = *data;
			if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
				letter_position_rect = get_rect(pixels, temp_buffer, x, y, w, h);
				max_y = letter_position_rect.y + letter_position_rect.h / 2;

				length = letter_position_rect.w;
				letter_present = 1;
				end_pos = letter_position_rect.y + letter_position_rect.h;
				for (size_t py = letter_position_rect.y; py != end_pos; py++) {
					memset(temp_buffer + py * w + letter_position_rect.x
						, 0, length);
				}
				if (max_y > y) {
					y = max_y;
				}
			}
		}

		if (letter_present == 1) {
			for (int x = rect->x; x < endx; x++) {
				pos = y * w + x;
				data = pixels + pos;
				temp = *data;
				if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
					//printf("x: %i y: %i\n", x, y);
					letter_position_rect = get_rect(pixels, temp_buffer, x, y, w, h);
					//printf("letter_position_rect x: %i y: %i w: %i h: %i\n",
					//	letter_position_rect.x, letter_position_rect.y,
					//	letter_position_rect.w, letter_position_rect.h);
					letter_present = find_letter(surf, temp_surf, &letter_position_rect, 1);
					if (letter_position_rect.letter != '!') {
						letter_present = 1;
						count_x++;
						lpa_add(&letters_posistion_array, &letter_position_rect);
					}
					length = letter_position_rect.w;
					end_pos = letter_position_rect.y + letter_position_rect.h;
					fill_white(surf, x, y);
				}
			}
			if (letter_present) {
				*letter_count_x = count_x > *letter_count_x ? count_x : *letter_count_x;
				*letter_count_y++;
			}
			//getchar();
		}
	}
	SDL_FreeSurface(temp_surf);
	free(temp_buffer);
	return letters_posistion_array;
}



struct str {
	char* data;
	size_t size, capacity;
};
void str_add(struct str* arr, char c);
void str_add_cstrl(struct str* arr, const char* cstr, size_t cstr_len);
struct str find_all_letters_mt(SDL_Surface* surf,
	SDL_Rect* rect, int* letter_count_x, int* letter_count_y,int classic) {
	size_t h = surf->h, w = surf->w;
	int* pixels = (int*)surf->pixels, * data, temp;
	struct str ret;
	memset(&ret, 0, sizeof(ret));
	struct letter_pos letter_position_rect[THREAD_COUNT];
	struct letter_pos* lp_cur;
	pthread_t threads[THREAD_COUNT];
	memset(threads, 0, sizeof(threads));
	size_t thread_pos = 0;
	memset(letter_position_rect, 0, THREAD_COUNT * sizeof(*letter_position_rect));
	size_t pos;
	int count_x = 0, letter_present;
	int looped = 0;
	int endy = rect->y + rect->h,
		endx = rect->x + rect->w;
	size_t length, end_pos;
	char** args=thread_args;
	SDL_Surface* fl_temp;
	int letter_count_xs[THREAD_COUNT], lcx_max = 0;
	Network* ntw_p = xor_net;
	memset(letter_count_xs, 0, sizeof(letter_count_xs));
	for (int i = 0; i != THREAD_COUNT; i++) {
		fl_temp = thread_surf[i];
		memcpy(args[i], &surf, PTRSIZE);
		lp_cur = letter_position_rect + i;
		memcpy(args[i] + PTRSIZE, &lp_cur, PTRSIZE);
		memcpy(args[i] + PTRSIZE * 2, &fl_temp, PTRSIZE);
		memset(args[i] + PTRSIZE * 3, 0, 4);
		if (classic==0) {
			//if (pthread_mutex_init(fl_mutex + i, NULL)) {
			//	puts("pthread_mutex_init()");
			//	exit(-77);
			//}
			ntw_p = xor_net + i;
			memcpy(args[i] + PTRSIZE * 3 + 4, &ntw_p, PTRSIZE);
		}
	}
	size_t thread_ret = 0;
	int max_y;
	for (int y = rect->y; y < endy; y++) {
		letter_present = 0;
		count_x = 0;
		for (int x = rect->x; x < endx; x++) {
			pos = y * w + x;
			data = pixels + pos;
			temp = *data;
			if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
				letter_position_rect[thread_pos] = get_rect(pixels, temp_buffer, x, y, w, h);
				max_y = letter_position_rect[thread_pos].y +
					letter_position_rect[thread_pos].h / 2;
				length = letter_position_rect[thread_pos].w;
				letter_present = 1;
				end_pos = letter_position_rect[thread_pos].y +
					letter_position_rect[thread_pos].h;
				
				for (size_t py = letter_position_rect[thread_pos].y; py != end_pos; py++) {
					memset(temp_buffer + py * w + letter_position_rect[thread_pos].x
						, 0, length);
				}
				if (max_y > y) {
					y = max_y;
				}
			}
		}

		if (letter_present == 1) {
			for (int x = rect->x; x < endx; x++) {
				pos = y * w + x;
				data = pixels + pos;
				temp = *data;
				if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
					//printf("x: %i y: %i  ISBLACK: %i   temp_buffer[pos]: %i\n", x, y , ISBLACK(temp), temp_buffer[pos]);
					letter_position_rect[thread_pos] = get_rect(pixels, temp_buffer, x, y, w, h);
					//letter_position_rect[thread_pos].letter = '!';
					//printf("x: %i y: %i w: %i h: %i\n", letter_position_rect[thread_pos].x,
					//	letter_position_rect[thread_pos].y, letter_position_rect[thread_pos].w,
					//	letter_position_rect[thread_pos].h);
					letter_count_xs[thread_pos] = ++count_x;
					
					//printf("x:%i y:%i w:%i h:%i\n", letter_position_rect[thread_pos].x,
					//	letter_position_rect[thread_pos].y, letter_position_rect[thread_pos].w,
					//	letter_position_rect[thread_pos].h);

					//printf(";;surf: %p : %p ; letter_position_rect: %p : %p\n", surf, args[thread_pos],
					//	letter_position_rect+thread_pos, args[thread_pos] + PTRSIZE);
					//printf("%i ", thread_pos);

						if (pthread_create(threads + thread_pos, NULL, th_func_comp, args[thread_pos])) {
							puts("pthread_create()");
							exit(-50);
						}

					//system("pause");
					//printf("letter_position_rect x: %i y: %i w: %i h: %i\n",
					//	letter_position_rect.x, letter_position_rect.y,
					//	letter_position_rect.w, letter_position_rect.h);
					length = letter_position_rect[thread_pos].w;
					//end_pos = letter_position_rect[thread_pos].y + letter_position_rect[thread_pos].h;
					//fill_white(surf, x, y);
					thread_ret = 0;
					if (thread_pos == THREAD_COUNT - 1) {
						thread_pos = 0;
						looped = 1;
						//if (looped) {
							pthread_join(threads[thread_pos], (void**)&thread_ret);
							//printf("thread_ret: %i ; letter_count_xs[%i]: %c\n", thread_ret,thread_pos,
							//	letter_position_rect[thread_pos].letter);
							//if (classic == 0)	pthread_mutex_unlock(fl_mutex + thread_pos);
						//}
						//printf("tutaj %i\n", pthread_join(threads[thread_pos], NULL));
					}
					else {
						thread_pos++;
						if (looped) {
							pthread_join(threads[thread_pos], (void**)&thread_ret);
							//if (classic == 0)	pthread_mutex_unlock(fl_mutex + thread_pos);
							//printf("thread_ret: %i ; letter_count_xs[%i]: %c\n", thread_ret, thread_pos,
							//	letter_position_rect[thread_pos].letter);
						}
					}

					//
					if (letter_position_rect[thread_pos].letter != '!' && thread_ret ) {
						letter_present = 1;
						//printf("lcx_max: %i ; letter_count_xs[thread_pos] %i\n", lcx_max, letter_count_xs[thread_pos]);
						lcx_max = letter_count_xs[thread_pos] > lcx_max ? letter_count_xs[thread_pos] : lcx_max;
						//printf("x:%i y:%i w:%i h:%i ", letter_position_rect[thread_pos].x,
						//letter_position_rect[thread_pos].y, letter_position_rect[thread_pos].w,
						//letter_position_rect[thread_pos].h);
						//printf("letter: %c\n",letter_position_rect[thread_pos].letter);

						if (thread_ret == 1) {
							str_add(&ret, letter_position_rect[thread_pos].letter);
						}
						else if (thread_ret == 2) {
							str_add_cstrl(&ret, "LAA", 3);
						}

					}

				}
			}
			if (letter_present) {
				*letter_count_x = lcx_max > *letter_count_x ? lcx_max : *letter_count_x;
			}
		}
	}
	int special_pos;
	if (looped) {
		//SDL_FreeSurface(*(SDL_Surface**)(args[thread_pos] + PTRSIZE * 2));
		//free(args[thread_pos]);
		//if (classic == 0)	pthread_mutex_destroy(fl_mutex + thread_pos);
		for (int i = 1; i != THREAD_COUNT; i++) {
			special_pos = (i + thread_pos) % THREAD_COUNT;
			if (pthread_join(threads[special_pos], (void**)&thread_ret))	continue;
			//if(classic==0)	pthread_mutex_destroy(fl_mutex + special_pos);
			//SDL_FreeSurface(*(SDL_Surface**)(args[special_pos] + PTRSIZE * 2));
			//free(args[special_pos]);
			if (letter_position_rect[special_pos].letter != '!' && thread_ret) {
				//printf("x:%i y:%i w:%i h:%i ", letter_position_rect[thread_pos].x,
				//	letter_position_rect[thread_pos].y, letter_position_rect[thread_pos].w,
				//	letter_position_rect[thread_pos].h);
				//printf("letter: %c\n", letter_position_rect[thread_pos].letter);
				if (thread_ret == 1) {
					str_add(&ret, letter_position_rect[special_pos].letter);
				}
				else if (thread_ret == 2) {
					str_add_cstrl(&ret, "LAA", 3);
				}
			}
		}
	}
	else {
		for (int i = 0; i != THREAD_COUNT; i++) {
			if (i < thread_pos) {
				if (pthread_join(threads[i], (void**)&thread_ret))	continue;
				if (letter_position_rect[i].letter != '!' && thread_ret ) {
					//printf("x:%i y:%i w:%i h:%i ", letter_position_rect[thread_pos].x,
					//	letter_position_rect[thread_pos].y, letter_position_rect[thread_pos].w,
					//	letter_position_rect[thread_pos].h);
					//printf("letter: %c\n", letter_position_rect[thread_pos].letter);
					if (thread_ret == 1) {
						str_add(&ret, letter_position_rect[i].letter);
					}
					else if (thread_ret == 2) {
						str_add_cstrl(&ret, "LAA", 3);
					}
				}
			}
			//if (classic == 0)	pthread_mutex_destroy(fl_mutex + i);
			//SDL_FreeSurface(*(SDL_Surface**)(args[i] + PTRSIZE * 2));
			//free(args[i]);
		}
	}
	return ret;
}

void str_add(struct str* arr, char c) {
	if (arr->capacity == arr->size) {
		arr->capacity = arr->capacity * 2 + 1;
		arr->data = (char*)realloc(arr->data, arr->capacity);
		if (arr->data == NULL) {
			exit(-1);
		}
	}
	arr->data[arr->size] = c;
	arr->size++;
}
void str_add_cstr(struct str* arr, const char* cstr) {
	size_t cstr_len = strlen(cstr);
	if (arr->capacity < arr->size + cstr_len) {
		arr->capacity = arr->capacity * 2 + cstr_len;
		arr->data = (char*)realloc(arr->data, arr->capacity);
		if (arr->data == NULL) {
			exit(-1);
		}
	}
	memcpy(arr->data + arr->size, cstr, cstr_len);
	arr->size += cstr_len;
}

void str_add_cstrl(struct str* arr, const char* cstr, size_t cstr_len) {
	if (arr->capacity < arr->size + cstr_len) {
		arr->capacity = arr->capacity * 2 + cstr_len;
		arr->data = (char*)realloc(arr->data, arr->capacity);
		if (arr->data == NULL) {
			exit(-1);
		}
	}
	memcpy(arr->data + arr->size, cstr, cstr_len);
	arr->size += cstr_len;
}
struct str_arr {
	struct str* data;
	size_t size, capacity;
};
void str_arr_add(struct str_arr* arr, struct str* string) {
	if (arr->capacity == arr->size) {
		arr->capacity = arr->capacity * 2 + 1;
		arr->data = (struct str*)realloc(arr->data, sizeof(struct str) * arr->capacity);
		if (arr->data == NULL) {
			exit(-1);
		}
	}
	memcpy(arr->data + arr->size, string, sizeof(struct str));
	arr->size++;
}


struct str_arr find_all_letters_word_list(SDL_Surface* surf,
	SDL_Rect* rect) {
	size_t h = surf->h, w = surf->w;
	int* pixels = (int*)surf->pixels, * data, temp;
	struct str_arr ret;
	memset(&ret, 0, sizeof(ret));
	struct str str;
	SDL_Surface* temp_surf = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000,
		0xFF0000, 0xFF00, 0xFF);
	if (!temp_surf)	exit(-1);
	struct letter_pos letter_position_rect;
	size_t pos;

	char* temp_buffer = (char*)calloc(h, w);
	if (temp_buffer == NULL) {
		exit(-1);
	}
	int letter_present;

	int endy = rect->y + rect->h,
		endx = rect->x + rect->w;
	size_t length, end_pos;


	for (int y = rect->y; y < endy; y++) {
		letter_present = 0;
		for (int x = rect->x; x < endx; x++) {
			pos = y * w + x;
			data = pixels + pos;
			temp = *data;
			if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
				letter_position_rect = get_rect(pixels, temp_buffer, x, y, w, h);
				y = letter_position_rect.y + letter_position_rect.h / 2;
				length = letter_position_rect.w;
				letter_present = 1;
				end_pos = letter_position_rect.y + letter_position_rect.h;
				for (size_t py = letter_position_rect.y; py != end_pos; py++) {
					memset(temp_buffer + py * w + letter_position_rect.x
						, 0, length);
				}
				break;
			}
		}

		if (letter_present == 1) {
			memset(&str, 0, sizeof(str));
			for (int x = rect->x; x < endx; x++) {
				pos = y * w + x;
				data = pixels + pos;
				temp = *data;
				if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
					//printf("x: %i y: %i\n", x, y);
					letter_position_rect = get_rect(pixels, temp_buffer, x, y, w, h);
					//printf("letter_position_rect x: %i y: %i w: %i h: %i\n",
					//	letter_position_rect.x, letter_position_rect.y,
					//	letter_position_rect.w, letter_position_rect.h);
					letter_present = find_letter(surf, temp_surf, &letter_position_rect, 0);
					if (letter_position_rect.letter == '|' && letter_present == 2) {
						//To read properly uLAA
						str_add_cstr(&str, "LAA");
						letter_present = 1;

					}
					else if (letter_position_rect.letter != '!') {
						letter_present = 1;
						str_add(&str, letter_position_rect.letter);
					}
					else {
						letter_present = 0;
					}
					length = letter_position_rect.w;
					end_pos = letter_position_rect.y + letter_position_rect.h;
					fill_white(surf, x, y);
				}
			}
			if (str.size != 0) {
				str_add(&str, 0);
				str_arr_add(&ret, &str);
			}
		}
	}
	SDL_FreeSurface(temp_surf);
	free(temp_buffer);
	return ret;
}





struct str_arr find_all_letters_word_list_mt(SDL_Surface* surf,
	SDL_Rect* rect,int classic) {
	size_t h = surf->h, w = surf->w;
	int* pixels = (int*)surf->pixels, * data, temp;
	struct str_arr ret;
	memset(&ret, 0, sizeof(ret));
	struct str str;
	memset(&str, 0, sizeof(ret));
	size_t pos;
	int letter_present;
	struct letter_pos letter_position_rect[THREAD_COUNT];
	struct letter_pos* lp_cur;
	pthread_t threads[THREAD_COUNT];
	memset(threads, 0, sizeof(threads));
	size_t thread_pos = 0;
	memset(letter_position_rect, 0, THREAD_COUNT * sizeof(*letter_position_rect));
	int endy = rect->y + rect->h,
		endx = rect->x + rect->w;
	size_t length, end_pos;
	Network* ntw_p = xor_net;
	char** args = thread_args;
	SDL_Surface* fl_temp;
	int looped = 0;
	int special_pos;
	size_t thread_ret;
	int update_last_y=-1, lcx_max = 0;
	int last_y = 0;
	int thread_add_or_not[THREAD_COUNT];
	memset(thread_add_or_not, 0, sizeof(thread_add_or_not));
	*thread_add_or_not = -1;
	int taon_init = 0;

	
	for (int i = 0; i != THREAD_COUNT; i++) {
		fl_temp = thread_surf[i];
		memcpy(args[i], &surf, PTRSIZE);
		lp_cur = letter_position_rect + i;
		memcpy(args[i] + PTRSIZE, &lp_cur, PTRSIZE);
		memcpy(args[i] + PTRSIZE * 2, &fl_temp, PTRSIZE);
		memset(args[i] + PTRSIZE * 3, 0, 4);
		if (classic == 0) {
			//if (pthread_mutex_init(fl_mutex + i, NULL)) {
			//	puts("pthread_mutex_init()");
			//	exit(-77);
			//}
			ntw_p = xor_net + i;
			memcpy(args[i] + PTRSIZE * 3 + 4, &ntw_p, PTRSIZE);
		}
	}
	int max_y ;
	for (int y = rect->y; y < endy; y++) {
		letter_present = 0;
		for (int x = rect->x; x < endx; x++) {
			pos = y * w + x;
			data = pixels + pos;
			temp = *data;
			if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
				letter_position_rect[thread_pos] = get_rect(pixels, temp_buffer, x, y, w, h);
				max_y = letter_position_rect[thread_pos].y + letter_position_rect[thread_pos].h / 2;
				length = letter_position_rect[thread_pos].w;
				letter_present = 1;
				end_pos = letter_position_rect[thread_pos].y + letter_position_rect[thread_pos].h;
				for (size_t py = letter_position_rect[thread_pos].y; py != end_pos; py++) {
					memset(temp_buffer + py * w + letter_position_rect[thread_pos].x
						, 0, length);
				}
				if (max_y > y) {
					y = max_y;
				}
				//break;
			}
		}

		if (letter_present == 1) {
			for (int x = rect->x; x < endx; x++) {
				pos = y * w + x;
				data = pixels + pos;
				temp = *data;
				if (ISBLACK(temp) && (temp_buffer[pos] == 0)) {
					//printf("x: %i y: %i\n", x, y);
					letter_position_rect[thread_pos] = get_rect(pixels, temp_buffer, x, y, w, h);
					//letter_position_rect[thread_pos].letter = '!';
					//printf("x: %i y: %i w: %i h: %i\n", letter_position_rect[thread_pos].x,
					//	letter_position_rect[thread_pos].y, letter_position_rect[thread_pos].w,
					//	letter_position_rect[thread_pos].h);
					
					if (thread_add_or_not[0]==-1) {
						thread_add_or_not[0] = 1;
						last_y = y;
					}
					else {
						thread_add_or_not[thread_pos] = last_y == y;
						last_y = y;
					}

				
						if (pthread_create(threads + thread_pos, NULL, th_func_comp, args[thread_pos])) {
							puts("pthread_create()");
							exit(-50);
						}
				
						
					length = letter_position_rect[thread_pos].w;
					thread_ret = 0;
					char letter_c;
					if (thread_pos == THREAD_COUNT - 1) {
						thread_pos = 0;
						looped = 1;
						pthread_join(threads[thread_pos], (void**)&thread_ret);
						//if (classic == 0)	pthread_mutex_unlock(fl_mutex + thread_pos);
					}

					else {
						thread_pos++;
						if (looped) {
							pthread_join(threads[thread_pos], (void**)&thread_ret);
							//if (classic == 0)	pthread_mutex_unlock(fl_mutex + thread_pos);
						}
					}
					

					if (letter_position_rect[thread_pos].letter != '!' && thread_ret) {
						int thread_string_build_mode= thread_add_or_not[thread_pos];
						if (thread_string_build_mode) {
							if (thread_ret == 1) {
								str_add(&str, letter_position_rect[thread_pos].letter);
							}
							else if (thread_ret == 2) {
								str_add_cstrl(&str, "LAA", 3);
							}
						}
						else {
							if (str.size) {
								str_add(&str, 0);
								str_arr_add(&ret, &str);
								memset(&str, 0, sizeof(str));
							}
							if (thread_ret == 1) {
								str_add(&str, letter_position_rect[thread_pos].letter);
							}
							else if (thread_ret == 2) {
								str_add_cstrl(&str, "LAA", 3);
							}
						}
					}
				}
			}
		}
	}
	if (looped) {
		//SDL_FreeSurface(*(SDL_Surface**)(args[thread_pos] + PTRSIZE * 2));
		//free(args[thread_pos]);
		//if (classic == 0)	pthread_mutex_destroy(fl_mutex + thread_pos);
		for (int i = 1; i != THREAD_COUNT; i++) {
			special_pos = (i + thread_pos) % THREAD_COUNT;
			pthread_join(threads[special_pos], (void**)&thread_ret);
			//if (classic == 0)	pthread_mutex_destroy(fl_mutex + special_pos);
			//SDL_FreeSurface(*(SDL_Surface**)(args[special_pos] + PTRSIZE * 2));
			//free(args[special_pos]);
			if (letter_position_rect[special_pos].letter != '!' && thread_ret) {
				int thread_string_build_mode = thread_add_or_not[special_pos];
				if (thread_string_build_mode) {
					if (thread_ret == 1) {
						str_add(&str, letter_position_rect[special_pos].letter);
					}
					else if (thread_ret == 2) {
						str_add_cstrl(&str, "LAA", 3);
					}
				}
				else {
					if (str.size) {
						str_add(&str, 0);
						str_arr_add(&ret, &str);
						memset(&str, 0, sizeof(str));
					}
					if (thread_ret == 1) {
						str_add(&str, letter_position_rect[special_pos].letter);
					}
					else if (thread_ret == 2) {
						str_add_cstrl(&str, "LAA", 3);
					}
				}
			}

		}
	}
	else {
		for (size_t i = 0; i != THREAD_COUNT; i++) {
			if (i < thread_pos) {
				if (pthread_join(threads[i], (void**)&thread_ret) == 0) {
					if (letter_position_rect[i].letter != '!' && thread_ret) {
						int thread_string_build_mode = thread_add_or_not[i];
						if (thread_string_build_mode) {
							if (thread_ret == 1) {
								str_add(&str, letter_position_rect[i].letter);
							}
							else if (thread_ret == 2) {
								str_add_cstrl(&str, "LAA", 3);
							}
						}
						else {
							if (str.size) {
								str_add(&str, 0);
								str_arr_add(&ret, &str);
								memset(&str, 0, sizeof(str));
							}
							if (thread_ret == 1) {
								str_add(&str, letter_position_rect[i].letter);
							}
							else if (thread_ret == 2) {
								str_add_cstrl(&str, "LAA", 3);
							}
						}
					}
				}
			}
			//if (classic == 0)	pthread_mutex_destroy(fl_mutex + i);
			
			//SDL_FreeSurface(*(SDL_Surface**)(args[i] + PTRSIZE * 2));
			//free(args[i]);
		}
	}
	if (str.size) {
		str_add(&str, 0);
		str_arr_add(&ret, &str);
		memset(&str, 0, sizeof(str));
	}
	return ret;
}



void find_grid(SDL_Surface* image, SDL_Rect* grid_rect, int* start_x) {
	int h = image->h, w = image->w;
	int* pixels = (int*)image->pixels,
		pixel;
	printf("Img  w: %i g: %i\n", w, h);
	int x, y, last_x = 0;
	memset(grid_rect, 0, sizeof(SDL_Rect));
	//this loop finds the most upward and leftward  black pixel
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			pixel = pixels[y * w + x];
			if (ISBLACK(pixel)) {
				*start_x = x;
				grid_rect->x = x;
				grid_rect->y = y;
				y = h;
				x = w;//Break out of the loop
			}
		}
	}
	y = grid_rect->y;
	x = grid_rect->x;
	//this loop will find the last black pixel on the right of the same y level
	last_x = grid_rect->x;
	for (; x < w; x++) {
		pixel = pixels[y * w + x];
		if (ISBLACK(pixel)) {
			last_x = x;
		}
		if (y < h - 1) {
			pixel = pixels[(y + 1) * w + x];
			if (ISBLACK(pixel)) {
				last_x = x;
			}
		}
		if (y < h - 2) {
			pixel = pixels[(y + 2) * w + x];
			if (ISBLACK(pixel)) {
				last_x = x;
			}
		}
	}
	grid_rect->w = last_x - grid_rect->x + 1;
	//}
	//else {
	//	last_x = w - 1;
	//	grid_rect->w = w - last_x + 1;
	//}
	x = grid_rect->x;
	int last_y = y, found;
	//this loop will compute the height of the grid
	for (; y < h; y++) {
		found = 0;

		for (int px = x; px < last_x; px++) {
			pixel = pixels[y * w + px];
			if (ISBLACK(pixel)) {
				found = 1;
				break;
			}
		}
		if (found == 1) {
			last_y = y;
			//check if there any pixels to the left that are black if yes
			//then update the rectangle horizontal info
			while ((x > 0) &&
				ISBLACK(pixels[y * w + x - 1])) {
				grid_rect->w++;
				x--;
			}
			while ((last_x < w - 1) &&
				ISBLACK(pixels[y * w + last_x + 1])) {
				grid_rect->w++;
				last_x++;
			}
		}

	}

	grid_rect->h = last_y - grid_rect->y + 1;
	grid_rect->x = x;

}




#ifdef SCANLINE
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
	//printf("tx: %i ty: %i\n", tx, ty);
	tx = x; ty = y;

	char* G_temp_buffer = temp_buffer;
	SDL_Rect rect;
	char* temp_buffer = G_temp_buffer+w*h;
	int px, py, temp_x, temp_y = 0;
	int first_x = x, first_y = y,
		last_x = x, last_y = y;
	int add_bottom, add_top;
	int cond;
	while (linked_list != NULL) {
		pos_ll_pop(&linked_list, &x, &y);
		temp_x = x;
		add_bottom = 1;
		add_top = 1;
		cond = !temp_buffer[y * w + x] && ISBLACK(pixels[y * w + x]);
		while (temp_x != -1 && !temp_buffer[y * w + temp_x]
			&& ISBLACK(pixels[y * w + temp_x])) {
			pos = y * w + temp_x;
			first_x = first_x > temp_x ? temp_x : first_x;
			temp_buffer[pos] = 1;
			temp_y = y + 1;
			if (temp_y != h) {
				pos = temp_y * w + temp_x;
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
				pos = temp_y * w + temp_x;
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
			while (temp_x != w && !temp_buffer[y * w + temp_x]
				&& ISBLACK(pixels[y * w + temp_x])) {
				pos = y * w + temp_x;
				last_x = last_x < temp_x ? temp_x : last_x;
				temp_buffer[pos] = 1;
				temp_y = y + 1;
				if (temp_y != h) {
					pos = temp_y * w + temp_x;
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
					pos = temp_y * w + temp_x;
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
	rect.x = first_x;
	rect.y = first_y;
	rect.w = last_x - first_x + 1;
	rect.h = last_y - first_y + 1;

	printf("rect x: %i y: %i w: %i h: %i\n", rect.x, rect.y, rect.w, rect.h);

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
			temp_x = x;
			add_bottom = 1;
			add_top = 1;
			cond = ISBLACK(pixels[y * w + x]);
			while (temp_x != -1 && ISBLACK(pixels[y * w + temp_x])) {
				pos = y * w + temp_x;
				first_x = first_x > temp_x ? temp_x : first_x;
				pixels[pos] = -1;
				temp_y = y + 1;
				if (temp_y != h) {
					pos = temp_y * w + temp_x;
					if (ISBLACK(pixels[pos])) {
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
					pos = temp_y * w + temp_x;
					if (ISBLACK(pixels[pos])) {
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
				while (temp_x != w && ISBLACK(pixels[y * w + temp_x])) {
					pos = y * w + temp_x;
					last_x = last_x < temp_x ? temp_x : last_x;
					pixels[pos] = -1;
					temp_y = y + 1;
					if (temp_y != h) {
						pos = temp_y * w + temp_x;
						if (ISBLACK(pixels[pos])) {
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
						pos = temp_y * w + temp_x;
						if (ISBLACK(pixels[pos])) {
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
		return 1;
	}
	else {
		return 0;
	}
}
#endif

//20 bytes for args + 
//Network create_network(int num_inputs, int num_hidden, int num_outputs, double lr);
void* create_network_mt2(void* args) {
	Network* n = (Network*)args;
	network_read(n, "res/xor.nw");
	for (int i = 1; i != THREAD_COUNT; i++)
		clone_network(n,n+i);
	return NULL;
}

void* free_network_mt2(void* args) {
	Network* n = (Network*)args;
	for (size_t i = 0; i != THREAD_COUNT; i++)
		free_network(n+i);
	return NULL;
}

int letter_init = 0;

void* build_letters_mt2(void* args) {
	SDL_Surface* nu_surface;
	SDL_Surface** let = (SDL_Surface**)args;
	for (size_t i = 0; i != LETTER_COUNT; i++) {
		let[i] = SDL_LoadBMP(letters_files[i]);
		if (letters[i] == NULL) {
			printf("errindex : %i %s\n", i, letters_files[i]);
			exit(-108);
		}
		nu_surface = SDL_ConvertSurfaceFormat(letters[i], SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET);
		if (nu_surface == NULL) {
			exit(-1);
		}
		SDL_FreeSurface(letters[i]);
		letters[i] = nu_surface;
	}
	return NULL;
}


void* init_surface_thread(void* a) {
	SDL_Surface** surfs = (SDL_Surface **)a;


	for (size_t i = 0; i != THREAD_COUNT; i++) {
		surfs[i]= SDL_CreateRGBSurfaceWithFormat(0, 256, 256, 32, SDL_PIXELFORMAT_ABGR32);
		if (!surfs[i])	exit(-1);
		thread_args[i] = (char*)malloc(PTRSIZE * 4 + 4);
		if (!thread_args[i])	exit(-1);
	}

	return NULL;
}
/*
struct itb_arg {
	int w, h;
	char** temp_buffer;
};


void* init_temp_buffer(void* a) {
	 struct itb_arg* tb_ptr = (struct itb_arg*)a;


	(*tb_ptr->temp_buffer) = (char*)calloc(tb_ptr->h*2, tb_ptr->w);
	if (!(*tb_ptr->temp_buffer)) {
		exit(-1);
	}
	return NULL;
}
*/

struct str_arr get_grid(SDL_Surface* image,int classic,int* letter_count_x,struct str* grid) {


	

	SDL_Rect grid_rect;
	int grid_start_x;
	clock_t start, end;
	pthread_t thr[THREAD_COUNT];

	start = clock();
	



	temp_buffer = (char*)calloc(image->h * 2, image->w);


	//struct itb_arg t_itb_arg;
	//t_itb_arg.w = image->w;
	//t_itb_arg.h = image->h;
	//t_itb_arg.temp_buffer = &temp_buffer;
	//if (!(*tb_ptr->temp_buffer)) {
	//	exit(-1);
	//}
	
	
	//if (pthread_create(thr + 1, NULL, init_temp_buffer, (void*)&t_itb_arg)) {
	//	exit(-80);
	//}



	find_grid(image, &grid_rect, &grid_start_x);


	
	if (letter_init == 0) {
		th_func_comp = classic ? find_letter_mt2_classic : find_letter_mt2_neuronal;
		if (pthread_create(thr + 4, NULL, build_letters_mt2, (void*)letters)) {
			exit(-80);
		}
		//letter_init = 1;
		if (pthread_create(thr + 2, NULL, init_surface_thread, (void*)thread_surf)) {
			exit(-80);
		}
		if (pthread_create(thr + 3, NULL, create_network_mt2, xor_net)) {
			exit(-80);
		}
	}
		


	printf("Found grid :  x : %i y :  %i w : %i h : %i\n",
		grid_rect.x, grid_rect.y, grid_rect.w, grid_rect.h);

	if (classic == 0)	pthread_join(thr[3], NULL);
	//pthread_join(thr[1], NULL);


	int ret = remove_grid_lines(image, &grid_rect, grid_start_x, grid_rect.y);
	
	if (letter_init == 0) {
		pthread_join(thr[4], NULL);
		pthread_join(thr[2], NULL);
		letter_init = 1;
	}
	if (ret == -1) {
		puts("remove_grid_lines() error : incorrect image or pos");
		getchar();
		exit(EXIT_FAILURE);
	}
	if (ret == 1) { puts("Removed grid lines"); }

	int letter_count_y;

	
	*grid = find_all_letters_mt(image,
		&grid_rect, letter_count_x, &letter_count_y,classic);

	size_t max_word_length = *letter_count_x > letter_count_y ? *letter_count_x : letter_count_y;
	size_t y = grid_rect.y, h = grid_rect.h,
		w = grid_rect.w, x = grid_rect.x;
	int* pixels = (int*)image->pixels;
	pixels += y * image->w + x;
	//while (h--) {
	//	memset(pixels, 255, sizeof(int) * w);
	//	pixels += image->w;
	//}
	//Find wordlist
	int word_list_x = 0, word_start_x = 0;
	SDL_Rect word_list_rect;
	size_t img_w = image->w, img_h = image->h;
	word_list_rect.y = y + 3;
	word_list_rect.h = img_h - word_list_rect.y;
	if (x > img_w - x - w) {
		word_list_rect.x = 0;
		word_list_rect.w = x;
	}
	else {
		word_list_rect.x = x + w + 1;
		word_list_rect.w = img_w - word_list_rect.x;
	}
	struct str_arr word_list_arr = find_all_letters_word_list_mt(image,
		&word_list_rect,classic);
	//if (classic == 0) {
	//	if (pthread_create(thr + 3, NULL, free_network_mt2, xor_net)) {
	//		exit(-80);
	//	}
	//}

	//if (classic == 0)	pthread_join(thr[3], NULL);
	free(temp_buffer);
	end = clock();

	printf("%li microsecs\n\n\n", (end - start));
	
	return word_list_arr;
}


