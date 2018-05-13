#include <gtk/gtk.h>
#include <vector>
#include <math.h>

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_shader_parameters.h"
#include "shaders/glsl.h"

static inline double snap_to_interval (double value, double interval)
{
    return round (value / interval) * interval;
}

void value_changed (GtkRange *range, gpointer user_data)
{
    GtkAdjustment *adj = gtk_range_get_adjustment (range);
    double interval = gtk_adjustment_get_step_increment (adj);
    double value = gtk_range_get_value (range);

    value = snap_to_interval (value, interval);

    gtk_range_set_value (range, value);
}


bool gtk_shader_parameters_dialog (GtkWindow *parent)
{
    GtkWidget *dialog;
    std::vector<GLSLParam> *params = (std::vector<GLSLParam> *) S9xDisplayGetDriver()->get_parameters();

    if (!params || params->size() == 0)
        return false;

    dialog = gtk_dialog_new_with_buttons (_("GLSL Shader Parameters"),
                                          parent,
                                          GTK_DIALOG_MODAL,
                                          "gtk-cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "gtk-apply",
                                          GTK_RESPONSE_APPLY,
                                          NULL);

    GtkWidget *scrolled_window;

    gtk_widget_set_size_request(dialog, 640, 480);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_hexpand (scrolled_window, TRUE);
    gtk_widget_set_vexpand (scrolled_window, TRUE);
    gtk_widget_set_margin_start (scrolled_window, 5);
    gtk_widget_set_margin_end (scrolled_window, 5);
    gtk_widget_set_margin_top (scrolled_window, 5);
    gtk_widget_set_margin_bottom (scrolled_window, 5);
    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG(dialog))),
                       scrolled_window);
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *grid = gtk_grid_new ();
    gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
    gtk_container_add (GTK_CONTAINER (vbox), grid);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);

    GtkWidget **value_holders = new GtkWidget*[params->size()];

    for (unsigned int i = 0; i < params->size(); i++)
    {
        GLSLParam *p = &(*params)[i];
        GtkWidget *label = gtk_label_new (p->name);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
        gtk_widget_show (label);

        gtk_grid_attach (GTK_GRID (grid), label, 0, i, 1, 1);

        if (p->min == 0.0 && p->max == 1.0 && p->step == 1.0)
        {
            GtkWidget *check = gtk_check_button_new ();
            gtk_grid_attach (GTK_GRID (grid), check, 1, i, 1, 1);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), (int) p->val);
            value_holders[i] = check;
        }
        else
        {
            GtkWidget *scale = gtk_hscale_new_with_range (p->min, p->max, p->step);
            gtk_widget_set_hexpand (scale, TRUE);
            gtk_grid_attach (GTK_GRID (grid), scale, 1, i, 1, 1);
            gtk_scale_set_value_pos (GTK_SCALE (scale), GTK_POS_RIGHT);
            gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
            gtk_scale_set_digits (GTK_SCALE (scale), 3);
            gtk_range_set_value (GTK_RANGE (scale), p->val);
            g_signal_connect_data (G_OBJECT (scale),
                                   "value-changed",
                                   G_CALLBACK (value_changed),
                                   NULL,
                                   NULL,
                                   (GConnectFlags) 0);

            value_holders[i] = scale;
        }
    }

    gtk_widget_show_all (dialog);
    int response = gtk_dialog_run (GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_APPLY)
    {
        for (unsigned int i = 0; i < params->size(); i++)
        {
            GLSLParam *p = &(*params)[i];
            if (p->min == 0.0 && p->max == 1.0 && p->step == 1.0)
            {
                int val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (value_holders[i]));
                p->val = (float) val;
            }
            else
            {
                p->val = gtk_range_get_value (GTK_RANGE (value_holders[i]));
            }

            p->val = snap_to_interval (p->val, p->step);

            p->val = CLAMP (p->val, p->min, p->max);
        }

        S9xDeinitUpdate (top_level->last_width, top_level->last_height);
    }

    delete[] value_holders;
    gtk_widget_destroy (dialog);

    return true;
}