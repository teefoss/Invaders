#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include "text.h"

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

// Describes the full movement pattern of the alien fleet.
typedef enum {
    DIR_RIGHT,
    DIR_DOWN1,
    DIR_LEFT,
    DIR_DOWN2,

    NUM_DIRECTIONS,
} Direction;

typedef struct {
    const Uint8 * key_state;
    bool is_running;
    int ticks;

    Actor bullet;
    Actor player;

    // Alien fleet
    int num_aliens; // The number of alive aliens.
    Actor aliens[MAX_ALIENS];
    int alien_index; // The current index to update.
    Direction direction;

    SDL_Texture * sprite_sheet;
    SDL_Renderer * renderer;
} Game;

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

    SDL_Rect actor_a = {
        .x = a->x,
        .y = a->y,
        .w = actor_widths[a->type],
        .h = SPRITE_HEIGHT,
    };
    
    SDL_Rect actor_b = {
        .x = b->x,
        .y = b->y,
        .w = actor_widths[b->type],
        .h = SPRITE_HEIGHT,
    };

    if (( actor_a.w >= actor_b.x && actor_a.h >= actor_b.y ) || 
        ( actor_a.x <= actor_b.w && actor_a.h >= actor_b.y )) {
        return true;
    } else if (( actor_b.w >= actor_a.x && actor_b.h >= actor_a.y ) || 
                ( actor_b.x <= actor_a.w && actor_b.h >= actor_a.y )) {
        return true;
    }
    return false;
}

void DoGameFrame(Game * game)
{
    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {
        if ( event.type == SDL_QUIT ) {
            game->is_running = false;
        } else if ( event.type == SDL_KEYDOWN ) {
            switch ( event.key.keysym.sym ) {
                case SDLK_ESCAPE:
                    game->is_running = false;
                    break;
                case SDLK_SPACE:
                    if ( !game->bullet.active ) {
                        // center the bullet horizontally over player.
                        // - 1 positions the bullet exactly over nozzle.
                        game->bullet.x = game->player.x + actor_widths[ACTOR_PLAYER] / 2 - 1;
                        game->bullet.y = game->player.y - SPRITE_HEIGHT + 2;
                        game->bullet.active = true;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    MovePlayer(game->key_state, &game->player);

    // NON-INPUT-RELATED UPDATE GAME

    // Update player bullet:

    if ( game->bullet.active ) {
        game->bullet.y -= 2;

        if ( game->bullet.y < -SPRITE_HEIGHT ) { // off top of screen?
            game->bullet.active = false;
        } else {
            // everything else:

            // if hits alien ... stuff
        }
    }

    // MOVE ONE ALIEN

    // deleting aliens:
    // int num_aliens
    // index to dlete = 3
    // ABCDEFGH
    // move all indices starting at index to del + 1
    // ABCEFGH_

    Actor * alien = &game->aliens[game->alien_index];

    // Move the alien.
    switch ( game->direction ) {
        case DIR_LEFT:
            alien->x--;
            break;
        case DIR_RIGHT:
            alien->x++;
            break;
        case DIR_DOWN1:
        case DIR_DOWN2:
            alien->y += 2;
            break;
        default:
            break;
    }

    // Check if alien hit a side:

    // Use full col width so aliens stay aligned.
    int alien_right_side = alien->x + FLEET_COL_WIDTH;
    int alien_left_side = alien->x;

    // FIXME: as it is now, aliens gradually get more and more out of sync vertically.
    if ( alien_right_side >= GAME_WIDTH || alien_left_side < 0 ) {
        game->direction = (game->direction + 1) % NUM_DIRECTIONS;
    }

    // Move to next index.
    game->alien_index = (game->alien_index + 1) % game->num_aliens;

    // RENDER

    SDL_SetRenderDrawColor(game->renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(game->renderer);

    for ( int i = 0; i < MAX_ALIENS; i++ ) {
        SDL_Rect src_rect = {
            .x = 0,
            .y = game->aliens[i].type * SPRITE_HEIGHT,
            .w = actor_widths[game->aliens[i].type],
            .h = SPRITE_HEIGHT,
        };

        SDL_Rect dst_rect = GetDestRect(&game->aliens[i]);
        SDL_RenderCopy(game->renderer, game->sprite_sheet, &src_rect, &dst_rect);
    }

    // Render player:

    SDL_Rect player_dst_rect = GetDestRect(&game->player);
    SDL_RenderCopy(game->renderer, game->sprite_sheet, &player_src_rect, &player_dst_rect);

    // Render player bullet:

    if ( game->bullet.active ) {
        SDL_Rect player_bullet_dst_rect = GetDestRect(&game->bullet);
        SDL_RenderCopy(game->renderer,
                       game->sprite_sheet,
                       &player_bullet_src_rect,
                       &player_bullet_dst_rect);
    }

    // Text testing region:

    SDL_RenderPresent(game->renderer);

    game->ticks++;
}

Game MakeGame(SDL_Renderer * renderer, SDL_Texture * sprite_sheet)
{
    Game game = {
        .ticks = 0,
        .is_running = true,
        .key_state = SDL_GetKeyboardState(NULL),
        .player = MakeActor(ACTOR_PLAYER, (GAME_WIDTH - PLAYER_WIDTH) / 2, 220),
        .bullet = MakeActor(ACTOR_PLAYER_BULLET, -100, -100),
        .num_aliens = MAX_ALIENS,
        .alien_index = 0,
        .sprite_sheet = sprite_sheet,
        .renderer = renderer,
    };

    // Init. alien fleet.

    for ( int i = 0; i < MAX_ALIENS; i++ ) {
        // 1D index (i) to 2D coords
        int x = i % FLEET_COLS;
        int y = i / FLEET_COLS;

        // Note: int i = y * FLEET_COLS + x; // 2D coord to 1D index

        int actor_x = FLEET_PADDING + (x * FLEET_COL_WIDTH);
        int actor_y = FLEET_PADDING + (y * FLEET_ROW_HEIGHT);

        if ( y == 0 ) {
            game.aliens[i] = MakeActor(ACTOR_SQUID, actor_x, actor_y);
        } else if ( y == 1 || y == 2 ) {
            game.aliens[i] = MakeActor(ACTOR_CRAB, actor_x, actor_y);
        } else {
            game.aliens[i] = MakeActor(ACTOR_FATSO, actor_x, actor_y);
        }
    }

    return game;
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

    LoadText(renderer);

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

    Game game = MakeGame(renderer, sprite_sheet);

    // Program

    // 60 FPS
    // each frame takes 1/60 sec (1000 / 60 = ~16 ms)

    // pixels per second not pixels per frame

    // 60 pixels / delta_time

    int last = SDL_GetTicks();

    while ( game.is_running )
    {
        // limit frame rate to 60 FPS (frame length = 1/60 (0.016) seconds)
        int now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f; // dt in seconds
        if ( dt < 0.016f ) {
            SDL_Delay(1);
            continue;
        }
        last = now;

        DoGameFrame(&game);
    }

    // Cleanup:

    SDL_DestroyTexture(sprite_sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
