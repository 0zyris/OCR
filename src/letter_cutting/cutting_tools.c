#include <SDL2/SDL.h>
#include "../../include/letter_cutting/cutting_tools.h"

struct head_surface *initialize_head_surface(struct letters_surface *letters) {
    struct head_surface *new = malloc(sizeof(struct head_surface));
    if(!new) {
        return NULL;
    }
    new->head = letters;
    new->next = NULL;
    return new;
}

void insert_head_surface(struct head_surface **head, struct head_surface *new){
    if(!new) {
        return;
    }
    if(*head == NULL) {
        *head = new;
        return;
    }
    struct head_surface *temp = *head;
    while(temp->next) {
        temp = temp->next;
    }
    temp->next = new;
}

struct letters_surface *initialize_letters_surface(SDL_Surface *surface) {
    struct letters_surface *new = malloc(sizeof(struct letters_surface));
    if(!new) {
        return NULL;
    }
    new->surface = surface;
    new->next = NULL;
    return new;
}

void insert_letters_surface(struct letters_surface **head, 
        struct letters_surface *new) {
    if(!new) {
        return;
    }
    if(*head == NULL) {
        *head = new;
        return;
    }
    struct letters_surface *temp = *head;
    while(temp->next) {
        temp = temp->next;
    }
    temp->next = new;
}

void free_letters_surface(struct letters_surface *head) {
    while(head)
    {
        struct letters_surface *temp = head;
        head = head->next;
        SDL_FreeSurface(temp->surface);
        free(temp);
    }
}

void free_head_surface(struct head_surface *head) {
    while(head)
    {
        struct head_surface *temp = head;
        head = head->next;
        free_letters_surface(temp->head);
        free(temp);
    }
}