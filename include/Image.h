/**
 *  @file   Image.h
 *  @brief  Image Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef IMAGE_H_
#define IMAGE_H_

#include <cstdlib>
#include <png.h>
#include <string>

class Image {

 public:

  enum class Type {Unknown, PNG};
  
  Image();
  
  ~Image();
  
  Image(const char *path);
  
  bool fail();

  bool good();

  int Read(const char *path);

  int depth;

  int bytes_per_line;

  int width;

  int height;

  int channels;

  unsigned char *data;

 private:

  std::string _path;

  Type _type;

  bool _fail;

};

inline bool Image::fail() {

  return _fail;
}

inline bool Image::good() {

  return !_fail;
}
#endif // End of IMAGE_H_
