#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H

#include "app_data.h"

// Update the GTK preview without changing the SDL_Surface resolution.
// Return FALSE on failure; the previous preview is kept.
gboolean image_view_set_surface(AppData *data, SDL_Surface *surface);
gboolean image_view_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

#endif