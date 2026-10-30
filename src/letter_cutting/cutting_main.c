#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../../include/letter_cutting/image_processing.h"
#include "../../include/letter_cutting/cutting_letters.h"
#include "../../include/letter_cutting/cutting_tools.h"

// Function to load an image into an SDL_Surface
SDL_Surface* load_image(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        errx(EXIT_FAILURE, "Error loading the image : %s",
            IMG_GetError());
    }
    
    SDL_Surface* optimized_surface = SDL_ConvertSurfaceFormat(surface,
        SDL_PIXELFORMAT_RGB888, 0);
    if (optimized_surface == NULL) {
        errx(EXIT_FAILURE, "Error during surface conversion : %s",
            SDL_GetError());
    }
    
    SDL_FreeSurface(surface);
    return optimized_surface;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        errx(EXIT_FAILURE, "Use: %s <file_image> <word_or_option>",
            argv[0]);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) errx(EXIT_FAILURE,
        "SDL initialization error : %s", SDL_GetError());
    
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        errx(EXIT_FAILURE, "SDL_image initialization error : %s",
            IMG_GetError());
    }

    SDL_Surface* image = load_image(argv[1]);
    printf("Image '%s' successfully uploaded.\n", argv[1]);
    printf("Dimensions : %d x %d pixels\n", image->w, image->h);

    binarize_image(image);
    printf("Binarization completed.\n");

    // 1. Allocation and calculation of the two histograms
    int *horiz_hist = malloc(image->h * sizeof(int));
    if (horiz_hist == NULL) errx(EXIT_FAILURE,
        "Memory allocation error for the horizontal histogram");
    compute_horizontal_histogram(image, horiz_hist);

    int *vert_hist = malloc(image->w * sizeof(int));
    if (vert_hist == NULL) errx(EXIT_FAILURE,
        "Memory allocation error for the vertical histogram");
    compute_vertical_histogram(image, vert_hist);

    // 2. Determination of the overall layout and initial cut
    int b1_xmin, b1_xmax, b1_ymin, b1_ymax;
    int b2_xmin, b2_xmax, b2_ymin, b2_ymax;

    find_layout_and_blocks(vert_hist, image->w, horiz_hist, image->h,
                           &b1_xmin, &b1_xmax, &b1_ymin, &b1_ymax,
                           &b2_xmin, &b2_xmax, &b2_ymin, &b2_ymax);

    // 3. Adjustment of the frames around the text of each block
    crop_box(image, &b1_xmin, &b1_xmax, &b1_ymin, &b1_ymax);
    crop_box(image, &b2_xmin, &b2_xmax, &b2_ymin, &b2_ymax);

    // 4. Identification by area (Width * Height)
    long area1 = (long)(b1_xmax - b1_xmin) * (b1_ymax - b1_ymin);
    long area2 = (long)(b2_xmax - b2_xmin) * (b2_ymax - b2_ymin);

    printf("\n--- Identification of blocks ---\n");
    if (area1 > area2) {
        printf("-> BLOCK 1 is the GRID (Blue/Yellow)\n");
        printf("-> BLOCK 2 is the WORD LIST (Red/Green/Purple)\n");
        struct head_surface *grid_letters = 
            segment_grid_letters_with_histogram(image, b1_xmin, b1_xmax,
                b1_ymin, b1_ymax, vert_hist, horiz_hist);
        draw_rectangle(image, b1_xmin, b1_xmax, b1_ymin, b1_ymax,
            0, 0, 255);
        draw_rectangle(image, b2_xmin, b2_xmax, b2_ymin, b2_ymax,
            255, 0, 0);
        struct head_surface *word_letters = segment_word_list(image,
            b2_xmin, b2_xmax, b2_ymin, b2_ymax, 0);
        save_surface(grid_letters, 1);
        save_surface(word_letters, 0);
    } else {
        printf("-> BLOCK 2 is the GRID (Blue/Yellow)\n");
        printf("-> BLOCK 1 is the WORD LIST (Red/Green/Purple)\n");
        struct head_surface *grid_letters = 
            segment_grid_letters_with_histogram(image, b2_xmin, b2_xmax,
                b2_ymin, b2_ymax, vert_hist, horiz_hist);
        draw_rectangle(image, b1_xmin, b1_xmax, b1_ymin, b1_ymax,
            255, 0, 0);
        draw_rectangle(image, b2_xmin, b2_xmax, b2_ymin, b2_ymax,
            0, 0, 255);
        struct head_surface *word_letters = segment_word_list(image, b1_xmin,
            b1_xmax, b1_ymin, b1_ymax, 0);
        save_surface(grid_letters, 1);
        save_surface(word_letters, 0);
    }

    // 5. Release of histograms
    free(horiz_hist);
    free(vert_hist);

    // Saving the result
    if (IMG_SavePNG(image, "image/image_storage/output_binarized.png")
        != 0) {
        printf("Saving error : %s\n", IMG_GetError());
    } else {
        printf("Result saved as 'output_binarized.png'.\n");
    }

    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}