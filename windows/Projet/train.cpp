#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "train.h"
#include "neural.h"
#include <SDL.h>
Network sim_net;
static inline double normalize(unsigned char p) {
    return p / 255.0;
}

void binarize(Image* img, unsigned char threshold)
{
    if (!img || !img->data) return;

    int total = img->width * img->height;
    unsigned char* px = img->data;

    for (int i = 0; i < total; i++) {
        unsigned char r = px[0];
        unsigned char g = px[1];
        unsigned char b = px[2];

        unsigned char lum = (unsigned char)((r + g + b) / 3);
        unsigned char out = (lum >= threshold) ? 255 : 0;

        px[0] = px[1] = px[2] = out;
        px[3] = 255;

        px += 4;
    }
}


static inline unsigned char denormalize(double x) {
    if (x < 0) x = 0;
    if (x > 1) x = 1;
    return (unsigned char)(x * 255.0 + 0.5);
}
static inline int is_on(const unsigned char v, unsigned char thr) {
    return v >= thr;
}

OverlapStats compute_overlap_stats(const Image* a, const Image* b, unsigned char thr)
{
    OverlapStats s = { 0 };
    if (!a || !b || a->width != b->width || a->height != b->height) return s;

    int total = a->width * a->height;
    for (int i = 0; i < total; i++) {
        unsigned char av = a->data[i * 4 + 0];
        unsigned char bv = b->data[i * 4 + 0];
        int A = is_on(av, thr);
        int B = is_on(bv, thr);
        s.countA += A;
        s.countB += B;
        s.both += (A && B);
        s.either += (A || B);
    }

    if (s.either > 0) s.iou = (double)s.both / (double)s.either;
    if ((s.countA + s.countB) > 0) s.dice = (2.0 * s.both) / (double)(s.countA + s.countB);
    return s;
}
void train_on_images_xor(Network* net, Image* a, Image* b, int epochs) {
    if (!a || !b || a->width != b->width || a->height != b->height) {
        fprintf(stderr, "[XOR] Image size mismatch or missing images.\n");
        return;
    }

    int total_pixels = a->width * a->height;

    for (int e = 0; e < epochs; e++) {
        double total_error = 0.0;
        double avg_output = 0.0;

        for (int i = 0; i < total_pixels; i++) {
            unsigned char a_r = a->data[i * 4];
            unsigned char b_r = b->data[i * 4];

            double a_bin = a_r / 255.0;
            double b_bin = b_r / 255.0;

            double in[2] = { a_bin, b_bin };
            double target[1] = { ((a_bin > 0.5) != (b_bin > 0.5)) ? 1.0 : 0.0 };

            backprop_alpha(net, in, target, 1.0);

            double out = net->layers[2].neurons[0].output;
            avg_output += out;
            total_error += fabs(target[0] - out);
        }

        if (e % 500 == 0) {
            printf("[XOR] Epoch %5d | Avg Error: %.5f | Avg Out: %.3f\n",
                e, total_error / total_pixels, avg_output / total_pixels);
        }
    }
}
#include<pthread.h>
void train_on_images_xor_SDL(Network* net, SDL_Surface* surf, SDL_Surface* comp, int epochs) {
    
   
    int h = comp->h, w = comp->w;   


    unsigned char* surf_pixels = (unsigned char*)surf->pixels,
        *comp_pixels= (unsigned char*)comp->pixels;
    
    int total_pixels =w * h;

    const double MUL = 1.0 / 255.0;
    surf_pixels++;
    comp_pixels++;
    for (int e = 0; e < epochs; e++) {
        double total_error = 0.0;
        double avg_output = 0.0;

        for (int y = 0; y != h; y++) {
            for (int x = 0; x != w; x++) {
                unsigned int a_r = surf_pixels[(y * 256 + x)*4];
                unsigned int b_r = comp_pixels[(y * w + x) * 4];
                
                double a_bin = a_r * MUL;
                double b_bin = b_r * MUL;
                
                double in[2] = { a_bin, b_bin };
                double target[1] = { a_r^ b_r ? 1.0 : 0.0 };

                backprop_alpha(net, in, target, 1.0);
                
                double out = net->layers[2].neurons[0].output;

                avg_output += out;
                total_error += fabs(target[0] - out);
            }
        }   
        //if (e % 500 == 0) {
            //printf("%lu :[XOR] Epoch %5d | Avg Error: %.5f | Avg Out: %.3f\n", pthread_self(),
          //      e, total_error / total_pixels, avg_output / total_pixels);
        //}
    }

    //printf("[XOR] Total error %lf\n",100.0* total_error / total_pixels/ epochs);
}

