
#include "homscreen.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "SDL3/SDL_surface.h"
#include "clay.h"
#include "main.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#define FONT_SIZE 20

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text,
                                              Clay_TextElementConfig *config,
                                              void *user_data) {
  TTF_Font *font = (TTF_Font *)user_data;
  int width, height;

  TTF_SetFontSize(font, config->fontSize);
  if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s",
                 SDL_GetError());
  }

  return (Clay_Dimensions){(float)width, (float)height};
}

void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Noko", "1.0", "com.foxmoss.noko");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("noko-desktop", width, height, 0, &window,
                                   &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (!TTF_Init()) {
    SDL_Log("Couldn't initialize TTF: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  lato_regular = TTF_OpenFont(PUBLIC_FOLDER "Lato-Bold.ttf", FONT_SIZE);
  TTF_SetFontWrapAlignment(lato_regular, TTF_HORIZONTAL_ALIGN_CENTER);

  SDL_GetWindowSize(window, &width, &height);
  wallpaper_texture =
      IMG_LoadTexture(renderer, PUBLIC_FOLDER "wallpaper_01.jpg");

  render_data.renderer = renderer;
  render_data.fonts = &lato_regular;
  render_data.textEngine = TTF_CreateRendererTextEngine(renderer);

  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena){
      .capacity = totalMemorySize,
      .memory = (char *)SDL_malloc(totalMemorySize),
  };
  Clay_Initialize(clayMemory, (Clay_Dimensions){(float)width, (float)height},
                  (Clay_ErrorHandler){HandleClayErrors});
  Clay_SetMeasureTextFunction(SDL_MeasureText, &lato_regular);

  home_screen = new HomeScreen();

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_WINDOW_RESIZED:
    SDL_GetWindowSize(window, &width, &height);
    SDL_UpdateWindowSurface(window);
    Clay_SetLayoutDimensions((Clay_Dimensions){(float)event->window.data1,
                                               (float)event->window.data2});
    break;
  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState((Clay_Vector2){event->motion.x, event->motion.y},
                         event->motion.state & SDL_BUTTON_LMASK);
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState((Clay_Vector2){event->button.x, event->button.y},
                         event->button.button == SDL_BUTTON_LEFT);
    break;
  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){event->wheel.x, event->wheel.y}, 0.01f);
    break;
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

  char time[256];
  sntime(&time[0], 256);
  SDL_Surface *text_surface = generate_font_surface(lato_regular, time);
  SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_DestroySurface(text_surface);
  const SDL_FRect text_rect = {(float)width / 2 - (float)text->w / 2, 0,
                               (float)text->w, (float)text->h};
  SDL_RenderTexture(renderer, text, NULL, &text_rect);

  SDL_DestroyTexture(text);

  home_screen->render();

  SDL_RenderPresent(renderer);
  printf("render %d x %d\n", width, height);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_DestroyTexture(wallpaper_texture);
  TTF_Quit();
  delete home_screen;
}
