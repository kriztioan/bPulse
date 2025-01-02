/**
 *  @file   WindowManager.cpp
 *  @brief  Window Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-12-21
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "WindowManager.h"
#include "GL/gl.h"
#include "GL/glx.h"
#include "ManagedWindow.h"
#include "X11/X.h"

WindowManager::WindowManager() { _init(0, nullptr); }

WindowManager::WindowManager(int argc, char **argv) { _init(argc, argv); }

WindowManager::~WindowManager() {

  std::list<ManagedWindow *>::iterator mwindow = _mwindows.begin();

  while (mwindow != _mwindows.end()) {

    delete *mwindow;

    mwindow = _mwindows.erase(mwindow);
  }

  XCloseDisplay(_xdisplay);
}

int WindowManager::_init(int argc, char **argv) {

  _argc = argc;

  _argv = argv;

  if ((_xdisplay = XOpenDisplay(nullptr)) == nullptr) {

    return (1);
  }

  int xshapeevent, xshapeerror;

  if (!XShapeQueryExtension(_xdisplay, &xshapeevent, &xshapeerror)) {

    return (1);
  }

  int xglxevent, xglxerror;

  if (!glXQueryExtension(_xdisplay, &xglxerror, &xglxevent)) {

    return (1);
  }

  return (0);
}

int WindowManager::EventHandler() {

  XEvent xevent;

  KeySym xkey;

  static int x, y, w = DisplayWidth(_xdisplay, DefaultScreen(_xdisplay)),
                   h = DisplayHeight(_xdisplay, DefaultScreen(_xdisplay));

  while (XPending(_xdisplay) > 0) {

    XNextEvent(_xdisplay, &xevent); // XNextEvent Flushes if empty!

    std::list<ManagedWindow *>::iterator mwindow = _mwindows.begin();

    while (mwindow != _mwindows.end()) {

      _event.type = WindowEvents::Zero;

      if ((*mwindow)->xwindow == xevent.xany.window) {

        switch (xevent.type) {

        case Expose:

          // printf("Expose\n");
          // Update(Rexctangle rec);
          break;

        case MotionNotify:

          (*mwindow)->xx = (*mwindow)->xx + xevent.xmotion.x - x;

          (*mwindow)->xy = (*mwindow)->xy + xevent.xmotion.y - y;

          if ((*mwindow)->xx < 0) {
            (*mwindow)->xx = 0;
          } else if ((*mwindow)->xx > (w - (*mwindow)->xwidth)) {
            (*mwindow)->xx = w - (*mwindow)->xwidth;
          }

          if ((*mwindow)->xy < 0) {
            (*mwindow)->xy = 0;
          } else if ((*mwindow)->xy > (h - (*mwindow)->xheight)) {
            (*mwindow)->xy = (h - (*mwindow)->xheight);
          }

          XMoveWindow(_xdisplay, (*mwindow)->xwindow, (*mwindow)->xx,
                      (*mwindow)->xy);

          ++mwindow;

          continue;

          break;

        case ButtonPress:

          if (xevent.xbutton.button == Button1) {

            (*mwindow)->Pause();

            x = xevent.xbutton.x;

            y = xevent.xbutton.y;

            ++mwindow;

            continue;
          }

          break;

        case ButtonRelease:

          if (xevent.xbutton.button == Button1) {

            (*mwindow)->Unpause();

            ++mwindow;

            continue;
          }

          break;

        case ConfigureNotify:

          _event.x = xevent.xconfigure.x;

          _event.y = xevent.xconfigure.y;

          if ((*mwindow)->EventHandler) {
            _event.type = WindowEvents::Move;

            if ((*mwindow)->EventHandler(&_event) != WindowEvents::Zero) {

              return 1;
            }
          }

          ++mwindow;

          continue;

          break;

        case KeyPress:

          xkey = XkbKeycodeToKeysym(_xdisplay, xevent.xkey.keycode, 0,
                                    xevent.xkey.state & ShiftMask ? 1 : 0);

          if (xkey == XK_Escape || xkey == XK_q)
            _event.type = WindowEvents::Destroy;

          break;

        case ClientMessage:

          Atom wmProtocols = XInternAtom(_xdisplay, "WM_PROTOCOLS", false);

          if (xevent.xclient.message_type == wmProtocols) {
            Atom wmDeleteWindow =
                XInternAtom(_xdisplay, "WM_DELETE_WINDOW", false);

            if (xevent.xclient.data.l[0] == (int)wmDeleteWindow) {

              if ((*mwindow)->EventHandler) {

                _event.type = WindowEvents::Destroy;

                if ((*mwindow)->EventHandler(&_event) == WindowEvents::Ignore) {

                  ++mwindow;

                  continue;
                }
              }

              delete *mwindow;

              mwindow = _mwindows.erase(mwindow);

              continue;
            }
          }

          break;
        };

        if ((*mwindow)->EventHandler) {

          if ((*mwindow)->EventHandler(&_event) != WindowEvents::Zero) {

            return 1;
          }
        }
      }

      ++mwindow;
    }
  }

  return 0;
}

int WindowManager::DestroyWindow(ManagedWindow *mwindow) {

  delete mwindow;

  _mwindows.remove(mwindow);

  mwindow = nullptr;

  return 0;
}

static int VisData[] = {GLX_DOUBLEBUFFER,
                        1,
                        GLX_RED_SIZE,
                        8,
                        GLX_GREEN_SIZE,
                        8,
                        GLX_BLUE_SIZE,
                        8,
                        GLX_ALPHA_SIZE,
                        8,
                        GLX_CONFIG_CAVEAT,
                        GLX_NONE,
                        GLX_X_VISUAL_TYPE,
                        GLX_TRUE_COLOR,
                        GLX_DRAWABLE_TYPE,
                        GLX_WINDOW_BIT,
                        None};

ManagedWindow *
WindowManager::CreateWindow(int xpos, int ypos, int width, int height,
                            std::function<WindowEvents(WindowEvent *)> handler,
                            Image *background, Image *icon) {

  ManagedWindow *mwindow = new ManagedWindow();

  mwindow->xdisplay = _xdisplay;

  mwindow->xx = xpos;

  mwindow->xy = ypos;

  mwindow->xwidth = width;

  mwindow->xheight = height;

  mwindow->EventHandler = handler;

  Pixmap xbackground = None, xbackgroundmask = None;

  if (background != nullptr) {

    Image2XPixmap(background, &xbackground, &xbackgroundmask);
  }

  XSetWindowAttributes xwindowattributes;

  xwindowattributes.event_mask = StructureNotifyMask | KeyPressMask |
                                 ExposureMask | ButtonPressMask |
                                 ButtonReleaseMask | Button1MotionMask;

  xwindowattributes.do_not_propagate_mask = false;

  Cursor xcursor = XCreateFontCursor(_xdisplay, XC_hand2);

  xwindowattributes.cursor = xcursor;

  xwindowattributes.border_pixel = 0;

  int nglxfbconfig;

  GLXFBConfig *glxfbconfigs = glXChooseFBConfig(
                  _xdisplay, DefaultScreen(_xdisplay), VisData, &nglxfbconfig),
              glxfbconfig = 0;

  XVisualInfo *xvisual = nullptr;

  int depth = -1, glxsamples = -1, glxvalue;

  for (int i = 0; i < nglxfbconfig; i++) {

    glXGetFBConfigAttrib(_xdisplay, glxfbconfigs[i], GLX_SAMPLE_BUFFERS,
                         &glxvalue);

    if (glxvalue) {

      glXGetFBConfigAttrib(_xdisplay, glxfbconfigs[i], GLX_SAMPLES, &glxvalue);

      if (glxvalue >= glxsamples) {

        xvisual =
            (XVisualInfo *)glXGetVisualFromFBConfig(_xdisplay, glxfbconfigs[i]);

        if (!xvisual) {

          continue;
        }

        if (xvisual->depth >= depth) {

          depth = xvisual->depth;

          glxsamples = glxvalue;

          glxfbconfig = glxfbconfigs[i];
        }
      }
    }

    XFree(xvisual);

    xvisual = nullptr;
  }

  XFree(glxfbconfigs);

  xvisual = (XVisualInfo *)glXGetVisualFromFBConfig(_xdisplay, glxfbconfig);

  GLXContext glxcontext = glXCreateContext(_xdisplay, xvisual, 0, GL_TRUE);

  mwindow->glxcontext = glxcontext;

  Colormap xcolormap =
      XCreateColormap(_xdisplay, RootWindow(_xdisplay, xvisual->screen),
                      xvisual->visual, AllocNone);

  xwindowattributes.colormap = xcolormap;

  unsigned long xwindowmask =
      CWEventMask | CWDontPropagate | CWCursor | CWBorderPixel | CWColormap;

  mwindow->xwindow =
      XCreateWindow(_xdisplay, RootWindow(_xdisplay, xvisual->screen), xpos,
                    ypos, width, height, 0, xvisual->depth, InputOutput,
                    xvisual->visual, xwindowmask, &xwindowattributes);

  XFree(xvisual);

  XFreeCursor(_xdisplay, xcursor);

  Atom wmProtocols = XInternAtom(_xdisplay, "WM_PROTOCOLS", false),
       wmDeleteWindow = XInternAtom(_xdisplay, "WM_DELETE_WINDOW", false),
       wmSaveState = XInternAtom(_xdisplay, "WM_SAVE_YOURSELF", false),
       wmHints = XInternAtom(_xdisplay, "_MOTIF_WM_HINTS", false);

  Atom wmProtocolsOnWindow[] = {wmDeleteWindow, wmSaveState};

  MwmHints mwmHints = {MWM_HINTS_DECORATIONS, MWM_FUNC_MOVE, 0, 0, 0};

  if (wmHints != None) {

    XChangeProperty(_xdisplay, mwindow->xwindow, wmHints, wmHints, 32,
                    PropModeReplace, (unsigned char *)&mwmHints,
                    sizeof(mwmHints));
  }

  wmHints = XInternAtom(_xdisplay, "KWM_WIN_DECORATION", True);

  if (wmHints != None) {

    long KWMHints = 0;

    XChangeProperty(_xdisplay, mwindow->xwindow, wmHints, wmHints, 32,
                    PropModeReplace, (unsigned char *)&KWMHints,
                    sizeof(KWMHints));
  }
  wmHints = XInternAtom(_xdisplay, "_WIN_HINTS", True);

  if (wmHints != None) {

    long GNOMEHints = 0;

    XChangeProperty(_xdisplay, mwindow->xwindow, wmHints, wmHints, 32,
                    PropModeReplace, (unsigned char *)&GNOMEHints,
                    sizeof(GNOMEHints));
  }

  XSetWMProtocols(_xdisplay, mwindow->xwindow, wmProtocolsOnWindow, 2);

  XSizeHints xsizehints;

  xsizehints.flags = PPosition | PSize;

  xsizehints.x = xpos;

  xsizehints.y = ypos;

  xsizehints.width = width;

  xsizehints.height = height;

  if (icon != nullptr) {

    Image2XPixmap(icon, &(mwindow->xicon), &(mwindow->xiconmask));
  }

  XWMHints *xwmhints = XAllocWMHints();

  xwmhints->flags = StateHint | InputHint | IconPixmapHint | IconMaskHint;

  xwmhints->initial_state = NormalState;

  xwmhints->input = true;

  xwmhints->icon_pixmap = mwindow->xicon;

  xwmhints->icon_mask = mwindow->xiconmask;

  XTextProperty windowname, iconname;

  XStringListToTextProperty(_argv, 1, &windowname);

  XStringListToTextProperty(_argv, 1, &iconname);

  XSetWMProperties(_xdisplay, mwindow->xwindow, &windowname, &iconname, _argv,
                   _argc, &xsizehints, xwmhints, nullptr);

  XFree(windowname.value);

  XFree(iconname.value);

  XFree(xwmhints);

  if (xbackgroundmask != None) {

    XShapeCombineMask(_xdisplay, mwindow->xwindow, ShapeBounding, 0, 0,
                      xbackgroundmask, ShapeSet);

    XShapeCombineMask(_xdisplay, mwindow->xwindow, ShapeClip, 0, 0,
                      xbackgroundmask, ShapeSet);

    XFreePixmap(_xdisplay, xbackgroundmask);
  }

  if (xbackground != None) {

    XFreePixmap(_xdisplay, xbackground);
  }

  XMapRaised(_xdisplay, mwindow->xwindow);

  XEvent xevent;

  while (true) {

    XNextEvent(_xdisplay, &xevent);

    if (xevent.type == MapNotify) {
      break;
    }
  }

  glXMakeCurrent(_xdisplay, mwindow->xwindow, glxcontext);

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  glOrtho(0.0, width, height, 0.0, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();

  glClearColor(0.0, 0.0, 0.0, 0.0);

  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);

  glEnableClientState(GL_VERTEX_ARRAY);

  glEnable(GL_POLYGON_SMOOTH);

  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  glEnable(GL_LINE_SMOOTH);

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  glEnable(GL_TEXTURE_2D);

  if (background != nullptr) {

    glGenTextures(1, &mwindow->xbackground);

    glBindTexture(GL_TEXTURE_2D, mwindow->xbackground);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, background->width,
                 background->height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 background->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  _mwindows.push_back(std::move(mwindow));

  return mwindow;
}

int WindowManager::Image2XPixmap(Image *image, Pixmap *pixmap, Pixmap *mask) {

  if (image->fail()) {

    return 0;
  }

  XImage *ximage =
      XCreateImage(_xdisplay, CopyFromParent, 32, ZPixmap, 0,
                   (char *)image->data, image->width, image->height, 32, 0);

  if (ImageByteOrder(_xdisplay) != LSBFirst) {

    ximage->byte_order = LSBFirst;
  }

  *pixmap = XCreatePixmap(_xdisplay, DefaultRootWindow(_xdisplay), image->width,
                          image->height, 32);

  XGCValues xgcvalues;

  xgcvalues.function = GXcopy;

  xgcvalues.graphics_exposures = false;

  xgcvalues.foreground = 0;

  xgcvalues.plane_mask = AllPlanes;

  unsigned long xgcvalues_mask =
      GCFunction | GCGraphicsExposures | GCForeground | GCPlaneMask;

  GC gc = XCreateGC(_xdisplay, *pixmap, xgcvalues_mask, &xgcvalues);

  XPutImage(_xdisplay, *pixmap, gc, ximage, 0, 0, 0, 0, ximage->width,
            ximage->height);

  *mask = XCreatePixmap(_xdisplay, DefaultRootWindow(_xdisplay), ximage->width,
                        ximage->height, 1);

  GC maskgc = XCreateGC(_xdisplay, *mask, xgcvalues_mask, &xgcvalues);

  XFillRectangle(_xdisplay, *mask, maskgc, 0, 0, ximage->width, ximage->height);

  XSetForeground(_xdisplay, maskgc, 1);

  for (int y = 0; y < ximage->height; y++) {

    for (int x = 0; x < ximage->width; x++) {

      if ((unsigned char)ximage->data[3 + (y * ximage->width + x) * 4] > 127) {

        XDrawPoint(_xdisplay, *mask, maskgc, x, y);
      }
    }
  }

  XFreeGC(_xdisplay, maskgc);

  XFreeGC(_xdisplay, gc);

  ximage->data = nullptr;

  XDestroyImage(ximage);

  return 0;
}
