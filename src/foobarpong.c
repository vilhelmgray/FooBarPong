/* Copyright (c) 2014, William Breathitt Gray
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"

const size_t WIDTH = 640;
const size_t HEIGHT = 480;
static SDL_Texture *background;
static SDL_Texture *divider;

static void closeDisplay(SDL_Window *const window, SDL_Renderer *const renderer);
static unsigned drawWorld(SDL_Renderer *const renderer);
static void freeFiles(void);
static void handleEvents(unsigned *const running);
static unsigned initDisplay(SDL_Window **const window, SDL_Renderer **const renderer);
static unsigned loadFiles(SDL_Renderer *const renderer);

int main(void){
        SDL_Window *window;
        SDL_Renderer *renderer;
        if(initDisplay(&window, &renderer)){
                fprintf(stderr, "*** Error: Unable to initialize display\n");
                return 1;
        }

        if(loadFiles(renderer)){
                fprintf(stderr, "*** Error: Unable to load files\n");
                goto err_loadFiles;
        }

        unsigned running = 1;
        do{
                if(drawWorld(renderer)){
                        fprintf(stderr, "*** Error: Unable to draw world\n");
                        goto err_drawWorld;
                }

                handleEvents(&running);
        }while(running);

        freeFiles();
        closeDisplay(window, renderer);

        return 0;

err_drawWorld:
        freeFiles();
err_loadFiles:
        closeDisplay(window, renderer);
        return 1;
}

static void closeDisplay(SDL_Window *const window, SDL_Renderer *const renderer){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
}

static unsigned drawWorld(SDL_Renderer *const renderer){
        if(SDL_RenderClear(renderer) < 0){
                fprintf(stderr, "*** Error: Unable to clear renderer: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, background, NULL, NULL)){
                fprintf(stderr, "*** Error: Unable to draw background: %s\n", SDL_GetError());
                return 1;
        }

        SDL_Rect divider_rect = { .w = 23,
                                  .h = 480 };
        divider_rect.x = (WIDTH - divider_rect.w)/2;
        divider_rect.y = (HEIGHT - divider_rect.h)/2;
        if(SDL_RenderCopy(renderer, divider, NULL, &divider_rect)){
                fprintf(stderr, "*** Error: Unable to draw divider: %s\n", SDL_GetError());
                return 1;
        }

        SDL_RenderPresent(renderer);

        return 0;
}

static void freeFiles(void){
        SDL_DestroyTexture(divider);
        SDL_DestroyTexture(background);
}

static void handleEvents(unsigned *const running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
                switch(event.type){
                        case SDL_KEYDOWN:
                                if(event.key.keysym.sym == SDLK_ESCAPE){
                                        *running = 0;
                                }
                                break;
                        case SDL_QUIT:
                                *running = 0;
                                break;
                        default:
                                break;
                }
        }
}

static unsigned initDisplay(SDL_Window **const window, SDL_Renderer **const renderer){
        if(SDL_Init(SDL_INIT_VIDEO) < 0){
                fprintf(stderr, "*** Error: Unable to initialize SDL: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, window, renderer) < 0){
                fprintf(stderr, "*** Error: Unable to create window and default renderer: %s\n", SDL_GetError());
                goto err_create_wind_rend;
        }

        if(SDL_RenderSetLogicalSize(*renderer, WIDTH, HEIGHT) < 0){
                fprintf(stderr, "*** Error: Unable to set resolution for rendering: %s\n", SDL_GetError());
                goto err_set_logical_size;
        }
        if(SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255) < 0){
                fprintf(stderr, "*** Error: Unable to set the renderer draw color: %s\n", SDL_GetError());
                goto err_set_rend_draw_color;
        }

        return 0;

err_set_rend_draw_color:
err_set_logical_size:
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
err_create_wind_rend:
        SDL_Quit();
        return 1;
}

static unsigned loadFiles(SDL_Renderer *const renderer){
        if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
                fprintf(stderr, "*** Error: Unable to initialize PNG image support: %s\n", IMG_GetError());
                return 1;
        }

        char path[256] = "images/background.png";
        SDL_Surface *surface = IMG_Load(path);
        if(!surface){
                fprintf(stderr, "*** Error: Unable to load \"%s\": %s\n", path, IMG_GetError());
                goto err_IMG_Load;
        }
        background = SDL_CreateTextureFromSurface(renderer, surface);
        if(!background){
                fprintf(stderr, "*** Error: Unable to create texture from \"%s\": %s\n", path, SDL_GetError());
                goto err_create_texture;
        }

        SDL_FreeSurface(surface);

        strcpy(path, "images/divider.png");
        surface = IMG_Load(path);
        if(!surface){
                fprintf(stderr, "*** Error: Unable to load \"%s\": %s\n", path, IMG_GetError());
                goto err_IMG_Load;
        }
        divider = SDL_CreateTextureFromSurface(renderer, surface);
        if(!divider){
                fprintf(stderr, "*** Error: Unable to create texture from \"%s\": %s\n", path, SDL_GetError());
                goto err_create_texture;
        }

        SDL_FreeSurface(surface);

        IMG_Quit();
        return 0;

err_create_texture:
        SDL_FreeSurface(surface);
err_IMG_Load:
        IMG_Quit();
        return 1;
}
