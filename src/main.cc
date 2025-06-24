#include "main.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#define PROG_NAME "noko"

void fatal_unwrap(bool cond, char *err) {
  if (!cond)
    return;
  printf("%s: %s\n", PROG_NAME, err);
  cleanup();
  exit(1);
  return;
}

int x11_warn(Display *display, XErrorEvent *event) {
  printf("%s: %s\n", PROG_NAME, "X11 had an unspecified error");
  return 0;
}

static Display *root_display = NULL;

typedef struct FrameData {
  Window child;
} FrameData;

std::unordered_map<Window, FrameData> frame_info;
std::unordered_map<Window, Window> frame_map;

static Window root_window;
void x11_create_display() {
  root_display = XOpenDisplay(NULL);
  root_window = DefaultRootWindow(root_display);
  fatal_unwrap(root_display == NULL, "Cannot open X11 display");
}
int fail_wm_detected(Display *display, XErrorEvent *event) {
  fatal_unwrap(true, "Another X11 WM was detected");
  return 0;
}
void x11_connect_display() {
  XSetErrorHandler(&fail_wm_detected);
  XSelectInput(root_display, root_window,
               SubstructureRedirectMask | SubstructureNotifyMask);
  XSync(root_display, false);
  XSetErrorHandler(&x11_warn);
}
void x11_handle_key(const XKeyEvent e) {
  if (frame_map.find(e.window) == frame_map.end())
    return;
  // not my window not my problem
  XDestroyWindow(root_display, e.window);
}
void x11_handle_map(const XMapRequestEvent e) {
  const unsigned int BORDER_WIDTH = 3;
  const unsigned long BORDER_COLOR = 0xff0000;
  const unsigned long BG_COLOR = 0x0000ff;

  XWindowAttributes x_window_attrs;
  XGetWindowAttributes(root_display, e.window, &x_window_attrs);

  char *name;
  XFetchName(root_display, e.window, &name);
  if (strcmp(name, "noko-desktop") == 0) {
    XWindowAttributes root_window_attrs;
    XGetWindowAttributes(root_display, root_window, &root_window_attrs);

    XMapWindow(root_display, e.window);
    XWindowChanges changes;
    changes.x = 0;
    changes.y = 0;
    changes.width = root_window_attrs.width;
    changes.height = root_window_attrs.height;
    changes.stack_mode = Below;

    XConfigureWindow(root_display, e.window,
                     CWX | CWY | CWWidth | CWHeight | CWStackMode, &changes);

    return;
  }

  const Window frame = XCreateSimpleWindow(
      root_display, root_window, x_window_attrs.x, x_window_attrs.y,
      x_window_attrs.width, x_window_attrs.height, BORDER_WIDTH, BORDER_COLOR,
      BG_COLOR);
  XSelectInput(root_display, frame,
               SubstructureRedirectMask | SubstructureNotifyMask |
                   KeyPressMask);
  XAddToSaveSet(root_display, e.window);
  XReparentWindow(root_display, e.window, frame, 0, 0);
  XMapWindow(root_display, frame);
  XMapWindow(root_display, e.window);

  XGrabKey(root_display, XKeysymToKeycode(root_display, XK_F4), Mod1Mask,
           e.window, false, GrabModeSync, GrabModeSync);

  frame_info[frame] = {e.window};
  frame_map[e.window] = frame;
}

void x11_handle_destroy_window(const XDestroyWindowEvent e) {
  if (frame_map.find(e.window) == frame_map.end())
    return;
  XDestroyWindow(root_display, frame_map[e.window]);
  frame_map.erase(frame_map.find(e.window));
  if (frame_info.find(frame_map[e.window]) == frame_info.end())
    return;
  frame_info.erase(frame_info.find(frame_map[e.window]));
}
void x11_handle_configure(const XConfigureRequestEvent e) {
  XWindowChanges changes;
  changes.x = e.x;
  changes.y = e.y;
  changes.width = e.width;
  changes.height = e.height;
  changes.border_width = e.border_width;
  changes.sibling = e.above;
  changes.stack_mode = e.detail;

  XConfigureWindow(root_display, e.window, e.value_mask, &changes);
}
void cleanup() { XCloseDisplay(root_display); }

int main(int argc, char **argv) {
  x11_create_display();
  x11_connect_display();

  while (1) {
    XEvent e;
    XNextEvent(root_display, &e);
    printf("event!\n");

    switch (e.type) {
    case CreateNotify: {
      break;
    }
    case ConfigureRequest: {
      x11_handle_configure(e.xconfigurerequest);
      break;
    }
    case MapRequest: {

      x11_handle_map(e.xmaprequest);
      break;
    }
    case DestroyNotify: {
      x11_handle_destroy_window(e.xdestroywindow);
      break;
    }
    case KeyPress: {
      x11_handle_key(e.xkey);

      break;
    }
    default:
      x11_warn(NULL, NULL);
    }
  }

  return 0;
}
