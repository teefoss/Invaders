#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

#define GAME_WIDTH 224
#define GAME_HEIGHT 256
#define PLAYER_WIDTH 13
#define SPRITE_HEIGHT 8

#define FLEET_ROWS 5
#define FLEET_COLS 11
#define FLEET_ROW_HEIGHT (SPRITE_HEIGHT * 2)
#define FLEET_COL_WIDTH 16
#define MAX_ALIENS (FLEET_ROWS * FLEET_COLS)

const SDL_Rect player_src_rect = { 0, 32, PLAYER_WIDTH, 8 };
const int alien_widths[] = { 8, 11, 12 };

typedef enum {
    ALIEN_SQUID,
    ALIEN_CRAB,
    ALIEN_FATSO
} AlienType;

typedef struct {
    AlienType type;

    // Alien's location
    int x;
    int y;
} Alien;


void MovePlayer(const Uint8 * key_state, SDL_Rect * player)
{
    if ( key_state[SDL_SCANCODE_LEFT] ) {
        player->x -= 1;
        if ( player->x < 0 ) {
            player->x = 0;
        }
    }

    if ( key_state[SDL_SCANCODE_RIGHT] ) {
        player->x += 1;
        if ( player->x + player->w >= GAME_WIDTH ) {
            player->x = GAME_WIDTH - player->w;
        }
    }
}

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

    Alien fleet[MAX_ALIENS];

    // Init. alien fleet.
    for ( int y = 0; y < FLEET_ROWS; y++ ) {
        for ( int x = 0; x < FLEET_COLS; x++ ) {

            int i = y * FLEET_COLS + x; // 2D coord to 1D array index
            fleet[i].x = 16 + (x * FLEET_COL_WIDTH);
            fleet[i].y = 16 + (y * FLEET_ROW_HEIGHT);

            if ( y == 0 ) {
                fleet[i].type = ALIEN_SQUID;
            } else if ( y == 1 || y == 2 ) {
                fleet[i].type = ALIEN_CRAB;
            } else {
                fleet[i].type = ALIEN_FATSO;
            }
        }
    }
    // TODO: assignment: how to convert from 1D to 2D coordinates?

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

        MovePlayer(key_state, &player);

        // NON-INPUT-RELATED UPDATE GAME

        // ...

        // RENDER

        SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for ( int i = 0; i < MAX_ALIENS; i++ ) {
            SDL_Rect src_rect = {
                .x = ticks % 20 < 10 ? 0 : 16, // flip anim frame every 10 sec
                .y = fleet[i].type * SPRITE_HEIGHT,
                .w = alien_widths[fleet[i].type],
                .h = SPRITE_HEIGHT,
            };

            SDL_Rect dst_rect = {
                fleet[i].x,
                fleet[i].y,
                alien_widths[fleet[i].type],
                SPRITE_HEIGHT
            };

            SDL_RenderCopy(renderer, sprite_sheet, &src_rect, &dst_rect);
        }

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
