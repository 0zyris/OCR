
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"

Image *image_create(size_t width, size_t height)
{
    if (width == 0 || height == 0)
        return NULL;

    if (width > SIZE_MAX / height)
        return NULL;

    size_t count = width * height;

    if (count > SIZE_MAX / sizeof(Pixel))
        return NULL;

    Image *image = malloc(sizeof(Image));

    if (image == NULL)
        return NULL;

    image->pixels = calloc(count, sizeof(Pixel));

    if (image->pixels == NULL)
    {
        free(image);
        return NULL;
    }

    image->width = width;
    image->height = height;

    return image;
}

void image_free(Image *image)
{
    if (image == NULL)
        return;

    free(image->pixels);
    free(image);
}

Pixel *image_get_pixel(Image *image, size_t x, size_t y)
{
    if (image == NULL)
        return NULL;

    if (x >= image->width || y >= image->height)
        return NULL;

    return &image->pixels[y * image->width + x];
}

Image *image_load(const char *path)
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
        loaded, SDL_PIXELFORMAT_RGB24, 0);

    SDL_FreeSurface(loaded);

    if (surface == NULL)
    {
        fprintf(stderr, "Image conversion failed: %s\n",
                SDL_GetError());
        return NULL;
    }

    Image *image = image_create(
        (size_t)surface->w, (size_t)surface->h);

    if (image == NULL)
    {
        SDL_FreeSurface(surface);
        return NULL;
    }

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0)
    {
        fprintf(stderr, "Surface lock failed: %s\n",
                SDL_GetError());
        image_free(image);
        SDL_FreeSurface(surface);
        return NULL;
    }

    for (int y = 0; y < surface->h; y++)
    {
        const uint8_t *row =
            (const uint8_t *)surface->pixels
            + (size_t)y * (size_t)surface->pitch;

        for (int x = 0; x < surface->w; x++)
        {
            Pixel *pixel = image_get_pixel(
                image, (size_t)x, (size_t)y);

            pixel->r = row[x * 3];
            pixel->g = row[x * 3 + 1];
            pixel->b = row[x * 3 + 2];
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    SDL_FreeSurface(surface);

    return image;
}
