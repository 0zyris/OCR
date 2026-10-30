#include <stddef.h>
#include "app/gui/image_view.h"

static GdkPixbuf *surface_to_pixbuf(SDL_Surface *surface)
{
    if (surface == NULL ||
        surface->format->format != SDL_PIXELFORMAT_RGB888)
        return NULL;

    GdkPixbuf *pixbuf = gdk_pixbuf_new(
        GDK_COLORSPACE_RGB, FALSE, 8, surface->w, surface->h);
    if (pixbuf == NULL)
        return NULL;

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0)
    {
        g_object_unref(pixbuf);
        return NULL;
    }

    guchar *data = gdk_pixbuf_get_pixels(pixbuf);
    int stride = gdk_pixbuf_get_rowstride(pixbuf);

    for (int y = 0; y < surface->h; y++)
    {
        const Uint32 *src_row = (const Uint32 *)(
            (const Uint8 *)surface->pixels + y * surface->pitch);
        guchar *dst_row = data + (size_t)y * (size_t)stride;
        for (int x = 0; x < surface->w; x++)
        {
            Uint8 r, g, b;
            SDL_GetRGB(src_row[x], surface->format, &r, &g, &b);
            dst_row[x * 3] = r;
            dst_row[x * 3 + 1] = g;
            dst_row[x * 3 + 2] = b;
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
    return pixbuf;
}

gboolean image_view_set_surface(AppData *data, SDL_Surface *surface)
{
    GdkPixbuf *new_pixbuf = surface_to_pixbuf(surface);
    if (new_pixbuf == NULL)
        return FALSE;

    if (data->display_pixbuf != NULL)
        g_object_unref(data->display_pixbuf);
    data->display_pixbuf = new_pixbuf;
    gtk_stack_set_visible_child_name(
        GTK_STACK(data->preview_stack), "preview");
    gtk_widget_queue_draw(data->drawing_area);
    return TRUE;
}

gboolean image_view_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    AppData *data = user_data;
    if (data->display_pixbuf == NULL)
        return FALSE;

    int available_w = gtk_widget_get_allocated_width(widget);
    int available_h = gtk_widget_get_allocated_height(widget);
    int image_w = gdk_pixbuf_get_width(data->display_pixbuf);
    int image_h = gdk_pixbuf_get_height(data->display_pixbuf);
    if (available_w <= 0 || available_h <= 0 || image_w <= 0 || image_h <= 0)
        return FALSE;

    const double margin = 24.0;
    double scale_x = (available_w - margin * 2) / image_w;
    double scale_y = (available_h - margin * 2) / image_h;
    double scale = MIN(1.0, MIN(scale_x, scale_y));
    if (scale <= 0)
        return FALSE;

    double shown_w = image_w * scale;
    double shown_h = image_h * scale;
    double left = (available_w - shown_w) / 2.0;
    double top = (available_h - shown_h) / 2.0;

    cairo_save(cr);
    cairo_translate(cr, left, top);
    cairo_scale(cr, scale, scale);
    gdk_cairo_set_source_pixbuf(cr, data->display_pixbuf, 0, 0);
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_BILINEAR);
    cairo_paint(cr);
    cairo_restore(cr);
    return FALSE;
}
