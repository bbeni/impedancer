// Copyright (C) 2026 Benjamin Froelich
// This file is part of https://github.com/bbeni/impedancer
// For conditions of distribution and use, see copyright notice in project root.
#include "s2p.h"

#include "stdio.h"
#include "math.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"


#include "mui.h"
#include "gra.h"
#include "uti.h"


char* next(int* count, char*** argv) {
    (*count)--;
    return *((*argv)++);
}

struct Complex_Array sel_s11(struct S2P_Info* info) {return info->s11;}
struct Complex_Array sel_s21(struct S2P_Info* info) {return info->s21;}
struct Complex_Array sel_s12(struct S2P_Info* info) {return info->s12;}
struct Complex_Array sel_s22(struct S2P_Info* info) {return info->s22;}

double mag(size_t i, void* x) {
    struct Complex *s = (struct Complex *)x;
    struct Complex si = s[i];
    return sqrtf(si.i * si.i + si.r* si.r);
}
double dB_from_squared(double x) {return 10*log10f(x);}
double dB(size_t i, void* x) {
    struct Complex *s = (struct Complex *)x;
    struct Complex si = s[i];
    return dB_from_squared(si.i * si.i + si.r* si.r);
}

void draw_graph(struct S2P_Info info, int posx, int posy, int width, int height, struct Complex_Array (* selector)(struct S2P_Info* info), Mui_Color color) {
    double max_f = info.freq.items[info.freq.count-1];
    double min_f = info.freq.items[0];

    double mag_min = INFINITY;
    double mag_max = -INFINITY;

    struct Complex_Array arr = selector(&info);

    for (size_t i=0; i < info.freq.count; i++) {
        struct Complex s = arr.items[i];
        double mag_s = s.r*s.r + s.i * s.i;
        if (mag_s < mag_min) mag_min = mag_s;
        if (mag_s > mag_max) mag_max = mag_s;
    }

    for (size_t i=0; i < info.freq.count; i++) {
        struct Complex s = arr.items[i];
        double mag_s = s.r*s.r + s.i * s.i;
        double f = info.freq.items[i];
        int x = (f-min_f)/(max_f - min_f) * width + posx;
        int y = height - (mag_s-mag_min)/(mag_max - mag_min)*height + posy;
        mui_draw_circle((Mui_Vector2){.x=x,.y=y}, 2, color);
    }
}


struct Stage_View {
    size_t active_setting;
    Mui_Checkbox_State show_s11_checkbox_state;
    Mui_Checkbox_State show_s21_checkbox_state;
    Mui_Checkbox_State show_s12_checkbox_state;
    Mui_Checkbox_State show_s22_checkbox_state;
    Mui_Checkbox_State show_Gopt_checkbox_state;
    Mui_Slider_State slider_state_1;
    Mui_Slider_State slider_state_2;
    Mui_Collapsable_Section_State collapsable_section_state_1;
    Mui_Collapsable_Section_State collapsable_section_state_2;
    Mui_Collapsable_Section_State collapsable_section_state_3;
    Mui_Collapsable_Section_State collapsable_section_state_4;

    // text selection
    #define SELECTABLE_TEXT_LENGTH 8*4096
    char selectable_text[SELECTABLE_TEXT_LENGTH];
    size_t selector_start;
    size_t selector_end;
    double min_f_before;
    double max_f_before;
    double min_f;
    double max_f;
    bool mask[4];
    // static data
    Mui_Color colors[4];
    char* labels[4];
    char* labels_index[4];

    // s2p data
    struct S2P_Info_Array *infos;

    // data to load
    size_t length;
    double *fs;
    struct Complex *s_params[4];
    struct Complex *Gopt;
    struct Complex *z_params[4];
    struct Complex *zGopt;
    size_t noise_length;
    double *noise_fs;
    double *NFmins;

    // interpolated data
    #define N_INTERPOL 2000
    double fs_interpolated[N_INTERPOL];
    double NFmins_interpolated[N_INTERPOL];
    struct Complex s_params_interpolated[4][N_INTERPOL];
    struct Complex z_params_interpolated[4][N_INTERPOL];
    struct Complex Gopt_interpolated[N_INTERPOL];
    struct Complex zGopt_interpolated[N_INTERPOL];

};

