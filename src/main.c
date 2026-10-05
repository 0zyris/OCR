#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../include/image_processing.h"
#include "../include/letter_cutting.h"

// Fonction pour charger une image dans une SDL_Surface
SDL_Surface* load_image(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        errx(EXIT_FAILURE, "Erreur lors du chargement de l'image : %s",
            IMG_GetError());
    }
    
    SDL_Surface* optimized_surface = SDL_ConvertSurfaceFormat(surface,
        SDL_PIXELFORMAT_RGB888, 0);
    if (optimized_surface == NULL) {
        errx(EXIT_FAILURE, "Erreur lors de la conversion de la surface : %s",
            SDL_GetError());
    }
    
    SDL_FreeSurface(surface);
    return optimized_surface;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage: %s <fichier_image> <mot_ou_option>",
            argv[0]);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) errx(EXIT_FAILURE,
        "Erreur d'initialisation de la SDL : %s", SDL_GetError());
    
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        errx(EXIT_FAILURE, "Erreur d'initialisation de SDL_image : %s",
            IMG_GetError());
    }

    SDL_Surface* image = load_image(argv[1]);
    printf("Image '%s' chargée avec succès.\n", argv[1]);
    printf("Dimensions : %d x %d pixels\n", image->w, image->h);

    binarize_image(image);
    printf("Binarisation terminée.\n");

    // 1. Allocation et calcul des deux histogrammes
    int *horiz_hist = malloc(image->h * sizeof(int));
    if (horiz_hist == NULL) errx(EXIT_FAILURE,
        "Erreur d'allocation mémoire pour l'histogramme horizontal");
    compute_horizontal_histogram(image, horiz_hist);

    int *vert_hist = malloc(image->w * sizeof(int));
    if (vert_hist == NULL) errx(EXIT_FAILURE,
        "Erreur d'allocation mémoire pour l'histogramme vertical");
    compute_vertical_histogram(image, vert_hist);

    // 2. Détermination de la disposition globale et coupe initiale
    int b1_xmin, b1_xmax, b1_ymin, b1_ymax;
    int b2_xmin, b2_xmax, b2_ymin, b2_ymax;

    find_layout_and_blocks(vert_hist, image->w, horiz_hist, image->h, 
                           &b1_xmin, &b1_xmax, &b1_ymin, &b1_ymax,
                           &b2_xmin, &b2_xmax, &b2_ymin, &b2_ymax);

    // 3. Ajustement des cadres autour du texte de chaque bloc
    crop_box(image, &b1_xmin, &b1_xmax, &b1_ymin, &b1_ymax);
    crop_box(image, &b2_xmin, &b2_xmax, &b2_ymin, &b2_ymax);

    // 4. Identification par superficie (Largeur * Hauteur)
    long area1 = (long)(b1_xmax - b1_xmin) * (b1_ymax - b1_ymin);
    long area2 = (long)(b2_xmax - b2_xmin) * (b2_ymax - b2_ymin);

    printf("\n--- Identification des blocs ---\n");
    if (area1 > area2) {
        printf("-> Le BLOC 1 est la GRILLE (Bleu)\n");
        printf("-> Le BLOC 2 est la LISTE DE MOTS (Rouge)\n");
        draw_rectangle(image, b1_xmin, b1_xmax, b1_ymin, b1_ymax,
            0, 0, 255); // Bleu
        draw_rectangle(image, b2_xmin, b2_xmax, b2_ymin, b2_ymax,
            255, 0, 0); // Rouge
            int *h_lines = malloc(sizeof(int)*1024);
	    int *w_lines = malloc(sizeof(int)*1024);
            int hcount = 0, wcount = 0;
            if(detect_grid_lines(image, b1_xmax - b1_xmin, b1_ymax - b1_ymin, &wcount, &hcount, &h_lines, &w_lines)){
                for(int i = 0; i < hcount - 1; i++){
                    for(int y = 0; y < wcount - 1; y++){
                        int x1 = w_lines[y];
                        int x2 = w_lines[y + 1];
                        int y1 = h_lines[i];
                        int y2 = h_lines[i + 1];
                        draw_rectangle(image, x1, y1, x2 - x1, y2 - y1, 0, 255, 0); // Vert
                    }
                }
            }
	    free(h_lines);
	    free(w_lines);
    } else {
        printf("-> Le BLOC 2 est la GRILLE (Bleu)\n");
        printf("-> Le BLOC 1 est la LISTE DE MOTS (Rouge)\n");
        draw_rectangle(image, b1_xmin, b1_xmax, b1_ymin, b1_ymax,
            255, 0, 0); // Rouge
        draw_rectangle(image, b2_xmin, b2_xmax, b2_ymin, b2_ymax,
            0, 0, 255); // Bleu
            printf("-> Le BLOC 3 est la LISTE DE MOTS (Vert)\n");
            int *h_lines = malloc(sizeof(int)*2000);
	    int *w_lines =malloc(sizeof(int)*2000);
            int hcount = 0, wcount = 0;
            if(detect_grid_lines(image, b2_xmax - b2_xmin, b2_ymax - b2_ymin, &wcount, &hcount, &h_lines, &w_lines)){
                for(int i = 0; i < hcount - 1; i++){
                    for(int y = 0; y < wcount - 1; y++){
                        int x1 = w_lines[y];
                        int x2 = w_lines[y + 1];
                        int y1 = h_lines[i];
                        int y2 = h_lines[i + 1];
                        draw_rectangle(image, x1, y1, x2 - x1, y2 - y1, 0, 255, 0); // Vert
                    }
                }
            }
	    free(h_lines);
	    free(w_lines);
        }
    }

    // 5. Libération des histogrammes
    free(horiz_hist);
    free(vert_hist);

    // Sauvegarde du résultat
    if (IMG_SavePNG(image, "output_binarized.png") != 0) {
        printf("Erreur de sauvegarde : %s\n", IMG_GetError());
    } else {
        printf("Résultat sauvegardé sous 'output_binarized.png'.\n");
    }

    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
