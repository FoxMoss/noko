#pragma once
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "inipp.h"
#include "main.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
extern "C" {
#include "trie.h"
}
#include <SDL3_image/SDL_image.h>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

struct AppCallbacks {
  void (*render)(HomeScreen *self, ProgState *);
  void (*key_event)(HomeScreen *self, SDL_Event *);
};

class App {
public:
  App(std::string name, std::optional<SDL_Texture *> icon,
      AppCallbacks callbacks)
      : name(name), icon(icon), type(APP_INTERNAL),
        internal_callbacks(callbacks) {
    name_str = (char *)malloc(name.size() + 1);
    memcpy(name_str, name.data(), name.size());
  }
  App(std::string name, std::optional<SDL_Texture *> icon, std::string path)
      : name(name), icon(icon), type(APP_EXTERNAL), external_path(path) {
    name_str = (char *)malloc(name.size());
    memcpy(name_str, name.data(), name.size());
  }
  ~App() {}

  std::string name;
  char *name_str;
  std::optional<SDL_Texture *> icon;

  enum AppType { APP_INTERNAL, APP_EXTERNAL } type;
  AppCallbacks internal_callbacks;

  std::string external_path;
};

class HomeScreen {
public:
  HomeScreen(ProgState *state, void (*display_app_callback)(const char *));
  ~HomeScreen() { trie_free(app_name_root); }

  void render(ProgState *state);
  void key_event(SDL_Event *event);

  static Clay_RenderCommandArray app_demo(ProgState *state, char *name);

private:
  std::optional<std::string> find_executable(std::string executable_name) {
    for (auto bin_dir : {"/usr/bin/", "/bin/", "/usr/local/bin/"}) {
      if (!std::filesystem::exists(bin_dir))
        continue;

      for (auto file : std::filesystem::directory_iterator(bin_dir)) {
        if (!file.is_regular_file())
          continue;
        if (file.path().filename() != executable_name)
          continue;
        return file.path().string();
      }
    }
    return {};
  }

  std::optional<std::string> find_icon(std::string icon_name) {
    for (auto icon_dir : {"/usr/share/icons/hicolor/256x256/apps/",
                          "/usr/share/icons/hicolor/scalable/apps/"}) {

      if (!std::filesystem::exists(icon_dir))
        continue;

      for (auto file : std::filesystem::directory_iterator(icon_dir)) {
        if (!file.is_regular_file())
          continue;
        if (file.path().stem() != icon_name)
          continue;
        return file.path().string();
      }
    }
    return {};
  }

  void read_desktop_apps_in(ProgState *state, std::string dir) {
    if (!std::filesystem::exists(dir))
      return;
    for (auto file : std::filesystem::directory_iterator(dir.c_str())) {
      if (!file.is_regular_file())
        continue;
      if (file.path().extension() != ".desktop")
        continue;

      inipp::Ini<char> ini;
      std::ifstream is(file.path());
      ini.parse(is);
      if (!ini.sections.contains("Desktop Entry"))
        continue;

      std::string executable_name;
      if (!inipp::get_value(ini.sections["Desktop Entry"], "Exec",
                            executable_name))
        continue;
      if (executable_name.size() < 2)
        continue;
      std::string name;
      if (!inipp::get_value(ini.sections["Desktop Entry"], "Name", name))
        continue;
      std::string icon_name;
      if (!inipp::get_value(ini.sections["Desktop Entry"], "Icon", icon_name))
        continue;

      auto icon_file_op = find_icon(icon_name);
      if (!icon_file_op.has_value())
        continue;

      std::string icon_file = icon_file_op.value();

      std::string executable_path;
      if (executable_name[0] == '/') // prob a path
        executable_path = executable_name;
      else {
        auto executable_path_op = find_executable(executable_name);
        if (!executable_path_op.has_value())
          continue;
        executable_path = executable_path_op.value();
      }

      App app(
          name,
          std::optional(IMG_LoadTexture(state->renderer, icon_file.c_str())),
          executable_path);
      apps.push_back(app);
    }
  }
  std::optional<void (*)(HomeScreen *self, ProgState *state)> subrender = {};
  std::optional<void (*)(HomeScreen *self, SDL_Event *event)> subkey_event = {};
  std::optional<void (*)(const char *name)> display_app_callback;
  std::vector<App> apps;
  Clay_RenderCommandArray generate_layout(ProgState *);
  SDL_Texture *blank_app_texture;

  char search_text[256];
  size_t search_cursor = 0;
  size_t search_index = 0;
  size_t search_apps_length = 0;
  static void traverse_apps(HomeScreen *self, ProgState *state, TrieNode *node,
                            size_t *index);
  static Clay_RenderCommandArray
  search_layout(HomeScreen *self, ProgState *state, const char *text);
  static void search_render(HomeScreen *self, ProgState *state);
  static void search_key_event(HomeScreen *self, SDL_Event *event);
  std::optional<App> get_n_child(HomeScreen *self, TrieNode *node,
                                 size_t *index, size_t search_index);

  TrieNode *app_name_root;
};
