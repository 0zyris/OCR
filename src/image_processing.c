#include <SDL2/SDL.h>
#include "../include/image_processing.h"

// Calcule le seuil d'Otsu
int calculate_otsu_threshold(SDL_Surface *surface) {
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pixel_count = surface->w * surface->h;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;
    int histogram[256] = {0};

    // 1. Créer l'histogramme des niveaux de gris
    for (int i = 0; i < pixel_count; i++) {
        SDL_GetRGB(pixels[i], format, &r, &g, &b);
        Uint8 luminance = 0.3 * r + 0.59 * g + 0.11 * b;
        histogram[luminance]++;
    }

    // 2. Méthode d'Otsu
    int total = pixel_count;
    float sum = 0;
    for (int i = 0; i < 256; i++) sum += i * histogram[i];

    float sumB = 0;
    int wB = 0, wF = 0;
    float varMax = 0;
    int threshold = 0;

    for (int i = 0; i < 256; i++) {
        wB += histogram[i];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += (float)(i * histogram[i]);
        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;

        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);
        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }
    return threshold;
}

// Fonction de binarisation
void binarize_image(SDL_Surface *surface) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

    int threshold = calculate_otsu_threshold(surface);
    
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pixel_count = surface->w * surface->h;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;

    for (int i = 0; i < pixel_count; i++) {
        SDL_GetRGB(pixels[i], format, &r, &g, &b);
        Uint8 luminance = 0.3 * r + 0.59 * g + 0.11 * b;
        
        // Si plus sombre que le seuil dynamique, on met noir
        if (luminance < threshold) {
            pixels[i] = SDL_MapRGB(format, 0, 0, 0);
        } else {
            pixels[i] = SDL_MapRGB(format, 255, 255, 255);
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

void compute_horizontal_histogram(SDL_Surface *surface, int *histogram) {
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    Uint32 *pixels = (Uint32 *)surface->pixels;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;

    // Initialisation de l'histogramme à 0
    for (int y = 0; y < surface->h; y++) {
        histogram[y] = 0;
    }

    // Parcours ligne par ligne
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            // L'index 1D correspondant aux coordonnées 2D (x, y)
            int index = y * surface->w + x;
            SDL_GetRGB(pixels[index], format, &r, &g, &b);
            
            // Puisque l'image est binarisée
            // -> on vérifie juste si le pixel est noir
            if (r == 0) {
                histogram[y]++;
            }
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}

void compute_vertical_histogram(SDL_Surface *surface, int *histogram) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

    Uint32 *pixels = (Uint32 *)surface->pixels;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;

    for (int x = 0; x < surface->w; x++) {
        histogram[x] = 0;
    }

    for (int x = 0; x < surface->w; x++) {
        for (int y = 0; y < surface->h; y++) {
            int index = y * surface->w + x;
            SDL_GetRGB(pixels[index], format, &r, &g, &b);
            
            if (r == 0) {
                histogram[x]++;
            }
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

// NFonction de dessin avec couleur paramétrable
void draw_rectangle(SDL_Surface *surface, int x_min, int x_max, int y_min,
                        int y_max, Uint8 r_val, Uint8 g_val, Uint8 b_val) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    Uint32 color = SDL_MapRGB(surface->format, r_val, g_val, b_val);

    for (int y = y_min; y <= y_max; y++) {
        if (x_min >= 0 && x_min < surface->w) {
            pixels[y * surface->w + x_min] = color;
        }
        if (x_max >= 0 && x_max < surface->w) {
            pixels[y * surface->w + x_max] = color;
        }
    }
    
    for (int x = x_min; x <= x_max; x++) {
        if (y_min >= 0 && y_min < surface->h) {
            pixels[y_min * surface->w + x] = color;
        }
        if (y_max >= 0 && y_max < surface->h) {
            pixels[y_max * surface->w + x] = color;
        }
    }
    
    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

// Détermine la disposition et coupe l'image en deux grands blocs
void find_layout_and_blocks(int *vert_hist, int width, int *horiz_hist,
                            int height, int *b1_xmin, int *b1_xmax,
                            int *b1_ymin, int *b1_ymax, int *b2_xmin,
                            int *b2_xmax, int *b2_ymin, int *b2_ymax) {
    int threshold = 5;

    // 1. Trouver le plus grand trou sur l'axe X (colonnes)
    int max_x_start = 0, max_x_width = 0, cur_x_start = 0, cur_x_len = 0;
    for (int x = 0; x < width; x++) {
        if (vert_hist[x] <= threshold) {
            if (cur_x_len == 0) cur_x_start = x;
            cur_x_len++;
        } else {
            if (cur_x_len > max_x_width) {
                max_x_width = cur_x_len; 
                max_x_start = cur_x_start; 
            }
            cur_x_len = 0;
        }
    }

    // 2. Trouver le plus grand trou sur l'axe Y (lignes)
    int max_y_start = 0, max_y_width = 0, cur_y_start = 0, cur_y_len = 0;
    for (int y = 0; y < height; y++) {
        if (horiz_hist[y] <= threshold) {
            if (cur_y_len == 0) cur_y_start = y;
            cur_y_len++;
        } else {
            if (cur_y_len > max_y_width) { max_y_width = cur_y_len; max_y_start = cur_y_start; }
            cur_y_len = 0;
        }
    }

    // On part du principe que chaque bloc prend toute l'image
    *b1_xmin = 0; *b1_xmax = width - 1; *b1_ymin = 0; *b1_ymax = height - 1;
    *b2_xmin = 0; *b2_xmax = width - 1; *b2_ymin = 0; *b2_ymax = height - 1;

    // 3. Le trou le plus grand dicte si on coupe verticalement ou horizontalement
    if (max_x_width > max_y_width) {
        // Disposition GAUCHE / DROITE
        *b1_xmax = max_x_start - 1;
        *b2_xmin = max_x_start + max_x_width;
    } else {
        // Disposition HAUT / BAS
        *b1_ymax = max_y_start - 1;
        *b2_ymin = max_y_start + max_y_width;
    }
}

// Resserre un cadre autour des pixels noirs qu'il contient
void crop_box(SDL_Surface *surface, int *xmin, int *xmax, int *ymin,
    int *ymax) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    SDL_PixelFormat *format = surface->format;
    Uint8 r, g, b;

    int new_ymin = surface->h, new_ymax = 0;
    int new_xmin = surface->w, new_xmax = 0;
    int found = 0;

    for (int y = *ymin; y <= *ymax; y++) {
        for (int x = *xmin; x <= *xmax; x++) {
            if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
                SDL_GetRGB(pixels[y * surface->w + x], format, &r, &g, &b);
                if (r == 0) { // Si on trouve de l'encre
                    if (x < new_xmin) new_xmin = x;
                    if (x > new_xmax) new_xmax = x;
                    if (y < new_ymin) new_ymin = y;
                    if (y > new_ymax) new_ymax = y;
                    found = 1;
                }
            }
        }
    }
    
    // MAJ des coordonnéess
    if (found) {
        *xmin = new_xmin; *xmax = new_xmax;
        *ymin = new_ymin; *ymax = new_ymax;
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}