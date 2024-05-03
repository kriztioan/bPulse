/**
 *  @file   WindowManager.cpp
 *  @brief  Window Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "WindowManager.h"

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

  int xrenderevent, xrendererror;

  if (!XRenderQueryExtension(_xdisplay, &xrenderevent, &xrendererror)) {

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

  mwindow->xmask =
      XCreatePixmap(_xdisplay, DefaultRootWindow(_xdisplay), width, height, 1);

  XGCValues xgcvalues;

  xgcvalues.function = GXcopy;

  xgcvalues.graphics_exposures = false;

  xgcvalues.foreground = 0;

  xgcvalues.plane_mask = AllPlanes;

  unsigned long xgcvalues_mask =
      GCFunction | GCGraphicsExposures | GCForeground | GCPlaneMask;

  mwindow->xmaskgc =
      XCreateGC(_xdisplay, mwindow->xmask, xgcvalues_mask, &xgcvalues);

  Pixmap xbackground = None, xbackgroundmask = None;

  if (background != nullptr) {

    Image2XPixmap(background, &xbackground, &xbackgroundmask);
  }

  XSetWindowAttributes xwindowattributes;

  xwindowattributes.background_pixmap = ParentRelative;

  xwindowattributes.save_under = true;

  xwindowattributes.override_redirect = false;

  xwindowattributes.event_mask = StructureNotifyMask | KeyPressMask |
                                 ExposureMask | ButtonPressMask |
                                 ButtonReleaseMask | Button1MotionMask;

  xwindowattributes.do_not_propagate_mask = false;

  Cursor xcursor = XCreateFontCursor(_xdisplay, XC_hand2);

  xwindowattributes.cursor = xcursor;

  unsigned long xwindowmask = CWBackPixmap | CWSaveUnder | CWOverrideRedirect |
                              CWEventMask | CWDontPropagate | CWCursor;

  mwindow->xwindow = XCreateWindow(
      _xdisplay, DefaultRootWindow(_xdisplay), xpos, ypos, width, height, 0,
      DefaultDepth(_xdisplay, DefaultScreen(_xdisplay)), InputOutput,
      DefaultVisual(_xdisplay, DefaultScreen(_xdisplay)), xwindowmask,
      &xwindowattributes);

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
                    sizeof(mwmHints) / sizeof(long));
  }

  wmHints = XInternAtom(_xdisplay, "KWM_WIN_DECORATION", True);

  if (wmHints != None) {

    long KWMHints = 0;

    XChangeProperty(_xdisplay, mwindow->xwindow, wmHints, wmHints, 32,
                    PropModeReplace, (unsigned char *)&KWMHints,
                    sizeof(KWMHints) / sizeof(long));
  }

  wmHints = XInternAtom(_xdisplay, "_WIN_HINTS", True);

  if (wmHints != None) {

    long GNOMEHints = 0;

    XChangeProperty(_xdisplay, mwindow->xwindow, wmHints, wmHints, 32,
                    PropModeReplace, (unsigned char *)&GNOMEHints,
                    sizeof(GNOMEHints) / sizeof(long));
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

  mwindow->xbackbuffer =
      XdbeAllocateBackBufferName(_xdisplay, mwindow->xwindow, XdbeBackground);

  mwindow->xswapinfo.swap_window = mwindow->xwindow;

  mwindow->xswapinfo.swap_action = XdbeBackground;

  XRenderPictFormat *xrenderpictformat = XRenderFindVisualFormat(
      _xdisplay, DefaultVisual(_xdisplay, DefaultScreen(_xdisplay)));

  XRenderPictureAttributes xrenderpictureattributes;

  xrenderpictureattributes.clip_mask = xbackgroundmask;

  xrenderpictureattributes.poly_edge = PolyEdgeSmooth;

  xrenderpictureattributes.poly_mode = PolyModePrecise;

  mwindow->xpicture = XRenderCreatePicture(
      _xdisplay, mwindow->xbackbuffer, xrenderpictformat,
      CPClipMask | CPPolyEdge | CPPolyMode, &xrenderpictureattributes);

  mwindow->xbackground =
      XRenderCreatePicture(_xdisplay, xbackground, xrenderpictformat,
                           CPClipMask, &xrenderpictureattributes);

  Pixmap xbrush =
      XCreatePixmap(_xdisplay, mwindow->xbackbuffer, 1, 1,
                    DefaultDepth(_xdisplay, DefaultScreen(_xdisplay)));

  xrenderpictureattributes.repeat = 1;

  mwindow->xbrush = XRenderCreatePicture(_xdisplay, xbrush, xrenderpictformat,
                                         CPRepeat, &xrenderpictureattributes);

  XFreePixmap(_xdisplay, xbrush);

  Pixmap xcanvas =
      XCreatePixmap(_xdisplay, mwindow->xbackbuffer, width, height,
                    DefaultDepth(_xdisplay, DefaultScreen(_xdisplay)));

  mwindow->xcanvas = XRenderCreatePicture(_xdisplay, xcanvas, xrenderpictformat,
                                          CPClipMask | CPPolyMode | CPPolyEdge,
                                          &xrenderpictureattributes);

  XSetForeground(_xdisplay, DefaultGC(_xdisplay, DefaultScreen(_xdisplay)),
                 0xff0000);

  XDrawArc(_xdisplay, xcanvas, DefaultGC(_xdisplay, DefaultScreen(_xdisplay)),
           0, 0, width / 2, height / 2, 0, 180 * 64);

  XFreePixmap(_xdisplay, xcanvas);

  /*Picture background_alpha_picture = XRenderCreatePicture(_xdisplay,
  xbackgroundmask, XRenderFindStandardFormat(_xdisplay, PictStandardA8), 0,
  nullptr);

  xrenderpictureattributes.alpha_map = background_alpha_picture;

  mwindow->xbackground_picture = XRenderCreatePicture(_xdisplay, xbackground,
  xrenderpictformat,  0, nullptr);//CPAlphaMap, &xrenderpictureattributes);



  XRenderFreePicture(_xdisplay, background_alpha_picture);

  */

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

  _mwindows.push_back(mwindow);

  return mwindow;
}

