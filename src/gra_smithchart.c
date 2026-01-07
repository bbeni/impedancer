//
// Smithchart Graphing Functions
//

#include "gra.h"
#include "string.h"



Mui_Color _color_bg_smith() {return mui_protos_theme.background_color;}
Mui_Color _color_border_smith() {return mui_protos_theme.border_color;}
Mui_Color _color_text_smith() {return mui_protos_theme.text_color;}


#define N_DEFAULT_REACTANCES 11
static double const_reactances[N_DEFAULT_REACTANCES] = {0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50};

void draw_smith_grid(Mui_Rectangle plot_area, bool plot_reactance_circles, bool plot_admittance_circles, double *custom_cicles, size_t n_custom_circles) {

    Mui_Vector2 center = mui_center_of_rectangle(plot_area);
    float r_outer = fmin(plot_area.height, plot_area.width) * 0.49f;

    mui_draw_rectangle_lines(plot_area, _color_border_smith(), 2.0f);
    mui_draw_circle_lines(center, r_outer, _color_bg_smith(), 2.0f);
    mui_draw_line(center.x - r_outer, center.y, center.x + r_outer, center.y, 2.0f, _color_bg_smith());

    if (!custom_cicles) {
        n_custom_circles = N_DEFAULT_REACTANCES;
    }

    if (plot_reactance_circles) {

        for (size_t i = 0; i < n_custom_circles; i++) {
            float x = const_reactances[i];
            float left_crossing = (x - 1) / (x + 1);
            float c1_x = (1 - left_crossing) * 0.5f;
            float r1_plot = (1-c1_x) * r_outer; 
            Mui_Vector2 c1_plot = {center.x + c1_x*r_outer, center.y + 0};
            mui_draw_arc_lines(c1_plot, r1_plot, 0, 360,  _color_bg_smith(), 1.0f);
        }
    }

    if (plot_admittance_circles) {
        for (size_t i = 0; i < n_custom_circles; i++) {
            float x = const_reactances[i];
            float left_crossing = (x - 1) / (x + 1);
            float c1_x = (1 - left_crossing) * 0.5f;
            float r1_plot = (1-c1_x) * r_outer; 
            Mui_Vector2 c1_plot = {center.x - c1_x*r_outer, center.y + 0};
            mui_draw_arc_lines(c1_plot, r1_plot, 0, 360,  _color_bg_smith(), 1.0f);
        }
    }

    for (size_t i = 0; i < n_custom_circles; i++) {
        float x = const_reactances[i];
        float r1 = 1/x;
        Mui_Vector2 c1_plot = {center.x + r_outer, center.y + r1 * r_outer};
        // r 0 -> inf, angle 0 -> 180
        float angle = 180 - atan(1/r1) * 2 * 180 / M_PI;
        mui_draw_arc_lines(c1_plot, r1*r_outer, 90+angle, 270,  MUI_RED, 1.0f);
        c1_plot.y = center.y - r1 * r_outer;
        mui_draw_arc_lines(c1_plot, r1*r_outer, 90, 270-angle,  MUI_ORANGE, 1.0f);
    }

}


/*
// draw the grid and labels. Returns plot_area rectangle.
Mui_Rectangle gra_smith_plot_labels_and_grid(char* x_label, char* y_label, double x_min, double x_max, double y_min, double y_max, double x_step, double y_step, Mui_Rectangle place) {
    
    (void) y_label;

    float label_text_size = mui_protos_theme.label_text_size;

    // make space for label
    Mui_Rectangle x_label_place;
    Mui_Rectangle rest = mui_cut_bot(place, label_text_size, &x_label_place);
    Mui_Rectangle y_label_place;
    rest = mui_cut_left(rest, label_text_size, &y_label_place);

    size_t l = strlen(x_label);
    Mui_Vector2 m = mui_measure_text(mui_protos_theme.label_font, x_label, label_text_size, 0.1f, l);
    Mui_Vector2 x_label_pos = {x_label_place.x + x_label_place.width/2 - m.x/2, x_label_place.y};
    mui_draw_text_line(mui_protos_theme.label_font, x_label_pos, 0.1f, label_text_size, x_label, _color_text(), l);

    Mui_Rectangle plot_area = rest;
    mui_draw_rectangle(plot_area, _color_bg());
    _draw_grid(plot_area, x_min, x_max, y_min, y_max, x_step, y_step);

    return plot_area;
} */

/*
void gra_smith_legend(char **labels, Mui_Color *colors, bool *mask, size_t n_labels_, Mui_Rectangle plot_area) {
    size_t n_labels = 0;
    for (size_t n = 0; n < n_labels_; n++) {
        if (mask[n]) n_labels++;
    }
    if (n_labels == 0) return;
    float legends_v_spacing = 0.1f;
    float legends_size = mui_protos_theme.label_text_size;
    float legends_padding = 10.0f;
    float legend_spacing = 10.0f;
    Mui_Rectangle  legends_rect;
    mui_cut_top(plot_area, n_labels * legends_size + (n_labels-1)*legends_v_spacing + 2*legends_padding + 2*legend_spacing, &legends_rect);
    legends_rect = mui_cut_left(legends_rect, legends_rect.width - 200, NULL);
    legends_rect = mui_shrink(legends_rect, legend_spacing);
    mui_draw_rectangle_rounded(legends_rect, 10.0f, _color_bg());
    mui_draw_rectangle_rounded_lines(legends_rect, 10.0f, _color_border(), 1.0f);
    legends_rect = mui_shrink(legends_rect, legends_padding);

    size_t index = 0;
    for (size_t i = 0; i < n_labels_; i++) {
        if (!mask[i]) continue;
        Mui_Vector2 pos = {legends_rect.x, legends_rect.y + index * legends_size + index * legends_v_spacing };
        mui_draw_text_line(mui_protos_theme.label_font, pos, 0.1f, legends_size, labels[i], colors[i], strlen(labels[i]));
        index++;
    }
}
*/


void gra_smith_plot_data(double *f_data, struct Complex *z_data, size_t data_length,
                 double f_min, double f_max, Mui_Color color, Mui_Vector2 smith_center, float smith_radius)
{
    for (size_t i = 0; i < data_length; i++) {
        double f = f_data[i];
        double q = (z_data[i].r+1)*(z_data[i].r+1) + z_data[i].i*z_data[i].i;
        double x = ((z_data[i].r+1)*(z_data[i].r-1) + z_data[i].i*z_data[i].i) / q;
        double y = 2*z_data[i].i/q;
         
        if (f >= f_min && f <= f_max){
            Mui_Vector2 screen_coords = {
                smith_center.x + x * smith_radius,
                smith_center.y + y * smith_radius
            };
            mui_draw_circle(screen_coords, 3.0f, color);
        }
    }
}
