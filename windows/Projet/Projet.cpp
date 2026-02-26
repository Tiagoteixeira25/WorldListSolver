#include <SDL.h>
#include "initialisers.h"
#include "Letter_Finder.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"solver.cpp"
#include"skew_detector.h"
#include"graphic_solver.h"
#define PTRSIZE sizeof(void*)
#define REGSIZE sizeof(size_t)
#define SINT sizeof(int)
#define SCHAR sizeof(char)
#include"image.h"
#include"letters.h"

void grayscale(SDL_Surface* img)
{
    if (!img)
    {
        return;
    }

    int w = img->w, h = img->h;
    unsigned char* data =(unsigned char*) img->pixels;
    data++;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int i = (y * w + x) * 4;
            unsigned char red = data[i+2];
            unsigned char green = data[i+1];
            unsigned char blue = data[i];
            unsigned char gray = (unsigned char)(0.299 * red + 0.587 * green + 0.114 * blue);

            data[i+2] = gray;
            data[i+1] = gray;
            data[i] = gray;
        }
    }
}

void enhance_contrast(Image* img)
{
    if (!img || img->channels < 3)
    {
        return;
    }

    int total = img->width * img->height * img->channels;
    unsigned char min = 255;
    unsigned char max = 0;
    for (int i = 1; i < total; i += img->channels)
    {
        unsigned char val = img->data[i];
        if (val < min) min = val;
        if (val > max) max = val;
    }
    
    if (max <= min)
    {
        return;
    }
    
    double scale = 255.0 / (max - min);

    for (int i = 1; i < total; i+=4)
    {
        int val = (int)((img->data[i] - min) * scale);

        if (val < 0)
        {
            val = 0;
        }
        if (val > 255)
        {
            val = 255;
        }
        
        memset(img->data + i, val, 3);
       
    }
}

void blackwhite(Image* img, int adaptive_block)
{
    if (!img || img->channels < 3)
    {
        return;
    }

    int w = img->width, h = img->height, ch = img->channels;
    unsigned char* data = img->data;
    unsigned char* output =(unsigned char*) malloc(w * h * ch);
    if (!output)
    {
        return;
    }

    if (adaptive_block <= 0)
    {
        adaptive_block = 16;
    }

    for (int by = 0; by < h; by += adaptive_block)
    {
        for (int bx = 0; bx < w; bx += adaptive_block)
        {
            double sum = 0;
            int count = 0;
            for (int y = by; y < by + adaptive_block && y < h; y++)
            {
                for (int x = bx; x < bx + adaptive_block && x < w; x++)
                {
                    int i = (y * w + x) * ch;
                    sum += data[i+1];
                    count++;
                }
            }

            double mean = sum / count;
            double threshold = mean - 10;
            if (threshold < 40) threshold = 40;
            if (threshold > 220) threshold = 220;

            for (int y = by; y < by + adaptive_block && y < h; y++)
            {
                for (int x = bx; x < bx + adaptive_block && x < w; x++)
                {
                    int i = (y * w + x) * ch;
                    unsigned char val = data[i+1];
                    unsigned char bw = (val < threshold) ? 0 : 255;
                    output[i ] = 0xFF;
                    output[i + 1] = bw;
                    output[i + 2] = bw;
                    output[i + 3] = bw;
                }
            }
        }
    }
    memcpy(data, output, w * h * ch);
    free(output);
}
void median_filter(Image* img, int kernel_size)
{
    if (!img || img->channels < 3)
    {
        return;
    }

    int w = img->width, h = img->height, ch = img->channels;
    int radius = kernel_size / 2;

    unsigned char* filtered =(unsigned char*) malloc(w * h * ch);
    if (!filtered)
    {
        return;
    }
    unsigned char* window =(unsigned char*) malloc(kernel_size * kernel_size);
    if (!window)
    {
        free(filtered);
        return;
    }

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int count = 0;
            for (int dy = -radius; dy <= radius; dy++)
            {
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h)
                    {
                        int ni = (ny * w + nx) * ch;
                        window[count++] = img->data[ni+1];
                    }
                }
            }
            for (int i = 1; i < count; i++)
            {
                unsigned char key = window[i];
                int j = i - 1;
                while (j >= 0 && window[j] > key)
                {
                    window[j + 1] = window[j];
                    j--;
                }
                window[j + 1] = key;
            }

            unsigned char median = window[count / 2];
            int i = (y * w + x) * ch;
            filtered[i + 1] = filtered[i + 2] = filtered[i + 3] = median;
            filtered[i] = 0xFF;
        }
    }

    memcpy(img->data, filtered, w * h * ch);
    free(window);
    free(filtered);
}
void adjust_brightness(Image* img, int delta)
{
    if (!img || img->channels < 1)
    {
        return;
    }

    int w = img->width;
    int h = img->height;
    int ch = img->channels;
    int total = w * h * ch;
    double shift = delta * 1.5;

    for (int i = 0; i < total; i++)
    {
        double val = (double)img->data[i] + shift;

        if (val < 0.0) val = 0.0;
        if (val > 255.0) val = 255.0;

        img->data[i] = (unsigned char)val;
    }
}

