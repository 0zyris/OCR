#include <SDL2/SDL.h>
#include "../include/image_processing.h"

void binarize_image(SDL_Surface *surface) {
    // Verrouiller la surface si nécessaire avant d'accéder aux pixels
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    // Le format RGB888 (256 x 256 x256) 
    // -> stocke les pixels sur 32 bits (Uint32)
    // Cela permet de traiter l'image plus rapidement
    // -> même si elle fait 8*8*8 = 24 bits
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pixel_count = surface->w * surface->h;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;

    // Parcours de chaque pixel de l'image
    for (int i = 0; i < pixel_count; i++) {
        // Extraction des composantes de couleur
        SDL_GetRGB(pixels[i], format, &r, &g, &b);
        
        // Calcul de la luminance (perception de l'oeil humain)
        Uint8 luminance = 0.3 * r + 0.59 * g + 0.11 * b;
        
        // Seuillage basique à 128 (si c'est sombre, on met noir, sinon blanc)
        if (luminance < 128) {
            pixels[i] = SDL_MapRGB(format, 0, 0, 0);       // Pixel noir
        } else {
            pixels[i] = SDL_MapRGB(format, 255, 255, 255); // Pixel blanc
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}