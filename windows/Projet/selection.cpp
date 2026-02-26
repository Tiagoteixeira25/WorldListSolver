#include<SDL.h>
#include<SDL_image.h>
#include<stdio.h>
#include<stdlib.h>
int mouse_pos[2];

SDL_Texture* selection,*selected[4];
SDL_Rect r[4];

void load_selection(SDL_Renderer* rend) {

    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(selection, SDL_BLENDMODE_BLEND);
    SDL_Surface* temp = IMG_Load("res/selection.png");
    if (!temp) { exit(-99999); }
    selection = SDL_CreateTextureFromSurface(rend, temp);
    if (!selection)  exit(-75);
    SDL_FreeSurface(temp);


    temp = IMG_Load("res/11s.png");
    if (!temp) { exit(-99999); }
    selected[0] = SDL_CreateTextureFromSurface(rend, temp);
    if (!selected[0])  exit(-75);
    SDL_FreeSurface(temp);
    r[0].x = 0;
    r[0].y = 0;
    r[0].w = 640;
    r[0].h = 384;


    temp = IMG_Load("res/12s.png");
    if (!temp) { exit(-99999); }
    selected[1] = SDL_CreateTextureFromSurface(rend, temp);
    if (!selected[1])  exit(-75);
    SDL_FreeSurface(temp);
    r[1].x = 640;
    r[1].y = 0;
    r[1].w = 640;
    r[1].h = 384;


    temp = IMG_Load("res/21s.png");
    if (!temp) { exit(-99999); }
    selected[2] = SDL_CreateTextureFromSurface(rend, temp);
    if (!selected[2])  exit(-75);
    SDL_FreeSurface(temp);
    r[2].x = 0;
    r[2].y = 384;
    r[2].w = 640;
    r[2].h = 384;



    temp = IMG_Load("res/22s.png");
    if (!temp) { exit(-99999); }
    selected[3] = SDL_CreateTextureFromSurface(rend, temp);
    if (!selected[3])  exit(-75);
    SDL_FreeSurface(temp);
    r[3].x = 640;
    r[3].y = 384;
    r[3].w = 640;
    r[3].h = 384;
}


int event_handler() {
    SDL_Event test_event;
    SDL_PollEvent(&test_event);
    
    SDL_GetMouseState(mouse_pos, mouse_pos + 1);
    switch (test_event.type) {
    case SDL_QUIT:
        exit(EXIT_SUCCESS);
    case SDL_MOUSEBUTTONDOWN:
        return 2;
    case SDL_MOUSEBUTTONUP:
        break;
    }
    return 1;
}

char* draw_selection(SDL_Renderer*rend){
    int eh;
    char* ret = NULL;
    while (eh=event_handler()) {
        
        //SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
        SDL_RenderCopy(rend, selection, NULL, NULL);
        
        //printf("%i %i\n", mouse_pos[0], mouse_pos[1]);
        if (mouse_pos[1] < 384) {
            if (mouse_pos[0] < 640) {
                ret =(char*) "res/11.png";
                SDL_RenderCopy(rend, selected[0], NULL, r);
            }
            else {
                ret = (char*)"res/12.png";
                SDL_RenderCopy(rend, selected[1], NULL, r+1);
            }
        }
        else if (mouse_pos[1] < 768) {
            if (mouse_pos[0] < 640) {
                ret = (char*)"res/21.png";
                SDL_RenderCopy(rend, selected[2], NULL, r + 2);
            }
            else {
                ret = (char*)"res/22.png";
                SDL_RenderCopy(rend, selected[3], NULL, r + 3);
            }
        }
        else {
            ret = NULL;
        }
        SDL_RenderPresent(rend);
        if (eh == 2&& ret) {
            SDL_DestroyTexture(selection);
            SDL_DestroyTexture(selected[0]);
            SDL_DestroyTexture(selected[1]);
            SDL_DestroyTexture(selected[2]);
            SDL_DestroyTexture(selected[3]);
            return ret;
        }
    }
    return NULL;
}