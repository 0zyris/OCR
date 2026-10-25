
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include "image.h"

SDL_Surface *image_load(const char *path)
{
    if (path == NULL)
        return NULL;

    SDL_Surface *loaded = IMG_Load(path);

    if (loaded == NULL)
    {
        fprintf(stderr, "Image loading failed: %s\n",
                IMG_GetError());
        return NULL;
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(
        loaded, SDL_PIXELFORMAT_RGB888, 0);

    SDL_FreeSurface(loaded);

    if (surface == NULL)
    {
        fprintf(stderr, "Image conversion failed: %s\n",
                SDL_GetError());
        return NULL;
    }

    return surface;
}

void image_free(SDL_Surface *surface)
{
    if (surface != NULL)
        SDL_FreeSurface(surface);
}
