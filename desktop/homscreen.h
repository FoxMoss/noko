#pragma once
#include "clay.h"
#include <SDL3_image/SDL_image.h>
#include <optional>
#include <vector>

struct App {
  const char *name;
  std::optional<SDL_Texture *> icon;

  enum AppType { APP_INTERNAL, APP_EXTERNAL } type;
  union AppDataUnion {
    void (*internal_callback)();
    const char *external_path;
  } data;
};

class HomeScreen {
public:
  HomeScreen();
  ~HomeScreen() {}

  void render();

private:
  std::optional<void (*)()> substep = {};
  std::vector<App> apps;
  static void search_step() {}
  Clay_RenderCommandArray generate_layout();
};