void train_on_images_and(Network* net, Image* a, Image* b, int epochs) {
    if (!a || !b || a->width != b->width || a->height != b->height) {
        fprintf(stderr, "[AND] Image size mismatch or missing images.\n");
        return;
    }

    int total_pixels = a->width * a->height;

    for (int e = 0; e < epochs; e++) {
        double total_error = 0.0;
        double avg_output = 0.0;

        for (int i = 0; i < total_pixels; i++) {
            unsigned char a_r = a->data[i * 4 + 0];
            unsigned char b_r = b->data[i * 4 + 0];

            double a_bin = (a_r > 128) ? 1.0 : 0.0;
            double b_bin = (b_r > 128) ? 1.0 : 0.0;

            double in[2] = { a_bin, b_bin };
            double target[1] = { (a_bin > 0.5 && b_bin > 0.5) ? 1.0 : 0.0 };

            double weight = target[0] == 1.0 ? 10.0 : 1.0;

            backprop_alpha(net, in, target, weight);

            double out = net->layers[2].neurons[0].output;
            avg_output += out;
            total_error += fabs(target[0] - out);
        }

        if (e % 500 == 0) {
            printf("[AND] Epoch %5d | Avg Error: %.5f | Avg Out: %.3f\n",
                e, total_error / total_pixels, avg_output / total_pixels);
        }
    }
}


void train_on_images_and_SDL(Network* net, SDL_Surface* surf, SDL_Surface* comp, int epochs) {

    int h = comp->h, w = comp->w;
    int* surf_pixels = (int*)surf->pixels,
        * comp_pixels = (int*)comp->pixels;

    int total_pixels = w * h;

    const double MUL = 1.0 / 255.0;
   

    for (int e = 0; e < epochs; e++) {
        double total_error = 0.0;
        double avg_output = 0.0;

        for (int y = 0; y != h; y++) {
            for (int x = 0; x != w; x++) {
                unsigned int a_r = surf_pixels[y * 256 + x] & 0xff;
                unsigned int b_r = comp_pixels[y * w + x] & 0xff;

                double a_bin = a_r * MUL;
                double b_bin = b_r * MUL;

                double in[2] = { a_bin, b_bin };
                double target[1] = { ((a_r > 127) && (b_r > 127)) ? 1.0 : 0.0 };

                double weight = target[0] == 1.0 ? 10.0 : 1.0;

                backprop_alpha(net, in, target, weight);

                double out = net->layers[2].neurons[0].output;
                avg_output += out;
                total_error += fabs(target[0] - out);
            }
        }
        if (e % 500 == 0) {
            printf("%lu :[AND] Epoch %5d | Avg Error: %.5f | Avg Out: %.3f\n",pthread_self(),
                e, total_error / total_pixels, avg_output / total_pixels);
        }

    }
    //printf("[AND] Total error %lf%\n", 100.0 * total_error / total_pixels / epochs);
}

void train_similarity_from_metrics(Network* net, int epochs)
{
    double dataset[][3] = {
        {1.00, 1.00, 1.00},
        {0.85, 0.90, 0.90},
        {0.70, 0.80, 0.70},
        {0.50, 0.65, 0.50},
        {0.30, 0.45, 0.25},
        {0.10, 0.20, 0.05}
    };

    int n = sizeof(dataset) / sizeof(dataset[0]);

    for (int e = 0; e < epochs; e++) {
        double total_err = 0.0;
        for (int i = 0; i < n; i++) {
            double in[2] = { dataset[i][0], dataset[i][1] };
            double target[1] = { dataset[i][2] };
            backprop_alpha(net, in, target, 1.0);

            forward(net, in);
            double out = net->layers[2].neurons[0].output;
            total_err += fabs(target[0] - out);
        }
        if (e % 1000 == 0) {
            printf("[SIMNET] Epoch %5d | Avg Error: %.5f\n", e, total_err / n);
        }
    }
}

