
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

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



SDL_Surface *rotate_image_90(const SDL_Surface *surface, int clockwise)
{
    if (surface == NULL)
        return NULL;

    // Format commun avec les autres modules du projet.
    if (surface->format->format != SDL_PIXELFORMAT_RGB888)
        return NULL;

    int old_width = surface->w;
    int old_height = surface->h;

    // Après une rotation de 90°, les dimensions s'inversent.
    SDL_Surface *rotated = SDL_CreateRGBSurfaceWithFormat(
        0,
        old_height,
        old_width,
        32,
        SDL_PIXELFORMAT_RGB888
    );

    if (rotated == NULL)
        return NULL;

    SDL_Surface *source = (SDL_Surface *)surface;

    if (SDL_MUSTLOCK(source) && SDL_LockSurface(source) != 0)
    {
        SDL_FreeSurface(rotated);
        return NULL;
    }

    if (SDL_MUSTLOCK(rotated) && SDL_LockSurface(rotated) != 0)
    {
        if (SDL_MUSTLOCK(source))
            SDL_UnlockSurface(source);

        SDL_FreeSurface(rotated);
        return NULL;
    }

    for (int y = 0; y < old_height; y++)
    {
        const Uint32 *src_row = (const Uint32 *)(
            (const Uint8 *)source->pixels + y * source->pitch
        );

        for (int x = 0; x < old_width; x++)
        {
            int new_x;
            int new_y;

            if (clockwise)
            {
                // Rotation droite.
                new_x = old_height - 1 - y;
                new_y = x;
            }
            else
            {
                // Rotation gauche.
                new_x = y;
                new_y = old_width - 1 - x;
            }

            Uint32 *dst_row = (Uint32 *)(
                (Uint8 *)rotated->pixels
                + new_y * rotated->pitch
            );

            dst_row[new_x] = src_row[x];
        }
    }

    if (SDL_MUSTLOCK(rotated))
        SDL_UnlockSurface(rotated);

    if (SDL_MUSTLOCK(source))
        SDL_UnlockSurface(source);

    return rotated;
}



SDL_Surface *rotate_image(const SDL_Surface *surface, int angle)
{
    if (surface == NULL ||
        surface->format->format != SDL_PIXELFORMAT_RGB888)
        return NULL;

    angle %= 360;

    // Rotations exactes, sans perte de qualité.
    if (angle == 90 || angle == -270)
        return rotate_image_90(surface, 1);

    if (angle == -90 || angle == 270)
        return rotate_image_90(surface, 0);

    if (angle == 0)
        return SDL_ConvertSurfaceFormat(
            (SDL_Surface *)surface, SDL_PIXELFORMAT_RGB888, 0);

    double radians = angle * 3.14159265358979323846 / 180.0;
    double c = cos(radians);
    double s = sin(radians);

    int width = surface->w;
    int height = surface->h;

    int new_width = (int)ceil(
        fabs(width * c) + fabs(height * s));

    int new_height = (int)ceil(
        fabs(width * s) + fabs(height * c));

    SDL_Surface *rotated = SDL_CreateRGBSurfaceWithFormat(
        0, new_width, new_height, 32, SDL_PIXELFORMAT_RGB888);

    if (rotated == NULL)
        return NULL;

    SDL_Surface *source = (SDL_Surface *)surface;

    if (SDL_MUSTLOCK(source) && SDL_LockSurface(source) != 0)
    {
        SDL_FreeSurface(rotated);
        return NULL;
    }

    if (SDL_MUSTLOCK(rotated) && SDL_LockSurface(rotated) != 0)
    {
        if (SDL_MUSTLOCK(source))
            SDL_UnlockSurface(source);

        SDL_FreeSurface(rotated);
        return NULL;
    }

    double src_cx = (width - 1) / 2.0;
    double src_cy = (height - 1) / 2.0;
    double dst_cx = (new_width - 1) / 2.0;
    double dst_cy = (new_height - 1) / 2.0;

    for (int y = 0; y < new_height; y++)
    {
        Uint32 *dst_row = (Uint32 *)(
            (Uint8 *)rotated->pixels + y * rotated->pitch);

        for (int x = 0; x < new_width; x++)
        {
            double dx = x - dst_cx;
            double dy = y - dst_cy;

            // Angle positif = rotation vers la droite.
            double src_x = c * dx + s * dy + src_cx;
            double src_y = -s * dx + c * dy + src_cy;

            int x0 = (int)floor(src_x);
            int y0 = (int)floor(src_y);

            double fx = src_x - x0;
            double fy = src_y - y0;

            // Chaque voisin hors image est considéré blanc.
            double gray = 0.0;

            for (int j = 0; j < 2; j++)
            {
                for (int i = 0; i < 2; i++)
                {
                    int sx = x0 + i;
                    int sy = y0 + j;

                    double wx = i ? fx : 1.0 - fx;
                    double wy = j ? fy : 1.0 - fy;
                    double weight = wx * wy;

                    Uint8 value = 255;

                    if (sx >= 0 && sx < width &&
                        sy >= 0 && sy < height)
                    {
                        const Uint32 *src_row =
                            (const Uint32 *)(
                                (const Uint8 *)source->pixels
                                + sy * source->pitch);

                        Uint8 r, g, b;
                        SDL_GetRGB(src_row[sx], source->format,
                                   &r, &g, &b);

                        // L'image source est déjà en gris.
                        value = r;
                    }

                    gray += weight * value;
                }
            }

            Uint8 result = (Uint8)(gray + 0.5);

            dst_row[x] = SDL_MapRGB(
                rotated->format, result, result, result);
        }
    }

    if (SDL_MUSTLOCK(rotated))
        SDL_UnlockSurface(rotated);

    if (SDL_MUSTLOCK(source))
        SDL_UnlockSurface(source);

    return rotated;
}
