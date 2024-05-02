/**
 *  @file   ManagedWindow.cpp
 *  @brief  Managed Window Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "ManagedWindow.h"

ManagedWindow::ManagedWindow()
    : xmask(None), xicon(None), xiconmask(None), xblack({0, 0, 0, 0}) {}

ManagedWindow::~ManagedWindow() {

  XRenderFreePicture(xdisplay, xbrush);

  XRenderFreePicture(xdisplay, xpicture);

  XRenderFreePicture(xdisplay, xbackground);

  XRenderFreePicture(xdisplay, xcanvas);

  if (_xfont) {

    delete _xglyphinfo;

    _xglyphinfo = nullptr;

    XRenderFreeGlyphSet(xdisplay, _xfont);

    _xfont = None;
  }

  XFreeGC(xdisplay, xmaskgc);

  XFreePixmap(xdisplay, xmask);

  XFreePixmap(xdisplay, xicon);

  XFreePixmap(xdisplay, xiconmask);

  XdbeDeallocateBackBufferName(xdisplay, xbackbuffer);

  XUnmapWindow(xdisplay, xwindow);

  XDestroyWindow(xdisplay, xwindow);
}

// Not working - probably not possible on a backbuffer...
int ManagedWindow::Scale(XFixed factor) {

  XRenderChangePicture(xdisplay, xpicture, CPClipMask, None);

  XTransform xtransform = {{{1, 0, 0}, {0, 1, 0}, {0, 0, factor}}};

  XRenderSetPictureTransform(xdisplay, xcanvas, &xtransform);

  return 0;
}

int ManagedWindow::Sync() {

  XRenderComposite(xdisplay, PictOpOver, xcanvas, None, xpicture, 0, 0, 0, 0, 0,
                   0, xwidth, xheight);

  XdbeSwapBuffers(xdisplay, &xswapinfo, 1);

  XSync(xdisplay, false);

  XRenderComposite(xdisplay, PictOpSrc, xbackground, None, xcanvas, 0, 0, 0, 0,
                   0, 0, xwidth, xheight);

  return 0;
}

int ManagedWindow::SetOpacity(float opacity) {

  Atom wmOpacity = XInternAtom(xdisplay, "_NET_WM_WINDOW_OPACITY", false);

  unsigned long property = opacity * 0xffffffff;

  XChangeProperty(xdisplay, xwindow, wmOpacity, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)&property, 1L);

  return 0;
}

int ManagedWindow::DrawArc(int x, int y, int radius1, int radius2, int angle1,
                           int angle2, std::string color) {

  XRenderColor xrendercolor;

  XRenderParseColor(xdisplay, const_cast<char *>(color.c_str()), &xrendercolor);

  XRenderFillRectangle(xdisplay, PictOpOver, xbrush, &xrendercolor, 0, 0, 1, 1);

  return DrawRenderedArc(x, y, radius1, radius2, angle1, angle2);
}

int ManagedWindow::DrawLine(int x1, int y1, int x2, int y2, int width,
                            std::string color) {

  XRenderColor xrendercolor;

  XRenderParseColor(xdisplay, const_cast<char *>(color.c_str()), &xrendercolor);

  XRenderFillRectangle(xdisplay, PictOpSrc, xbrush, &xrendercolor, 0, 0, 1, 1);

  return DrawRenderedLine(x1, y1, x2, y2, width);
}

int ManagedWindow::SetFont(std::string font, int size) {

  if (_xfont) {

    delete _xglyphinfo;

    _xglyphinfo = nullptr;

    XRenderFreeGlyphSet(xdisplay, _xfont);

    _xfont = None;
  }

  FT_Library library;

  if (FT_Init_FreeType(&library) != 0) {

    return 1;
  }

  FT_Face face;

  if (FT_New_Face(library, font.c_str(), 0, &face) != 0) {

    return 1;
  }

  if (FT_Set_Pixel_Sizes(face, 0, size) != 0) {

    return 1;
  }

  _xfont = XRenderCreateGlyphSet(
      xdisplay, XRenderFindStandardFormat(xdisplay, PictStandardA8));

  _xglyphinfo = new XGlyphInfo['~' - ' '];

  for (char ch = ' '; ch <= '~'; ch++) {

    FT_UInt g_index = FT_Get_Char_Index(face, ch);

    if (FT_Load_Glyph(face, g_index, FT_LOAD_RENDER) != 0) {

      return 1;
    }

    FT_Bitmap *bitmap = &(face->glyph->bitmap);

    Glyph g_id = ch;

    _xglyphinfo[ch - ' '].x = -face->glyph->bitmap_left;

    _xglyphinfo[ch - ' '].y = face->glyph->bitmap_top;

    _xglyphinfo[ch - ' '].width = bitmap->width;

    _xglyphinfo[ch - ' '].height = bitmap->rows;

    _xglyphinfo[ch - ' '].xOff = face->glyph->advance.x / 64;

    _xglyphinfo[ch - ' '].yOff = face->glyph->advance.y / 64;

    int stride = (_xglyphinfo[ch - ' '].width + 3) & ~3;

    char map[stride * _xglyphinfo[ch - ' '].height];

    for (int row = 0; row < _xglyphinfo[ch - ' '].height; row++) {

      memcpy(map + row * stride,
             bitmap->buffer + row * _xglyphinfo[ch - ' '].width,
             _xglyphinfo[ch - ' '].width);
    }

    XRenderAddGlyphs(xdisplay, _xfont, &g_id, &(_xglyphinfo[ch - ' ']), 1, map,
                     stride * _xglyphinfo[ch - ' '].height);
  }

  FT_Done_Face(face);

  return 0;
}

int ManagedWindow::DrawText(int x, int y, std::string text, std::string color,
                            int align) {

  if (!_xfont) {

    return 1;
  }

  unsigned int xpixels = 0, ypixels = 0;

  for (std::string::iterator it = text.begin(); it != text.end(); it++) {

    xpixels += _xglyphinfo[*it - ' '].width;

    ypixels = std::max(
        ypixels, static_cast<unsigned int>(_xglyphinfo[*it - ' '].height));
  }

  switch (align) {
  case TEXT::ALIGN::LEFT:
    break;
  case TEXT::ALIGN::CENTER:
    x -= (xpixels) / 2;
    y -= (ypixels) / 2;
    break;
  case TEXT::ALIGN::RIGHT:
    x -= xpixels;
    break;
  };

  XRenderColor xrendercolor;

  XRenderParseColor(xdisplay, const_cast<char *>(color.c_str()), &xrendercolor);

  XRenderFillRectangle(xdisplay, PictOpSrc, xbrush, &xrendercolor, 0, 0, 1, 1);

  XRenderCompositeString8(xdisplay, PictOpOver, xbrush, xcanvas, 0, _xfont, 0,
                          0, x, y, text.c_str(), text.length());

  return 0;
}

int ManagedWindow::DrawRenderedArc0(int x, int y, int radius1, int radius2,
                                    int angle1, int angle2) {

  int nxtriangles = ceil((angle2 - angle1) * radius2 / 360.);

  if (nxtriangles == 0) {

    return 1;
  }

  XTriangle xtriangles[2 * nxtriangles + 1];

  double a1 = M_PI * angle1 / 180, a2 = M_PI * angle2 / 180;

  int i = 0;

  xtriangles[i].p1.x = XDoubleToFixed(cosf(a1) * radius1 + x);
  xtriangles[i].p2.x = XDoubleToFixed(cosf(a1) * radius2 + x);
  xtriangles[i].p3.x =
      XDoubleToFixed(cosf(a1 + (a2 - a1) * 0.5 / nxtriangles) * radius2 + x);

  xtriangles[i].p1.y = XDoubleToFixed(-sinf(a1) * radius1 + y);
  xtriangles[i].p2.y = XDoubleToFixed(-sinf(a1) * radius2 + y);
  xtriangles[i].p3.y =
      XDoubleToFixed(-sinf(a1 + (a2 - a1) * 0.5 / nxtriangles) * radius2 + y);

  ++i;

  for (int j = 0; j < nxtriangles; j++) {

    xtriangles[i].p1.x =
        XDoubleToFixed(cosf(a1 + (a2 - a1) * (j) / nxtriangles) * radius1 + x);
    xtriangles[i].p2.x = XDoubleToFixed(
        cosf(a1 + (a2 - a1) * (j + 1) / nxtriangles) * radius1 + x);
    xtriangles[i].p3.x = XDoubleToFixed(
        cosf(a1 + (a2 - a1) * (j + 0.5) / nxtriangles) * radius2 + x);

    xtriangles[i].p1.y =
        XDoubleToFixed(-sinf(a1 + (a2 - a1) * (j) / nxtriangles) * radius1 + y);
    xtriangles[i].p2.y = XDoubleToFixed(
        -sinf(a1 + (a2 - a1) * (j + 1) / nxtriangles) * radius1 + y);
    xtriangles[i].p3.y = XDoubleToFixed(
        -sinf(a1 + (a2 - a1) * (j + 0.5) / nxtriangles) * radius2 + y);

    xtriangles[i + 1].p1.x = xtriangles[i].p2.x;
    xtriangles[i + 1].p2.x = xtriangles[i].p3.x;
    xtriangles[i + 1].p3.x = XDoubleToFixed(
        cosf(a1 + (a2 - a1) * (j + 0.5 + 1) / nxtriangles) * radius2 + x);

    xtriangles[i + 1].p1.y = xtriangles[i].p2.y;
    xtriangles[i + 1].p2.y = xtriangles[i].p3.y;
    xtriangles[i + 1].p3.y = XDoubleToFixed(
        -sinf(a1 + (a2 - a1) * (j + 0.5 + 1) / nxtriangles) * radius2 + y);

    /*xtriangles[i+2].p1.x = xtriangles[i].p1.x;
    xtriangles[i+2].p2.x = xtriangles[i].p2.x;
    xtriangles[i+2].p3.x = xtriangles[i+1].p3.x;

    xtriangles[i+2].p1.y = xtriangles[i].p1.y;
    xtriangles[i+2].p2.y = xtriangles[i].p2.y;
    xtriangles[i+2].p3.y = xtriangles[i+1].p3.y;

    xtriangles[i+3].p1.x = xtriangles[i].p1.x;
    xtriangles[i+3].p2.x = xtriangles[i].p3.x;
    xtriangles[i+3].p3.x = xtriangles[i+1].p3.x;

    xtriangles[i+3].p1.y = xtriangles[i].p1.y;
    xtriangles[i+3].p2.y = xtriangles[i].p3.y;
    xtriangles[i+3].p3.y = xtriangles[i+1].p3.y;*/

    i += 2;
  }

  xtriangles[i - 1].p1.x = XDoubleToFixed(cosf(a2) * radius1 + x);
  xtriangles[i - 1].p2.x = XDoubleToFixed(cosf(a2) * radius2 + x);
  xtriangles[i - 1].p3.x =
      XDoubleToFixed(cosf(a2 - 0.5 * (a2 - a1) / nxtriangles) * radius2 + x);

  xtriangles[i - 1].p1.y = XDoubleToFixed(-sinf(a2) * radius1 + y);
  xtriangles[i - 1].p2.y = XDoubleToFixed(-sinf(a2) * radius2 + y);
  xtriangles[i - 1].p3.y =
      XDoubleToFixed(-sinf(a2 - (a2 - a1) * 0.5 / nxtriangles) * radius2 + y);

  XRenderCompositeTriangles(xdisplay, PictOpOver, xbrush, xcanvas, 0, 0, 0,
                            xtriangles, 2 * nxtriangles + 1);

  return 0;
}

