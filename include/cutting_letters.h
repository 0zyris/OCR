#ifndef CUTTING_LETTERS_H
#define CUTTING_LETTERS_H

#include <SDL2/SDL.h>

void crop_box(SDL_Surface *surface, int *xmin, int *xmax, int *ymin,
    int *ymax);
void draw_rectangle(SDL_Surface *surface, int x_min, int x_max, int y_min,
    int y_max, Uint8 r_val, Uint8 g_val, Uint8 b_val);
struct head_surface *segment_word_list(SDL_Surface *surface, int xmin, 
    int xmax, int ymin, int ymax, int grid);
struct head_surface *segment_grid_letters_with_histogram(SDL_Surface *surface,
    int xmin, int xmax, int ymin, int ymax, int* vert_hist, int* horiz_hist);
void save_surface(struct head_surface *head, int grid);

#endif // CUTTING_LETTERS_H