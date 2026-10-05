#ifndef LETTER_CUTTING_H
#define LETTER_CUTTING_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Surface ***cells; //matrice contenant les lettres
    int rows; //nombre de lignes
    int cols; //nombre de colonnes
} LetterGrid;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
} LetterRect;

int detect_grid_lines(SDL_Surface *surface, int *width, int *height, int *hcount, int *wcount);
SDL_Surface* cut_letter(SDL_Surface *surface, int x, int y, int width, int height);
LetterGrid extract_letters(SDL_Surface *surface, SDL_Rect rectangle);

#endif