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
    h = 1100;

    mui_open_window(w, h, 10, 40, "Impedancer (s2p stats for impedance matching) - by bbeni", 1.0f, MUI_WINDOW_RESIZEABLE | MUI_WINDOW_UNDECORATED | MUI_WINDOW_MAXIMIZED, NULL);
    mui_load_ttf_font_for_theme("resources/font/NimbusSans-Regular.ttf", &mui_protos_theme);


    size_t selected = 0;
    Mui_Checkbox_State show_s11_checkbox_state = {0};
    Mui_Checkbox_State show_s21_checkbox_state = {0};
    Mui_Checkbox_State show_s12_checkbox_state = {0};
    Mui_Checkbox_State show_s22_checkbox_state = {0};
    Mui_Checkbox_State show_Gopt_checkbox_state = {0};
    Mui_Slider_State slider_state = {0};
    Mui_Slider_State slider_state_2 = {0};
    //Mui_Textinput_Multiline_State textinput_ml_state = {0};

    show_s11_checkbox_state.checked = true;
    show_s21_checkbox_state.checked = true;
    show_s12_checkbox_state.checked = true;
    show_s22_checkbox_state.checked = true;
    show_Gopt_checkbox_state.checked = true;
    slider_state.value = 0.5f;
    slider_state_2.value = 0.2f;

    // text selection
    size_t selector_start = 0;
    size_t selector_end = 0;

    #define SELECTABLE_TEXT_LENGTH 8*4096
    char selectable_text[SELECTABLE_TEXT_LENGTH];
    char* selectable_text_fmt =
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

    // data to load
    size_t length;
    double *fs;
    struct Complex *s_params[4];
    struct Complex *z_params[4];

    size_t noise_length = infos.items[selected].noise.freq.count;
    double *noise_fs = infos.items[selected].noise.freq.items;
    double *NFmins = infos.items[selected].noise.freq.items;

