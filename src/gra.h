//
// Graphing Functions
//
#ifndef GRA_H_
#define GRA_H_

#include "mui.h"
#include "mma.h"


// draw a complete xy plot
void gra_xy_plot(double *x_data, void *y_data, double (* y_map)(size_t i, void *x), size_t data_length,
    char* x_label, char* y_label, double x_min, double x_max, double y_min, double y_max,
    double x_step, double y_step, Mui_Color color, Mui_Rectangle place);
// draw the grid and labels first. Returns plot_area rectangle.
Mui_Rectangle gra_xy_plot_labels_and_grid( char* x_label, char* y_label, double x_min, double x_max,
    double y_min, double y_max, double x_step, double y_step, Mui_Rectangle place);
// draw all xy data
void gra_xy_plot_data(double *x_data, void *y_data, double (* y_map)(size_t i, void *x), size_t data_length,
    double x_min, double x_max, double y_min, double y_max, Mui_Color color, Mui_Rectangle plot_area);
// draw the legend last
void gra_xy_legend(char **labels, Mui_Color *colors, bool *mask, size_t n_labels_, Mui_Rectangle plot_area);
void draw_smith_grid(Mui_Rectangle plot_area, bool plot_reactance_circles, bool plot_admittance_circles, double *custom_cicles, size_t n_custom_circles);
void gra_smith_plot_data(double *f_data, struct Complex *z_data, size_t data_length,
                 double f_min, double f_max, Mui_Color color, Mui_Vector2 smith_center, float smith_radius);

#endif //GRA_H_
