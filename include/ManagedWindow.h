/**
 *  @file   ManagedWindow.h
 *  @brief  Managed Window Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef MANAGEDWINDOW_H_
#define MANAGEDWINDOW_H_

#include <cmath>

#include <string>

#include <functional>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <X11/extensions/Xdbe.h>
#include <X11/extensions/Xrender.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "WindowEvents.h"

namespace TEXT {
enum ALIGN { LEFT = 0, CENTER, RIGHT };
};

class ManagedWindow {

public:
  ManagedWindow();

  ~ManagedWindow();

  int DrawArc(int x, int y, int radius1, int radius2, int angle1, int angle2,
              std::string color);

  int DrawCircle(int x, int y, int radius1, int radius2, std::string color);

  int DrawLine(int x1, int y1, int x2, int y2, int width, std::string color);

  int DrawText(int x1, int y1, std::string text, std::string color,
               int align = TEXT::ALIGN::LEFT);

  int SetOpacity(float opacity);

  bool SetAlwaysOnTop(bool state);

  int SetFont(std::string font, int size);

  int Sync();

  int Scale(XFixed factor);

  std::string id; // for saving

  std::function<WindowEvents(WindowEvent *)> EventHandler;

  Display *xdisplay;

  Window xwindow;

  Pixmap xicon, xiconmask;

  Picture xbackground, xbrush, xcanvas, xpict;

  XRenderColor xblack;

  XdbeBackBuffer xbackbuffer;

  XdbeSwapInfo xswapinfo;

  int xx, xy, xwidth, xheight;

  void Pause() { _paused = true; }

  void Unpause() { _paused = false; }

  bool IsPaused() { return _paused; }

private:
  GlyphSet _xfont = None;

  XGlyphInfo *_xglyphinfo = nullptr;

  XRenderColor _clear = {0x0000, 0x0000, 0x0000, 0x0000};

  bool _paused = false;

  int DrawRenderedArc0(int x, int y, int radius1, int radius2, int angle1,
                       int angle2);

  int DrawRenderedArc(int x, int y, int radius1, int radius2, int angle1,
                      int angle2);

  int DrawRenderedLine(int x1, int y1, int x2, int y2, int width);
};

inline int ManagedWindow::DrawCircle(int x, int y, int radius1, int radius2,
                                     std::string color) {

  return DrawArc(x, y, radius1, radius2, 0, 360, color);
}

#endif // End of MANAGEDWINDOW_H_
