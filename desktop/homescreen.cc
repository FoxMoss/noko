#include "homescreen.h"
#include "SDL3/SDL_keycode.h"
#include "main.h"
#include "sdl_clay.h"
#include "trie.h"
#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <math.h>
#include <stddef.h>

char char_to_t9(char c) {
  switch (tolower(c)) {
  case 'a':
  case 'b':
  case 'c':
    return '2';
  case 'd':
  case 'e':
  case 'f':
    return '3';
  case 'g':
  case 'h':
  case 'i':
    return '4';
  case 'j':
  case 'k':
  case 'l':
    return '5';
  case 'm':
  case 'n':
  case 'o':
    return '6';
  case 'p':
  case 'q':
  case 'r':
  case 's':
    return '7';
  case 't':
  case 'u':
  case 'v':
    return '8';
  case 'w':
  case 'x':
  case 'y':
  case 'z':
    return '9';
  default:
    return '\0';
  }
}

void to_t9(const char *word, char **output) {
  int index = 0;
  for (int i = 0; word[i] != '\0'; i++) {
    char t9_char = char_to_t9(word[i]);
    if (t9_char != '\0') {
      (*output)[index] = t9_char;
      index++;
    }
  }
  (*output)[index] = '\0';
}

HomeScreen::HomeScreen(ProgState *state,
                       void (*display_app_callback)(const char *))
    : display_app_callback(display_app_callback) {
  App search("Search",
             std::optional(IMG_LoadTexture(state->renderer,
                                           PUBLIC_FOLDER "search_app.png")),
             {&HomeScreen::search_render, &HomeScreen::search_key_event});
  apps.push_back(search);
  App editor("Editor",
             std::optional(IMG_LoadTexture(state->renderer, PUBLIC_FOLDER
                                           "placeholder_app.png")),
             "/usr/bin/gedit");
  apps.push_back(editor);
  App terminal("Terminal",
               std::optional(IMG_LoadTexture(state->renderer, PUBLIC_FOLDER
                                             "placeholder_app.png")),
               "/usr/bin/kitty");
  apps.push_back(terminal);

  read_desktop_apps_in(state, "/usr/share/applications/");

  app_name_root = trie_create_root();
  for (auto app : apps) {
    char *output = (char *)SDL_malloc(strlen(app.name.c_str()));
    to_t9(app.name.c_str(), &output);
    TrieNode *node = trie_fillout_path(app_name_root, output);
    trie_appened_child(node, trie_create_word({(char *)app.name_str, 1}));
    free(output);
  }
  search_text[0] = 0;

  blank_app_texture =
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "blank_app.png");
}

std::optional<App> HomeScreen::get_n_child(HomeScreen *self, TrieNode *node,
                                           size_t *index, size_t search_index) {
  if (node->type == TRIE_WORD) {
    if (*index != search_index) {
      (*index)++;
      return {};
    }

    std::optional<App> selected_app;
    for (auto app : self->apps) {
      if (app.name_str == node->data.w.str) {
        selected_app = app;
        break;
      }
    }
    return selected_app;
  }
  for (size_t i = 0; i < node->children_len; i++) {
    auto val = get_n_child(self, node->children[i], index, search_index);
    if (val.has_value())
      return val;
  }
  return {};
}
void HomeScreen::search_key_event(HomeScreen *self, SDL_Event *event) {
  char new_char = 0;

  if (event->key.key > SDLK_KP_DIVIDE && event->key.key < SDLK_KP_PERIOD) {
#ifndef FLIPED_KP
    if (event->key.key == SDLK_KP_7)
      new_char = '1';
    else if (event->key.key == SDLK_KP_8)
      new_char = '2';
    else if (event->key.key == SDLK_KP_9)
      new_char = '3';
    else if (event->key.key == SDLK_KP_4)
      new_char = '4';
    else if (event->key.key == SDLK_KP_5)
      new_char = '5';
    else if (event->key.key == SDLK_KP_6)
      new_char = '6';
    else if (event->key.key == SDLK_KP_1)
      new_char = '7';
    else if (event->key.key == SDLK_KP_2)
      new_char = '8';
    else if (event->key.key == SDLK_KP_3)
      new_char = '9';
#else
    new_char = event->key.key - SDLK_KP_1 + '1';
#endif
  } else {
    new_char = event->key.key - SDLK_1 + '1';
  }
  if (new_char < '1' || '9' < new_char) {
    new_char = 0;
  }

  if (new_char != 0) {
    self->search_index = 0;
    self->search_text[self->search_cursor] = new_char;
    if (self->search_cursor < 255)
      self->search_cursor += 1;
    self->search_text[self->search_cursor] = 0;
  }

  if (event->key.key == SDLK_KP_MULTIPLY || event->key.key == SDLK_DOWN) {
    self->search_index++;
    self->search_index %= self->search_apps_length;
  }
  if (event->key.key == SDLK_UP) {
    if (self->search_index == 0)
      self->search_index = self->search_apps_length - 1;
    else
      self->search_index--;
  }

  if (event->key.key == SDLK_KP_MINUS || event->key.key == SDLK_MINUS) {
    if (self->search_cursor != 0)
      self->search_cursor--;
    self->search_text[self->search_cursor] = 0;
  }
  if (event->key.key == SDLK_KP_ENTER || event->key.key == SDLK_RETURN ||
      event->key.key == SDLK_RIGHT) {
    TrieNode *node = trie_fillout_path(self->app_name_root, self->search_text);
    size_t index = 0;
    auto val = self->get_n_child(self, node, &index, self->search_index);
    if (val.has_value()) {
      self->subrender = {};
      self->subkey_event = {};
      self->search_index = 0;
      self->search_cursor = 0;
      self->search_text[0] = 0;
      switch (val.value().type) {
      case App::APP_INTERNAL:
        self->subrender = val.value().internal_callbacks.render;
        self->subkey_event = val.value().internal_callbacks.key_event;
        break;
      case App::APP_EXTERNAL:
        if (!self->display_app_callback.has_value())
          break;
        self->display_app_callback.value()(val.value().external_path.c_str());
      }
    }
  }
}
void HomeScreen::search_render(HomeScreen *self, ProgState *state) {
  auto layout = HomeScreen::search_layout(self, state, self->search_text);
  SDL_Clay_RenderClayCommandsProxy(&state->render_data, &layout);
}

