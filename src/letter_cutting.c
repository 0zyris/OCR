#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../include/letter_cutting.h"

int detect_grid_lines(SDL_Surface *surface, int *width, int *height, int *hcount, int *wcount)
{
    // Détection des lignes verticales
    for(int i = 0; i < surface->w; i++)
    {
        int black = 0;
        for(int y = 0; y < surface->h; y++)
        {
            Uint32 pixel = get_pixel(surface, i, y);
            if(pixel == 0)
            {
                black++;
            }
        }
        if(black == surface->h)
        {
            w_lines[*wcount] = i;
            (*wcount)++;
        }
    }
    // Détection des lignes horizontales
    for(int j = 0; j < surface->h; j++)
    {
        int black = 0;
        for(int x = 0; x < surface->w; x++)
        {
            Uint32 pixel = get_pixel(surface, x, j);
            if(pixel == 0)
            {
                black++;
            }
        }
        if(black == surface->w)
        {
            h_lines[*hcount] = j;
            (*hcount)++;
        }
    }
    return (*hcount >= 2 && *wcount >= 2);
}

SDL_Surface* cut_letter(SDL_Surface *surface, int x, int y, int width, int height)
{
    SDL_Surface *letter_surface = SDL_CreateRGBSurface(0, width, height, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
    if(letter_surface == NULL)
    {
        errx(EXIT_FAILURE, "Erreur lors de la création de la surface pour la lettre : %s", SDL_GetError());
    }
    SDL_Rect rectangle = {x, y, width, height};
    SDL_BlitSurface(surface, &rectangle, letter_surface, NULL);
    return letter_surface;
}

LetterGrid extract_letters(SDL_Surface *surface, SDL_Rect rectangle)
{
    LetterGrid grid;
    SDL_Surface *grid_surface = cut_letter(surface, rectangle.x, rectangle.y, rectangle.w, rectangle.h);
    int h_lines[1024], w_lines[1024];
    int hcount = 0, wcount = 0;
    if(detect_grid_lines(grid_surface, &wcount, &hcount, h_lines, w_lines))
    {
        //cas avec grille
        int rows = hcount - 1;
        int cols = wcount - 1;
        grid.rows = rows;
        grid.cols = cols;
        grid.cells = malloc(rows * sizeof(SDL_Surface**));
        for(int i = 0; i < rows; i++)
        {
            grid.cells[i] = malloc(cols * sizeof(SDL_Surface*));
            for(int y = 0; y < cols; y++)
            {
                int x1 = w_lines[y];
                int x2 = w_lines[y + 1];
                int y1 = h_lines[i];
                int y2 = h_lines[i + 1];
                grid.cells[i][y] = cut_letter(grid_surface, x1, y1, x2 - x1, y2 - y1);
            }
        }
    }
    else
    {
        //cas sans grille
    }
    SDL_FreeSurface(grid);
    return letters;
}

