#pragma once
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "main.h"
#include <SDL3_image/SDL_image.h>
#include <optional>
#include <vector>

struct App {
  const char *name;
  std::optional<SDL_Texture *> icon;

  enum AppType { APP_INTERNAL, APP_EXTERNAL } type;
  union AppDataUnion {
    struct {
      void (*render)(ProgState *);
      void (*key_event)(SDL_Event *);

    } internal_callbacks;

    const char *external_path;
  } data;
};

class HomeScreen {
public:
  HomeScreen(ProgState *state, void (*display_app_callback)(const char *));
  ~HomeScreen() {}

  void render(ProgState *state);
  void key_event(SDL_Event *event);

  static Clay_RenderCommandArray app_demo(ProgState *state);

private:
  std::optional<void (*)(ProgState *state)> subrender = {};
  std::optional<void (*)(SDL_Event *event)> subkey_event = {};
  std::optional<void (*)(const char *name)> display_app_callback;
  std::vector<App> apps;
  Clay_RenderCommandArray generate_layout(ProgState *);
  SDL_Texture *blank_app_texture;

  static void search_render(ProgState *state) {}
  static void search_key_event(SDL_Event *event) {}
};
