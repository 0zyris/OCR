#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <SDL2/SDL.h>

SDL_Surface *image_load(const char *path);
void image_free(SDL_Surface *surface);

void grayscale_image(SDL_Surface *surface);
void binarize_image(SDL_Surface *surface);
SDL_Surface *rotate_image(const SDL_Surface *surface, int angle);
SDL_Surface *rotate_image_90(const SDL_Surface *surface, int clockwise);

#endif