void HomeScreen::key_event(SDL_Event *event) {
  // close submenue

  if (subkey_event.has_value()) {
    subkey_event.value()(this, event);
    if (event->key.key != SDLK_KP_PLUS && event->key.key != SDLK_EQUALS)
      return;
    subkey_event = {};
    subrender = {};
    return;
  }

  if ((event->key.key < SDLK_0 && event->key.key > SDLK_1) ||
      (event->key.key < SDLK_KP_DIVIDE && event->key.key > SDLK_KP_PERIOD))
    return;

  size_t real_i = 0;
  if (event->key.key > SDLK_KP_DIVIDE && event->key.key < SDLK_KP_PERIOD) {
#ifndef FLIPED_KP
    if (event->key.key == SDLK_KP_7)
      real_i = 0;
    else if (event->key.key == SDLK_KP_8)
      real_i = 1;
    else if (event->key.key == SDLK_KP_9)
      real_i = 2;
    else if (event->key.key == SDLK_KP_4)
      real_i = 3;
    else if (event->key.key == SDLK_KP_5)
      real_i = 4;
    else if (event->key.key == SDLK_KP_6)
      real_i = 5;
    else if (event->key.key == SDLK_KP_1)
      real_i = 6;
    else if (event->key.key == SDLK_KP_2)
      real_i = 7;
    else if (event->key.key == SDLK_KP_3)
      real_i = 8;
    else
      return;
#else
    real_i = event->key.key - SDLK_KP_1;
#endif
  } else {
    real_i = event->key.key - SDLK_1;
  }

  if (apps.size() <= real_i)
    return;

  switch (apps[real_i].type) {
  case App::APP_INTERNAL:
    subrender = apps[real_i].internal_callbacks.render;
    subkey_event = apps[real_i].internal_callbacks.key_event;
    break;
  case App::APP_EXTERNAL:
    if (!display_app_callback.has_value())
      break;
    display_app_callback.value()(apps[real_i].external_path.c_str());
  }
}

void HomeScreen::render(ProgState *state) {
  if (subrender.has_value()) {
    subrender.value()(this, state);
    return;
  }

  auto layout = generate_layout(state);
  SDL_Clay_RenderClayCommandsProxy(&state->render_data, &layout);
}

