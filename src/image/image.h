
#ifndef IMAGE_H
#define IMAGE_H

#include <SDL2/SDL.h>

SDL_Surface *image_load(const char *path);

void image_free(SDL_Surface *surface);
void grayscale_image(SDL_Surface *surface);

#endif
