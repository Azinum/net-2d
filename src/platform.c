// platform.c

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // XSizeHints

#include "common.h"
#include "renderer.h"
#include "platform.h"

typedef struct Window_state {
  i32 width;
  i32 height;
  Display* display;
  Window window;
  Visual* visual;
  i32 screen;
  i32 depth;

  XImage* image;
  GC graphics_context;
  Atom atom_wm_delete;
} Window_state;

Window_state win = {};

i32 platform_open_window(i32 width, i32 height, const char* title) {
  win.width = width;
  win.height = height;

  win.display = XOpenDisplay(NULL);
  if (!win.display) {
    errprintf("X11: Failed to open display\n");
    return Error;
  }

  XSetWindowAttributes attribs = {};
  win.screen = XDefaultScreen(win.display);
  win.depth = XDefaultDepth(win.display, win.screen);
  win.visual = XDefaultVisual(win.display, win.screen);
  Window parent = XRootWindow(win.display, win.screen);
  win.window = XCreateWindow(
    win.display,
    parent,
    0,  // x
    0,  // y
    win.width,
    win.height,
    0,  // border width
    win.depth,
    InputOutput,
    win.visual,
    CWEventMask | CWColormap,
    &attribs
  );

  if (!win.window) {
    errprintf("X11: Failed to create window\n");
    return Error;
  }

  XSizeHints hints;
  memset(&hints, 0, sizeof(hints));
  hints.flags = PSize | PMinSize | PMaxSize;
  hints.min_width = hints.max_width = hints.base_width = win.width;
  hints.min_height = hints.max_height = hints.base_height = win.height;

  XSetWMNormalHints(win.display, win.window, &hints);

  win.atom_wm_delete = XInternAtom(win.display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(win.display, win.window, &win.atom_wm_delete, 1);

  XFlush(win.display);

  XSelectInput(win.display, win.window, StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask);

  win.graphics_context = XCreateGC(win.display, win.window, 0, NULL);
  if (!win.graphics_context) {
    errprintf("X11: Failed to create graphics context\n");
    return Error;
  }

  win.image = XCreateImage(win.display, NULL, 24, ZPixmap, 0, 0, win.width, win.height, 32, 0);
  if (!win.image) {
    errprintf("X11: Failed to create image\n");
    return Error;
  }

  XStoreName(win.display, win.window, title);
  XMapWindow(win.display, win.window);
  XMapRaised(win.display, win.window);

  return NoError;
}

i32 platform_process_events() {
  XEvent event;
  while (XPending(win.display)) {
    XNextEvent(win.display, &event);
    switch (event.type) {
      case KeyPress: {
        i64 key_code = XLookupKeysym(&event.xkey, 0);
        switch (key_code) {
          case XK_Escape:
            return -1;
          default: {
            break;
          }
        }
        break;
      }
    }
  }
  XFlush(win.display);
  return 0;
}

void platform_swap_buffers(struct Render_state* renderer) {
  win.image->data = (void*)renderer->framebuffer->data;
  XPutImage(win.display, win.window, win.graphics_context, win.image, 0, 0, 0, 0, renderer->framebuffer->width, renderer->framebuffer->height);
  win.image->data = NULL;
  XSync(win.display, False);
}

void platform_close_window() {
  if (win.image) {
    XDestroyImage(win.image);
  }

  if (win.graphics_context) {
    XFreeGC(win.display, win.graphics_context);
  }

  if (win.display && win.window) {
    XUnmapWindow(win.display, win.window);
    XDestroyWindow(win.display, win.window);
    XCloseDisplay(win.display);
  }
}
