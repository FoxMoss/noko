#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include <stddef.h>

float min(float a, float b);
SDL_FRect cover_rect(int texture_width, int texture_height, int width,
                     int height);
SDL_Surface *generate_font_surface(TTF_Font *font, const char *text);
void sntime(char *output, size_t max_len);
