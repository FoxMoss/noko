#include "homescreen.h"
#include "SDL3/SDL_keycode.h"
#include "main.h"
#include "sdl_clay.h"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>

HomeScreen::HomeScreen(ProgState *state,
                       void (*display_app_callback)(const char *))
    : display_app_callback(display_app_callback) {
  App search = {"Search",
                std::optional(IMG_LoadTexture(state->renderer,
                                              PUBLIC_FOLDER "search_app.png")),
                App::APP_INTERNAL,
                {.internal_callbacks = {&HomeScreen::search_render,
                                        &HomeScreen::search_key_event}}};
  apps.push_back(search);
  App editor = {"Editor",
                std::optional(IMG_LoadTexture(state->renderer, PUBLIC_FOLDER
                                              "placeholder_app.png")),
                App::APP_EXTERNAL,
                {.external_path = "gedit"}};
  apps.push_back(editor);
  blank_app_texture =
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "blank_app.png");
}

void HomeScreen::key_event(SDL_Event *event) {
  // close submenue
  if (event->key.key == SDLK_KP_PLUS)
    subkey_event = {};
  subrender = {};

  if (subkey_event.has_value()) {
    subkey_event.value()(event);
    return;
  }

  size_t real_i = 0;
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

  if (apps.size() <= real_i)
    return;

  switch (apps[real_i].type) {
  case App::APP_INTERNAL:
    subrender = apps[real_i].data.internal_callbacks.render;
    subkey_event = apps[real_i].data.internal_callbacks.key_event;
    break;
  case App::APP_EXTERNAL:
    if (!display_app_callback.has_value())
      break;
    display_app_callback.value()(apps[real_i].data.external_path);
  }
}

void HomeScreen::render(ProgState *state) {
  if (subrender.has_value()) {
    subrender.value()(state);
    return;
  }

  auto layout = generate_layout(state);
  SDL_Clay_RenderClayCommandsProxy(&state->render_data, &layout);
}

Clay_RenderCommandArray HomeScreen::app_demo(ProgState *state) {
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
  for (auto app : apps) {
    if (strlen(app.name) > max_strlen) {
      max_strlen = strlen(app.name);
    }
  }
  float font_size = (float)state->height / 25;
  size_t index = 0;
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = layoutExpand,
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                               .y = CLAY_ALIGN_Y_CENTER},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        }}) {
    CLAY({.id = CLAY_ID("ColoredContainer"),
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_FIT(0),
                             .height = CLAY_SIZING_FIT(0)},
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,

              },
          .backgroundColor = {0, 0, 0, 100},
          .cornerRadius = CLAY_CORNER_RADIUS(10)}) {
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
                      .sizing = {.width = CLAY_SIZING_FIXED(
                                     (float)(font_size * 0.8 * max_strlen)),
                                 .height = CLAY_SIZING_GROW(0)},
                      .padding = CLAY_PADDING_ALL(
                          (uint16_t)min((float)state->width / 40, 16)),
                      .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                         .y = CLAY_ALIGN_Y_CENTER},
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  }}) {
              if (index < apps.size()) {
                App app = apps[index];

                Clay_String name =
                    Clay_String{.isStaticallyAllocated = true,
                                .length = (int32_t)strlen(app.name),
                                .chars = app.name};
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
