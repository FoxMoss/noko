#include "clay.h"
#include "homscreen.h"
#include "main.h"
#include "sdl_clay.h"
#include <cstddef>
#include <cstdio>

HomeScreen::HomeScreen() {
  App search = {"Search",
                std::optional(IMG_LoadTexture(renderer, PUBLIC_FOLDER
                                              "placeholder_app.png")),
                App::APP_INTERNAL,
                {.internal_callback = &HomeScreen::search_step}};
  for (size_t i = 0; i < 9; i++) {
    apps.push_back(search);
  }
}

void HomeScreen::render() {
  if (substep.has_value()) {
    substep.value()();
    return;
  }

  auto layout = generate_layout();
  SDL_Clay_RenderClayCommandsProxy(&render_data, &layout);
}

Clay_RenderCommandArray HomeScreen::generate_layout() {
  Clay_BeginLayout();

  Clay_Sizing layoutExpand = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)};

  size_t index = 0;
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout = {
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        }}) {
    for (size_t row = 0; row < 3; row++) {
      CLAY({.id = CLAY_IDI("Row", row),
            .layout = {
                .sizing = layoutExpand,
                .padding = CLAY_PADDING_ALL(16),
                .childGap = 16,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            }}) {
        for (size_t row = 0; row < 3; row++) {
          if (index >= apps.size()) {
            goto end_construction;
            App app = apps[index];
            Clay_String name = Clay_String{.isStaticallyAllocated = true,
                                           .length = (int32_t)strlen(app.name),
                                           .chars = app.name};

            CLAY({.id = CLAY_IDI("App", index),
                  .layout = {
                      .sizing = layoutExpand,
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  }}) {
              if (app.icon.has_value()) {
                CLAY({.id = CLAY_IDI("AppImage", index),
                      .layout = {.sizing = layoutExpand},
                      .aspectRatio = {1},
                      .image = {
                          .imageData = app.icon.value(),
                      }});
              }
              CLAY_TEXT(name, CLAY_TEXT_CONFIG({
                                  .textColor = {255, 255, 255, 255},
                                  .fontId = 0,
                                  .fontSize = 16,
                              }));
            }
            index++;
          }
        }
      }
    end_construction: {}
    }
  }

  return Clay_EndLayout();
}
