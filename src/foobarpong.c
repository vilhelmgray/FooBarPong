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

int main(void){
        if(SDL_Init(SDL_INIT_VIDEO) < 0){
                fprintf(stderr, "*** Error: Unable to initialize SDL: %s\n", SDL_GetError());
                return 1;
        }

        SDL_Window *window;
        SDL_Renderer *renderer;
        if(SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer) < 0){
                fprintf(stderr, "*** Error: Unable to create window and default renderer: %s\n", SDL_GetError());
                goto exit_wind_rend;
        }

        const size_t WIDTH = 640;
        const size_t HEIGHT = 480;
        if(SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT) < 0){
                fprintf(stderr, "*** Error: Unable to set resolution for rendering: %s\n", SDL_GetError());
                goto exit_logical_size;
        }
        if(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0){
                fprintf(stderr, "*** Error: Unable to set the renderer draw color: %s\n", SDL_GetError());
                goto exit_draw_color;
        }

        if(SDL_RenderClear(renderer) < 0){
                fprintf(stderr, "*** Error: Unable to clear renderer: %s\n", SDL_GetError());
                goto exit_clear_renderer;
        }

        SDL_RenderPresent(renderer);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;

exit_clear_renderer:
exit_draw_color:
exit_logical_size:
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
exit_wind_rend:
        SDL_Quit();
        return 1;
}
