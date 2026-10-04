#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H


#include <SDL2/SDL.h>

void binarize_image(SDL_Surface *surface);
void compute_horizontal_histogram(SDL_Surface *surface, int *histogram);
void compute_vertical_histogram(SDL_Surface *surface, int *histogram);
void find_layout_and_blocks(int *vert_hist, int width, int *horiz_hist,
    int height, int *b1_xmin, int *b1_xmax, int *b1_ymin, int *b1_ymax,
    int *b2_xmin, int *b2_xmax, int *b2_ymin, int *b2_ymax);
void crop_box(SDL_Surface *surface, int *xmin, int *xmax, int *ymin,
    int *ymax);
void draw_rectangle(SDL_Surface *surface, int x_min, int x_max, int y_min,
    int y_max, Uint8 r_val, Uint8 g_val, Uint8 b_val);

#endif // IMAGE_PROCESSING_H