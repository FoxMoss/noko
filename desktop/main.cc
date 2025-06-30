
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_stdinc.h"
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <pthread.h>
#include <spawn.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "SDL3/SDL_surface.h"
#include "homescreen.h"
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
                                              void *userData) {
  TTF_Font **fonts = (TTF_Font **)userData;
  TTF_Font *font = fonts[config->fontId];
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

#ifdef EMSCRIPTEN
bool show_app = false;
#endif

const char *app_name = NULL;
static void display_app(const char *c_app_name) {
#ifdef EMSCRIPTEN
  show_app = true;
  app_name = c_app_name;
#else

  int pid = fork();
  if (pid == 0) {
    char *args[] = {(char *)c_app_name, NULL};
    execv(c_app_name, args);
    exit(1);
  }
#endif
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Noko", "1.0", "com.foxmoss.noko");

  ProgState *state = (ProgState *)SDL_calloc(1, sizeof(ProgState));
  *appstate = state;
  state->width = 240;
  state->height = 320;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("noko-desktop", state->width, state->height,
                                   SDL_WINDOW_RESIZABLE, &state->window,
                                   &state->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!TTF_Init()) {
    SDL_Log("Couldn't initialize TTF: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  state->lato_regular = TTF_OpenFont(PUBLIC_FOLDER "Lato-Bold.ttf", FONT_SIZE);
  if (state->lato_regular == NULL) {
    SDL_Log("Couldn't initialize font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  TTF_SetFontWrapAlignment(state->lato_regular, TTF_HORIZONTAL_ALIGN_CENTER);

  SDL_GetWindowSize(state->window, &state->width, &state->height);
  state->wallpaper_texture =
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "wallpaper_01.jpg");

  state->render_data.renderer = state->renderer;
  state->render_data.fonts = (TTF_Font **)SDL_malloc(sizeof(TTF_Font *));
  state->render_data.textEngine = TTF_CreateRendererTextEngine(state->renderer);
  state->render_data.fonts[0] = state->lato_regular;

  size_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena){
      .capacity = totalMemorySize,
      .memory = (char *)SDL_malloc(totalMemorySize),
  };
  Clay_Initialize(clayMemory,
                  (Clay_Dimensions){(float)state->width, (float)state->height},
                  (Clay_ErrorHandler){HandleClayErrors});
  Clay_SetMeasureTextFunction(SDL_MeasureText, state->render_data.fonts);

  state->home_screen = new HomeScreen(state, &display_app);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ProgState *state = (ProgState *)appstate;

  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_WINDOW_RESIZED:
    SDL_GetWindowSize(state->window, &state->width, &state->height);
    SDL_UpdateWindowSurface(state->window);
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
  case SDL_EVENT_KEY_DOWN:
#ifdef EMSCRIPTEN
    if (show_app && event->key.key == SDLK_KP_PLUS)
      show_app = false;
    if (show_app)
      break;
#endif

    state->home_screen->key_event(event);
    break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  ProgState *state = (ProgState *)appstate;

  const int charsize = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(state->renderer);

  SDL_FRect wallpaper_rect =
      cover_rect(state->wallpaper_texture->w, state->wallpaper_texture->h,
                 state->width, state->height);

  SDL_RenderTexture(state->renderer, state->wallpaper_texture, &wallpaper_rect,
                    NULL);

  char time[256];
  sntime(&time[0], 256);
  TTF_SetFontSize(state->lato_regular, (float)state->height / 20);
  SDL_Surface *text_surface = generate_font_surface(state->lato_regular, time);
  SDL_Texture *text =
      SDL_CreateTextureFromSurface(state->renderer, text_surface);
  SDL_DestroySurface(text_surface);
  const SDL_FRect text_rect = {(float)state->width / 2 - (float)text->w / 2, 0,
                               (float)text->w, (float)text->h};
  SDL_RenderTexture(state->renderer, text, NULL, &text_rect);

  SDL_DestroyTexture(text);

  state->home_screen->render(state);

#ifdef EMSCRIPTEN
  if (show_app) {
    auto layout = HomeScreen::app_demo(state, (char *)app_name);
    SDL_Clay_RenderClayCommandsProxy(&state->render_data, &layout);
  }
#endif

  SDL_RenderPresent(state->renderer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  ProgState *state = (ProgState *)appstate;

  SDL_DestroyTexture(state->wallpaper_texture);
  TTF_Quit();
  delete state->home_screen;
}