int ManagedWindow::DrawRenderedArc(int x, int y, int radius1, int radius2,
                                   int angle1, int angle2) {

  int nxpoints = ceil((angle2 - angle1) * radius2 * M_PI / 180.0f);

  if (nxpoints < 4)
    nxpoints = 4;

  XPointFixed xpoints[nxpoints];

  int nxtriangles = nxpoints - 3;

  auto xrender = [&](int angle1, int angle2) {
    float a1 = M_PI * angle1 / 180.0, a2 = M_PI * angle2 / 180.0,
          da = (a2 - a1) / nxtriangles, cosfa1 = cosf(a1), sinfa1 = -sinf(a1);

    int i = 0;

    xpoints[i].x = XDoubleToFixed(cosfa1 * radius2 + x);
    xpoints[i++].y = XDoubleToFixed(sinfa1 * radius2 + y);

    xpoints[i].x = XDoubleToFixed(cosfa1 * radius1 + x);
    xpoints[i++].y = XDoubleToFixed(sinfa1 * radius1 + y);

    xpoints[i].x = XDoubleToFixed(cosf(a1 + da) * radius2 + x);
    xpoints[i++].y = XDoubleToFixed(-sinf(a1 + da) * radius2 + y);

    float radius = radius1;
    for (; i < nxpoints - 1; i++) {
      xpoints[i].x = XDoubleToFixed(cosf(a1 + da * (i - 1)) * radius + x);
      xpoints[i].y = XDoubleToFixed(-sinf(a1 + da * (i - 1)) * radius + y);
      radius = (i % 2 == 0) ? radius1 : radius2;
    }

    radius = (i % 2 == 0) ? radius2 : radius1;
    xpoints[i].x = XDoubleToFixed(cosf(a2) * radius + x);
    xpoints[i].y = XDoubleToFixed(-sinf(a2) * radius + y);

    XRenderCompositeTriStrip(xdisplay, PictOpOver, xbrush, xcanvas, 0, 0, 0,
                             xpoints, nxpoints);
  };

  xrender(angle1, angle2);

  int delta = (angle2 - angle1) / nxpoints / 2;
  xrender(angle1 + delta, angle2 - delta);

  return 0;
}

