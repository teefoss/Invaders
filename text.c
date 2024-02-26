//
//  text.c
//  Invaders
//
//  Created by Thomas Foster on 2/26/24.
//

#include "text.h"

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define NUM_TEXT_SHEET_COLS 16

static SDL_Texture * text_texture;
// TODO: clean up text_texture

void CleanUp(void) {
    SDL_DestroyTexture(text_texture);
}

void LoadText(SDL_Renderer * renderer) {
    SDL_Surface * temp = SDL_LoadBMP("arcade.bmp");
    if ( temp == NULL ) {
        puts("Could not load text surface");
        return;
    }

    // Set black as transparent pixel.
    Uint32 key = SDL_MapRGB(temp->format, 0, 0, 0);
    SDL_SetColorKey(temp, SDL_TRUE, key);

    text_texture = SDL_CreateTextureFromSurface(renderer, temp);
    if ( text_texture == NULL ) {
        puts("Could not load text texture");
    }

    SDL_FreeSurface(temp);

    atexit(CleanUp);
}

void DrawChar(SDL_Renderer * renderer, char ascii, int color24, int x, int y)
{
    char ch = ascii - ' ';
    SDL_Rect src = {
        .x = (ch % NUM_TEXT_SHEET_COLS) * CHAR_WIDTH,
        .y = (ch / NUM_TEXT_SHEET_COLS) * CHAR_HEIGHT,
        .w = CHAR_WIDTH,
        .h = CHAR_HEIGHT
    };
    SDL_Rect dst = { x, y, CHAR_WIDTH, CHAR_HEIGHT };

    Uint8 r = (color24 & 0xFF0000) >> 16;
    Uint8 g = (color24 & 0x00FF00) >> 8;
    Uint8 b = (color24 & 0x0000FF);
    SDL_SetTextureColorMod(text_texture, r, g, b);
    SDL_RenderCopy(renderer, text_texture, &src, &dst);
}

void DrawString(SDL_Renderer * renderer, const char * string, int x, int y, int color24)
{
    // For each char in string, call DrawChar()
}

void Test(SDL_Renderer * renderer) {
    SDL_RenderCopy(renderer, text_texture, NULL, NULL);
}
