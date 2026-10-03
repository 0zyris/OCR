#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Fonction pour charger une image dans une SDL_Surface
SDL_Surface* load_image(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        errx(EXIT_FAILURE, "Erreur lors du chargement de l'image : %s", IMG_GetError());
    }
    
    // Conversion de la surface dans un format standard (ex: 32 bits, RGB)
    // Cela facilite l'itération sur les pixels par la suite
    SDL_Surface* optimized_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB888, 0);
    if (optimized_surface == NULL) {
        errx(EXIT_FAILURE, "Erreur lors de la conversion de la surface : %s", SDL_GetError());
    }
    
    SDL_FreeSurface(surface);
    return optimized_surface;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage: %s <fichier_image> <mot_ou_option>", argv[0]);
    }

    // Initialisation de SDL et SDL_image
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        errx(EXIT_FAILURE, "Erreur d'initialisation de la SDL : %s", SDL_GetError());
    }
    
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        errx(EXIT_FAILURE, "Erreur d'initialisation de SDL_image : %s", IMG_GetError());
    }

    // Chargement de l'image
    SDL_Surface* image = load_image(argv[1]);
    printf("Image '%s' chargée avec succès.\n", argv[1]);
    printf("Dimensions : %d x %d pixels\n", image->w, image->h);

    // TODO: Implémenter ici la détection de la liste de mots (binarisation, segmentation...)

    // Libération de la mémoire et fermeture
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}