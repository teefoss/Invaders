//
//  text.c
//  Invaders
//
//  Created by Thomas Foster on 2/26/24.
//

#include "text.h"
#include <stdarg.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define NUM_TEXT_SHEET_COLS 16

static SDL_Texture * text_texture;

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
}

void DrawChar(SDL_Renderer * renderer, char ascii, int x, int y)
{
    char ch = ascii - ' ';
    SDL_Rect src = {
        .x = (ch % NUM_TEXT_SHEET_COLS) * CHAR_WIDTH,
        .y = (ch / NUM_TEXT_SHEET_COLS) * CHAR_HEIGHT,
        .w = CHAR_WIDTH,
        .h = CHAR_HEIGHT
    };
    SDL_Rect dst = { x, y, CHAR_WIDTH, CHAR_HEIGHT };

    Uint8 r, g, b;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, NULL);
    SDL_SetTextureColorMod(text_texture, r, g, b);
    SDL_RenderCopy(renderer, text_texture, &src, &dst);
}

void DrawString(SDL_Renderer * renderer, int x, int y, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    char output_string[100] = { 0 };
    vsnprintf(output_string, sizeof(output_string) - 1, format, args);
    va_end(args);

    char * ch = output_string;
    int draw_x = x;
    while ( *ch != '\0' ) {
        DrawChar(renderer, *ch, draw_x, y);
        draw_x += CHAR_WIDTH;
        ch++;
    }
}

void Test(SDL_Renderer * renderer) {
    SDL_RenderCopy(renderer, text_texture, NULL, NULL);
}
