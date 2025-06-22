#include "main.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
void x11_handle_map(const XMapRequestEvent e) {
  const unsigned int BORDER_WIDTH = 3;
  const unsigned long BORDER_COLOR = 0xff0000;
  const unsigned long BG_COLOR = 0x0000ff;

  XWindowAttributes x_window_attrs;
  XGetWindowAttributes(root_display, e.window, &x_window_attrs);

  const Window frame = XCreateSimpleWindow(
      root_display, root_window, x_window_attrs.x, x_window_attrs.y,
      x_window_attrs.width, x_window_attrs.height, BORDER_WIDTH, BORDER_COLOR,
      BG_COLOR);
  XSelectInput(root_display, frame,
               SubstructureRedirectMask | SubstructureNotifyMask);
  XAddToSaveSet(root_display, e.window);
  XReparentWindow(root_display, e.window, frame, 0, 0);
  XMapWindow(root_display, frame);
  XMapWindow(root_display, e.window);
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
    case MapRequest:
      x11_handle_map(e.xmaprequest);
      break;

    case DestroyNotify:
      break;
    case ReparentNotify:
      break;
    default:
      x11_warn(NULL, NULL);
    }
  }

  return 0;
}