const char* stage_view_selectable_text_fmt =
    "impedances z = Z/Z_0 at %f Hz\n"
    "\n"
    "z%s      = %.5f + j * %.5f\n"
    "z%s      = %.5f + j * %.5f\n"
    "z%s      = %.5f + j * %.5f\n"
    "z%s      = %.5f + j * %.5f\n"
    "z%s   = %.5f + j * %.5f\n"
    "\n"
    "Z%s      = (%.0f + j * %.0f) Ohm\n"
    "Z%s      = (%.0f + j * %.0f) Ohm\n"
    "Z%s      = (%.0f + j * %.0f) Ohm\n"
    "Z%s      = (%.0f + j * %.0f) Ohm\n"
    "Z%s   = (%.0f + j * %.0f) Ohm\n";

void stage_view_init(struct Stage_View* stage_view, struct S2P_Info_Array* infos) {
    memset(stage_view, 0, sizeof(*stage_view));

    stage_view->active_setting = 0;
    stage_view->show_s11_checkbox_state.checked = true;
    stage_view->show_s21_checkbox_state.checked = true;
    stage_view->show_s12_checkbox_state.checked = true;
    stage_view->show_s22_checkbox_state.checked = true;
    stage_view->show_Gopt_checkbox_state.checked = true;
    stage_view->slider_state_1.value = 0.5f;
    stage_view->slider_state_2.value = 0.2f;
    stage_view->selector_start = 0;
    stage_view->selector_end = 0;
    stage_view->infos = infos;

    stage_view->colors[0] = MUI_RED;
    stage_view->colors[1] = MUI_ORANGE;
    stage_view->colors[2] = MUI_GREEN;
    stage_view->colors[3] = MUI_BLUE;

    stage_view->labels[0] = "dB(S11)";
    stage_view->labels[1] = "dB(S21)";
    stage_view->labels[2] = "dB(S12)";
    stage_view->labels[3] = "dB(S22)";

    stage_view->labels_index[0] = "11";
    stage_view->labels_index[1] = "21";
    stage_view->labels_index[2] = "12";
    stage_view->labels_index[3] = "22";
};

void stage_view_update_active_setting(struct Stage_View* stage_view, size_t new_setting) {
    stage_view->active_setting = new_setting;
    size_t active_setting = stage_view->active_setting;
    struct S2P_Info* info = &stage_view->infos->items[active_setting];

    stage_view->length = info->freq.count;
    stage_view->fs = info->freq.items;
    stage_view->s_params[0] = info->s11.items;
    stage_view->s_params[1] = info->s21.items;
    stage_view->s_params[2] = info->s12.items;
    stage_view->s_params[3] = info->s22.items;
    stage_view->z_params[0] = info->z11.items;
    stage_view->z_params[1] = info->z21.items;
    stage_view->z_params[2] = info->z12.items;
    stage_view->z_params[3] = info->z22.items;
    stage_view->Gopt = info->noise.GammaOpt.items;
    stage_view->zGopt = info->zGopt.items;
    stage_view->noise_length = info->noise.NFmin.count;
    stage_view->NFmins = info->noise.NFmin.items;
    stage_view->noise_fs = info->noise.freq.items;

    size_t i = 0;
    double Z0 = info->R_ref;
    snprintf(stage_view->selectable_text, SELECTABLE_TEXT_LENGTH, stage_view_selectable_text_fmt,
        stage_view->fs[i],
        stage_view->labels_index[0], stage_view->z_params[0][i].r, stage_view->z_params[0][i].i,
        stage_view->labels_index[1], stage_view->z_params[1][i].r, stage_view->z_params[1][i].i,
        stage_view->labels_index[2], stage_view->z_params[2][i].r, stage_view->z_params[2][i].i,
        stage_view->labels_index[3], stage_view->z_params[3][i].r, stage_view->z_params[3][i].i,
        "Gopt",          stage_view->zGopt[i].r, stage_view->zGopt[i].i,
        stage_view->labels_index[0], stage_view->z_params[0][i].r * Z0, stage_view->z_params[0][i].i * Z0,
        stage_view->labels_index[1], stage_view->z_params[1][i].r * Z0, stage_view->z_params[1][i].i * Z0,
        stage_view->labels_index[2], stage_view->z_params[2][i].r * Z0, stage_view->z_params[2][i].i * Z0,
        stage_view->labels_index[3], stage_view->z_params[3][i].r * Z0, stage_view->z_params[3][i].i * Z0,
        "Gopt",          stage_view->zGopt[i].r * Z0, stage_view->zGopt[i].i * Z0
    );


}

