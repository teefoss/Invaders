#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

#define GAME_WIDTH 224
#define GAME_HEIGHT 256
#define PLAYER_WIDTH 13
#define SPRITE_HEIGHT 8
#define BULLET_WIDTH 3

#define FLEET_ROWS 5
#define FLEET_COLS 11
#define FLEET_ROW_HEIGHT (SPRITE_HEIGHT * 2)
#define FLEET_COL_WIDTH 16
#define MAX_ALIENS (FLEET_ROWS * FLEET_COLS)
#define FLEET_PADDING FLEET_COL_WIDTH // TODO: factor in how far over player can go (bullet)

typedef enum {
    ACTOR_SQUID,
    ACTOR_CRAB,
    ACTOR_FATSO,
    ACTOR_UFO,
    ACTOR_PLAYER,
    ACTOR_PLAYER_BULLET,

    NUM_ACTOR_TYPES
} ActorType;

typedef struct {
    ActorType type;

    // Location
    int x;
    int y;

    bool active;
} Actor;

const SDL_Rect player_src_rect = { 0, 32, PLAYER_WIDTH, SPRITE_HEIGHT };
const SDL_Rect player_bullet_src_rect = { 32, 32, BULLET_WIDTH, SPRITE_HEIGHT };

const int actor_widths[NUM_ACTOR_TYPES] = {
    [ACTOR_SQUID] = 8,
    [ACTOR_CRAB] = 11,
    [ACTOR_FATSO] = 12,
    [ACTOR_PLAYER] = 13,
    [ACTOR_PLAYER_BULLET] = BULLET_WIDTH,
};

Actor MakeActor(ActorType type, int x, int y)
{
    return (Actor){
        .type = type,
        .x = x,
        .y = y,
    };
}

void MovePlayer(const Uint8 * key_state, Actor * player)
{
    if ( key_state[SDL_SCANCODE_LEFT] ) {
        player->x -= 1;
        if ( player->x < 0 ) {
            player->x = 0;
        }
    }

    if ( key_state[SDL_SCANCODE_RIGHT] ) {
        player->x += 1;

        int w = actor_widths[ACTOR_PLAYER];
        if ( player->x + w >= GAME_WIDTH ) {
            player->x = GAME_WIDTH - w;
        }
    }
}

SDL_Rect GetDestRect(Actor * actor)
{
    return (SDL_Rect){
        .x = actor->x,
        .y = actor->y,
        .w = actor_widths[actor->type],
        .h = SPRITE_HEIGHT,
    };
}

bool AreActorsOverlapping(Actor * a, Actor * b)
{
    // TODO: implement
    return false;
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

    Actor player = MakeActor(ACTOR_PLAYER, (GAME_WIDTH - PLAYER_WIDTH) / 2, 220);
    Actor aliens[MAX_ALIENS];
    Actor bullet = MakeActor(ACTOR_PLAYER_BULLET, -100, -100);

    // Init. alien fleet.

    for ( int i = 0; i < MAX_ALIENS; i++ ) {
        // 1D index (i) to 2D coords
        int x = i % FLEET_COLS;
        int y = i / FLEET_COLS;

        // Note: int i = y * FLEET_COLS + x; // 2D coord to 1D index

        int actor_x = FLEET_PADDING + (x * FLEET_COL_WIDTH);
        int actor_y = FLEET_PADDING + (y * FLEET_ROW_HEIGHT);

        if ( y == 0 ) {
            aliens[i] = MakeActor(ACTOR_SQUID, actor_x, actor_y);
        } else if ( y == 1 || y == 2 ) {
            aliens[i] = MakeActor(ACTOR_CRAB, actor_x, actor_y);
        } else {
            aliens[i] = MakeActor(ACTOR_FATSO, actor_x, actor_y);
        }
    }
    
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
                    case SDLK_SPACE:
                        if ( !bullet.active ) {
                            // center the bullet horizontally over player.
                            // - 1 positions the bullet exactly over nozzle.
                            bullet.x = player.x + actor_widths[ACTOR_PLAYER] / 2 - 1;
                            bullet.y = player.y - SPRITE_HEIGHT + 2;
                            bullet.active = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        MovePlayer(key_state, &player);

        // NON-INPUT-RELATED UPDATE GAME

        // Update player bullet:

        if ( bullet.active ) {
            bullet.y -= 2;

            if ( bullet.y < -SPRITE_HEIGHT ) { // off top of screen?
                bullet.active = false;
            } else {
                // everything else:

                // if hits alien ... stuff
            }
        }

        // RENDER

        SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for ( int i = 0; i < MAX_ALIENS; i++ ) {
            SDL_Rect src_rect = {
                .x = 0,
                .y = aliens[i].type * SPRITE_HEIGHT,
                .w = actor_widths[aliens[i].type],
                .h = SPRITE_HEIGHT,
            };

            SDL_Rect dst_rect = GetDestRect(&aliens[i]);
            SDL_RenderCopy(renderer, sprite_sheet, &src_rect, &dst_rect);
        }

        // Render player:

        SDL_Rect player_dst_rect = GetDestRect(&player);
        SDL_RenderCopy(renderer, sprite_sheet, &player_src_rect, &player_dst_rect);

        // Render player bullet:

        if ( bullet.active ) {
            SDL_Rect player_bullet_dst_rect = GetDestRect(&bullet);
            SDL_RenderCopy(renderer,
                           sprite_sheet,
                           &player_bullet_src_rect,
                           &player_bullet_dst_rect);
        }

        SDL_RenderPresent(renderer);



        ticks++;
    }

    // Cleanup:

    SDL_DestroyTexture(sprite_sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
