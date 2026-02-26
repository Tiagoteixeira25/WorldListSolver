#pragma once
#include"POSLL.h"
#include"skew_detector.h"
struct letter_pos get_rect(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h);
void get_rect_special_top(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h, struct skew_info* info);
void get_rect_special_bot(int* pixels, char* temp_buffer, int x, int y,
	int pixels_w, int pixels_h, struct skew_info* info);
