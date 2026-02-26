#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *data;
} Image;


Image *load_bmp(const char *filename);
int save_bmp(const char *filename, const Image *img);
void grayscale(Image *img);
void enhance_contrast(Image *img);
void blackwhite_local_contrast(Image *img, int block_size);
void median_filter(Image *img, int kernel_size);



#endif

