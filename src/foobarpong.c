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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#define WIDTH 640
#define HEIGHT 480

struct sprite{
        SDL_Rect dimensions;
        SDL_Texture *texture;
};
static struct sprite background;

struct character{
        struct sprite sprite;
        int x_vel;
        int y_vel;
};
static struct character ball = { .x_vel = 5 };

struct player{
        struct character avatar;
        unsigned score;
        struct sprite score_display;
};
static struct player player1 = { .score_display.dimensions.x = WIDTH/4 };
static struct player player2 = { .score_display.dimensions.x = WIDTH - WIDTH/4 };

static TTF_Font *font;

static void closeDisplay(SDL_Window *const window, SDL_Renderer *const renderer);
static unsigned drawWorld(SDL_Renderer *const renderer);
static void freeFiles(void);
static unsigned handleEvents(unsigned *const running, SDL_Renderer *const renderer);
static unsigned initDisplay(SDL_Window **const window, SDL_Renderer **const renderer);
static unsigned loadFiles(SDL_Renderer *const renderer);
static unsigned loadSprite(const char *const PATH, struct sprite *const sprite, SDL_Renderer *const renderer);
static void moveCharacter(struct character *character);
static unsigned newGame(SDL_Renderer *const renderer);
static void paddleBounce(void);
static unsigned processWorld(SDL_Renderer *const renderer);
static void resetBall(void);
static unsigned updateScoreTexture(struct player *const player, SDL_Renderer *const renderer);

int main(void){
        srand(time(NULL));

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
        Uint32 lastFrame = 0;
        do{
                Uint32 currFrame = SDL_GetTicks();
                if(currFrame - lastFrame > 33){
                        lastFrame = currFrame;

                        if(drawWorld(renderer)){
                                fprintf(stderr, "*** Error: Unable to draw world\n");
                                goto err_drawWorld;
                        }

                        if(handleEvents(&running, renderer)){
                                fprintf(stderr, "*** Error: Unable to handle events\n");
                                goto err_handleEvents;
                        }
                        if(processWorld(renderer)){
                                fprintf(stderr, "*** Error: Unable to process world\n");
                                goto err_processWorld;
                        }
                }
        }while(running);

        freeFiles();
        closeDisplay(window, renderer);

        return 0;

err_processWorld:
err_handleEvents:
err_drawWorld:
        freeFiles();
err_loadFiles:
        closeDisplay(window, renderer);
        return 1;
}

static void closeDisplay(SDL_Window *const window, SDL_Renderer *const renderer){
        TTF_Quit();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
}

