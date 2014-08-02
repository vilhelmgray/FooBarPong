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

#include "SDL.h"

static void closeDisplay(SDL_Window *window, SDL_Renderer *renderer);
static unsigned initDisplay(SDL_Window **window, SDL_Renderer **renderer, const size_t HEIGHT, const size_t WIDTH);

int main(void){
        if(SDL_Init(SDL_INIT_VIDEO) < 0){
                fprintf(stderr, "*** Error: Unable to initialize SDL: %s\n", SDL_GetError());
                return 1;
        }

        SDL_Window *window;
        SDL_Renderer *renderer;
        if(initDisplay(&window, &renderer, 640, 480)){
                fprintf(stderr, "*** Error: Unable to initialize display: %s\n", SDL_GetError());
                goto err_initDisplay;
        }

        unsigned running = 1;
        do{
                SDL_Event event;
                while(SDL_PollEvent(&event)){
                        switch(event.type){
                                case SDL_KEYDOWN:
                                        if(event.key.keysym.sym == SDLK_ESCAPE){
                                                running = 0;
                                        }
                                        break;
                                case SDL_QUIT:
                                        running = 0;
                                        break;
                                default:
                                        break;
                        }
                }

                if(SDL_RenderClear(renderer) < 0){
                        fprintf(stderr, "*** Error: Unable to clear renderer: %s\n", SDL_GetError());
                        goto err_rend_clear;
                }
                SDL_RenderPresent(renderer);
        }while(running);

        closeDisplay(window, renderer);
        SDL_Quit();

        return 0;

err_rend_clear:
        closeDisplay(window, renderer);
err_initDisplay:
        SDL_Quit();
        return 1;
}

static void closeDisplay(SDL_Window *window, SDL_Renderer *renderer){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
}

static unsigned initDisplay(SDL_Window **window, SDL_Renderer **renderer, const size_t HEIGHT, const size_t WIDTH){
        if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, window, renderer) < 0){
                fprintf(stderr, "*** Error: Unable to create window and default renderer: %s\n", SDL_GetError());
                return 1;
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
        closeDisplay(*window, *renderer);
        return 1;
}
