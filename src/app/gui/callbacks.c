#include <stdio.h>
#include <gtk/gtk.h>

#include "app/gui/callbacks.h"
#include "app/gui/image_view.h"
#include "app/image/image.h"

static void show_status(AppData *data, const char *message)
{
    gtk_label_set_text(GTK_LABEL(data->status_label), message);
}

void on_load_image(GtkWidget *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Open an image", GTK_WINDOW(data->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images (PNG, JPEG, BMP)");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(dialog));
        SDL_Surface *original = image_load(filename);
        if (original == NULL)
        {
            show_status(data, "Unable to load this image.");
        }
        else
        {
            // The original kept for rotations is already grayscale.
            grayscale_image(original);
            SDL_Surface *shown = SDL_ConvertSurfaceFormat(
                original, SDL_PIXELFORMAT_RGB888, 0);
            if (shown != NULL && image_view_set_surface(data, shown))
            {
                image_free(data->surface);
                image_free(data->original_surface);
                data->original_surface = original;
                data->surface = shown;
                gtk_spin_button_set_value(
                    GTK_SPIN_BUTTON(data->angle_spin), 0);
                char *status = g_strdup_printf("Image loaded · %d × %d px",
                                               shown->w, shown->h);
                show_status(data, status);
                g_free(status);
            }
            else
            {
                image_free(shown);
                image_free(original);
                show_status(data, "Unable to display this image.");
            }
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void apply_rotated_surface(AppData *data, SDL_Surface *rotated)
{
    if (rotated == NULL)
    {
        show_status(data, "Rotation failed.");
        fprintf(stderr, "Rotation failed: %s\n", SDL_GetError());
        return;
    }
    if (!image_view_set_surface(data, rotated))
    {
        image_free(rotated);
        show_status(data, "Unable to display the rotated image.");
        return;
    }
    image_free(data->surface);
    data->surface = rotated;
    char *status = g_strdup_printf("Preview · %d × %d px",
                                   rotated->w, rotated->h);
    show_status(data, status);
    g_free(status);
}

void on_rotate_left(GtkWidget *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;
    if (data->surface == NULL)
        return;
    apply_rotated_surface(data, rotate_image_90(data->surface, 0));
}

void on_rotate_right(GtkWidget *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;
    if (data->surface == NULL)
        return;
    apply_rotated_surface(data, rotate_image_90(data->surface, 1));
}

void on_rotate_custom(GtkWidget *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;
    if (data->original_surface == NULL)
        return;

    gtk_spin_button_update(GTK_SPIN_BUTTON(data->angle_spin));
    int angle = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(data->angle_spin));
    // Always rotate from the original; 0 degrees resets the image.
    apply_rotated_surface(data, rotate_image(data->original_surface, angle));
}

void on_angle_insert_text(GtkEditable *editable, const gchar *text,
                          gint length, gint *position, gpointer user_data)
{
    (void)user_data;
    const gchar *current = gtk_entry_get_text(GTK_ENTRY(editable));
    gint start = 0;
    gint end = 0;
    if (!gtk_editable_get_selection_bounds(editable, &start, &end))
        start = end = *position;

    // Input is restricted to ASCII, so character indices equal byte indices.
    GString *result = g_string_new_len(current, start);
    g_string_append_len(result, text, length);
    g_string_append(result, current + end);

    int digits = 0;
    gboolean valid = TRUE;
    for (gsize i = 0; i < result->len; i++)
    {
        char ch = result->str[i];
        if (ch >= '0' && ch <= '9')
        {
            if (++digits > 3)
                valid = FALSE;
        }
        else if (ch != '-' || i != 0)
        {
            valid = FALSE;
        }
        if (!valid)
            break;
    }
    if (!valid)
        g_signal_stop_emission_by_name(editable, "insert-text");
    g_string_free(result, TRUE);
}
