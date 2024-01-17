#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

#define GAME_WIDTH 224
#define GAME_HEIGHT 256

int main(void)
{
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        printf("SDL_Init failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    int scale = 3;
    SDL_Window * window = SDL_CreateWindow("Space Invaders",
                                           0, 0, // x, y
                                           GAME_WIDTH * scale,
                                           GAME_HEIGHT * scale, // size
                                           0); // flags
    if ( window == NULL ) {
        printf("Could not create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    if ( renderer == NULL ) {
        printf("Could not create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_RenderSetLogicalSize(renderer, GAME_WIDTH, GAME_HEIGHT);

    SDL_Surface * surface = SDL_LoadBMP("invaders.bmp");
    if ( surface == NULL ) {
        printf("Could not load sprite sheet BMP: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture * sprite_sheet = SDL_CreateTextureFromSurface(renderer, surface);
    if ( sprite_sheet == NULL ) {
        printf("Could not load sprite sheet texture: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_FreeSurface(surface);

    // Program

    bool is_running = true;
    while ( is_running )
    {
        SDL_Event event;
        while ( SDL_PollEvent(&event) ) {
            if ( event.type == SDL_QUIT ) {
                is_running = false;
            }

            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_ESCAPE ) {
                    is_running = false;
                }
            }
        }

        // update game

        // RENDER

        SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // rendering

        SDL_Rect src_rect = {
            .x = 0,
            .y = 0,
            .w = 8,
            .h = 8,
        };

        SDL_Rect dst_rect = { 0, 0, 8, 8 };
        SDL_RenderCopy(renderer, sprite_sheet, &src_rect, &dst_rect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // wait a bit for now, TODO: proper framerate limiting
    }

    // Cleanup:

    SDL_DestroyTexture(sprite_sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
