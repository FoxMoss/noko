#pragma once

struct ProgState;
class HomeScreen;

#include "SDL3_ttf/SDL_ttf.h"
#include "homescreen.h"
#include "sdl_clay.h"

#ifndef EMSCRIPTEN
// Ive replaced this with a cmake injected macro.
//#define PUBLIC_FOLDER "../public/"
#else
#define PUBLIC_FOLDER "./"
#endif

struct ProgState {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *wallpaper_texture = NULL;
  TTF_Font *lato_regular = NULL;
  HomeScreen *home_screen;
  Clay_SDL3RendererData render_data;

  int width;
  int height;
};
Clay_Dimensions SDL_MeasureText(Clay_StringSlice text,
                                Clay_TextElementConfig *config, void *userData);
