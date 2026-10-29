#ifndef TOOLS_H
#define TOOLS_H

#include <SDL2/SDL.h>

struct letters_surface {
    SDL_Surface *surface;
    struct letters_surface *next;
};

struct head_surface {
    struct letters_surface *head;
    struct head_surface *next;
};

struct head_surface *initialize_head_surface(struct letters_surface *letters);
void insert_head_surface(struct head_surface **head, struct head_surface *new);
struct letters_surface *initialize_letters_surface(SDL_Surface *surface);
void insert_letters_surface(struct letters_surface **head, 
    struct letters_surface *new);
void free_letters_surface(struct letters_surface *head);
void free_head_surface(struct head_surface *head);

#endif // TOOLS_H