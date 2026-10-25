
#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>

#include "../image/image.h"
#include "app.h"

typedef struct
{
    GtkWidget *window;
    GtkWidget *image_widget;
    SDL_Surface *surface;
} AppData;

static GdkPixbuf *surface_to_pixbuf(SDL_Surface *surface)
{
    if (surface == NULL || surface->format->BytesPerPixel != 4)
        return NULL;

    GdkPixbuf *pixbuf = gdk_pixbuf_new(
        GDK_COLORSPACE_RGB, FALSE, 8,
        surface->w, surface->h);

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
        const Uint8 *src_row =
            (const Uint8 *)surface->pixels + y * surface->pitch;

        guchar *dst_row = data + (size_t)y * (size_t)stride;

        for (int x = 0; x < surface->w; x++)
        {
            Uint32 pixel;
            Uint8 r, g, b;

            const Uint32 *src_pixel = (const Uint32 *)src_row;
            pixel = src_pixel[x];

            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            dst_row[x * 3] = r;
            dst_row[x * 3 + 1] = g;
            dst_row[x * 3 + 2] = b;
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    return pixbuf;
}

static void on_load_image(GtkWidget *button, gpointer user_data)
{
    (void)button;

    AppData *data = user_data;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Load image",
        GTK_WINDOW(data->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.bmp");

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) ==
        GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(dialog));

        SDL_Surface *new_surface = image_load(filename);
        if (new_surface != NULL)
        {
            printf("Width: %d\n", new_surface->w);
            printf("Height: %d\n", new_surface->h);
            printf("Bytes per pixel: %d\n",
                new_surface->format->BytesPerPixel);
            printf("Pitch: %d\n", new_surface->pitch);
        }

        if (new_surface != NULL)
        {
            GdkPixbuf *pixbuf = surface_to_pixbuf(new_surface);

            if (pixbuf != NULL)
            {
                image_free(data->surface);
                data->surface = new_surface;

                gtk_image_set_from_pixbuf(
                    GTK_IMAGE(data->image_widget), pixbuf);

                g_object_unref(pixbuf);
            }
            else
            {
                image_free(new_surface);
                fprintf(stderr, "Cannot display image\n");
            }
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void on_window_destroy(GtkWidget *widget,
                              gpointer user_data)
{
    (void)widget;

    AppData *data = user_data;

    image_free(data->surface);
    g_free(data);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    AppData *data = g_new0(AppData, 1);

    data->window = gtk_application_window_new(app);

    gtk_window_set_title(
        GTK_WINDOW(data->window),
        "OCR Word Search Solver");

    gtk_window_set_default_size(
        GTK_WINDOW(data->window), 900, 700);

    GtkWidget *box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 10);

    gtk_container_add(
        GTK_CONTAINER(data->window), box);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);

    gtk_box_pack_start(
        GTK_BOX(box), scroll, TRUE, TRUE, 0);

    data->image_widget = gtk_image_new();

    gtk_container_add(
        GTK_CONTAINER(scroll), data->image_widget);

    GtkWidget *button = gtk_button_new_with_label(
        "Load Image");

    gtk_box_pack_start(
        GTK_BOX(box), button, FALSE, FALSE, 0);

    g_signal_connect(
        button, "clicked",
        G_CALLBACK(on_load_image), data);

    g_signal_connect(
        data->window, "destroy",
        G_CALLBACK(on_window_destroy), data);

    gtk_widget_show_all(data->window);
}

int app_run(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new(
        "fr.epita.ocr",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(
        app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(
        G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}