Image* generate_xor_output(Network* net, Image* a, Image* b) {
    if (!a || !b || a->width != b->width || a->height != b->height)
        return NULL;

    Image* out =(Image *) malloc(sizeof(Image));
    out->width = a->width;
    out->height = a->height;
    out->channels = 4;
    out->data =(unsigned char*) malloc(out->width * out->height * 4);

    int total_pixels = out->width * out->height;

    for (int i = 0; i < total_pixels; i++) {
        double in[2] = { normalize(a->data[i * 4 + 0]), normalize(b->data[i * 4 + 0]) };
        forward(net, in);
        double val = net->layers[2].neurons[0].output;
        unsigned char gray = denormalize(val);
        out->data[i * 4 + 0] = out->data[i * 4 + 1] = out->data[i * 4 + 2] = gray;
        out->data[i * 4 + 3] = 255;
    }
    return out;
}
void generate_xor_output_SDL(Network* net, SDL_Surface* a, SDL_Surface* b, SDL_Surface*ret) {
   

   

    unsigned char* a_pixels = (unsigned char*)a->pixels, *ret_pixels=(unsigned char*)ret->pixels,
        *b_pixels=(unsigned char*)b->pixels;
    a_pixels++;
    b_pixels++;
    int total_pixels = a->w * a->h;
    for (int i = 0; i < total_pixels; i++) {
        double in[2] = { normalize(a_pixels[i * 4]), normalize(b_pixels[i * 4]) };
        forward(net, in);
        double val = net->layers[2].neurons[0].output;
        unsigned int gray = denormalize(val);
        ret_pixels[i * 4+1] = ret_pixels[i * 4 + 2] = ret_pixels[i * 4 + 3] = gray;
        ret_pixels[i * 4] = 255;
    }
}
void generate_xor_output_SDL_special(Network* net, SDL_Surface* a, SDL_Surface* b, SDL_Surface* ret) {


    int w = b->w, h = b->h;

    unsigned char* a_pixels = (unsigned char*)a->pixels, * ret_pixels = (unsigned char*)ret->pixels,
        * b_pixels = (unsigned char*)b->pixels;
    for (int y = 0; y != h; y++) {
        for (int x = 0; x != w; x++) {
            double in[2] = { normalize(a_pixels[(y*256+x) * 4+1]), normalize(b_pixels[(y*w+x) * 4+1]) };
            forward(net, in);
            double val = net->layers[2].neurons[0].output;
            unsigned int gray = denormalize(val);
            ret_pixels[(y * 256 + x) * 4 + 1] = ret_pixels[(y * 256 + x) * 4 + 2] = ret_pixels[(y * 256 + x) * 4 + 3] = gray;
            ret_pixels[(y * 256 + x) * 4] = 255;
        }
    }
}


size_t generate_xor_output_diff_SDL(Network* net, SDL_Surface* surf, SDL_Surface* comp) {
    size_t ret = 0;
    unsigned char* s_pixels = (unsigned char*)surf->pixels, * c_pixels = (unsigned char*)comp->pixels;

    //s_pixels++;
    //c_pixels++;

    int w = comp->w, h = comp->h;

    /*
    for (int y = 0; y  != h; y++) {
        for (int x = 0; x != w; x++) {
            double in[2] = { normalize(s_pixels[(y * surf->w + x) * 4]), normalize(c_pixels[(y*w+x) * 4]) };
            forward(net, in);
            double val = net->layers[2].neurons[0].output;
            unsigned int gray = denormalize(val);
            //printf("%p %hhu %hhu %i\n", net, s_pixels[(y * surf->w + x) * 4+1], c_pixels[(y * w + x) * 4+1], gray);
            ret += gray;
        }
    }
    */
    for (int y = 0; y != h; y++) {
        for (int x = 0; x != w; x++) {
            double in[2] = { normalize(s_pixels[(y * 256 + x) * 4 + 1]), normalize(c_pixels[(y * w + x) * 4 + 1]) };

            forward(net, in);
            double val = net->layers[2].neurons[0].output;
            //printf("%p %lf %lf %lf\n", net, in[0], in[1],val);
           // unsigned int gray = denormalize(val);
            ret += val>0.5?1:0;
        }
    }

    return ret;
}


