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

int ManagedWindow::DrawText(int x, int y, std::string text, std::string font,
                            int size, std::string color, int align) {

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

  GlyphSet xfont = XRenderCreateGlyphSet(
      xdisplay, XRenderFindStandardFormat(xdisplay, PictStandardA8));

  unsigned int xpixels = 0, ypixels = 0;

  for (std::string::iterator ch = text.begin(); ch != text.end(); ++ch) {

    FT_UInt g_index = FT_Get_Char_Index(face, *ch);

    if (FT_Load_Glyph(face, g_index, FT_LOAD_RENDER) != 0) {

      return 1;
    }

    FT_Bitmap *bitmap = &(face->glyph->bitmap);

    Glyph g_id;

    XGlyphInfo g_info;

    g_info.x = -face->glyph->bitmap_left;

    g_info.y = face->glyph->bitmap_top;

    g_info.width = bitmap->width;

    g_info.height = bitmap->rows;

    g_info.xOff = face->glyph->advance.x / 64;

    g_info.yOff = face->glyph->advance.y / 64;

    g_id = *ch;

    int stride = (g_info.width + 3) & ~3;

    char map[stride * g_info.height];

    for (int row = 0; row < g_info.height; row++) {

      memcpy(map + row * stride, bitmap->buffer + row * g_info.width,
             g_info.width);
    }

    XRenderAddGlyphs(xdisplay, xfont, &g_id, &g_info, 1, map,
                     stride * g_info.height);

    xpixels += bitmap->width;

    ypixels = std::max(ypixels, bitmap->rows);
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

  FT_Done_Face(face);

  XRenderColor xrendercolor;

  XRenderParseColor(xdisplay, const_cast<char *>(color.c_str()), &xrendercolor);

  XRenderFillRectangle(xdisplay, PictOpSrc, xbrush, &xrendercolor, 0, 0, 1, 1);

  XRenderCompositeString8(xdisplay, PictOpOver, xbrush, xcanvas, 0, xfont, 0, 0,
                          x, y, text.c_str(), text.length());

  return 0;
}

int ManagedWindow::DrawRenderedArc(int x, int y, int radius1, int radius2,
                                   int angle1, int angle2) {

  int nxtriangles = ceil((angle2 - angle1) * radius2 / 360.);

  if (nxtriangles == 0) {

    return 1;
  }

  XTriangle xtriangles[2 * nxtriangles + 1];

  double a1 = M_PI * angle1 / 180, a2 = M_PI * angle2 / 180;

  int i = 0;

  xtriangles[i].p1.x = XDoubleToFixed(cos(a1) * radius1 + x);
  xtriangles[i].p2.x = XDoubleToFixed(cos(a1) * radius2 + x);
  xtriangles[i].p3.x =
      XDoubleToFixed(cos(a1 + (a2 - a1) * 0.5 / nxtriangles) * radius2 + x);

  xtriangles[i].p1.y = XDoubleToFixed(-sin(a1) * radius1 + y);
  xtriangles[i].p2.y = XDoubleToFixed(-sin(a1) * radius2 + y);
  xtriangles[i].p3.y =
      XDoubleToFixed(-sin(a1 + (a2 - a1) * 0.5 / nxtriangles) * radius2 + y);

  ++i;

  for (int j = 0; j < nxtriangles; j++) {

    xtriangles[i].p1.x =
        XDoubleToFixed(cos(a1 + (a2 - a1) * (j) / nxtriangles) * radius1 + x);
    xtriangles[i].p2.x = XDoubleToFixed(
        cos(a1 + (a2 - a1) * (j + 1) / nxtriangles) * radius1 + x);
    xtriangles[i].p3.x = XDoubleToFixed(
        cos(a1 + (a2 - a1) * (j + 0.5) / nxtriangles) * radius2 + x);

    xtriangles[i].p1.y =
        XDoubleToFixed(-sin(a1 + (a2 - a1) * (j) / nxtriangles) * radius1 + y);
    xtriangles[i].p2.y = XDoubleToFixed(
        -sin(a1 + (a2 - a1) * (j + 1) / nxtriangles) * radius1 + y);
    xtriangles[i].p3.y = XDoubleToFixed(
        -sin(a1 + (a2 - a1) * (j + 0.5) / nxtriangles) * radius2 + y);

    xtriangles[i + 1].p1.x = xtriangles[i].p2.x;
    xtriangles[i + 1].p2.x = xtriangles[i].p3.x;
    xtriangles[i + 1].p3.x = XDoubleToFixed(
        cos(a1 + (a2 - a1) * (j + 0.5 + 1) / nxtriangles) * radius2 + x);

    xtriangles[i + 1].p1.y = xtriangles[i].p2.y;
    xtriangles[i + 1].p2.y = xtriangles[i].p3.y;
    xtriangles[i + 1].p3.y = XDoubleToFixed(
        -sin(a1 + (a2 - a1) * (j + 0.5 + 1) / nxtriangles) * radius2 + y);

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

  xtriangles[i - 1].p1.x = XDoubleToFixed(cos(a2) * radius1 + x);
  xtriangles[i - 1].p2.x = XDoubleToFixed(cos(a2) * radius2 + x);
  xtriangles[i - 1].p3.x =
      XDoubleToFixed(cos(a2 - 0.5 * (a2 - a1) / nxtriangles) * radius2 + x);

  xtriangles[i - 1].p1.y = XDoubleToFixed(-sin(a2) * radius1 + y);
  xtriangles[i - 1].p2.y = XDoubleToFixed(-sin(a2) * radius2 + y);
  xtriangles[i - 1].p3.y =
      XDoubleToFixed(-sin(a2 - (a2 - a1) * 0.5 / nxtriangles) * radius2 + y);

  XRenderCompositeTriangles(xdisplay, PictOpOver, xbrush, xcanvas, 0, 0, 0,
                            xtriangles, 2 * nxtriangles + 1);

  return 0;
}

int ManagedWindow::DrawRenderedLine(int x1, int y1, int x2, int y2, int width) {

  XTriangle xtriangle[2];

  double angle = atan2(y2 - y1, x2 - x1),
         radius = sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));

  xtriangle[0].p1.x = XDoubleToFixed(x1 - width * sin(angle) / 2);
  xtriangle[0].p1.y = XDoubleToFixed(y1 + width * cos(angle) / 2);

  xtriangle[0].p2.x = XDoubleToFixed(x1 + width * sin(angle) / 2);
  xtriangle[0].p2.y = XDoubleToFixed(y1 - width * cos(angle) / 2);

  xtriangle[0].p3.x =
      XDoubleToFixed(x1 + radius * cos(angle) - width * sin(angle) / 2);
  xtriangle[0].p3.y =
      XDoubleToFixed(y1 + radius * sin(angle) + width * cos(angle) / 2);

  xtriangle[1].p1.x = xtriangle[0].p3.x;
  xtriangle[1].p1.y = xtriangle[0].p3.y;

  xtriangle[1].p2.x =
      XDoubleToFixed(x1 + radius * cos(angle) + width * sin(angle) / 2);
  xtriangle[1].p2.y =
      XDoubleToFixed(y1 + radius * sin(angle) - width * cos(angle) / 2);

  xtriangle[1].p3.x = xtriangle[0].p2.x;
  xtriangle[1].p3.y = xtriangle[0].p2.y;

  XRenderCompositeTriangles(xdisplay, PictOpOver, xbrush, xcanvas, 0, 0, 0,
                            xtriangle, 2);

  return 0;
}
