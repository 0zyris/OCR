
#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

typedef struct
{
    size_t width;
    size_t height;
    Pixel *pixels;
} Image;

Image *image_create(size_t width, size_t height);
Image *image_load(const char *path);

void image_free(Image *image);

Pixel *image_get_pixel(Image *image, size_t x, size_t y);

#endif
