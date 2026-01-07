#include "s2p.h"

#include "stdio.h"
#include "math.h"
#include "stdbool.h"
#include "stdlib.h"

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
    if(parse_s2p_files(&infos) != 0) return 1;

    int w, h;
    w = 1700;
    h = 1100;

    mui_open_window(w, h, 10, 40, "Impedancer (s2p stats for impedance matching) - by bbeni", 1.0f, MUI_WINDOW_RESIZEABLE & MUI_WINDOW_RESIZEABLE, NULL);

    size_t selected = 0;
    Mui_Checkbox_State show_s11_checkbox_state = {0};
    Mui_Checkbox_State show_s21_checkbox_state = {0};
    Mui_Checkbox_State show_s12_checkbox_state = {0};
    Mui_Checkbox_State show_s22_checkbox_state = {0};
    Mui_Slider_State slider_state = {0};
    Mui_Slider_State slider_state_2 = {0};
    Mui_Textinput_Multiline_State textinput_ml_state = {0};

    show_s11_checkbox_state.checked = true;
    show_s21_checkbox_state.checked = true;
    show_s12_checkbox_state.checked = true;
    show_s22_checkbox_state.checked = true;
    slider_state.value = 0.5f;
    slider_state_2.value = 0.5f;

    mui_load_ttf_font_for_theme("resources/font/NimbusSans-Regular.ttf", &mui_protos_theme);


    while (!mui_window_should_close())
    {
        mui_update_core();

        w = mui_screen_width();
        h = mui_screen_height();

        int selected_before = selected;
        if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_DOWN)) {
            selected = (selected + 1) % infos.count;
        }
        if (mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
            if (selected == 0) selected = infos.count-1;
            selected = (selected - 1) % infos.count;
        }

        mui_begin_drawing();
            mui_clear_background(mui_protos_theme.global_background_color, NULL);

            Mui_Rectangle screen = {0, 0, w, h};
            Mui_Rectangle t_r = {0};
            Mui_Rectangle rest = mui_cut_top(screen, 100, &t_r);

            
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


            left = mui_shrink(left, padding);
            
            
            /*
            if (selected_before != selected){
                char* text = nob_temp_sprintf("%s\n===========================\n%.*s\n",infos.items[selected].file_name, infos.items[selected].file_content.count, infos.items[selected].file_content.items);
                size_t size = strlen(text)+1;
                nob_da_resize(&textinput_ml_state.buffer, size);
                memcpy(textinput_ml_state.buffer.items, text, size);
            }

            mui_textinput_multiline(&textinput_ml_state, "Hint...", left); */

            // plot views
            //right = mui_shrink(right, padding);

            // 4 plot windows.
            Mui_Rectangle r11, r12, r21, r22;
            mui_grid_22(right, 0.5f, 0.5f, &r11, &r12, &r21, &r22);
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

            double min_y = slider_state_2.value * (-30);
            double max_y = slider_state_2.value * 60;
            double step_y = 5;
            double min_f = 0;
            double max_f = slider_state.value * 2e11;
            double step_f = 2e9;

            size_t length = infos.items[selected].freq.count;
            double *fs = infos.items[selected].freq.items;
            struct Complex *s_params[4] = {infos.items[selected].s11.items, infos.items[selected].s21.items, infos.items[selected].s12.items, infos.items[selected].s22.items};
            Mui_Color colors[4] = {MUI_RED, MUI_ORANGE, MUI_GREEN, MUI_BLUE};
            char* labels[4] = {"dB(S11)", "dB(S21)", "dB(S12)", "dB(S22)"};
            bool mask[4] = {show_s11_checkbox_state.checked, show_s21_checkbox_state.checked, show_s12_checkbox_state.checked, show_s22_checkbox_state.checked};

            
            Mui_Rectangle plot_area = gra_xy_plot_labels_and_grid("frequency [Hz]", "mag(S11)", min_f, max_f, min_y, max_y, step_f, step_y, r11);

            for (int i = 0; i < 4; i++) {
                if (mask[i]) {
                    gra_xy_plot_data(fs, s_params[i], dB, length, min_f, max_f, min_y, max_y, colors[i], plot_area);
                }
            }
            gra_xy_legend(labels, colors, mask, 4, plot_area);

        mui_end_drawing();
        uti_temp_reset();
    }

    mui_close_window();

    return 0;
}