void stage_view_update_data(struct Stage_View* stage_view) {

    //
    // update data from sliders (it is one fram delayed, but thats fine)
    //
    stage_view->min_f_before = stage_view->min_f;
    stage_view->max_f_before = stage_view->max_f;
    stage_view->mask[0] = stage_view->show_s11_checkbox_state.checked;
    stage_view->mask[1] = stage_view->show_s21_checkbox_state.checked;
    stage_view->mask[2] = stage_view->show_s12_checkbox_state.checked;
    stage_view->mask[3] = stage_view->show_s22_checkbox_state.checked;

    stage_view->min_f = 1;
    stage_view->max_f = stage_view->slider_state_1.value * 1.9999999999e11 + stage_view->min_f + 0.00000000001;


    //
    // interpoaltion here in drawing now, beacause it should be responsive
    //
    // F = Fmin + 4*Rn*|Gs - Gopt|^2 / (Z0(1 - |Gopt|^2)*|1+Gopt|^2)
    // we assume optimal noise match Gs == Gopt for now
    // -> F = Fmin

    double min_f = stage_view->min_f;
    double max_f = stage_view->max_f;
    size_t length = stage_view->length;
    size_t noise_length = stage_view->noise_length;


    for (size_t i = 0; i < N_INTERPOL; i ++) {
        stage_view->fs_interpolated[i] = min_f + i * (max_f - min_f) / N_INTERPOL;
    }

    mma_spline_cubic_natural_linear_complex(stage_view->noise_fs, stage_view->Gopt, noise_length, stage_view->Gopt_interpolated, N_INTERPOL, min_f, max_f);
    mma_spline_cubic_natural_linear(stage_view->noise_fs, stage_view->NFmins, noise_length, stage_view->NFmins_interpolated, N_INTERPOL, min_f, max_f);
    for (size_t i = 0; i < 4; i ++) {
        mma_spline_cubic_natural_linear_complex(stage_view->fs, stage_view->s_params[i], length, stage_view->s_params_interpolated[i], N_INTERPOL, min_f, max_f);
    }

    // insted of this just calculate z again
    //mma_spline_cubic_natural_linear_complex(fs, z_params[i], length, z_params_interpolated[i], N_INTERPOL, min_f, max_f);
    for (size_t j = 0; j < N_INTERPOL; j ++) {
        calc_z_from_s(
            (struct Complex [2][2]){{ stage_view->s_params_interpolated[0][j],  stage_view->s_params_interpolated[1][j]},{ stage_view->s_params_interpolated[2][j],  stage_view->s_params_interpolated[3][j]}},
            (struct Complex*[2][2]){{&stage_view->z_params_interpolated[0][j], &stage_view->z_params_interpolated[1][j]},{&stage_view->z_params_interpolated[2][j], &stage_view->z_params_interpolated[3][j]}}
        );
        calc_z_from_gamma(stage_view->Gopt_interpolated[j], &stage_view->zGopt_interpolated[j]);
    }
}