int WindowManager::Image2XPixmap(Image *image, Pixmap *pixmap, Pixmap *mask) {

  if (image->fail()) {

    return 0;
  }

  XImage *ximage = XCreateImage(
      _xdisplay, DefaultVisual(_xdisplay, DefaultScreen(_xdisplay)),
      DefaultDepth(_xdisplay, DefaultScreen(_xdisplay)), ZPixmap, 0,
      (char *)image->data, image->width, image->height, image->depth,
      image->bytes_per_line);

  if (ImageByteOrder(_xdisplay) != LSBFirst) {

    ximage->byte_order = LSBFirst;
  }

  *pixmap = XCreatePixmap(_xdisplay, DefaultRootWindow(_xdisplay),
                          ximage->width, ximage->height,
                          DefaultDepth(_xdisplay, DefaultScreen(_xdisplay)));

  XPutImage(_xdisplay, *pixmap, DefaultGC(_xdisplay, DefaultScreen(_xdisplay)),
            ximage, 0, 0, 0, 0, ximage->width, ximage->height);

  if (image->channels == 4) {

    *mask = XCreatePixmap(_xdisplay, DefaultRootWindow(_xdisplay),
                          ximage->width, ximage->height, 1);

    XGCValues xgcvalues;

    xgcvalues.function = GXcopy;

    xgcvalues.graphics_exposures = false;

    xgcvalues.foreground = 0;

    xgcvalues.plane_mask = AllPlanes;

    unsigned long xgcvalues_mask =
        GCFunction | GCGraphicsExposures | GCForeground | GCPlaneMask;

    GC maskgc = XCreateGC(_xdisplay, *mask, xgcvalues_mask, &xgcvalues);

    XFillRectangle(_xdisplay, *mask, maskgc, 0, 0, ximage->width,
                   ximage->height);

    XSetForeground(_xdisplay, maskgc, 1);

    char alpha;

    char alpha_map[ximage->width * ximage->height];

    for (int y = 0; y < ximage->height; y++) {

      for (int x = 0; x < ximage->width; x++) {

        alpha = ximage->data[3 + (y * ximage->width + x) * 4];

        // alpha_map[y * ximage->width + x] = ximage->data[3 + (y *
        // ximage->width + x) * 4];

        if (alpha == -1) {

          XDrawPoint(_xdisplay, *mask, maskgc, x, y);
        }
      }
    }

    // XImage *alpha_image = XCreateImage(_xdisplay, 0, 8, XYPixmap, 0,
    // alpha_map, image->width, image->height, 8, 0);

    // XPutImage(_xdisplay, *mask, maskgc, alpha_image, 0, 0, 0, 0,
    // alpha_image->width, alpha_image->height);

    XFreeGC(_xdisplay, maskgc);

    // alpha_image->data = nullptr;

    // XDestroyImage(alpha_image);
  }

  ximage->data = nullptr;

  XDestroyImage(ximage);

  return 0;
}
