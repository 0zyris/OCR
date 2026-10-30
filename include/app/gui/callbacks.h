#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "app_data.h"

void on_load_image(GtkWidget *button, gpointer user_data);
void on_rotate_left(GtkWidget *button, gpointer user_data);
void on_rotate_right(GtkWidget *button, gpointer user_data);
void on_rotate_custom(GtkWidget *button, gpointer user_data);
void on_angle_insert_text(GtkEditable *editable, const gchar *text,
                          gint length, gint *position, gpointer user_data);

#endif