static unsigned drawWorld(SDL_Renderer *const renderer){
        if(SDL_RenderClear(renderer) < 0){
                fprintf(stderr, "*** Error: Unable to clear renderer: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, background.texture, NULL, &background.dimensions)){
                fprintf(stderr, "*** Error: Unable to draw background: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, ball.sprite.texture, NULL, &ball.sprite.dimensions)){
                fprintf(stderr, "*** Error: Unable to draw ball: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, player1.avatar.sprite.texture, NULL, &player1.avatar.sprite.dimensions)){
                fprintf(stderr, "*** Error: Unable to draw player 1: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, player2.avatar.sprite.texture, NULL, &player2.avatar.sprite.dimensions)){
                fprintf(stderr, "*** Error: Unable to draw player 2: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, player1.score_display.texture, NULL, &player1.score_display.dimensions)){
                fprintf(stderr, "*** Error: Unable to draw player 1 score: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_RenderCopy(renderer, player2.score_display.texture, NULL, &player2.score_display.dimensions)){
                fprintf(stderr, "*** Error: Unable to draw player 2 score: %s\n", SDL_GetError());
                return 1;
        }

        SDL_RenderPresent(renderer);

        return 0;
}

static void freeFiles(void){
        SDL_DestroyTexture(player2.score_display.texture);
        SDL_DestroyTexture(player1.score_display.texture);

        SDL_DestroyTexture(player2.avatar.sprite.texture);
        SDL_DestroyTexture(player1.avatar.sprite.texture);
        SDL_DestroyTexture(ball.sprite.texture);
        SDL_DestroyTexture(background.texture);

        TTF_CloseFont(font);
}

static unsigned handleEvents(unsigned *const running, SDL_Renderer *const renderer){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
                switch(event.type){
                        case SDL_KEYDOWN:
                                switch(event.key.keysym.sym){
                                        case SDLK_s:
                                                player1.avatar.y_vel = 10;
                                                break;
                                        case SDLK_DOWN:
                                                player2.avatar.y_vel = 10;
                                                break;
                                        case SDLK_ESCAPE:
                                                *running = 0;
                                                break;
                                        case SDLK_RETURN:
                                                if(newGame(renderer)){
                                                        fprintf(stderr, "*** Error: Unable to start new game\n");
                                                        return 1;
                                                }
                                                break;
                                        case SDLK_UP:
                                                player2.avatar.y_vel = -10;
                                                break;
                                        case SDLK_w:
                                                player1.avatar.y_vel = -10;
                                                break;
                                        default:
                                                break;
                                }
                                break;
                        case SDL_KEYUP:
                                switch(event.key.keysym.sym){
                                        case SDLK_DOWN:
                                        case SDLK_UP:
                                                player2.avatar.y_vel = 0;
                                                break;
                                        case SDLK_s:
                                        case SDLK_w:
                                                player1.avatar.y_vel = 0;
                                                break;
                                        default:
                                                break;
                                }
                                break;
                        case SDL_QUIT:
                                *running = 0;
                                break;
                        default:
                                break;
                }
        }

        return 0;
}

static unsigned initDisplay(SDL_Window **const window, SDL_Renderer **const renderer){
        if(SDL_Init(SDL_INIT_VIDEO) < 0){
                fprintf(stderr, "*** Error: Unable to initialize SDL: %s\n", SDL_GetError());
                return 1;
        }

        if(SDL_ShowCursor(SDL_DISABLE) < 0){
                fprintf(stderr, "*** Error: Unable to hide cursor: %s\n", SDL_GetError());
                goto err_show_cursor;
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

        if(TTF_Init()){
                fprintf(stderr, "*** Error: Unable to initialize TrueType font support: %s\n", TTF_GetError());
                goto err_ttf_init;
        }

        return 0;

err_ttf_init:
err_set_rend_draw_color:
err_set_logical_size:
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
err_create_wind_rend:
err_show_cursor:
        SDL_Quit();
        return 1;
}

static unsigned loadFiles(SDL_Renderer *const renderer){
        char *font_path = "images/boingium.ttf";
        font = TTF_OpenFont(font_path, 32);
        if(!font){
                fprintf(stderr, "*** Error: Unable to open font file \"%s\": %s\n", font_path, TTF_GetError());
                return 1;
        }

        if(updateScoreTexture(&player1, renderer)){
                fprintf(stderr, "*** Error: Unable to update player 1 score texture\n");
                goto err_updateScoreTexture;
        }
        if(updateScoreTexture(&player2, renderer)){
                fprintf(stderr, "*** Error: Unable to update player 1 score texture\n");
                goto err_updateScoreTexture;
        }

        if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
                fprintf(stderr, "*** Error: Unable to initialize PNG image support: %s\n", IMG_GetError());
                goto err_img_init;
        }

        if(loadSprite("images/background.png", &background, renderer)){
                fprintf(stderr, "*** Error: Unable to load background sprite\n");
                goto err_load_background;
        }
        background.dimensions.x = 0;
        background.dimensions.y = 0;

        if(loadSprite("images/ball.png", &ball.sprite, renderer)){
                fprintf(stderr, "*** Error: Unable to load ball sprite\n");
                goto err_load_ball;
        }
        resetBall();

        if(loadSprite("images/paddle1.png", &player1.avatar.sprite, renderer)){
                fprintf(stderr, "*** Error: Unable to load player 1 sprite\n");
                goto err_load_player1;
        }
        player1.avatar.sprite.dimensions.x = (2*player1.avatar.sprite.dimensions.w > WIDTH) ? 0 : player1.avatar.sprite.dimensions.w;
        player1.avatar.sprite.dimensions.y = (HEIGHT - player1.avatar.sprite.dimensions.h)/2;

        if(loadSprite("images/paddle2.png", &player2.avatar.sprite, renderer)){
                fprintf(stderr, "*** Error: Unable to load player 2 sprite\n");
                goto err_load_player2;
        }
        player2.avatar.sprite.dimensions.x = (WIDTH < 2*player2.avatar.sprite.dimensions.w) ? 0 : WIDTH - 2*player2.avatar.sprite.dimensions.w;
        player2.avatar.sprite.dimensions.y = (HEIGHT - player2.avatar.sprite.dimensions.h)/2;

        IMG_Quit();
        return 0;

err_load_player2:
        SDL_DestroyTexture(player1.avatar.sprite.texture);
err_load_player1:
        SDL_DestroyTexture(ball.sprite.texture);
err_load_ball:
        SDL_DestroyTexture(background.texture);
err_load_background:
        IMG_Quit();
err_img_init:
err_updateScoreTexture:
        TTF_CloseFont(font);
        return 1;
}

static unsigned loadSprite(const char *const PATH, struct sprite *const sprite, SDL_Renderer *const renderer){
        SDL_Surface *surface = IMG_Load(PATH);
        if(!surface){
                fprintf(stderr, "*** Error: Unable to load \"%s\": %s\n", PATH, IMG_GetError());
                return 1;
        }
        sprite->dimensions.w = (surface->w > WIDTH) ? WIDTH : surface->w;
        sprite->dimensions.h = (surface->h > HEIGHT) ? HEIGHT : surface->h;
        sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
        if(!sprite->texture){
                fprintf(stderr, "*** Error: Unable to create texture from \"%s\": %s\n", PATH, SDL_GetError());
                goto err_create_texture;
        }

        SDL_FreeSurface(surface);

        return 0;

err_create_texture:
        SDL_FreeSurface(surface);
        return 1;
}

static void moveCharacter(struct character *character){
        const int X_START = character->sprite.dimensions.x;
        const int X_END = X_START + (character->sprite.dimensions.w - 1);
        const int X_VEL = character->x_vel;
        const int Y_START = character->sprite.dimensions.y;
        const int Y_END = Y_START + (character->sprite.dimensions.h - 1);
        const int Y_VEL = character->y_vel;

        character->sprite.dimensions.x = (X_END + X_VEL > WIDTH - 1) ? WIDTH - character->sprite.dimensions.w : ((X_START + X_VEL < 0) ? 0 : X_START + X_VEL);
        character->sprite.dimensions.y = (Y_END + Y_VEL > HEIGHT - 1) ? HEIGHT - character->sprite.dimensions.h : ((Y_START + Y_VEL < 0) ? 0 : Y_START + Y_VEL);
}

static unsigned newGame(SDL_Renderer *const renderer){
        player1.score = 0;
        if(updateScoreTexture(&player1, renderer)){
                fprintf(stderr, "*** Error: Unable to update player 1 score texture\n");
                return 1;
        }
        player2.score = 0;
        if(updateScoreTexture(&player2, renderer)){
                fprintf(stderr, "*** Error: Unable to update player 2 score texture\n");
                return 1;
        }

        player1.avatar.sprite.dimensions.x = (2*player1.avatar.sprite.dimensions.w > WIDTH) ? 0 : player1.avatar.sprite.dimensions.w;
        player1.avatar.sprite.dimensions.y = (HEIGHT - player1.avatar.sprite.dimensions.h)/2;

        player2.avatar.sprite.dimensions.x = (WIDTH < 2*player2.avatar.sprite.dimensions.w) ? 0 : WIDTH - 2*player2.avatar.sprite.dimensions.w;
        player2.avatar.sprite.dimensions.y = (HEIGHT - player2.avatar.sprite.dimensions.h)/2;

        ball.x_vel = 5;
        resetBall();

        return 0;
}

static void paddleBounce(void){
        ball.y_vel = rand() % 6;
        ball.y_vel *= (rand() % 2) ? -1 : 1;
}

static unsigned processWorld(SDL_Renderer *const renderer){
        moveCharacter(&player1.avatar);
        moveCharacter(&player2.avatar);
        moveCharacter(&ball);

        const int BALL_X_START = ball.sprite.dimensions.x;
        const int BALL_X_END = BALL_X_START + (ball.sprite.dimensions.w-1);
        const int BALL_Y_START = ball.sprite.dimensions.y;
        const int BALL_Y_END = BALL_Y_START + (ball.sprite.dimensions.h-1);

        const int PLAYER1_X_START = player1.avatar.sprite.dimensions.x;
        const int PLAYER1_X_END = PLAYER1_X_START + (player1.avatar.sprite.dimensions.w-1);
        const int PLAYER1_Y_START = player1.avatar.sprite.dimensions.y;
        const int PLAYER1_Y_END = PLAYER1_Y_START + (player1.avatar.sprite.dimensions.h-1);

        const int PLAYER2_X_START = player2.avatar.sprite.dimensions.x;
        const int PLAYER2_X_END = PLAYER2_X_START + (player2.avatar.sprite.dimensions.w-1);
        const int PLAYER2_Y_START = player2.avatar.sprite.dimensions.y;
        const int PLAYER2_Y_END = PLAYER2_Y_START + (player2.avatar.sprite.dimensions.h-1);

        if(BALL_X_START == 0){
                player2.score++;
                if(updateScoreTexture(&player2, renderer)){
                        fprintf(stderr, "*** Error: Unable to update player 1 score texture\n");
                        return 1;
                }
                resetBall();
        }else if(BALL_X_END == WIDTH - 1){
                player1.score++;
                if(updateScoreTexture(&player1, renderer)){
                        fprintf(stderr, "*** Error: Unable to update player 2 score texture\n");
                        return 1;
                }
                resetBall();
        }else if((BALL_X_START >= PLAYER1_X_START && BALL_X_START <= PLAYER1_X_END) || (BALL_X_END >= PLAYER1_X_START && BALL_X_END <= PLAYER1_X_END)){
                if((BALL_Y_START >= PLAYER1_Y_START && BALL_Y_START <= PLAYER1_Y_END) || (BALL_Y_END >= PLAYER1_Y_START && BALL_Y_END <= PLAYER1_Y_END)){
                        ball.sprite.dimensions.x = PLAYER1_X_END + 1;
                        ball.x_vel *= -1;
                        paddleBounce();
                }
        }else if((BALL_X_START >= PLAYER2_X_START && BALL_X_START <= PLAYER2_X_END) || (BALL_X_END >= PLAYER2_X_START && BALL_X_END <= PLAYER2_X_END)){
                if((BALL_Y_START >= PLAYER2_Y_START && BALL_Y_START <= PLAYER2_Y_END) || (BALL_Y_END >= PLAYER2_Y_START && BALL_Y_END <= PLAYER2_Y_END)){
                        ball.sprite.dimensions.x = PLAYER2_X_START - ball.sprite.dimensions.w;
                        ball.x_vel *= -1;
                        paddleBounce();
                }
        }else if(BALL_Y_START == 0 || BALL_Y_END == HEIGHT -1){
                ball.y_vel *= -1;
        }

        return 0;
}

static void resetBall(void){
        ball.sprite.dimensions.x = (WIDTH - ball.sprite.dimensions.w)/2;
        ball.sprite.dimensions.y = rand() % (HEIGHT - ball.sprite.dimensions.h);

        paddleBounce();
}

static unsigned updateScoreTexture(struct player *const player, SDL_Renderer *const renderer){
        char score_str[16];
        snprintf(score_str, sizeof(score_str), "%u", player->score);

        const SDL_Color COLOR = { .r = 255, .g = 255, .b = 255 };
        SDL_Surface *font_surface = TTF_RenderText_Solid(font, score_str, COLOR);
        if(!font_surface){
                fprintf(stderr, "*** Error: Unable to render score text: %s\n", TTF_GetError());
                return 1;
        }

        player->score_display.dimensions.w = (font_surface->w > WIDTH) ? WIDTH : font_surface->w;
        player->score_display.dimensions.h = (font_surface->h > HEIGHT) ? HEIGHT : font_surface->h;
        player->score_display.texture = SDL_CreateTextureFromSurface(renderer, font_surface);
        if(!player->score_display.texture){
                fprintf(stderr, "*** Error: Unable to create texture from score text surface: %s\n", SDL_GetError());
                goto err_create_texture;
        }

        SDL_FreeSurface(font_surface);

        return 0;

err_create_texture:
        SDL_FreeSurface(font_surface);
        return 1;
}
