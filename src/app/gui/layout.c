#include <gtk/gtk.h>
#include "../../../include/app/gui/layout.h"
#include "../../../include/app/gui/callbacks.h"
#include "../../../include/app/gui/image_view.h"

static void add_class(GtkWidget *widget, const char *class_name)
{
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, class_name);
}

static GtkWidget *make_label(const char *text, const char *class_name)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    if (class_name != NULL)
        add_class(label, class_name);
    return label;
}

static GtkWidget *make_button(const char *label, const char *class_name,
                              GCallback callback, AppData *data)
{
    GtkWidget *button = gtk_button_new_with_label(label);
    if (class_name != NULL)
        add_class(button, class_name);
    g_signal_connect(button, "clicked", callback, data);
    return button;
}

void layout_load_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    GError *error = NULL;
    // Run ./ocr from the project root.
    if (!gtk_css_provider_load_from_path(
            provider, "src/app/gui/style.css", &error))
    {
        g_warning("Unable to load src/gui/style.css: %s", error->message);
        g_clear_error(&error);
    }
    else
    {
        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(provider);
}

void layout_build(GtkApplication *app, AppData *data)
{
    layout_load_css();
    data->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(data->window), "OCR Word Search Solver");
    gtk_window_set_default_size(GTK_WINDOW(data->window), 1120, 740);
    gtk_widget_set_size_request(data->window, 720, 480);

    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "OCR Word Search Solver");
    gtk_header_bar_set_subtitle(
        GTK_HEADER_BAR(header),
        "Image preparation workspace");
    add_class(header, "app-header");
    gtk_window_set_titlebar(GTK_WINDOW(data->window), header);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    add_class(root, "app-root");
    gtk_container_add(GTK_CONTAINER(data->window), root);

    // Main preview: centered and responsive.
    // The original SDL_Surface dimensions are preserved.
    GtkWidget *main_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(main_area, 24);
    gtk_widget_set_margin_end(main_area, 16);
    gtk_widget_set_margin_top(main_area, 24);
    gtk_widget_set_margin_bottom(main_area, 24);
    gtk_box_pack_start(GTK_BOX(root), main_area, TRUE, TRUE, 0);

    GtkWidget *preview_title = make_label("Image preview", "section-title");
    gtk_box_pack_start(GTK_BOX(main_area), preview_title, FALSE, FALSE, 0);

    GtkWidget *preview_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    add_class(preview_card, "preview-card");
    gtk_box_pack_start(GTK_BOX(main_area), preview_card, TRUE, TRUE, 0);

    data->preview_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(data->preview_stack),
                                  GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_box_pack_start(GTK_BOX(preview_card), data->preview_stack,
                       TRUE, TRUE, 0);

    GtkWidget *empty_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(empty_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(empty_box, GTK_ALIGN_CENTER);
    GtkWidget *empty_title = make_label(
        "Your image will appear here",
        "empty-title");
    gtk_label_set_xalign(GTK_LABEL(empty_title), 0.5);
    GtkWidget *empty_hint = make_label(
        "Choose an image to start preparing your puzzle",
        "muted");
    gtk_label_set_xalign(GTK_LABEL(empty_hint), 0.5);
    gtk_box_pack_start(GTK_BOX(empty_box), empty_title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(empty_box), empty_hint, FALSE, FALSE, 0);
    gtk_stack_add_named(GTK_STACK(data->preview_stack), empty_box, "empty");

    data->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(data->drawing_area, TRUE);
    gtk_widget_set_vexpand(data->drawing_area, TRUE);
    g_signal_connect(data->drawing_area, "draw",
                     G_CALLBACK(image_view_draw), data);
    gtk_stack_add_named(GTK_STACK(data->preview_stack),
                        data->drawing_area, "preview");

    data->status_label = make_label("No image loaded", "muted");
    gtk_box_pack_start(
        GTK_BOX(main_area), data->status_label, FALSE, FALSE, 0);

    // Tool sidebar.
    GtkWidget *side = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_size_request(side, 276, -1);
    gtk_widget_set_margin_start(side, 6);
    gtk_widget_set_margin_end(side, 24);
    gtk_widget_set_margin_top(side, 24);
    gtk_widget_set_margin_bottom(side, 24);
    gtk_box_pack_start(GTK_BOX(root), side, FALSE, FALSE, 0);

    GtkWidget *tools_title = make_label("Tools", "section-title");
    gtk_box_pack_start(GTK_BOX(side), tools_title, FALSE, FALSE, 0);

    GtkWidget *load_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    add_class(load_card, "tool-card");
    gtk_box_pack_start(GTK_BOX(side), load_card, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(load_card),
                       make_label("SOURCE IMAGE", "eyebrow"), FALSE, FALSE, 0);
    GtkWidget *load_button = make_button(
        "+  Load image",
        "primary-button",
        G_CALLBACK(on_load_image),
        data);
    gtk_box_pack_start(GTK_BOX(load_card), load_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(load_card),
                    make_label(
                        "PNG, JPG or BMP · Grayscale is automatic",
                        "small-hint"),
                    FALSE, FALSE, 0);

    GtkWidget *rotate_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    add_class(rotate_card, "tool-card");
    gtk_box_pack_start(GTK_BOX(side), rotate_card, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rotate_card),
                       make_label("ROTATION", "eyebrow"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rotate_card),
                       make_label("Quick rotation", "field-label"),
                       FALSE, FALSE, 0);

    GtkWidget *quick_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(rotate_card), quick_row, FALSE, FALSE, 0);
    GtkWidget *left = make_button("↶  90° left", "secondary-button",
                                  G_CALLBACK(on_rotate_left), data);
    GtkWidget *right = make_button("90° right  ↷", "secondary-button",
                                   G_CALLBACK(on_rotate_right), data);
    gtk_box_pack_start(GTK_BOX(quick_row), left, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(quick_row), right, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(rotate_card),
                       make_label("Custom angle (degrees)", "field-label"),
                       FALSE, FALSE, 0);
    GtkWidget *angle_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(rotate_card), angle_row, FALSE, FALSE, 0);
    data->angle_spin = gtk_spin_button_new_with_range(-999, 999, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(data->angle_spin), 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(data->angle_spin), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->angle_spin), 0);
    gtk_entry_set_width_chars(GTK_ENTRY(data->angle_spin), 5);
    add_class(data->angle_spin, "angle-input");
    g_signal_connect(data->angle_spin, "insert-text",
                     G_CALLBACK(on_angle_insert_text), NULL);
    gtk_box_pack_start(GTK_BOX(angle_row), data->angle_spin, TRUE, TRUE, 0);

    GtkWidget *apply = make_button("Apply rotation", "secondary-button",
                                   G_CALLBACK(on_rotate_custom), data);
    gtk_box_pack_start(GTK_BOX(rotate_card), apply, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rotate_card),
                       make_label(
                           "0° restores the loaded image", "small-hint"),
                       FALSE, FALSE, 0);

    GtkWidget *tip = make_label(
        "Tip: adjust the angle, then apply.\nThe original image is preserved.",
        "sidebar-tip");
    gtk_label_set_line_wrap(GTK_LABEL(tip), TRUE);
    gtk_box_pack_end(GTK_BOX(side), tip, FALSE, FALSE, 0);
}