void stage_view_draw(struct Stage_View* stage_view, Mui_Rectangle widget_area) {

    stage_view_update_data(stage_view);

    float padding = 5;
    Mui_Rectangle top_lable_rect;
    Mui_Rectangle rest = mui_cut_top(widget_area, 50, &top_lable_rect);
    //top_lable_rect = mui_shrink(top_lable_rect, padding);
    char* label = stage_view->infos->items[stage_view->active_setting].file_name;
    mui_label(&mui_protos_theme, label, top_lable_rect);

    const float checkbox_s = 40;

    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    collabsable_area_1 = mui_cut_top(collabsable_area_1, 1, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_1, "Settings", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_checkbox(&stage_view->show_s11_checkbox_state, "Show S11", sg_r);
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_checkbox(&stage_view->show_s21_checkbox_state, "Show S21", sg_r);
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_checkbox(&stage_view->show_s12_checkbox_state, "Show S12", sg_r);
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_checkbox(&stage_view->show_s22_checkbox_state, "Show S22", sg_r);
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_checkbox(&stage_view->show_Gopt_checkbox_state, "Show Gopt", sg_r);
    }

    double step_f = 2e9;
    double step_y = 1;
    double min_nfmin = 0;
    double max_nfmin = 0.1;
    double step_nfmin = 0.01;
    double min_y = stage_view->slider_state_2.value * (-30);
    double max_y = stage_view->slider_state_2.value * 60;
    double min_f = stage_view->min_f;
    double max_f = stage_view->max_f;

    Mui_Rectangle collabsable_area_2;
    rest = mui_cut_top(rest, 36, &collabsable_area_2);
    collabsable_area_2 = mui_cut_top(collabsable_area_2, 1, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_2, "S-parameter plot", collabsable_area_2)) {
        Mui_Rectangle s_param_plot_area;
        rest = mui_cut_top(rest, 300, &s_param_plot_area);

        Mui_Rectangle slider_rect;
        s_param_plot_area = mui_cut_top(s_param_plot_area, 50, &slider_rect);
        slider_rect = mui_cut_left(slider_rect, 24, NULL);
        slider_rect = mui_cut_right(slider_rect, 24*2, NULL);
        slider_rect = mui_shrink(slider_rect, padding);
        mui_simple_slider(&stage_view->slider_state_1, false, slider_rect);

        Mui_Rectangle slider_rect2;
        s_param_plot_area = mui_cut_right(s_param_plot_area, 50, &slider_rect2);
        slider_rect2 = mui_cut_bot(slider_rect2, 24, NULL);
        slider_rect2 = mui_shrink(slider_rect2, padding);
        mui_simple_slider(&stage_view->slider_state_2, true, slider_rect2);

        //
        // s parameter plot draw
        //
        Mui_Rectangle plot_area = gra_xy_plot_labels_and_grid("frequency [Hz]", "mag(S11)", min_f, max_f, min_y, max_y, step_f, step_y, true, s_param_plot_area);
        for (int i = 0; i < 4; i++) {
            if (stage_view->mask[i]) {
                gra_xy_plot_data_points(stage_view->fs_interpolated, stage_view->s_params_interpolated[i], dB, N_INTERPOL, min_f, max_f, min_y, max_y, stage_view->colors[i], 1.0, plot_area);
                //gra_xy_plot_data_points(fs, s_params[i], dB, length, min_f, max_f, min_y, max_y, MUI_RED, 2.0, plot_area);
            }
        }
        gra_xy_legend(stage_view->labels, stage_view->colors, stage_view->mask, 4, plot_area);
    }

    Mui_Rectangle collabsable_area_3;
    rest = mui_cut_top(rest, 36, &collabsable_area_3);
    collabsable_area_3 = mui_cut_top(collabsable_area_3, 1, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_3, "Smith chart", collabsable_area_3)) {
        Mui_Rectangle smith_plot_area;
        rest = mui_cut_top(rest, 500, &smith_plot_area);

        //
        // smith chart draw
        //
        draw_smith_grid(true, true, NULL, 0, smith_plot_area);
        for (int i = 0; i < 4; i++) {
            if (stage_view->mask[i]) {
                gra_smith_plot_data(stage_view->fs_interpolated, stage_view->z_params_interpolated[i], N_INTERPOL, min_f, max_f, stage_view->colors[i], '-', 2, smith_plot_area);
            }
        }
        if (stage_view->show_Gopt_checkbox_state.checked) {
            gra_smith_plot_data(stage_view->fs_interpolated, stage_view->zGopt_interpolated, N_INTERPOL-1, min_f, max_f, MUI_BEIGE, '-', 2, smith_plot_area);
        }
        //
        // Text data view
        //
        Mui_Rectangle text_data_view;
        rest = mui_cut_top(rest, 440, &text_data_view);
        mui_text_selectable(stage_view->selectable_text, &stage_view->selector_start, &stage_view->selector_end, text_data_view);

    }

    Mui_Rectangle collabsable_area_4;
    rest = mui_cut_top(rest, 36, &collabsable_area_4);
    collabsable_area_4 = mui_cut_top(collabsable_area_4, 1, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_4, "NFmin plot", collabsable_area_4)) {
        Mui_Rectangle noise_plot_area;
        rest = mui_cut_top(rest, 500, &noise_plot_area);
        //
        // noise plot draw
        //
        Mui_Rectangle plot_area2 = gra_xy_plot_labels_and_grid("frequency [Hz]", "NFmin", min_f, max_f, min_nfmin, max_nfmin, step_f, step_nfmin, true, noise_plot_area);
        gra_xy_plot_data_points(stage_view->fs_interpolated, stage_view->NFmins_interpolated, NULL, N_INTERPOL, min_f, max_f, min_nfmin, max_nfmin, MUI_GREEN, 1.0f, plot_area2);
        gra_xy_plot_data_points(stage_view->noise_fs, stage_view->NFmins, NULL, stage_view->noise_length, min_f, max_f, min_nfmin, max_nfmin, MUI_BLUE, 3.0f, plot_area2);
    }
}

