
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
    GtkWidget *angle_spin;
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



static void on_grayscale(GtkWidget *button, gpointer user_data)
{
    (void)button;

    AppData *data = user_data;

    // Aucune image chargée.
    if (data->surface == NULL)
        return;

    // Modifier les pixels directement.
    grayscale_image(data->surface);

    // Convertir la surface modifiée pour GTK.
    GdkPixbuf *pixbuf = surface_to_pixbuf(data->surface);

    if (pixbuf == NULL)
        return;

    // Actualiser l'image dans la fenêtre.
    gtk_image_set_from_pixbuf(
        GTK_IMAGE(data->image_widget), pixbuf);

    g_object_unref(pixbuf);
}


static void rotate_current_image(AppData *data, int clockwise)
{
    if (data->surface == NULL)
        return;

    SDL_Surface *rotated = rotate_image_90(
        data->surface, clockwise);

    if (rotated == NULL)
    {
        fprintf(stderr, "Image rotation failed: %s\n",
                SDL_GetError());
        return;
    }

    GdkPixbuf *pixbuf = surface_to_pixbuf(rotated);

    if (pixbuf == NULL)
    {
        image_free(rotated);
        return;
    }

    // L'ancienne surface n'est plus nécessaire.
    image_free(data->surface);
    data->surface = rotated;

    // Afficher l'image tournée.
    gtk_image_set_from_pixbuf(
        GTK_IMAGE(data->image_widget), pixbuf);

    g_object_unref(pixbuf);
}

static void on_rotate_left(GtkWidget *button, gpointer user_data)
{
    (void)button;

    AppData *data = user_data;
    rotate_current_image(data, 0);
}

static void on_rotate_right(GtkWidget *button, gpointer user_data)
{
    (void)button;

    AppData *data = user_data;
    rotate_current_image(data, 1);
}



static void on_angle_insert_text(GtkEditable *editable,
                                 const gchar *text,
                                 gint length,
                                 gint *position,
                                 gpointer user_data)
{
    (void)user_data;

    const gchar *current = gtk_entry_get_text(GTK_ENTRY(editable));

    gint start = 0;
    gint end = 0;

    gboolean has_selection = gtk_editable_get_selection_bounds(
        editable, &start, &end);

    if (!has_selection)
        start = end = *position;

    // Construire la valeur résultant de l'insertion.
    GString *result = g_string_new_len(current, start);
    g_string_append_len(result, text, length);
    g_string_append(result, current + end);

    gboolean valid = TRUE;
    int digit_count = 0;

    for (gsize i = 0; i < result->len; i++)
    {
        char ch = result->str[i];

        if (ch >= '0' && ch <= '9')
        {
            digit_count++;

            if (digit_count > 3)
            {
                valid = FALSE;
                break;
            }
        }
        else if (ch == '-' && i == 0)
        {
            // Un seul signe moins, au début.
        }
        else
        {
            valid = FALSE;
            break;
        }
    }

    if (!valid)
        g_signal_stop_emission_by_name(
            editable, "insert-text");

    g_string_free(result, TRUE);
}


static void on_rotate_custom(GtkWidget *button, gpointer user_data)
{
    (void)button;

    AppData *data = user_data;

    if (data->surface == NULL)
        return;

    // Valider la saisie du GtkSpinButton.
    gtk_spin_button_update(GTK_SPIN_BUTTON(data->angle_spin));

    int angle = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(data->angle_spin));

    if (angle == 0)
        return;

    SDL_Surface *rotated = rotate_image(data->surface, angle);

    if (rotated == NULL)
    {
        fprintf(stderr, "Rotation failed: %s\n", SDL_GetError());
        return;
    }

    GdkPixbuf *pixbuf = surface_to_pixbuf(rotated);

    if (pixbuf == NULL)
    {
        image_free(rotated);
        return;
    }

    image_free(data->surface);
    data->surface = rotated;

    gtk_image_set_from_pixbuf(
        GTK_IMAGE(data->image_widget), pixbuf);

    g_object_unref(pixbuf);
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

    GtkWidget *gray_button = gtk_button_new_with_label(
        "Grayscale");

    gtk_box_pack_start(
        GTK_BOX(box), gray_button, FALSE, FALSE, 0);

    g_signal_connect(
        gray_button, "clicked",
        G_CALLBACK(on_grayscale), data);


    GtkWidget *rotation_box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL, 10);

    gtk_box_pack_start(
        GTK_BOX(box), rotation_box, FALSE, FALSE, 0);

    // Bouton rotation gauche.
    GtkWidget *left_button = gtk_button_new_with_label(
        "Rotate Left 90°");

    gtk_box_pack_start(
        GTK_BOX(rotation_box), left_button, TRUE, TRUE, 0);

    g_signal_connect(
        left_button, "clicked",
        G_CALLBACK(on_rotate_left), data);

    // Bouton rotation droite.
    GtkWidget *right_button = gtk_button_new_with_label(
        "Rotate Right 90°");

    gtk_box_pack_start(
        GTK_BOX(rotation_box), right_button, TRUE, TRUE, 0);

    g_signal_connect(
        right_button, "clicked",
        G_CALLBACK(on_rotate_right), data);


    GtkWidget *custom_rotation_box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL, 10);

    gtk_box_pack_start(
        GTK_BOX(box), custom_rotation_box, FALSE, FALSE, 0);

    // Libellé.
    GtkWidget *angle_label = gtk_label_new("Angle (°):");

    gtk_box_pack_start(
        GTK_BOX(custom_rotation_box),
        angle_label, FALSE, FALSE, 0);

    // Champ numérique avec flèches.
    data->angle_spin = gtk_spin_button_new_with_range(
        -999, 999, 1);

    gtk_spin_button_set_digits(
        GTK_SPIN_BUTTON(data->angle_spin), 0);

    gtk_spin_button_set_numeric(
        GTK_SPIN_BUTTON(data->angle_spin), TRUE);

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(data->angle_spin), 0);

    gtk_entry_set_width_chars(
        GTK_ENTRY(data->angle_spin), 5);

    g_signal_connect(
        data->angle_spin,
        "insert-text",
        G_CALLBACK(on_angle_insert_text),
        NULL);

    gtk_box_pack_start(
        GTK_BOX(custom_rotation_box),
        data->angle_spin, FALSE, FALSE, 0);

    // Bouton pour appliquer la rotation.
    GtkWidget *custom_rotate_button =
        gtk_button_new_with_label("Rotate");

    gtk_box_pack_start(
        GTK_BOX(custom_rotation_box),
        custom_rotate_button, FALSE, FALSE, 0);

    g_signal_connect(
        custom_rotate_button,
        "clicked",
        G_CALLBACK(on_rotate_custom),
        data);



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
