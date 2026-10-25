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

// Isole chaque mot de la liste et l'encadre en vert au plus près
void segment_word_list(SDL_Surface *surface, int xmin, int xmax, int ymin,
                       int ymax, int grid) {
    int in_word = 0;
    int word_ymin = 0;
    int threshold = 2; // Tolérance

    for (int y = ymin; y <= ymax; y++) {
        int black_pixels = 0;
        
        if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
        Uint32 *pixels = (Uint32 *)surface->pixels;
        SDL_PixelFormat *format = surface->format;
        Uint8 r, g, b;

        for (int x = xmin; x <= xmax; x++) {
            if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
                SDL_GetRGB(pixels[y * surface->w + x], format, &r, &g, &b);
                if (r == 0) black_pixels++;
            }
        }
        if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

        if (black_pixels > threshold) {
            if (!in_word) {
                in_word = 1;
                word_ymin = y;
            }
        } else {
            if (in_word) {
                in_word = 0;
                int w_xmin = xmin, w_xmax = xmax;
                int w_ymin = word_ymin, w_ymax = y - 1;
                
                // Resserre le cadre vert autour du mot précis
                crop_box(surface, &w_xmin, &w_xmax, &w_ymin, &w_ymax);
                w_xmin--, w_ymin--; 
                w_xmax++, w_ymax++; // Ajout d'une marge de 1 pixel
                draw_rectangle(surface, w_xmin, w_xmax, w_ymin, w_ymax, 0, 255, 0);
                segment_letters(surface, w_xmin, w_xmax, w_ymin, w_ymax, grid);
            }
        }
    }
    
    // Dernier mot
    if (in_word) {
        int w_xmin = xmin, w_xmax = xmax;
        int w_ymin = word_ymin, w_ymax = ymax;
        
        crop_box(surface, &w_xmin, &w_xmax, &w_ymin, &w_ymax);
        w_xmin--, w_ymin--; 
        w_xmax++, w_ymax++; // Ajout d'une marge de 1 pixel
        if(!grid) {
            draw_rectangle(surface, w_xmin, w_xmax, w_ymin, w_ymax, 0, 255, 0);
        }
        segment_letters(surface, w_xmin, w_xmax, w_ymin, w_ymax, grid);
    }
}

// Isoler les lettres de la grille et les encadrer en violet au plus près
void segment_letters(SDL_Surface *surface, int xmin, int xmax, int ymin,
                          int ymax, int grid) {
    int in_letter = 0;
    int letter_xmin = 0;
    int threshold = 2; // Tolérance
    for (int x = xmin; x <= xmax; x++) {
        int black_pixels = 0;

        if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
        Uint32 *pixels = (Uint32 *)surface->pixels;
        SDL_PixelFormat *format = surface->format;
        Uint8 r, g, b;
        for (int y = ymin; y <= ymax; y++) {
            if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
                SDL_GetRGB(pixels[y * surface->w + x], format, &r, &g, &b);
                if (r == 0) black_pixels++;
            }
        }
        if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

        if (black_pixels > threshold) {
            if (!in_letter) {
                in_letter = 1;
                letter_xmin = x;
            }
        } else {
            if (in_letter) {
                in_letter = 0;
                int l_xmin = letter_xmin, l_xmax = x - 1;
                int l_ymin = ymin, l_ymax = ymax;
                crop_box(surface, &l_xmin, &l_xmax, &l_ymin, &l_ymax);
                if(grid) {
                    draw_rectangle(surface, l_xmin, l_xmax, l_ymin, l_ymax,
                               128, 128, 0);
                } else {
                    draw_rectangle(surface, l_xmin, l_xmax, l_ymin, l_ymax,
                               128, 0, 128);
                }
            }
        }
    }
    // Dernière lettre
    if (in_letter) {
        int l_xmin = letter_xmin, l_xmax = xmax;
        int l_ymin = ymin, l_ymax = ymax;
        crop_box(surface, &l_xmin, &l_xmax, &l_ymin, &l_ymax);
        if(grid) {
            draw_rectangle(surface, l_xmin, l_xmax, l_ymin, l_ymax,
                        128, 128, 0);
        } else {
            draw_rectangle(surface, l_xmin, l_xmax, l_ymin, l_ymax,
                        128, 0, 128);
        }
    }
}

