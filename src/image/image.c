
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


void grayscale_image(SDL_Surface *surface)
{
    if (surface == NULL)
        return;

    // Notre application utilise RGB888 : 4 octets par pixel.
    if (surface->format->format != SDL_PIXELFORMAT_RGB888)
        return;

    if (SDL_MUSTLOCK(surface))
    {
        if (SDL_LockSurface(surface) != 0)
            return;
    }

    for (int y = 0; y < surface->h; y++)
    {
        // pitch = nombre d'octets entre deux lignes.
        Uint8 *row = (Uint8 *)surface->pixels
                   + y * surface->pitch;

        Uint32 *pixels = (Uint32 *)row;

        for (int x = 0; x < surface->w; x++)
        {
            Uint8 r, g, b;

            SDL_GetRGB(pixels[x], surface->format,
                       &r, &g, &b);

            // Calcul de la luminance.
            Uint8 gray = (Uint8)(
                0.299 * r +
                0.587 * g +
                0.114 * b
            );

            // Les trois composantes deviennent identiques.
            pixels[x] = SDL_MapRGB(
                surface->format, gray, gray, gray);
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}
