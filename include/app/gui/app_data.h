#ifndef APP_DATA_H
#define APP_DATA_H

#include <gtk/gtk.h>
#include <SDL2/SDL.h>

typedef struct
{
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *preview_stack;
    GtkWidget *angle_spin;
    GtkWidget *status_label;

    SDL_Surface *original_surface;
    SDL_Surface *surface;
    GdkPixbuf *display_pixbuf;
} AppData;

#endif
