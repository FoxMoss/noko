#include "main.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

#define PROG_NAME "noko"

void fatal_unwrap(bool cond, char *err) {
  if (!cond)
    return;
  printf("%s: %s\n", PROG_NAME, err);
  cleanup();
  exit(1);
  return;
}

static Display *root_display = NULL;

int x11_warn(Display *display, XErrorEvent *event) {
  printf("%s: %s\n", PROG_NAME, "X11 had an unspecified error");
  return 0;
}

typedef struct FrameData {
  Window child;
} FrameData;

std::unordered_map<Window, FrameData> frame_info;
std::unordered_map<Window, Window> frame_map;
std::vector<Window> window_queue;
std::optional<Window> focused_window;
static Atom take_focus;
static Atom active_window;
static Atom protocols_atom;

static Window root_window;
void x11_create_display() {
  root_display = XOpenDisplay(NULL);
  root_window = DefaultRootWindow(root_display);
  fatal_unwrap(root_display == NULL, "Cannot open X11 display");
  take_focus = XInternAtom(root_display, "WM_TAKE_FOCUS", False);
  active_window = XInternAtom(root_display, "_NET_ACTIVE_WINDOW", False);
  protocols_atom = XInternAtom(root_display, "WM_PROTOCOLS", False);
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

  XGrabServer(root_display);
  Window returned_root, returned_parent;
  Window *top_level_windows;
  unsigned int num_top_level_windows;
  XQueryTree(root_display, root_window, &returned_root, &returned_parent,
             &top_level_windows, &num_top_level_windows);
  for (unsigned int i = 0; i < num_top_level_windows; ++i) {
    frame(top_level_windows[i], true);
  }
  XFree(top_level_windows);
  XUngrabServer(root_display);
}

