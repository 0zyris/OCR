#include <gtk/gtk.h>

#include "app.h"
#include "app_data.h"
#include "layout.h"
#include "../image/image.h"

static void on_window_destroy(GtkWidget *widget, gpointer user_data)
{
    (void)widget;
    AppData *data = user_data;
    image_free(data->surface);
    image_free(data->original_surface);
    if (data->display_pixbuf != NULL)
        g_object_unref(data->display_pixbuf);
    g_free(data);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;
    AppData *data = g_new0(AppData, 1);
    layout_build(app, data);
    g_signal_connect(data->window, "destroy",
                     G_CALLBACK(on_window_destroy), data);
    gtk_widget_show_all(data->window);
    gtk_stack_set_visible_child_name(GTK_STACK(data->preview_stack), "empty");
}

int app_run(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new(
        "fr.epita.ocr", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
