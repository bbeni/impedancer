// Copyright (C) 2026 Benjamin Froelich
// This file is part of https://github.com/bbeni/impedancer
// For conditions of distribution and use, see copyright notice in project root.
//
// Smithchart Graphing Functions
//

#include "gra.h"
#include "string.h"
#include "assert.h"

Mui_Color _color_bg_smith() {return mui_protos_theme_g.bg_light;}
Mui_Color _color_border_smith() {return mui_protos_theme_g.border;}
Mui_Color _color_text_smith() {return mui_protos_theme_g.text_muted;}

#define N_DEFAULT_REACTANCES 6
static double const_reactances[N_DEFAULT_REACTANCES] = { 0.1, 0.2, 0.5, 1, 2, 5};

void draw_smith_grid(bool plot_reactance_circles, bool plot_admittance_circles, double *custom_cicles, size_t n_custom_circles, Mui_Rectangle plot_area)
{
    Mui_Vector2 center = mui_center_of_rectangle(plot_area);
    float r_outer = fmin(plot_area.height, plot_area.width) * 0.49f;

    mui_draw_rectangle_lines(mui_shrink(plot_area, 1.0f), _color_border_smith(), 2.0f);

    mui_draw_circle(center, r_outer, _color_bg_smith());
    mui_draw_circle_lines(center, r_outer, _color_text_smith(), 2.0f);
    mui_draw_line(center.x - r_outer, center.y, center.x + r_outer, center.y, 2.0f, _color_text_smith());

    if (!custom_cicles) {
        n_custom_circles = N_DEFAULT_REACTANCES;
    }

    if (plot_reactance_circles) {

        for (size_t i = 0; i < n_custom_circles; i++) {
            float x = const_reactances[i];
            float left_crossing = (x - 1) / (x + 1);
            float c1_x = (1 - left_crossing) * 0.5f;
            float r1_plot = (1 - c1_x) * r_outer;
            Mui_Vector2 c1_plot;
            c1_plot.x = center.x + c1_x * r_outer;
            c1_plot.y = center.y + 0;
            mui_draw_arc_lines(c1_plot, r1_plot, 0, 360,  _color_text_smith(), 1.0f);
        }
    }

    if (plot_admittance_circles) {
        for (size_t i = 0; i < n_custom_circles; i++) {
            float x = const_reactances[i];
            float left_crossing = (x - 1) / (x + 1);
            float c1_x = (1 - left_crossing) * 0.5f;
            float r1_plot = (1-c1_x) * r_outer;
            Mui_Vector2 c1_plot;
            c1_plot.x = center.x - c1_x * r_outer,
            c1_plot.y = center.y + 0;
            mui_draw_arc_lines(c1_plot, r1_plot, 0, 360,  _color_text_smith(), 1.0f);
        }
    }

    for (size_t i = 0; i < n_custom_circles; i++) {
        float x = const_reactances[i];
        float r1 = 1 / x;
        Mui_Vector2 c1_plot;
        c1_plot.x = center.x + r_outer,
        c1_plot.y =  center.y + r1 * r_outer;
        // r 0 -> inf, angle 0 -> 180
        float angle = 180 - atan(1 / r1) * 2 * 180 / M_PI;
        mui_draw_arc_lines(c1_plot, r1 * r_outer, 90 + angle, 270,  _color_text_smith(), 1.0f);
        c1_plot.y = center.y - r1 * r_outer;
        mui_draw_arc_lines(c1_plot, r1 * r_outer, 90, 270 - angle,  _color_text_smith(), 1.0f);
    }

}

// fmt_marker options: 'o', '-'
void gra_smith_plot_data(double *f_data, struct Complex *z_data, size_t data_length,
                 double f_min, double f_max, Mui_Color color,
                 char fmt_marker, double marker_size, Mui_Rectangle plot_area)
{
    float r_outer = fmin(plot_area.height, plot_area.width) * 0.49f;
    Mui_Vector2 smith_center = mui_center_of_rectangle(plot_area);
    if (fmt_marker == 'o') {
        for (size_t i = 0; i < data_length; i++) {
            double f = f_data[i];

            if (f >= f_min && f <= f_max){
                double q = (z_data[i].r+1)*(z_data[i].r+1) + z_data[i].i*z_data[i].i;
                double x = ((z_data[i].r+1)*(z_data[i].r-1) + z_data[i].i*z_data[i].i) / q;
                double y = 2*z_data[i].i/q;

                Mui_Vector2 screen_coords;
                screen_coords.x = smith_center.x + x * r_outer;
                screen_coords.y = smith_center.y - y * r_outer;
                mui_draw_circle(screen_coords, marker_size, color);
            }
        }
    } else if(fmt_marker == '-') {
        double x_last, y_last;
        for (size_t i = 0; i < data_length; i++) {
            double f = f_data[i];
            double q = (z_data[i].r + 1) * (z_data[i].r + 1) + z_data[i].i * z_data[i].i;
            double x = ((z_data[i].r + 1) * (z_data[i].r - 1) + z_data[i].i * z_data[i].i) / q;
            double y = 2 * z_data[i].i / q;
            // skip the first round to get x_last, y_last
            if (i > 0 && f >= f_min && f <= f_max){
                Mui_Vector2 screen_coords_1 = {
                    smith_center.x + x_last * r_outer,
                    smith_center.y - y_last * r_outer
                };
                Mui_Vector2 screen_coords_2 = {
                    smith_center.x + x * r_outer,
                    smith_center.y - y * r_outer
                };

                mui_draw_line(screen_coords_1.x, screen_coords_1.y, screen_coords_2.x, screen_coords_2.y, marker_size, color);
            }
            x_last = x;
            y_last = y;
        }
    }
}
