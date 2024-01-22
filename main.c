#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

#define GAME_WIDTH 224
#define GAME_HEIGHT 256
#define PLAYER_WIDTH 13

const SDL_Rect player_src_rect = { 0, 32, PLAYER_WIDTH, 8 };

int main(void)
{
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        printf("SDL_Init failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    int scale = 4;
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

    const Uint8 * key_state = SDL_GetKeyboardState(NULL);

    // Game Data:

    SDL_Rect player = {
        .x = (GAME_WIDTH - PLAYER_WIDTH) / 2, // (w1 / 2) - (w2 / 2) -> (w1 - w2) / 2
        .y = 220,
        .w = PLAYER_WIDTH,
        .h = 8
    };



    // Program

    // 60 FPS
    // each frame takes 1/60 sec (1000 / 60 = ~16 ms)

    // pixels per second not pixels per frame

    // 60 pixels / delta_time

    int last = SDL_GetTicks();
    int ticks = 0; // aka game frame number
    bool is_running = true;

    while ( is_running )
    {
        // limit frame rate to 60 FPS (frame length = 1/60 (0.016) seconds)
        int now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f; // dt in seconds
        if ( dt < 0.016f ) {
            SDL_Delay(1);
            continue;
        }
        last = now;

        SDL_Event event;
        while ( SDL_PollEvent(&event) ) {
            if ( event.type == SDL_QUIT ) {
                is_running = false;
            } else if ( event.type == SDL_KEYDOWN ) {
                switch ( event.key.keysym.sym ) {
                    case SDLK_ESCAPE:
                        is_running = false;
                        break;
                    default:
                        break;
                }
            }
        }

        if ( key_state[SDL_SCANCODE_LEFT] ) {
            player.x -= 1;
            if ( player.x < 0 ) {
                player.x = 0;
            }
        }

        if ( key_state[SDL_SCANCODE_RIGHT] ) {
            player.x += 1;
            if ( player.x + player.w >= GAME_WIDTH ) {
                player.x = GAME_WIDTH - player.w;
            }
        }

        // NON-INPUT-RELATED UPDATE GAME

        // ...

        // RENDER

        SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // squid source rect: test animation
        SDL_Rect src_rect = {
            .x = ticks % 20 < 10 ? 0 : 16, // flip anim frame every 10 sec
            .y = 0,
            .w = 8,
            .h = 8,
        };
        SDL_Rect dst_rect = { 10, 10, 8, 8 };
        SDL_RenderCopy(renderer, sprite_sheet, &src_rect, &dst_rect);

        SDL_RenderCopy(renderer, sprite_sheet, &player_src_rect, &player);

        SDL_RenderPresent(renderer);

        ticks++;
    }

    // Cleanup:

    SDL_DestroyTexture(sprite_sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
