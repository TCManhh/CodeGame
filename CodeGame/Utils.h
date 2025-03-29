#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <string>
#include "Structs.h"


SDL_Texture* loadTexture(const char* path);
void saveHighScore();
void loadHighScore();
void renderText(const std::string& text, int x, int y, SDL_Color color);
void renderButton(Button& button);
void cleanup();

#endif