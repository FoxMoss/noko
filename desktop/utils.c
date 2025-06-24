#include "utils.h"
#include "SDL3/SDL_surface.h"
#include <string.h>

#define OUTLINE_SIZE 2

float min(float a, float b) {
  if (a < b)
    return a;
  return b;
}
SDL_FRect cover_rect(int texture_width, int texture_height, int width,
                     int height) {
  float min_size = min(texture_width, texture_height);
  float y_ratio = (float)height / (float)width;
  float x_ratio = (float)width / (float)height;
  SDL_FRect dest_rect = {};
  if (width > height) {
    dest_rect.w = min_size;
    dest_rect.h = min_size * y_ratio;
  } else {
    dest_rect.w = min_size * x_ratio;
    dest_rect.h = min_size;
  }
  dest_rect.x = (float)texture_width / 2 - dest_rect.w / 2;
  dest_rect.y = (float)texture_height / 2 - dest_rect.h / 2;
  return dest_rect;
}

SDL_Surface *generate_font_surface(TTF_Font *font, char *text) {
  TTF_Font *font_outline = TTF_CopyFont(font);
  TTF_SetFontOutline(font_outline, OUTLINE_SIZE);

  size_t text_len = strlen(text);
  SDL_Color white = {0xFF, 0xFF, 0xFF};
  SDL_Color black = {0x00, 0x00, 0x00};
  SDL_Surface *bg_surface =
      TTF_RenderText_Blended(font_outline, text, text_len, black);
  SDL_Surface *fg_surface = TTF_RenderText_Blended(font, text, text_len, white);
  SDL_Rect rect = {OUTLINE_SIZE, OUTLINE_SIZE, fg_surface->w, fg_surface->h};

  SDL_SetSurfaceBlendMode(fg_surface, SDL_BLENDMODE_BLEND);
  SDL_BlitSurface(fg_surface, NULL, bg_surface, &rect);
  SDL_DestroySurface(fg_surface);
  TTF_CloseFont(font_outline);
  return bg_surface;
}
