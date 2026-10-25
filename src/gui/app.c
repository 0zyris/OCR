
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>

#include "../image/image.h"
#include "app.h"

typedef struct
{
    GtkWidget *window;
    GtkWidget *image_widget;
    Image *image;
} AppData;

static GdkPixbuf *image_to_pixbuf(const Image *image)
{
    if (image == NULL)
        return NULL;

    if (image->width > G_MAXINT / 3 ||
        image->height > G_MAXINT)
        return NULL;

    GdkPixbuf *pixbuf = gdk_pixbuf_new(
        GDK_COLORSPACE_RGB,
        FALSE,
        8,
        (int)image->width,
        (int)image->height);

    if (pixbuf == NULL)
        return NULL;

    guchar *data = gdk_pixbuf_get_pixels(pixbuf);
    int stride = gdk_pixbuf_get_rowstride(pixbuf);

    for (size_t y = 0; y < image->height; y++)
    {
        guchar *row = data + y * (size_t)stride;

        for (size_t x = 0; x < image->width; x++)
        {
            Pixel pixel =
                image->pixels[y * image->width + x];

            row[x * 3] = pixel.r;
            row[x * 3 + 1] = pixel.g;
            row[x * 3 + 2] = pixel.b;
        }
    }

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

        Image *new_image = image_load(filename);

        if (new_image != NULL)
        {
            GdkPixbuf *pixbuf = image_to_pixbuf(new_image);

            if (pixbuf != NULL)
            {
                image_free(data->image);
                data->image = new_image;

                gtk_image_set_from_pixbuf(
                    GTK_IMAGE(data->image_widget),
                    pixbuf);

                g_object_unref(pixbuf);
            }
            else
            {
                image_free(new_image);
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

    image_free(data->image);
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