extern int mouse_pos[2];
extern SDL_Texture *button[4];
extern SDL_Surface* letters[85];
static Uint32 get_pixel(SDL_Surface* surface, int x, int y) {
    Uint8* p = (Uint8*)surface->pixels + y* surface->pitch + x * surface->format->BytesPerPixel;
    Uint32 pixel;
    memcpy(&pixel, p, surface->format->BytesPerPixel);
    return pixel;
}
static void put_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
    Uint8* p = (Uint8*)surface->pixels + y* surface->pitch + x * surface->format->BytesPerPixel;
    memcpy(p, &pixel, surface->format->BytesPerPixel);
}
static void rotate_surface(SDL_Surface* src, double angle_deg, SDL_Surface* dst) {
    double angle = angle_deg * M_PI / 180.0;
    int w = src->w;
    int h = src->h;


    Uint32 white = SDL_MapRGB(dst->format, 255, 255, 255);
    SDL_FillRect(dst, NULL, white);

    double cx = w / 2.0,c= cos(angle),s= sin(angle);
    double cy = h / 2.0;

    double* x_arr = (double*)malloc(sizeof(double) * w*2),
        * y_arr = (double*)malloc(sizeof(double) * h*2),
        dx,dy,sx,sy;
    if (!x_arr || !y_arr) { return; }
    for (int y = 0; y < h; y++) {
        dy = y - cy;
        y_arr[y * 2] = s * dy;
        y_arr[y * 2 + 1] = c*dy;
    }
    for (int x = 0; x < w; x++) {
        dx = x - cx;
        x_arr[x * 2] = c * dx+cx;
        x_arr[x * 2+1] = -s * dx+cy;
     }

    SDL_LockSurface(src);
    SDL_LockSurface(dst);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++)
        {

            sx = x_arr[x * 2] + y_arr[y * 2];
            sy = x_arr[x * 2+1] + y_arr[y * 2+1];
            if (sx >= 0 && sx < w && sy >= 0 && sy < h)
            {
                Uint32 pixel = get_pixel(src, (int)sx, (int)sy);
                put_pixel(dst, x, y, pixel);
            }
        }
    }
    free(x_arr);
    free(y_arr);
    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dst);
}


#include<SDL_image.h>


#include<SDL_ttf.h>
#include"selection.h"

