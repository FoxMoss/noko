#pragma once
#include "clay.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
  SDL_Renderer *renderer;
  TTF_TextEngine *textEngine;
  TTF_Font **fonts;
} Clay_SDL3RendererData;

extern "C" {

void SDL_Clay_RenderClayCommandsProxy(Clay_SDL3RendererData *rendererData,
                                      Clay_RenderCommandArray *rcommands);
}