Image* generate_and_output(Network* net, Image* a, Image* b)
{
    if (!a || !b || a->width != b->width || a->height != b->height)
        return NULL;

    const unsigned char THR = 200;
    int total = a->width * a->height;

    Image* out =(Image*) malloc(sizeof(Image));
    out->width = a->width;
    out->height = a->height;
    out->channels = 4;
    out->data =(unsigned char*) malloc(total * 4);

    int white_out = 0;

    for (int i = 0; i < total; i++)
    {
        int A = (a->data[i * 4 + 0] >= THR);
        int B = (b->data[i * 4 + 0] >= THR);

        double in[2] = { A ? 1.0 : 0.0, B ? 1.0 : 0.0 };
        forward(net, in);

        unsigned char gray = (net->layers[2].neurons[0].output > 0.5) ? 255 : 0;
        white_out += (gray == 255);

        out->data[i * 4 + 0] = gray;
        out->data[i * 4 + 1] = gray;
        out->data[i * 4 + 2] = gray;
        out->data[i * 4 + 3] = 255;
    }



    return out;
}


double compute_similarity_from_metrics(Network* net, double iou, double dice)
{
    double in[2] = { iou, dice };
    forward(net, in);
    return net->layers[2].neurons[0].output;
}


void normalize_image_auto(Image* img)
{
    if (!img || !img->data) return;

    int total_pixels = img->width * img->height;
    double avg_brightness = 0.0;

    for (int i = 0; i < total_pixels; i++) {
        unsigned char* p = &img->data[i * 4];
        double lum = 0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2];
        avg_brightness += lum;
    }
    avg_brightness /= total_pixels;

    unsigned char threshold = (unsigned char)(avg_brightness * 0.8);

    for (int i = 0; i < total_pixels; i++) {
        unsigned char* p = &img->data[i * 4];
        unsigned char avg = (p[0] + p[1] + p[2]) / 3;
        unsigned char val = (avg >= threshold) ? 255 : 0;
        p[0] = p[1] = p[2] = val;
        p[3] = 255;
    }
}
static void compute_surface_features(SDL_Surface* a, SDL_Surface* b, double feat[5])
{
    SDL_Surface* fa = SDL_ConvertSurfaceFormat(a, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_Surface* fb = SDL_ConvertSurfaceFormat(b, SDL_PIXELFORMAT_RGBA32, 0);
    if (!fa || !fb) {
        if (fa) SDL_FreeSurface(fa);
        if (fb) SDL_FreeSurface(fb);
        for (int i = 0; i < 5; i++) feat[i] = 0.0;
        return;
    }

    int w = fa->w < fb->w ? fa->w : fb->w;
    int h = fa->h < fb->h ? fa->h : fb->h;
    const int total = w * h;
    const unsigned char thr = 128;

    Uint8* pa = (Uint8*)fa->pixels;
    Uint8* pb = (Uint8*)fb->pixels;
    int pitchA = fa->pitch;
    int pitchB = fb->pitch;

    Uint8* maskA =(Uint8*) malloc(total);
    Uint8* maskB = (Uint8*)malloc(total);
    if (!maskA || !maskB) {
        if (maskA) free(maskA);
        if (maskB) free(maskB);
        SDL_FreeSurface(fa);
        SDL_FreeSurface(fb);
        for (int i = 0; i < 5; i++) feat[i] = 0.0;
        return;
    }

    int countA = 0, countB = 0, both = 0, either = 0;

    for (int y = 0; y < h; y++) {
        Uint8* rowA = pa + y * pitchA;
        Uint8* rowB = pb + y * pitchB;
        for (int x = 0; x < w; x++) {
            Uint8* pA = rowA + x * 4;
            Uint8* pB = rowB + x * 4;

            Uint8 rA = pA[0], gA = pA[1], bA = pA[2];
            Uint8 rB = pB[0], gB = pB[1], bB = pB[2];

            Uint8 lumA = (Uint8)((rA + gA + bA) / 3);
            Uint8 lumB = (Uint8)((rB + gB + bB) / 3);

            Uint8 A = (lumA >= thr) ? 1 : 0;
            Uint8 B = (lumB >= thr) ? 1 : 0;

            int idx = y * w + x;
            maskA[idx] = A;
            maskB[idx] = B;

            countA += A;
            countB += B;
            both += (A && B);
            either += (A || B);
        }
    }

    double iou = (either > 0) ? (double)both / (double)either : 0.0;
    double dice = (countA + countB > 0) ? (2.0 * both) / (double)(countA + countB) : 0.0;
    double densA = (double)countA / (double)total;
    double densB = (double)countB / (double)total;

    double matchesVA = 0.0, pairsV = 0.0;
    double matchesVB = 0.0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w / 2; x++) {
            int idx1 = y * w + x;
            int idx2 = y * w + (w - 1 - x);

            matchesVA += (maskA[idx1] == maskA[idx2]);
            matchesVB += (maskB[idx1] == maskB[idx2]);
            pairsV += 1.0;
        }
    }
    double vSymA = (pairsV > 0.0) ? matchesVA / pairsV : 1.0;
    double vSymB = (pairsV > 0.0) ? matchesVB / pairsV : 1.0;

    double matchesHA = 0.0, pairsH = 0.0;
    double matchesHB = 0.0;
    for (int y = 0; y < h / 2; y++) {
        for (int x = 0; x < w; x++) {
            int idx1 = y * w + x;
            int idx2 = (h - 1 - y) * w + x;

            matchesHA += (maskA[idx1] == maskA[idx2]);
            matchesHB += (maskB[idx1] == maskB[idx2]);
            pairsH += 1.0;
        }
    }
    double hSymA = (pairsH > 0.0) ? matchesHA / pairsH : 1.0;
    double hSymB = (pairsH > 0.0) ? matchesHB / pairsH : 1.0;

    free(maskA);
    free(maskB);
    SDL_FreeSurface(fa);
    SDL_FreeSurface(fb);

    double symV_sim = 1.0 - fabs(vSymA - vSymB);
    double symH_sim = 1.0 - fabs(hSymA - hSymB);
    double dens_sim = 1.0 - fabs(densA - densB);

    if (symV_sim < 0.0) symV_sim = 0.0;
    if (symH_sim < 0.0) symH_sim = 0.0;
    if (dens_sim < 0.0) dens_sim = 0.0;

    feat[0] = iou;
    feat[1] = dice;
    feat[2] = symV_sim;
    feat[3] = symH_sim;
    feat[4] = dens_sim;
}
double neural_surface_similarity(SDL_Surface* a, SDL_Surface* b)
{
    if (!a || !b) return 0.0;

    double in[5];
    compute_surface_features(a, b, in);
    forward(&sim_net, in);

    Layer* out = &sim_net.layers[sim_net.num_layers - 1];
    double val = out->neurons[0].output;
    if (val < 0.0) val = 0.0;
    if (val > 1.0) val = 1.0;
    return val;
}
void train_similarity_surface(Network* net, int epochs)
{
    double dataset[][6] = {
        {1.00, 1.00, 1.00, 1.00, 1.00, 1.00},
        {0.90, 0.95, 0.95, 0.95, 0.95, 0.95},
        {0.80, 0.85, 0.90, 0.90, 0.90, 0.85},
        {0.60, 0.70, 0.80, 0.80, 0.80, 0.70},
        {0.40, 0.50, 0.60, 0.60, 0.60, 0.40},
        {0.20, 0.30, 0.40, 0.40, 0.40, 0.20},
        {0.05, 0.10, 0.20, 0.20, 0.20, 0.05}
    };

    int n = (int)(sizeof(dataset) / sizeof(dataset[0]));

    for (int e = 0; e < epochs; e++) {
        double total_err = 0.0;

        for (int i = 0; i < n; i++) {
            double in[5] = {
                dataset[i][0],
                dataset[i][1],
                dataset[i][2],
                dataset[i][3],
                dataset[i][4]
            };
            double target[1] = { dataset[i][5] };

            backprop_alpha(net, in, target, 1.0);

            forward(net, in);
            double out = net->layers[net->num_layers - 1].neurons[0].output;
            total_err += fabs(target[0] - out);
        }

        if (e % 1000 == 0) {
            printf("[SIM_SURF] Epoch %5d | Avg Error: %.5f\n", e, total_err / n);
        }
    }
}