#define N_INTERPOL 2000
    double noise_fs_interpolated[N_INTERPOL];
    double NFmins_interpolated[N_INTERPOL];

    // static data
    Mui_Color colors[4] = {MUI_RED, MUI_ORANGE, MUI_GREEN, MUI_BLUE};
    char* labels[4] = {"dB(S11)", "dB(S21)", "dB(S12)", "dB(S22)"};
    char* labels_index[4] = {"11", "21", "12", "22"};
    bool mask[4];

    bool first_frame = true;
    while (!mui_window_should_close())
    {
        mui_update_core();

        w = mui_screen_width();
        h = mui_screen_height();

        //
        // input handling
        //
        size_t selected_before = selected;
        if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_DOWN)) {
            selected = (selected + 1) % infos.count;
        }
        if (mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
            if (selected == 0) selected = infos.count-1;
            selected = (selected - 1) % infos.count;
        }

        if (mui_is_key_down(MUI_KEY_LEFT_CONTROL) || mui_is_key_down(MUI_KEY_RIGHT_CONTROL)) {
            if (mui_is_key_pressed(MUI_KEY_C)) {
                if (selector_start < selector_end) {
                    char* temp_str = uti_temp_strndup(&selectable_text[selector_start], selector_end - selector_start);
                    mui_set_clipboard_text(temp_str);
                }
            }
        }


        //
        // data loading
        //
        if (selected != selected_before || first_frame) {
            length = infos.items[selected].freq.count;
            fs = infos.items[selected].freq.items;
            s_params[0] = infos.items[selected].s11.items;
            s_params[1] = infos.items[selected].s21.items;
            s_params[2] = infos.items[selected].s12.items;
            s_params[3] = infos.items[selected].s22.items;
            z_params[0] = infos.items[selected].z11.items;
            z_params[1] = infos.items[selected].z21.items;
            z_params[2] = infos.items[selected].z12.items;
            z_params[3] = infos.items[selected].z22.items;

            noise_length = infos.items[selected].noise.NFmin.count;
            NFmins = infos.items[selected].noise.NFmin.items;
            noise_fs = infos.items[selected].noise.freq.items;

        }


        //
        // drawing
        //
        mui_begin_drawing();
            mui_clear_background(mui_protos_theme.global_background_color, NULL);

            Mui_Rectangle screen = {0, 0, w, h};
            Mui_Rectangle t_r = mui_window_decoration(50.0f, true, true, true, true, true, screen);
            Mui_Rectangle rest = mui_cut_top(screen, 50, NULL);

            float padding = 5;

            t_r = mui_shrink(t_r, padding);
            mui_label(&mui_protos_theme, infos.items[selected].file_name, t_r);

            Mui_Rectangle left;
            Mui_Rectangle right = mui_cut_left(rest, 200, &left);

            Mui_Rectangle sg_r;
            left = mui_cut_top(left, 50, &sg_r);
            sg_r = mui_shrink(sg_r, padding);
            mui_checkbox(&show_s11_checkbox_state, "Show S11", sg_r);
            left = mui_cut_top(left, 50, &sg_r);
            sg_r = mui_shrink(sg_r, padding);
            mui_checkbox(&show_s21_checkbox_state, "Show S21", sg_r);
            left = mui_cut_top(left, 50, &sg_r);
            sg_r = mui_shrink(sg_r, padding);
            mui_checkbox(&show_s12_checkbox_state, "Show S12", sg_r);
            left = mui_cut_top(left, 50, &sg_r);
            sg_r = mui_shrink(sg_r, padding);
            mui_checkbox(&show_s22_checkbox_state, "Show S22", sg_r);
            left = mui_cut_top(left, 50, &sg_r);
            sg_r = mui_shrink(sg_r, padding);
            mui_checkbox(&show_Gopt_checkbox_state, "Show Gopt", sg_r);

            left = mui_shrink(left, padding);

            // plot views
            right = mui_shrink(right, padding);

            // 4 plot windows.
            Mui_Rectangle r11, r12, r21, r22;
            mui_grid_22(right, 0.667f, 0.5f, &r11, &r12, &r21, &r22);
            r11 = mui_shrink(r11, padding);
            r12 = mui_shrink(r12, padding);
            r21 = mui_shrink(r21, padding);
            r22 = mui_shrink(r22, padding);

            Mui_Rectangle slider_rect;
            r11 = mui_cut_top(r11, 50, &slider_rect);
            slider_rect = mui_cut_left(slider_rect, 24, NULL);
            slider_rect = mui_cut_right(slider_rect, 24*2, NULL);
            slider_rect = mui_shrink(slider_rect, padding);
            mui_simple_slider(&slider_state, false, slider_rect);

            Mui_Rectangle slider_rect2;
            r11 = mui_cut_right(r11, 50, &slider_rect2);
            slider_rect2 = mui_cut_bot(slider_rect2, 24, NULL);
            slider_rect2 = mui_shrink(slider_rect2, padding);
            mui_simple_slider(&slider_state_2, true, slider_rect2);


            mask[0] = show_s11_checkbox_state.checked;
            mask[1] = show_s21_checkbox_state.checked;
            mask[2] = show_s12_checkbox_state.checked;
            mask[3] = show_s22_checkbox_state.checked;
            double min_y = slider_state_2.value * (-30);
            double max_y = slider_state_2.value * 60;
            double min_f = 1;
            double max_f = slider_state.value * 2e11 + min_f;
            double step_f = 2e9;
            double step_y = 1;
            //
            // dB plot
            //
            Mui_Rectangle plot_area = gra_xy_plot_labels_and_grid("frequency [Hz]", "mag(S11)", min_f, max_f, min_y, max_y, step_f, step_y, true, r11);
            for (int i = 0; i < 4; i++) {
                if (mask[i]) {
                    gra_xy_plot_data_points(fs, s_params[i], dB, length, min_f, max_f, min_y, max_y, colors[i], 2.0, plot_area);
                }
            }
            gra_xy_legend(labels, colors, mask, 4, plot_area);

            //
            // smith chart
            //
            draw_smith_grid(true, true, NULL, 0, r21);
            Mui_Vector2 r21c = mui_center_of_rectangle(r21);
            for (int i = 0; i < 4; i++) {
                if (mask[i]) {
                    gra_smith_plot_data(fs, z_params[i], length, min_f, max_f, colors[i], r21c, r21);
                    //printf("%s@200GHz = %f + %f * i\n, z = %f + %f * i\n",labels[i], s_params[i][length-1].r, s_params[i][length-1].i, z_params[i][length-1].r, z_params[i][length-1].i);
                }
            }

            struct Complex *zGopt = infos.items[selected].zGopt.items;
            if (show_Gopt_checkbox_state.checked) {
                gra_smith_plot_data(fs, zGopt, infos.items[selected].zGopt.count, min_f, max_f, MUI_BROWN, r21c, r21);
            }

            //
            // noise plot
            //
            // F = Fmin + 4*Rn*|Gs - Gopt|^2 / (Z0(1 - |Gopt|^2)*|1+Gopt|^2)
            // we assume optimal noise match Gs == Gopt for now
            // -> F = Fmin
            float min_nfmin = 0;
            float max_nfmin = 0.1;
            float step_nfmin = 0.01;
            Mui_Rectangle plot_area2 = gra_xy_plot_labels_and_grid("frequency [Hz]", "NFmin", min_f, max_f, min_nfmin, max_nfmin, step_f, step_nfmin, true, r12);

            for (size_t i = 0; i < N_INTERPOL; i ++) {
                noise_fs_interpolated[i] = min_f + i * (max_f - min_f) / N_INTERPOL;
            }
            mma_spline_cubic_natural_linear(noise_fs, NFmins, noise_length, NFmins_interpolated, N_INTERPOL, min_f, max_f);
            gra_xy_plot_data_points(noise_fs_interpolated, NFmins_interpolated, NULL, N_INTERPOL, min_f, max_f, min_nfmin, max_nfmin, MUI_GREEN, 1.0f, plot_area2);
            gra_xy_plot_data_points(noise_fs, NFmins, NULL, noise_length, min_f, max_f, min_nfmin, max_nfmin, MUI_BLUE, 3.0f, plot_area2);


            //
            // Text data view
            //
            if (selected_before != selected || first_frame) {
                size_t i = 0;
                double Z0 = infos.items[selected].R_ref;
                snprintf(selectable_text, SELECTABLE_TEXT_LENGTH, selectable_text_fmt,
                    fs[i],
                    labels_index[0], z_params[0][i].r, z_params[0][i].i,
                    labels_index[1], z_params[1][i].r, z_params[1][i].i,
                    labels_index[2], z_params[2][i].r, z_params[2][i].i,
                    labels_index[3], z_params[3][i].r, z_params[3][i].i,
                    "Gopt",          zGopt[i].r, zGopt[i].i,
                    labels_index[0], z_params[0][i].r * Z0, z_params[0][i].i * Z0,
                    labels_index[1], z_params[1][i].r * Z0, z_params[1][i].i * Z0,
                    labels_index[2], z_params[2][i].r * Z0, z_params[2][i].i * Z0,
                    labels_index[3], z_params[3][i].r * Z0, z_params[3][i].i * Z0,
                    "Gopt",          zGopt[i].r * Z0, zGopt[i].i * Z0
                );
            }

            mui_text_selectable(selectable_text, &selector_start, &selector_end, r22);

            /*//
            // Text data view
            //
            if (selected_before != selected) {
                char* text = infos.items[selected].file__content;
                size_t size =infos.items[selected].file__content_size;
                // it is already null terminated
                textinput_ml_state.buffer.items = realloc(textinput_ml_state.buffer.items, size);
                textinput_ml_state.buffer.count = size;
                memcpy(textinput_ml_state.buffer.items, text, size);
            }

            mui_textinput_multiline(&textinput_ml_state, "Hint...", r22);
            */

        mui_end_drawing();
        uti_temp_reset();
        first_frame = false;
    }

    mui_close_window();

    return 0;
}