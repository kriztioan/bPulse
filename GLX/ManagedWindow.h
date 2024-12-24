/**
 *  @file   ManagedWindow.h
 *  @brief  Managed Window Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-12-21
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef MANAGEDWINDOW_H_
#define MANAGEDWINDOW_H_

#include <cmath>

#include <array>
#include <string>
#include <unordered_map>

#include <functional>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "WindowEvents.h"

namespace TEXT {
enum ALIGN { LEFT = 0, CENTER, RIGHT };
};

struct _GLXFontInfo {
  unsigned int width;
  unsigned int height;
};

class ManagedWindow {

public:
  ManagedWindow();

  ~ManagedWindow();

  int DrawArc(int x, int y, int radius1, int radius2, int angle1, int angle2,
              const std::string &color);

  int DrawCircle(int x, int y, int radius1, int radius2, const std::string &color);

  int DrawLine(int x1, int y1, int x2, int y2, int width, const std::string &color);

  int DrawText(int x1, int y1, std::string text, const std::string &color,
               int align = TEXT::ALIGN::LEFT);

  int SetOpacity(float opacity);

  bool SetAlwaysOnTop(bool state);

  int SetFont(const std::string &font, int size);

  int RenderLayer();

  int Sync();

  int Scale(float factor);

  std::string id; // for saving

  std::function<WindowEvents(WindowEvent *)> EventHandler;

  Display *xdisplay;

  GLXContext glxcontext;

  Window xwindow;

  Pixmap xicon, xiconmask;

  unsigned char *xbackground = nullptr;

  int xx, xy, xwidth, xheight;

  void Pause() { _paused = true; }

  void Unpause() { _paused = false; }

  bool IsPaused() { return _paused; }

private:
  GLuint _glxfont;

  _GLXFontInfo *_glxfontinfo = nullptr;

  std::unordered_map<std::string, std::array<unsigned char, 4>> _glxcolors;

  bool _paused = false;

  int SetColor(const std::string &color);

  int DrawGLXArc(int x, int y, int radius1, int radius2, int angle1,
                 int angle2);

  int DrawGLXLine(int x1, int y1, int x2, int y2, int width);

  int DrawGLXText(int x, int y, const std::string &text);
};

inline int ManagedWindow::DrawCircle(int x, int y, int radius1, int radius2,
                                     const std::string &color) {

  return DrawArc(x, y, radius1, radius2, 0, 360, color);
}

#endif // End of MANAGEDWINDOW_H_