int ManagedWindow::DrawRenderedLine(int x1, int y1, int x2, int y2, int width) {

  XTriangle xtriangle[2];

  double angle = atan2(y2 - y1, x2 - x1),
         radius = sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));

  xtriangle[0].p1.x = XDoubleToFixed(x1 - width * sinf(angle) / 2);
  xtriangle[0].p1.y = XDoubleToFixed(y1 + width * cosf(angle) / 2);

  xtriangle[0].p2.x = XDoubleToFixed(x1 + width * sinf(angle) / 2);
  xtriangle[0].p2.y = XDoubleToFixed(y1 - width * cosf(angle) / 2);

  xtriangle[0].p3.x =
      XDoubleToFixed(x1 + radius * cosf(angle) - width * sinf(angle) / 2);
  xtriangle[0].p3.y =
      XDoubleToFixed(y1 + radius * sinf(angle) + width * cosf(angle) / 2);

  xtriangle[1].p1.x = xtriangle[0].p3.x;
  xtriangle[1].p1.y = xtriangle[0].p3.y;

  xtriangle[1].p2.x =
      XDoubleToFixed(x1 + radius * cosf(angle) + width * sinf(angle) / 2);
  xtriangle[1].p2.y =
      XDoubleToFixed(y1 + radius * sinf(angle) - width * cosf(angle) / 2);

  xtriangle[1].p3.x = xtriangle[0].p2.x;
  xtriangle[1].p3.y = xtriangle[0].p2.y;

  XRenderCompositeTriangles(xdisplay, PictOpOver, xbrush, xcanvas, 0, 0, 0,
                            xtriangle, 2);

  return 0;
}

bool ManagedWindow::SetAlwaysOnTop(bool state) {

  Atom netWmState = XInternAtom(xdisplay, "_NET_WM_STATE", 1),
       netWmStateAbove = XInternAtom(xdisplay, "_NET_WM_STATE_ABOVE", 1);

  if (netWmStateAbove != None && netWmState != None) {
    XClientMessageEvent xclient;
    bzero(&xclient, sizeof(XClientMessageEvent));
    xclient.type = ClientMessage;
    xclient.window = xwindow;
    xclient.message_type = netWmState;
    xclient.format = 32;
    xclient.data.l[0] = state;
    xclient.data.l[1] = netWmStateAbove;
    xclient.data.l[2] = 0;
    xclient.data.l[3] = 0;
    xclient.data.l[4] = 0;

    XSendEvent(xdisplay, DefaultRootWindow(xdisplay), false,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent *)&xclient);

    XFlush(xdisplay);

    return true;
  }

  return false;
}
