//
// Graphing Functions
//

#include "mui.h"

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
    rest = mui_cut_left(place, label_text_size, &y_label_place);

    Mui_Vector2 x_label_pos = {x_label_place.x, x_label_place.y};
    mui_draw_text_line(mui_protos_theme.label_font, x_label_pos, 0.1f, label_text_size, x_label, color, strlen(x_label));

    Mui_Rectangle plot_area = rest;
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
            mui_draw_circle(screen_coords, 3.0f, MUI_RED);
        }
    }

    /*
    for (size_t i=0; i < info.freq.count; i++) {
        Complex s = arr.items[i];
        double mag_s = s.r*s.r + s.i * s.i;
        if (mag_s < mag_min) mag_min = mag_s;
        if (mag_s > mag_max) mag_max = mag_s;
    }

    for (size_t i=0; i < info.freq.count; i++) {
        Complex s = arr.items[i];
        double mag_s = s.r*s.r + s.i * s.i;
        double f = info.freq.items[i];
        int x = (f-min_f)/(max_f - min_f) * width + posx;
        int y = height - (mag_s-mag_min)/(mag_max - mag_min)*height + posy;
        mui_draw_circle((Mui_Vector2){.x=x,.y=y}, 2, color);
    }*/
  

}
