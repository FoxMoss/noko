#pragma once
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "main.h"
extern "C" {
#include "trie.h"
}
#include <SDL3_image/SDL_image.h>
#include <cstddef>
#include <optional>
#include <vector>

struct App {
  const char *name;
  std::optional<SDL_Texture *> icon;

  enum AppType { APP_INTERNAL, APP_EXTERNAL } type;
  union AppDataUnion {
    struct {
      void (*render)(HomeScreen *self, ProgState *);
      void (*key_event)(HomeScreen *self, SDL_Event *);

    } internal_callbacks;

    const char *external_path;
  } data;
};

class HomeScreen {
public:
  HomeScreen(ProgState *state, void (*display_app_callback)(const char *));
  ~HomeScreen() { trie_free(app_name_root); }

  void render(ProgState *state);
  void key_event(SDL_Event *event);

  static Clay_RenderCommandArray app_demo(ProgState *state, char *name);

private:
  std::optional<void (*)(HomeScreen *self, ProgState *state)> subrender = {};
  std::optional<void (*)(HomeScreen *self, SDL_Event *event)> subkey_event = {};
  std::optional<void (*)(const char *name)> display_app_callback;
  std::vector<App> apps;
  Clay_RenderCommandArray generate_layout(ProgState *);
  SDL_Texture *blank_app_texture;

  char search_text[256];
  size_t search_cursor = 0;
  static void traverse_apps(HomeScreen *self, ProgState *state, TrieNode *node,
                            size_t *index);
  static Clay_RenderCommandArray
  search_layout(HomeScreen *self, ProgState *state, const char *text);
  static void search_render(HomeScreen *self, ProgState *state);
  static std::optional<App> get_first_child(HomeScreen *self, TrieNode *node);
  static void search_key_event(HomeScreen *self, SDL_Event *event);

  TrieNode *app_name_root;
};
