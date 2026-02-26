#ifndef TRAIN_H
#define TRAIN_H
#include <SDL.h>
#include "neural.h"
#include "image.h"
extern Network sim_net;
typedef struct {
    int countA;
    int countB;
    int both;
    int either;
    double iou;
    double dice;
} OverlapStats;
void binarize(Image* img, unsigned char threshold);
OverlapStats compute_overlap_stats(const Image* a, const Image* b, unsigned char thr);
void train_on_images_xor(Network* net, Image* a, Image* b, int epochs);
void train_on_images_xor_SDL(Network* net, SDL_Surface* surf, SDL_Surface* comp, int epochs);
void train_on_images_and(Network* net, Image* a, Image* b, int epochs);
void train_on_images_and_SDL(Network* net, SDL_Surface* surf, SDL_Surface* comp, int epochs);
void train_similarity_from_metrics(Network* net, int epochs);
Image* generate_xor_output(Network* net, Image* a, Image* b);
Image* generate_and_output(Network* net, Image* a, Image* b);
double compute_similarity_from_metrics(Network* net, double iou, double dice);
Image* image_xor(const Image* a, const Image* b);
void normalize_image_auto(Image* img);
double neural_surface_similarity(SDL_Surface* a, SDL_Surface* b);
void train_similarity_surface(Network* net, int epochs);
void generate_xor_output_SDL(Network* net, SDL_Surface* a, SDL_Surface* b, SDL_Surface* ret);
size_t generate_xor_output_diff_SDL(Network* net, SDL_Surface* surf, SDL_Surface* comp);
void generate_xor_output_SDL_special(Network* net, SDL_Surface* a, SDL_Surface* b, SDL_Surface* ret);
#endif