// test si le mot mélé possède une grille en utilisant l'histogramme
int has_grid(int xmin, int xmax, int ymin, int ymax, int* vert_hist, int* horiz_hist)
{
    int width = xmax - xmin + 1;
    int height = ymax - ymin + 1;
    int vertical_threshold = (height * 8) / 10;
    int horizontal_threshold = (width * 8) / 10;
    int vertical_lines = 0;
    int horizontal_lines = 0;
    int in_vertical_line = 0;
    for (int x = xmin; x <= xmax; x++) {
        if (vert_hist[x] >= vertical_threshold) {
            if (!in_vertical_line) {
                vertical_lines++;
                in_vertical_line = 1;
            }
        } else {
            in_vertical_line = 0;
        }
    }
    int in_horizontal_line = 0;
    for (int y = ymin; y <= ymax; y++) {
        if (horiz_hist[y] >= horizontal_threshold) {
            if (!in_horizontal_line) {
                horizontal_lines++;
                in_horizontal_line = 1;
            }
        } else {
            in_horizontal_line = 0;
        }
    }
    return (vertical_lines >= 2 && horizontal_lines >= 2);
}

//calcul l'épesseur max du bord de la grille
int compute_cell_margin(int xmin, int xmax, int ymin, int ymax, int *vert_hist, int *horiz_hist, double threshold) {
    int vert_limit = (int)((ymax-ymin+1)*threshold);
    int horiz_limit = (int)((xmax-xmin+1)*threshold);
    int max_thickness = 0;
    int thickness = 0;
    for (int x = xmin; x <= xmax; x++) {
        if (vert_hist[x] >= vert_limit) {
            thickness++;
        }
        else {
            if (thickness > max_thickness)
                max_thickness = thickness;

            thickness = 0;
        }
    }
    if (thickness > max_thickness)
        max_thickness = thickness;
    thickness = 0;
    for (int y = ymin; y <= ymax; y++) {
        if (horiz_hist[y] >= horiz_limit) {
            thickness++;
        }
        else {
            if (thickness > max_thickness)
                max_thickness = thickness;
            thickness = 0;
        }
    }
    if (thickness > max_thickness)
        max_thickness = thickness;
    return max_thickness+1;
}

//permet d'enlever le vert dans le cas sans grille
void remove_green(SDL_Surface *surface, int xmin, int xmax, int ymin, int ymax) {
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    SDL_PixelFormat *format = surface->format;
    Uint32 green = SDL_MapRGB(format, 0, 255, 0);
    Uint32 white = SDL_MapRGB(format, 255, 255, 255);
    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            if (pixels[y * surface->w + x] == green) {
                pixels[y * surface->w + x] = white;
            }
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

// Isole les lettres de la grille et les encadre en jaune
void segment_grid_letters_with_histogram(SDL_Surface *surface, int xmin, int xmax, int ymin, int ymax, int* vert_hist, int* horiz_hist) {
    if (!has_grid(xmin, xmax, ymin, ymax, vert_hist, horiz_hist)) {
        segment_word_list(surface, xmin, xmax, ymin, ymax, 1);
        remove_green(surface, xmin-1, xmax+1, ymin-1, ymax+1); //mesure temporaire (normalement)
    } else {
        //cas avec du bruit ou pixel en plus
        const double threshold = 0.65;
        const int LINE_GAP = 5;
        const int CELL_MARGIN = compute_cell_margin(xmin, xmax, ymin, ymax, vert_hist, horiz_hist, threshold); //nombre de pixel à retirer au bord
        int MAX_LINES = 256;
        int vert_lines[MAX_LINES];
        int horiz_lines[MAX_LINES];
        int nb_vert = 0;
        int nb_horiz = 0;
        int vert_limit = (int)((ymax-ymin+1)*threshold);
        int horiz_limit = (int)((xmax-xmin+1)*threshold);
        for (int x = xmin; x <= xmax; x++) {
            if (vert_hist[x] >= vert_limit) {
                if (nb_vert == 0 || x - vert_lines[nb_vert - 1] > LINE_GAP) {
                    vert_lines[nb_vert++] = x;
                }
            }
        }
        for (int y = ymin; y <= ymax; y++) {
            if (horiz_hist[y] >= horiz_limit) {
                if (nb_horiz == 0 || y - horiz_lines[nb_horiz - 1] > LINE_GAP) {
                    horiz_lines[nb_horiz++] = y;
                }
            }
        }
        for (int row = 0; row < nb_horiz - 1; row++) {
            for (int col = 0; col < nb_vert - 1; col++) {
                int l_xmin = vert_lines[col] + CELL_MARGIN;
                int l_xmax = vert_lines[col + 1] - CELL_MARGIN;
                int l_ymin = horiz_lines[row] + CELL_MARGIN;
                int l_ymax = horiz_lines[row + 1] - CELL_MARGIN;
                crop_box(surface, &l_xmin, &l_xmax, &l_ymin, &l_ymax);
                if(l_xmin>l_xmax || l_ymin>l_ymax) continue;
                draw_rectangle(surface, l_xmin, l_xmax, l_ymin, l_ymax, 128, 128, 0);
            }
        }
    }
}