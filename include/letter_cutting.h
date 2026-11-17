#ifndef LETTER_CUTTING_H
#define LETTER_CUTTING_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Surface **letters;
    int len;
} CutWord;

typedef struct {
    SDL_Surface **cells;
    int rows;
    int cols;
    CutWord *words;
    int nb_words;
} CutResult;

CutResult *cutting_run(SDL_Surface *image);
void cutting_result_free(CutResult *r);
void run_image_pipeline(const char *filepath);
#endif