int main(int argc, char** argv) {

    char* prog_name = next(&argc, &argv);

    if (argc == 0) {
        printf("ERROR: Need 's2p_dir'\n");
        printf("Usage: %s 's2p_dir'\n", prog_name);
        exit(1);
    }

    char* directory = next(&argc, &argv);

    struct S2P_Info_Array infos = {0};
    if(read_s2p_files(directory, &infos) != 0) return 1;
    if(parse_s2p_files(&infos, true) != 0) return 1;

    int w, h;
    w = 1700;
    h = 1150;

    mui_open_window(w, h, 10, 40, "Impedancer (s2p stats for impedance matching) - by bbeni", 1.0f, MUI_WINDOW_RESIZEABLE | MUI_WINDOW_UNDECORATED, NULL);
    mui_load_ttf_font_for_theme("resources/font/NimbusSans-Regular.ttf", &mui_protos_theme);

    struct Stage_View stage_view = {0};
    stage_view_init(&stage_view, &infos);
    stage_view_update_active_setting(&stage_view, 0);

    struct Stage_View stage_view_2 = {0};
    stage_view_init(&stage_view_2, &infos);
    stage_view_update_active_setting(&stage_view_2, 0);

    while (!mui_window_should_close())
    {
        mui_update_core();

        w = mui_screen_width();
        h = mui_screen_height();

        //
        // input handling
        //
        size_t selected_before = stage_view.active_setting;
        size_t active_setting = selected_before;
        if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_DOWN)) {
            active_setting = (active_setting + 1) % infos.count;
        }
        if (mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
            if (active_setting == 0) active_setting = infos.count-1;
            active_setting = (active_setting - 1) % infos.count;
        }

        if (mui_is_key_down(MUI_KEY_LEFT_CONTROL) || mui_is_key_down(MUI_KEY_RIGHT_CONTROL)) {
            if (mui_is_key_pressed(MUI_KEY_C)) {
                if (stage_view.selector_start < stage_view.selector_end) {
                    char* temp_str = uti_temp_strndup(&stage_view.selectable_text[stage_view.selector_start], stage_view.selector_end - stage_view.selector_start);
                    mui_set_clipboard_text(temp_str);
                }
            }
        }

        //
        // data loading
        //
        if (active_setting != selected_before) {
            stage_view_update_active_setting(&stage_view, active_setting);
        }


        //
        // drawing
        //
        mui_begin_drawing();
        mui_clear_background(mui_protos_theme.global_background_color, NULL);

        Mui_Rectangle whole_screen = {0, 0, w, h};
        const float decoration_height = 36.0f;
        mui_window_decoration(decoration_height, true, true, true, true, true, whole_screen);
        //Mui_Rectangle screen = mui_cut_top(whole_screen, 1, NULL);
        Mui_Rectangle screen = whole_screen;
        screen.width = 1400;

        Mui_Rectangle stage_view_rect;
        Mui_Rectangle screen_inset = mui_shrink(screen, 5);
        Mui_Rectangle rest = mui_cut_left(screen_inset, 600, &stage_view_rect);
        stage_view_draw(&stage_view, stage_view_rect);

        Mui_Rectangle stage_view_rect_2;
        rest = mui_cut_left(rest, 600, &stage_view_rect_2);
        stage_view_draw(&stage_view_2, stage_view_rect_2);


        mui_end_drawing();
        uti_temp_reset();
    }

    mui_close_window();

    return 0;
}