void HomeScreen::traverse_apps(HomeScreen *self, ProgState *state,
                               TrieNode *node, size_t *index) {
  if (node->type == TRIE_WORD) {
    std::optional<App> selected_app;
    for (auto app : self->apps) {
      if (app.name_str == node->data.w.str) { // uses same pointer
        selected_app = app;
        break;
      }
    }
    if (!selected_app.has_value())
      return;

    float font_size = (float)state->height / 25;
    float row_height = (float)state->height / 7;

    CLAY({.id = CLAY_IDI("App", *index),
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_GROW(0),
                             .height = CLAY_SIZING_FIXED(row_height)},
                  .childGap = 30,
                  .childAlignment = {.x = CLAY_ALIGN_X_LEFT,
                                     .y = CLAY_ALIGN_Y_CENTER},
                  .layoutDirection = CLAY_LEFT_TO_RIGHT,

              },
          .backgroundColor = {255, 255, 255,
                              (float)(*index == self->search_index ? 50 : 0)},
          .cornerRadius = CLAY_CORNER_RADIUS(10)}) {
      Clay_String name =
          Clay_String{.isStaticallyAllocated = true,
                      .length = (int32_t)selected_app->name.size(),
                      .chars = selected_app->name_str};
      CLAY({.id = CLAY_IDI("AppImage", *index),
            .layout =
                {
                    .sizing = {.width = {(float)state->height / 7},
                               .height = {(float)state->height / 7}},
                },
            .aspectRatio = {1},
            .image = {
                .imageData = selected_app->icon.has_value()
                                 ? selected_app->icon.value()
                                 : self->blank_app_texture,
            }});
      CLAY_TEXT(name,
                CLAY_TEXT_CONFIG({.textColor = {255, 255, 255, 255},
                                  .fontId = 0,
                                  .fontSize = (uint16_t)(font_size),
                                  .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
      *index = *index + 1;
    }
  }

  for (size_t i = 0; i < node->children_len; i++) {
    traverse_apps(self, state, node->children[i], index);
  }
}

Clay_RenderCommandArray HomeScreen::search_layout(HomeScreen *self,
                                                  ProgState *state,
                                                  const char *text) {
  Clay_BeginLayout();

  Clay_Sizing layoutExpand = {.width = {(float)state->width},
                              .height = {(float)state->height}};
  float font_size = (float)state->height / 25;
  float row_height = (float)state->height / 7;
  size_t index = 0;
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = layoutExpand,
            .padding = {.top = (uint16_t)((float)state->height / 14)},
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_TOP},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        }}) {
    CLAY({
        .id = CLAY_ID("ColoredContainer"),
        .layout =
            {
                .sizing = {.width = CLAY_SIZING_FIXED(
                               state->width - (float)state->height / 20),
                           .height = CLAY_SIZING_GROW(0)},
                .padding = CLAY_PADDING_ALL(10),
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        .backgroundColor = {0, 0, 0, 100},
        .cornerRadius = CLAY_CORNER_RADIUS(10),
        .clip = {.vertical = true},
    }) {
      Clay_String search_text =
          Clay_String{.isStaticallyAllocated = true,
                      .length = (int32_t)strlen(self->search_text),
                      .chars = self->search_text};
      if (self->search_cursor == 0) {
        search_text = CLAY_STRING("Search via T9...");
      }

      CLAY_TEXT(
          search_text,
          CLAY_TEXT_CONFIG({
              .textColor = {255, 255, 255,
                            (float)(self->search_cursor == 0 ? 200 : 255)},
              .fontId = 0,
              .fontSize = (uint16_t)(font_size),
          }));

      TrieNode *node =
          trie_fillout_path(self->app_name_root, self->search_text);
      size_t index = 0;
      float offset = fmin((-row_height * self->search_index) + 20, 0);
      CLAY({
          .id = CLAY_ID("AppList"),
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_GROW(0),
                             .height = CLAY_SIZING_FIT(0)},
                  .padding = CLAY_PADDING_ALL(10),
                  .childAlignment = {.x = CLAY_ALIGN_X_LEFT,
                                     .y = CLAY_ALIGN_Y_TOP},
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
          .clip = {.vertical = true, .childOffset = {.y = offset}},
      }) {

        traverse_apps(self, state, node, &index);
        self->search_apps_length = index;
      }
    }
  }

  return Clay_EndLayout();
}

