#include "SDL3/SDL_surface.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <stdio.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *wallpaper_texture = NULL;
static TTF_Font *lato_regular = NULL;

static int width = 640;
static int height = 480;

#define FONT_SIZE 20
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Noko", "1.0", "com.foxmoss.noko");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("noko-desktop", width, height,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!TTF_Init()) {
    SDL_Log("Couldn't initialize TTF: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  lato_regular = TTF_OpenFont("../public/Lato-Bold.ttf", FONT_SIZE);
  TTF_SetFontWrapAlignment(lato_regular, TTF_HORIZONTAL_ALIGN_CENTER);

  SDL_GetWindowSize(window, &width, &height);
  wallpaper_texture = IMG_LoadTexture(renderer, "../public/wallpaper_01.jpg");

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_WINDOW_RESIZED: {
    SDL_GetWindowSize(window, &width, &height);
    SDL_UpdateWindowSurface(window);

  } break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  const int charsize = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  SDL_FRect wallpaper_rect =
      cover_rect(wallpaper_texture->w, wallpaper_texture->h, width, height);

  SDL_RenderTexture(renderer, wallpaper_texture, &wallpaper_rect, NULL);

  SDL_Surface *text_surface = generate_font_surface(lato_regular, "8:00");
  SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_DestroySurface(text_surface);
  const SDL_FRect text_rect = {(float)width / 2 - (float)text->w / 2, 0,
                               text->w, text->h};
  SDL_RenderTexture(renderer, text, NULL, &text_rect);

  SDL_DestroyTexture(text);
  SDL_RenderPresent(renderer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_DestroyTexture(wallpaper_texture);
}
