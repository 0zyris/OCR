#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H

#include "app_data.h"

// Met à jour l'aperçu GTK sans changer la résolution de la SDL_Surface.
// Renvoie FALSE en cas d'échec ; l'ancien aperçu est alors conservé.
gboolean image_view_set_surface(AppData *data, SDL_Surface *surface);
gboolean image_view_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

#endif
