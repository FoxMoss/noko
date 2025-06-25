#pragma once

#include "SDL3_ttf/SDL_ttf.h"
#include "clay.h"
#include "homscreen.h"
#include "sdl_clay.h"
#define PUBLIC_FOLDER "../public/"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *wallpaper_texture = NULL;
static TTF_Font *lato_regular = NULL;
static HomeScreen *home_screen;
static Clay_SDL3RendererData render_data;

static int width = 640;
static int height = 480;
