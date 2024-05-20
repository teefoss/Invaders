//
//  text.h
//  Invaders
//
//  Created by Thomas Foster on 2/26/24.
//

#ifndef text_h
#define text_h

#include <SDL2/SDL.h>

void LoadText(SDL_Renderer * renderer);
void Test(SDL_Renderer * renderer);
void DrawChar(SDL_Renderer * renderer, char ascii, int x, int y);
void DrawString(SDL_Renderer * renderer, int x, int y, const char * format, ...);

#endif /* text_h */
