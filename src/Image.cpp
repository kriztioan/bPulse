/**
 *  @file   Image.cpp
 *  @brief  Image Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "Image.h"

Image::Image() : data(nullptr), _fail(false) {}

Image::Image(const char *path) : data(nullptr), _fail(false) {

  if (Read(path) != 0) {

    // throw something!
  }
}

Image::~Image() {

  free(data);

  data = nullptr;
}

int Image::Read(const char *path) {

  _path = std::string(path);

  FILE *fp;

  if (!(fp = fopen(path, "rb"))) {

    _fail = true;

    return 1;
  }

  png_byte header[8];

  size_t size = fread(header, sizeof(png_byte), 8, fp);

  if (size <= 0 || !png_check_sig(header, 8)) {

    _type = Image::Type::Unknown;

    fclose(fp);

    _fail = true;

    return 1;
  }

  _type = Image::Type::PNG;

  png_structp png_ptr;

  if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                         nullptr, nullptr))) {

    fclose(fp);

    _fail = true;

    return 1;
  }

  png_infop info_ptr;

  if (!(info_ptr = png_create_info_struct(png_ptr))) {

    png_destroy_read_struct(&png_ptr, (png_infopp) nullptr,
                            (png_infopp) nullptr);

    fclose(fp);

    _fail = true;

    return 1;
  }

  png_infop end_info;

  if (!(end_info = png_create_info_struct(png_ptr))) {

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) nullptr);

    fclose(fp);

    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    fclose(fp);

    _fail = true;

    return 1;
  }

  png_init_io(png_ptr, fp);

  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  int color_type, interlace_type;

  png_uint_32 _width, _height;

  png_get_IHDR(png_ptr, info_ptr, &_width, &_height, &depth, &color_type,
               &interlace_type, (int *)NULL, (int *)NULL);

  height = _height;

  width = _width;

  if (color_type == PNG_COLOR_TYPE_PALETTE) {

    png_set_expand(png_ptr);
  }

  if (color_type == PNG_COLOR_TYPE_GRAY && depth < 8) {

    png_set_expand(png_ptr);
  }

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {

    png_set_expand(png_ptr);
  }

  if (depth == 16) {

    png_set_strip_16(png_ptr);
  }

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {

    png_set_gray_to_rgb(png_ptr);

    png_set_invert_alpha(png_ptr);
  }

  double gamma, display_exponent = 2.2 * 1.0;

  if (png_get_gAMA(png_ptr, info_ptr, &gamma)) {

    png_set_gamma(png_ptr, display_exponent, gamma);
  }

  if (color_type == PNG_COLOR_TYPE_RGB ||
      color_type == PNG_COLOR_TYPE_RGB_ALPHA) {

    png_set_bgr(png_ptr);
  }

  channels = 4;

  png_read_update_info(png_ptr, info_ptr);

  png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  bytes_per_line = rowbytes;

  if ((data = (unsigned char *)malloc(rowbytes * height *
                                      sizeof(unsigned char))) == nullptr) {

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    fclose(fp);

    _fail = true;

    return 1;
  }

  png_bytep *row_pointers;

  if ((row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep))) ==
      nullptr) {

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    fclose(fp);

    _fail = true;

    return 1;
  }

  for (png_uint_32 i = 0; i < static_cast<png_uint_32>(height); ++i) {

    row_pointers[i] = data + i * rowbytes;
  }

  png_read_image(png_ptr, row_pointers);

  png_read_end(png_ptr, end_info);

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  free(row_pointers);

  fclose(fp);

  return 0;
}
