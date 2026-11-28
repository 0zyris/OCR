#ifndef CUTTING_LETTERS_H
#define CUTTING_LETTERS_H

#include <SDL2/SDL.h>
#include "cutting_tools.h"

struct Point {
    int x;
    int y;
};

void crop_box(SDL_Surface *surface, int *xmin, int *xmax, int *ymin,
    int *ymax);
struct head_surface *segment_word_list(SDL_Surface *surface, int xmin,
    int xmax, int ymin, int ymax, int grid);
struct head_surface *segment_grid_letters_with_histogram(SDL_Surface *surface,
    int xmin, int xmax, int ymin, int ymax, int* vert_hist, int* horiz_hist);
void save_surface(struct head_surface *head, int grid);

#endif // CUTTING_LETTERS_H
