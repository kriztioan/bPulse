/**
 *  @file   ManagedWindow.cpp
 *  @brief  Managed Window Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-12-21
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "ManagedWindow.h"

ManagedWindow::ManagedWindow() : xicon(None), xiconmask(None) {}

ManagedWindow::~ManagedWindow() {

  if (xbackground) {

    glDeleteTextures(1, &xbackground);
  }

  if (_glxfontinfo) {

    for (char ch = ' '; ch <= '~'; ch++) {

      glDeleteTextures(1, &(_glxfontinfo[ch - ' '].texture));
    }

    glDeleteLists(_glxfont, '~' - ' ' + 1);
  }

  glXDestroyContext(xdisplay, glxcontext);

  XFreePixmap(xdisplay, xicon);

  XFreePixmap(xdisplay, xiconmask);

  XUnmapWindow(xdisplay, xwindow);

  XDestroyWindow(xdisplay, xwindow);
}

int ManagedWindow::Scale(float factor) {

  glScalef(factor, factor, 1.0f);

  return 0;
}

int ManagedWindow::Sync() {

  glXSwapBuffers(xdisplay, xwindow);

  glClear(GL_COLOR_BUFFER_BIT);

  if (xbackground) {

    glBindTexture(GL_TEXTURE_2D, xbackground);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);

    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(xwidth, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(xwidth, xheight);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, xheight);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  return 0;
}

int ManagedWindow::RenderLayer() {

  // nop

  return 0;
}

int ManagedWindow::SetOpacity(float opacity) {

  Atom wmOpacity = XInternAtom(xdisplay, "_NET_WM_WINDOW_OPACITY", false);

  union {
    unsigned int opacity;
    unsigned char rgba[4];
  } property;

  for (int i = 0; i < 4; i++) {

    property.rgba[i] = (unsigned int)(opacity * 255.0f);
  }

  XChangeProperty(xdisplay, xwindow, wmOpacity, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)&property.opacity, 1L);

  return 0;
}

int ManagedWindow::SetColor(const std::string &color) {

  auto it = _glxcolors.find(color);

  if (it == _glxcolors.end()) {

    std::array<unsigned char, 4> glxcolor;

    for (int i = 0; i < 4; i++) {

      glxcolor[i] =
          (unsigned char)strtol(color.substr(5 + i * 3, 2).c_str(), NULL, 16);
    }

    it = _glxcolors.emplace(color, std::move(glxcolor)).first;
  }

  glColor4ub(it->second[0], it->second[1], it->second[2], it->second[3]);

  return 0;
}

int ManagedWindow::DrawArc(int x, int y, int radius1, int radius2, int angle1,
                           int angle2, const std::string &color) {

  SetColor(color);

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                      GL_ONE_MINUS_SRC_ALPHA);

  return DrawGLXArc(x, y, radius1, radius2, angle1, angle2);
}

int ManagedWindow::DrawGLXArc(int x, int y, int radius1, int radius2,
                              int angle1, int angle2) {

  int nglxpoints = ceilf(0.5 * (angle2 - angle1) * radius2 * M_PI / 180.0);

  if (nglxpoints < 4) {

    nglxpoints = 4;
  }

  if (nglxpoints % 2 != 0) {

    ++nglxpoints;
  }

  std::unique_ptr<GLfloat[]> glxpoints =
      std::make_unique<GLfloat[]>(nglxpoints * 2);

  glVertexPointer(2, GL_FLOAT, 0, glxpoints.get());

  float a1 = M_PI * angle1 / 180.0, a2 = M_PI * angle2 / 180.0,
        da = (a2 - a1) / (nglxpoints - 3), cosfa1 = cosf(a1),
        sinfa1 = -sinf(a1);

  int i = 0;

  glxpoints[i * 2] = cosfa1 * radius2 + x;
  glxpoints[i * 2 + 1] = sinfa1 * radius2 + y;
  ++i;

  glxpoints[i * 2] = cosfa1 * radius1 + x;
  glxpoints[i * 2 + 1] = sinfa1 * radius1 + y;
  ++i;

  glxpoints[i * 2] = cosf(a1 + da) * radius2 + x;
  glxpoints[i * 2 + 1] = -sinf(a1 + da) * radius2 + y;
  ++i;

  for (; i < nglxpoints - 1; i += 2) {
    glxpoints[i * 2] = cosf(a1 + da * (i - 1)) * radius1 + x;
    glxpoints[i * 2 + 1] = -sinf(a1 + da * (i - 1)) * radius1 + y;

    glxpoints[i * 2 + 2] = cosf(a1 + da * i) * radius2 + x;
    glxpoints[i * 2 + 3] = -sinf(a1 + da * i) * radius2 + y;
  }

  glxpoints[i * 2] = cosf(a2) * radius1 + x;
  glxpoints[i * 2 + 1] = -sinf(a2) * radius1 + y;

  glDrawArrays(GL_TRIANGLE_STRIP, 0, nglxpoints);

  return 0;
}

int ManagedWindow::DrawLine(int x1, int y1, int x2, int y2, int width,
                            const std::string &color) {

  SetColor(color);

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                      GL_ONE_MINUS_SRC_ALPHA);

  return DrawGLXLine(x1, y1, x2, y2, width);
}

int ManagedWindow::SetFont(const std::string &font, int size) {

  if (_glxfont) {

    glDeleteLists(_glxfont, '~' - ' ' + 1);

    delete[] _glxfontinfo.release();
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

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  _glxfont = glGenLists('~' - ' ' + 1);

  _glxfontinfo = std::make_unique<GLXFontInfo[]>('~' - ' ' + 1);

  for (char ch = ' '; ch <= '~'; ch++) {

    FT_UInt g_index = FT_Get_Char_Index(face, ch);

    if (FT_Load_Glyph(face, g_index, FT_LOAD_RENDER) != 0) {

      return 1;
    }

    _glxfontinfo[ch - ' '].width = face->glyph->advance.x / 64.0;

    _glxfontinfo[ch - ' '].height = face->glyph->bitmap.rows;

    std::unique_ptr<GLubyte[]> buffer = std::make_unique<GLubyte[]>(
        2 * face->glyph->bitmap.width * face->glyph->bitmap.rows);

    for (int row = 0; row < face->glyph->bitmap.rows; row++) {

      for (int col = 0; col < face->glyph->bitmap.width; col++) {

        buffer[2 * (col + row * face->glyph->bitmap.width)] =
            buffer[2 * (col + row * face->glyph->bitmap.width) + 1] =
                face->glyph->bitmap
                    .buffer[col + face->glyph->bitmap.width * row];
      }
    }

    glGenTextures(1, &_glxfontinfo[ch - ' '].texture);

    glBindTexture(GL_TEXTURE_2D, _glxfontinfo[ch - ' '].texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, face->glyph->bitmap.width,
                 face->glyph->bitmap.rows, 0, GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE, buffer.get());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glNewList(_glxfont + ch, GL_COMPILE);

    glTranslatef(0.0f, -face->glyph->bitmap_top, 0);

    glBindTexture(GL_TEXTURE_2D, _glxfontinfo[ch - ' '].texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f);
    glVertex2f(0.f, 0.f);
    glTexCoord2f(1.f, 0.f);
    glVertex2f(_glxfontinfo[ch - ' '].width, 0.f);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(_glxfontinfo[ch - ' '].width, _glxfontinfo[ch - ' '].height);
    glTexCoord2f(0.f, 1.f);
    glVertex2f(0.f, _glxfontinfo[ch - ' '].height);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);

    glTranslatef(face->glyph->advance.x / 64.0, face->glyph->bitmap_top, 0);

    glEndList();
  }

  FT_Done_Face(face);

  FT_Done_FreeType(library);

  return 0;
}

int ManagedWindow::DrawText(int x, int y, std::string text,
                            const std::string &color, int align) {

  if (!_glxfont) {

    return 1;
  }

  unsigned int xpixels = 0, ypixels = 0;

  if (align != TEXT::ALIGN::LEFT) {
    for (std::string::iterator it = text.begin(); it != text.end(); it++) {

      xpixels += _glxfontinfo[*it - ' '].width;

      ypixels = std::max(ypixels, _glxfontinfo[*it - ' '].height);
    }
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

  SetColor(color);

  return DrawGLXText(x, y, text);
}

int ManagedWindow::DrawGLXText(int x, int y, const std::string &text) {

  glLoadIdentity();

  glTranslatef(x, y, 0);

  glPushAttrib(GL_LIST_BIT);

  glListBase(_glxfont);

  glCallLists((GLsizei)text.length(), GL_UNSIGNED_BYTE,
              (GLubyte *)text.c_str());

  glPopAttrib();

  return 0;
}

int ManagedWindow::DrawGLXLine(int x1, int y1, int x2, int y2, int width) {

  float angle = atan2f(y2 - y1, x2 - x1),
        radius = sqrtf((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1)),
        sina = sinf(angle), cosa = cosf(angle), hwsina = width * sina / 2.0,
        hwcosa = width * cosa / 2.0;

  GLfloat glxpoints[] = {x1 - hwsina,
                         y1 + hwcosa,
                         x1 + hwsina,
                         y1 - hwcosa,
                         x1 + radius * cosa - hwsina,
                         y1 + radius * sina + hwcosa,
                         x1 + radius * cosa + hwsina,
                         y1 + radius * sina - hwcosa};

  glLoadIdentity();

  glVertexPointer(2, GL_FLOAT, 0, glxpoints);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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

    return true;
  }

  return false;
}
