//
// Graphing Functions
//

#include "mui.h"


float _next_multiple_of(float start, float step) {
    float x = ceilf(start / step) * step;
    if (x == start) {
        x += step;
    }
    return x;
}

void gra_xy_plot(double *x_data,
                 void *y_data,
                 double (* y_map)(size_t i, void *x),
                 size_t data_length,
                 char* x_label,
                 char* y_label,
                 double x_min, double x_max,
                 double y_min, double y_max,
                 double x_step, double y_step,
                 Mui_Color color,
                 Mui_Rectangle place)
{
    float label_text_size = mui_protos_theme.label_text_size;

    // make space for label
    Mui_Rectangle x_label_place;
    Mui_Rectangle rest = mui_cut_bot(place, label_text_size, &x_label_place);
    Mui_Rectangle y_label_place;
    rest = mui_cut_left(rest, label_text_size, &y_label_place);

    Mui_Vector2 x_label_pos = {x_label_place.x + x_label_place.width/2, x_label_place.y};
    mui_draw_text_line(mui_protos_theme.label_font, x_label_pos, 0.1f, label_text_size, x_label, color, strlen(x_label));

    Mui_Rectangle plot_area = rest;

    // draw border
    mui_draw_rectangle_lines(plot_area, MUI_BLACK, 2.0f);

    // draw grid
    float grid_x = _next_multiple_of(x_min, x_step);
    float grid_norm_x = (grid_x - x_min)/ (x_max - x_min);
    Mui_Vector2 upper = {plot_area.x + grid_norm_x * plot_area.width, plot_area.y};
    Mui_Vector2 lower = {plot_area.x + grid_norm_x * plot_area.width, plot_area.y + plot_area.height};
    int iter = 0;
#define MAX_LINES 1000
    while (grid_x < x_max && iter < MAX_LINES) {
        mui_draw_line(upper, lower, 1.0f, MUI_BLACK);
        grid_x = _next_multiple_of(grid_x, x_step);
        grid_norm_x = (grid_x - x_min) / (x_max - x_min);
        upper.x = plot_area.x + grid_norm_x * plot_area.width;
        lower.x = plot_area.x + grid_norm_x * plot_area.width;
        iter++;
    }
    float grid_y = _next_multiple_of(y_min, y_step);
    float grid_norm_y = 1 - (grid_y - y_min)/ (y_max - y_min);
    Mui_Vector2 left = {plot_area.x , plot_area.y + grid_norm_y * plot_area.height};
    Mui_Vector2 right = {plot_area.x + plot_area.width, plot_area.y + grid_norm_y * plot_area.height};
    iter = 0;
    while (grid_y < y_max && iter < MAX_LINES) {
        mui_draw_line(left, right, 1.0f, MUI_BLACK);
        grid_y = _next_multiple_of(grid_y, y_step);
        grid_norm_y = 1 - (grid_y - y_min) / (y_max - y_min);
        left.y = plot_area.y + grid_norm_y * plot_area.height;
        right.y = plot_area.y + grid_norm_y * plot_area.height;
        iter++;
    }

    for (int i = 0; i < data_length; i++) {
        double x = x_data[i];
        double y = y_map(i, y_data);
        if (x >= x_min && x <= x_max && y <= y_max && y >= y_min){
            float norm_x = (x - x_min)/ (x_max - x_min);
            float norm_y = (y - y_min)/ (y_max - y_min);
            Mui_Vector2 screen_coords = {
                norm_x * plot_area.width + plot_area.x,
                (1 - norm_y) * plot_area.height + plot_area.y
            };
            mui_draw_circle(screen_coords, 3.0f, color);
        }
    }
}
