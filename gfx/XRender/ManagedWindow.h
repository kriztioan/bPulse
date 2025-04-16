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
#include <unordered_map>

#include <functional>
#include <memory>

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
              const std::string &color);

  int DrawCircle(int x, int y, int radius1, int radius2,
                 const std::string &color);

  int DrawLine(int x1, int y1, int x2, int y2, int width,
               const std::string &color);

  int DrawText(int x1, int y1, std::string text, const std::string &color,
               int align = TEXT::ALIGN::LEFT);

  int SetOpacity(float opacity);

  bool SetAlwaysOnTop(bool state);

  int SetFont(const std::string &font, int size);

  int RenderLayer();

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

  std::unique_ptr<XGlyphInfo[]> _xglyphinfo;

  XRenderColor _clear = {0x0000, 0x0000, 0x0000, 0x0000};

  std::unordered_map<std::string, XRenderColor> _xrendercolors;

  bool _paused = false;

  int SetColor(const std::string &color);

  int DrawRenderedArc(int x, int y, int radius1, int radius2, int angle1,
                      int angle2);

  int DrawRenderedLine(int x1, int y1, int x2, int y2, int width);

  int DrawRenderedText(int x, int y, const std::string &text);
};

inline int ManagedWindow::DrawCircle(int x, int y, int radius1, int radius2,
                                     const std::string &color) {

  return DrawArc(x, y, radius1, radius2, 0, 360, color);
}

#endif // End of MANAGEDWINDOW_H_