int SDL_main(int argc,char** argv){
    char* imgfln=NULL;
    int classic = 1;
    int choosed_file = 0;
    char* second_arg ;
    int flags = IMG_INIT_JPG | IMG_INIT_PNG ;
    if (IMG_Init(flags)!= flags) {
        puts("IMG_Init()");
        puts(IMG_GetError());
        exit(-555);
    }



    switch (argc) {
       //case 1:
            
            //imgfln = (char*)"2u.bmp";
       // break;
       case 2:
           second_arg = argv[1];
           if (strcmp(second_arg, "-neural") == 0) {
               classic = 0;
           } 
           else{
               imgfln = second_arg;
           }
         break;
       case 3:
           imgfln = argv[1];
           second_arg = argv[2];
           if (strcmp(second_arg, "-neural") == 0) {
               classic = 0;
           }
           else {
               fputs("Did you mean -neural ?", stderr);
               exit(EXIT_FAILURE);
           }
           break;
    }
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* image, * image2;
    SDL_Texture* texture;
    render_init(&window, &renderer,
        1280, 960);
    SDL_RenderClear(renderer);
    TTF_Init();


    
    if (imgfln == NULL) {
        load_selection(renderer);
        imgfln=draw_selection(renderer);
    }
    
    /*TTF_Font* font = TTF_OpenFont("ROBOTO.ttf", 128);
    
    SDL_Color fg = { 0,0,0 };

    for (int i = 0; i != 26; i++) {
        SDL_Surface* texte = TTF_RenderText_Blended(font, letter, fg);

        IMG_SavePNG(texte, save);
        save[16]++;
        letter[0]++;

        SDL_FreeSurface(texte);
    }
   
    TTF_CloseFont(font);*/
    //puts(imgfln);
    //int imgFlags = IMG_INIT_PNG;

    //if (!(IMG_Init(imgFlags) & imgFlags)){
     //   printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
    //}
   


    image = IMG_Load(imgfln);
    if (!image) {
        puts(SDL_GetError());
    }

    int img_w= image->w, img_h = image->h;
    texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture)    puts(SDL_GetError());
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);


    image2=SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);
    if (image2 == NULL) {
        puts(SDL_GetError());
    }
    SDL_FreeSurface(image);
    image = image2;



    image2= SDL_CreateRGBSurface(0, image->w, image->h, 32, 0xFF000000,
       0xFF0000, 0xFF00, 0xFF);
   
    SDL_Rect image_rect;
    image_rect.x = (1280 - 800) / 2;
    image_rect.y = 720 - 600;
    image_rect.w = 800;
    image_rect.h = 600;
  
    Image img;
    
    
    grayscale(image);
    img.data = (unsigned char*)image->pixels;
    img.width = image->w;
    img.height = image->h;
    img.channels = 4;
    median_filter(&img, 3);
    adjust_brightness(&img, 14);
    median_filter(&img, 3);
    
    enhance_contrast(&img);
    blackwhite(&img, 20);
    median_filter(&img, 3);

   
    double deg=find_skew(image);
    //deg=3.9;
    SDL_SaveBMP(image, "after.bmp");
    rotate_surface(image, deg, image2);
    SDL_FreeSurface(image);
    
    image = image2;
    SDL_SaveBMP(image, "after_rot.bmp");
    texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture)    puts(SDL_GetError());
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);
   

    //texture = SDL_CreateTextureFromSurface(renderer, image);
    //if (!texture)    puts(SDL_GetError());
    //SDL_RenderCopy(renderer, texture, NULL,NULL);
    //SDL_RenderPresent(renderer);
    int w, h;
    //SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    //SDL_DestroyTexture(texture);
   
    struct str grid;
    int letter_count_x=0;
    struct str_arr word_arr=get_grid(image, classic,&letter_count_x,&grid);

    FILE* grid_output = fopen("grid.txt", "w");
    if (grid_output == NULL) { exit(-50); }
    for (size_t i = 0; i < grid.size; i += letter_count_x) {
        fwrite(grid.data + i, letter_count_x, 1, grid_output);
        if (i != grid.size - 1) {
            fwrite("\n", 1, 1, grid_output);
        }
    }
    fclose(grid_output);


    char* s;
    struct solution* solutions = (struct solution*)malloc(sizeof(*solutions)* word_arr.size);
    if (!solutions)  exit(-1);




    for (size_t i = 0; i != word_arr.size; i++) {
        s = word_arr.data[i].data;
        //printf("%.*s : ", word_arr.data[i].size, s);
        solutions[i]=solver_find(&grid, letter_count_x,word_arr.data+i);
    }

    graphical_solver(renderer, &grid, letter_count_x, grid.size / letter_count_x, &word_arr, solutions);
    IMG_Quit();
    int eh;

    SDL_Rect button_rect;
    button_rect.x = 960;
    button_rect.y = 0;
    button_rect.w = 320;
    button_rect.h = 81;

    int button_mode = 0;
    SDL_RenderCopy(renderer, button[0], NULL, &button_rect);
    SDL_RenderPresent(renderer);
    int cond;
    while (eh=event_handler()) {
        cond = mouse_pos[0] > 959 && mouse_pos[1] < 81;
        if (cond) {
            if (eh == 2&& button_mode!=2) {
                SDL_RenderCopy(renderer, button[3], NULL, &button_rect);
                SDL_RenderCopy(renderer, button[2], NULL, &button_rect);
                SDL_RenderPresent(renderer);
                break;
            }

            if (button_mode != 1) {
                SDL_RenderCopy(renderer, button[3], NULL, &button_rect);
                SDL_RenderCopy(renderer, button[1], NULL, &button_rect);
                SDL_RenderPresent(renderer);
                button_mode = 1;
            }
           
        }
        else{
             if(button_mode != 0){
                SDL_RenderCopy(renderer, button[3], NULL, &button_rect);
                SDL_RenderCopy(renderer, button[0], NULL, &button_rect);
                SDL_RenderPresent(renderer);
                button_mode = 0;
            }
        }

        SDL_Delay(1);

    }
    if (eh == 2) {
        while (1) {
            for (size_t i = 0; i != word_arr.size; i++) {
                s = word_arr.data[i].data;
                free(s);
            }
            free(solutions);
            free(word_arr.data);
            free(grid.data);
            memset(&grid, 0, sizeof(grid));
            memset(&word_arr, 0, sizeof(word_arr));


            load_selection(renderer);
            imgfln = draw_selection(renderer);
            image = IMG_Load(imgfln);
            if (!image) {
                puts(SDL_GetError());
            }

            img_w = image->w;
            img_h = image->h;
            texture = SDL_CreateTextureFromSurface(renderer, image);
            if (!texture)    puts(SDL_GetError());
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(texture);


            image2 = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);
            if (image2 == NULL) {
                puts(SDL_GetError());
            }
            SDL_FreeSurface(image);
            image = image2;



            image2 = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0xFF000000,
                0xFF0000, 0xFF00, 0xFF);

            image_rect.x = (1280 - 800) / 2;
            image_rect.y = 720 - 600;
            image_rect.w = 800;
            image_rect.h = 600;


            grayscale(image);
            img.data = (unsigned char*)image->pixels;
            img.width = image->w;
            img.height = image->h;
            img.channels = 4;
            median_filter(&img, 3);
            adjust_brightness(&img, 14);
            median_filter(&img, 3);

            enhance_contrast(&img);
            blackwhite(&img, 20);
            median_filter(&img, 3);

             deg = find_skew(image);
            //deg=3.9;
            rotate_surface(image, deg, image2);
            SDL_FreeSurface(image);

            image = image2;
            texture = SDL_CreateTextureFromSurface(renderer, image);
            if (!texture)    puts(SDL_GetError());
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(texture);
            letter_count_x = 0;
            word_arr = get_grid(image, classic, &letter_count_x, &grid);
             
             FILE* grid_output = fopen("grid.txt", "w");
             if (grid_output == NULL) { exit(-50); }
             for (size_t i = 0; i < grid.size; i += letter_count_x) {
                 fwrite(grid.data + i, letter_count_x, 1, grid_output);
                 if (i != grid.size - 1) {
                     fwrite("\n", 1, 1, grid_output);
                 }
             }
             fclose(grid_output);
            solutions = (struct solution*)malloc(sizeof(*solutions) * word_arr.size);
            if (!solutions)  exit(-1);

            for (size_t i = 0; i != word_arr.size; i++) {
                s = word_arr.data[i].data;
                //printf("%.*s : ", word_arr.data[i].size, s);
                solutions[i] = solver_find(&grid, letter_count_x, word_arr.data + i);
            }
            graphical_solver(renderer, &grid, letter_count_x, grid.size / letter_count_x, &word_arr, solutions);

            SDL_RenderCopy(renderer, button[0], NULL, &button_rect);
            SDL_RenderPresent(renderer);
            while (eh = event_handler()) {
                cond = mouse_pos[0] > 959 && mouse_pos[1] < 81;
                if (cond) {
                    if (eh == 2 && button_mode != 2) {
                        SDL_RenderCopy(renderer, button[3], NULL, &button_rect);
                        SDL_RenderCopy(renderer, button[2], NULL, &button_rect);
                        SDL_RenderPresent(renderer);
                        break;
                    }

                    if (button_mode != 1) {
                        SDL_RenderCopy(renderer, button[3], NULL, &button_rect);
                        SDL_RenderCopy(renderer, button[1], NULL, &button_rect);
                        SDL_RenderPresent(renderer);
                        button_mode = 1;
                    }

                }
                else {
                    if (button_mode != 0) {
                        SDL_RenderCopy(renderer, button[3], NULL, &button_rect);
                        SDL_RenderCopy(renderer, button[0], NULL, &button_rect);
                        SDL_RenderPresent(renderer);
                        button_mode = 0;
                    }
                }

                SDL_Delay(1);

            }
        }
        
    }
    free(word_arr.data);
    free(grid.data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for (size_t i = 0; i != LETTER_COUNT; i++) {
        SDL_FreeSurface(letters[i]);
    }
    SDL_FreeSurface(image);
    
    return 0;
}