void x11_grab_keys(Window window) {
  XUngrabKey(root_display, AnyKey, AnyModifier, root_window);
  XGrabKey(root_display, XKeysymToKeycode(root_display, XK_KP_Add), AnyModifier,
           window, false, GrabModeAsync, GrabModeSync);
  XGrabKey(root_display, XKeysymToKeycode(root_display, XK_KP_Multiply),
           AnyModifier, window, false, GrabModeAsync, GrabModeSync);
}
void x11_focus(Window win) {

  char *name;
  XFetchName(root_display, win, &name);
  if (name != NULL) {
    printf("%s\n", name);
    XFree(name);
  }
  else {
    printf("win name not found\n");
  }

  if (frame_map.find(win) == frame_map.end()) {
    printf("noko: failed to find frame\n");
    return;
  }

  XRaiseWindow(root_display, frame_map[win]);
  for (auto other_win : window_queue) {
    if (other_win == win)
      continue;
    XLowerWindow(root_display, other_win);
  }
  if (focused_window.has_value()) {
    XSetInputFocus(root_display, root_window, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(root_display, root_window, active_window);
  }
  focused_window = win;

  int n;
  Atom *protocols;
  int exists = 0;
  XEvent ev;

  if (XGetWMProtocols(root_display, win, &protocols, &n)) {
    while (!exists && n--)
      exists = protocols[n] == active_window;
    XFree(protocols);
  }
  if (exists) {
    ev.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = protocols_atom;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = active_window;
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(root_display, win, False, NoEventMask, &ev);
  }
  x11_grab_keys(win);
  XSetInputFocus(root_display, win, RevertToPointerRoot, CurrentTime);
  XChangeProperty(root_display, root_window, active_window, XA_WINDOW, 32,
                  PropModeReplace, (unsigned char *)&win, 1);
}
void x11_kill_window(Window win) {
  XKillClient(root_display, win); // will destroy elsewher
}

void x11_handle_key(const XKeyEvent e) {
  if (frame_map.find(e.window) == frame_map.end())
    return;
  if (e.keycode == XKeysymToKeycode(root_display, XK_KP_Add)) {
    x11_kill_window(e.window);
  } else if (e.keycode == XKeysymToKeycode(root_display, XK_KP_Multiply)) {
    size_t index = 0;
    for (auto window : window_queue) {
      char *name;
      XFetchName(root_display, window, &name);
      printf("[%zu] %s\n", index, name);
      index++;
      XFree(name);
    }

    if (window_queue.size() == 0)
      return;
    if (window_queue.size() == 1) {
      printf("just the one\n");
      x11_focus(window_queue[0]);
      return;
    }

    if (!focused_window.has_value()) {
      x11_focus(window_queue[0]);
      return;
    }

    auto queue_iter = std::find(window_queue.begin(), window_queue.end(),
                                focused_window.value());
    if (queue_iter == window_queue.end() ||
        queue_iter + 1 == window_queue.end()) {
      x11_focus(window_queue[0]);
      return;
    }
    x11_focus(*(queue_iter + 1).base());
  }
}

void frame(Window window, bool old) {
  const unsigned int BORDER_WIDTH = 1;
  const unsigned long BORDER_COLOR = 0xffffff;
  const unsigned long BG_COLOR = 0x000000;

  XWindowAttributes x_window_attrs;
  XGetWindowAttributes(root_display, window, &x_window_attrs);

  if (old) {
    if (x_window_attrs.override_redirect ||
        x_window_attrs.map_state != IsViewable) {
      return;
    }
  }

  const Window frame = XCreateSimpleWindow(
      root_display, root_window, x_window_attrs.x, x_window_attrs.y,
      x_window_attrs.width, x_window_attrs.height, BORDER_WIDTH, BORDER_COLOR,
      BG_COLOR);
  XSelectInput(root_display, frame,
                SubstructureRedirectMask |
                   SubstructureNotifyMask);
  XSelectInput(root_display, window,
             EnterWindowMask | FocusChangeMask | SubstructureRedirectMask |
                 SubstructureNotifyMask);

  XAddToSaveSet(root_display, window);
  XReparentWindow(root_display, window, frame, 0, 0);
  XMapWindow(root_display, frame);
  XMapWindow(root_display, window);

  frame_info[frame] = {window};
  frame_map[window] = frame;
  window_queue.push_back(window);
  x11_focus(window);
}
void x11_handle_map(const XMapRequestEvent e) {
  XWindowAttributes root_window_attrs;
  XGetWindowAttributes(root_display, root_window, &root_window_attrs);

  char *name;
  XFetchName(root_display, e.window, &name);
  if (strcmp(name, "noko-desktop") == 0) {
    XFree(name);
    XWindowChanges changes;
    changes.x = 0;
    changes.y = 0;
    changes.width = root_window_attrs.width;
    changes.height = root_window_attrs.height;
    changes.stack_mode = Below;

    XConfigureWindow(root_display, e.window,
                     CWX | CWY | CWWidth | CWHeight | CWStackMode, &changes);
    XMapWindow(root_display, e.window);

    return;
  }
  XFree(name);

  XWindowChanges changes;
  changes.x = root_window_attrs.height / 40;
  changes.y = root_window_attrs.height / 14;
  changes.width = root_window_attrs.width - root_window_attrs.height / 20;
  changes.height = root_window_attrs.height - root_window_attrs.height / 20 -
                   root_window_attrs.height / 14;

  XConfigureWindow(root_display, e.window, CWX | CWY | CWWidth | CWHeight,
                   &changes);

  frame(e.window, false);
}

void x11_handle_destroy_window(const XDestroyWindowEvent e) {
  if (frame_map.find(e.window) == frame_map.end())
    return;
  XDestroyWindow(root_display, frame_map[e.window]);
  auto win_iter = frame_map.find(e.window);
  auto frame_iter = frame_info.find(frame_map[e.window]);
  if (win_iter != frame_map.end())
    frame_map.erase(win_iter);
  if (frame_iter != frame_info.end())
    frame_info.erase(frame_iter);
  auto queue_iter =
      std::find(window_queue.begin(), window_queue.end(), e.window);
  if (queue_iter != window_queue.end())
    window_queue.erase(queue_iter);
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
    case FocusIn: {
      XFocusChangeEvent *ev = &e.xfocus;

      if (focused_window.has_value() && ev->window != focused_window.value())
        x11_focus(ev->window);
      break;
    }
      case EnterNotify: {
      XCrossingEvent *ev = &e.xcrossing;

      x11_focus(ev->window);
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