Clay_RenderCommandArray HomeScreen::app_demo(ProgState *state, char *name) {
  Clay_BeginLayout();

  Clay_Sizing layoutExpand = {.width = {(float)state->width},
                              .height = {(float)state->height}};
  float font_size = (float)state->height / 25;
  size_t index = 0;
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = layoutExpand,
            .padding = {.top = (uint16_t)((float)state->height / 14)},
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                               .y = CLAY_ALIGN_Y_CENTER},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        }}) {
    CLAY({.id = CLAY_ID("ColoredContainer"),
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_FIXED(
                                 state->width - (float)state->height / 20),
                             .height = CLAY_SIZING_FIXED(
                                 state->height - (float)state->height / 20 -
                                 (float)state->height / 14)},
                  .padding = CLAY_PADDING_ALL(10),
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
          .backgroundColor = {255, 255, 255, 255},
          .cornerRadius = CLAY_CORNER_RADIUS(10)}) {
      Clay_String clay_name = Clay_String{.isStaticallyAllocated = true,
                                          .length = (int32_t)strlen(name),
                                          .chars = name};

      CLAY_TEXT(clay_name, CLAY_TEXT_CONFIG({
                               .textColor = {0, 0, 0, 255},
                               .fontId = 0,
                               .fontSize = (uint16_t)(font_size * 2),
                           }));
      CLAY_TEXT(
          CLAY_STRING("This is a fake application for the point of the demo!"),
          CLAY_TEXT_CONFIG({
              .textColor = {0, 0, 0, 255},
              .fontId = 0,
              .fontSize = (uint16_t)(font_size),
          }));
    }
  }

  return Clay_EndLayout();
}

Clay_RenderCommandArray HomeScreen::generate_layout(ProgState *state) {
  Clay_BeginLayout();

  Clay_Sizing layoutExpand = {.width = {(float)state->width},
                              .height = {(float)state->height}};
  float max_strlen = 0;
  float font_size = (float)state->height / 30;
  for (int i = 0; i < 9; i++) {
    App app = apps[i];
    int width, height;

    TTF_SetFontSize(state->lato_regular, font_size);
    if (!TTF_GetStringSize(state->lato_regular, app.name.c_str(), 0, &width,
                           &height)) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s",
                   SDL_GetError());
    }

    if (width > max_strlen) {
      max_strlen = width;
    }
  }

  size_t index = 0;
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = layoutExpand,
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                               .y = CLAY_ALIGN_Y_CENTER},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        }}) {
    CLAY({
        .id = CLAY_ID("ColoredContainer"),
        .layout =
            {
                .sizing = {.width = CLAY_SIZING_FIT(0),
                           .height = CLAY_SIZING_FIT(0)},
                .padding = CLAY_PADDING_ALL(
                    (uint16_t)min((float)state->width / 40, 16)),
                .layoutDirection = CLAY_TOP_TO_BOTTOM,

            },
        .backgroundColor = {0, 0, 0, 100},
        .cornerRadius = CLAY_CORNER_RADIUS(10),
    }) {
      for (size_t row = 0; row < 3; row++) {
        CLAY({.id = CLAY_IDI("Row", row),
              .layout =
                  {
                      .layoutDirection = CLAY_LEFT_TO_RIGHT,
                  }

        }) {
          for (size_t col = 0; col < 3; col++) {

            CLAY({.id = CLAY_IDI("App", index),
                  .layout = {
                      .sizing = {.width =
                                     CLAY_SIZING_FIXED((float)(max_strlen) + 8),
                                 .height = CLAY_SIZING_GROW(0)},
                      .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                         .y = CLAY_ALIGN_Y_CENTER},
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  }}) {
              if (index < apps.size()) {
                App app = apps[index];

                Clay_String name =
                    Clay_String{.isStaticallyAllocated = true,
                                .length = (int32_t)app.name.size(),
                                .chars = app.name_str};

                CLAY({.id = CLAY_IDI("AppImage", index),
                      .layout = {.sizing = {.width = {(float)state->height / 7},
                                            .height = {(float)state->height /
                                                       7}}},
                      .aspectRatio = {1},
                      .image = {
                          .imageData = app.icon.has_value() ? app.icon.value()
                                                            : blank_app_texture,
                      }});
                CLAY_TEXT(name, CLAY_TEXT_CONFIG(
                                    {.textColor = {255, 255, 255, 255},
                                     .fontId = 0,
                                     .fontSize = (uint16_t)(font_size),
                                     .textAlignment = CLAY_TEXT_ALIGN_CENTER}));

              } else { // No app just need to fill blank space
                CLAY({.id = CLAY_IDI("AppImage", index),
                      .layout = {.sizing = {.width = {(float)state->height / 7},
                                            .height = {(float)state->height /
                                                       7}}},
                      .aspectRatio = {1},
                      .image = {
                          .imageData = blank_app_texture,
                      }});
                CLAY({
                    .id = CLAY_IDI("AppText", index),
                    .layout = {.sizing = {.width = {0},
                                          .height = {font_size + 1}}},
                });
              }
            }
            index++;
          }
        }
      end_construction: {}
      }
    }
  }

  return Clay_EndLayout();
}
