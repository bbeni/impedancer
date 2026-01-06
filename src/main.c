#include "s2p.h"

#include "stdio.h"
#include "math.h"

#include "mui.h"
#include "gra.h"

#define NOB_IMPLEMENTATION
#include "nob.h"


char* next(int* count, char*** argv) {
    (*count)--;
    return *((*argv)++);
}

Complex_Array sel_s11(S2P_Info* info) {return info->s11;}
Complex_Array sel_s21(S2P_Info* info) {return info->s21;}
Complex_Array sel_s12(S2P_Info* info) {return info->s12;}
Complex_Array sel_s22(S2P_Info* info) {return info->s22;}

double mag(size_t i, void* x) {
    Complex *s = (Complex *)x;
    Complex si = s[i];
    return sqrtf(si.i * si.i + si.r* si.r);
}

void draw_graph(S2P_Info info, int posx, int posy, int width, int height, Complex_Array (* selector)(S2P_Info* info), Mui_Color color) {
    double max_f = info.freq.items[info.freq.count-1];
    double min_f = info.freq.items[0];

    double mag_min = INFINITY;
    double mag_max = -INFINITY;

    Complex_Array arr = selector(&info);

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
    S2P_Infos infos = {0};
    if(read_s2p_files(directory, &infos) != 0) return 1;
    if(parse_s2p_files(&infos) != 0) return 1;

    int w, h;
    w = 1600;
    h = 1000;

    mui_open_window(w, h, 10, 40, "Impedancer (s2p stats for impedance matching) - by bbeni", 1.0f, MUI_WINDOW_RESIZEABLE & MUI_WINDOW_RESIZEABLE, NULL);

    size_t selected = 0;
    Mui_Checkbox_State show_s11_checkbox_state = {0};
    Mui_Checkbox_State show_s21_checkbox_state = {0};
    Mui_Checkbox_State show_s12_checkbox_state = {0};
    Mui_Checkbox_State show_s22_checkbox_state = {0};
    Mui_Slider_State slider_state = {0};
    slider_state.value = 0.5f;
    Mui_Slider_State slider_state_2 = {0};
    slider_state_2.value = 0.5f;
    Mui_Textinput_Multiline_State textinput_ml_state = {0};

    mui_load_ttf_font_for_theme("resources/font/NimbusSans-Regular.ttf", &mui_protos_theme);


    while (!mui_window_should_close())
    {
        mui_update_core();

        w = mui_screen_width();
        h = mui_screen_height();

        int selected_before = selected;
        if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
            selected = (selected + 1) % infos.count;
        }
        if (mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
            selected = (selected - 1) % infos.count;
        }

        mui_begin_drawing();
            mui_clear_background(mui_protos_theme.global_background_color, NULL);

            Mui_Rectangle screen = {0, 0, w, h};
            Mui_Rectangle t_r = {0};
            Mui_Rectangle rest = mui_cut_top(screen, 100, &t_r);

            
            float padding = 5;

            t_r = mui_shrink(t_r, padding);
            mui_label(&mui_protos_theme, nob_temp_sprintf("%lld: %s", selected, infos.items[selected].file_name), t_r);

            Mui_Rectangle left;
            Mui_Rectangle right = mui_cut_left(rest, 450, &left);

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

            Mui_Rectangle slider_rect2;
            left = mui_cut_right(left, 50, &slider_rect2);
            slider_rect2 = mui_shrink(slider_rect2, padding);
            mui_simple_slider(&slider_state_2, true, slider_rect2);

            left = mui_shrink(left, padding);
            
            
            /*
            if (selected_before != selected){
                char* text = nob_temp_sprintf("%s\n===========================\n%.*s\n",infos.items[selected].file_name, infos.items[selected].file_content.count, infos.items[selected].file_content.items);
                size_t size = strlen(text)+1;
                nob_da_resize(&textinput_ml_state.buffer, size);
                memcpy(textinput_ml_state.buffer.items, text, size);
            }

            mui_textinput_multiline(&textinput_ml_state, "Hint...", left); */

            // plot view

            Mui_Rectangle slider_rect;
            right = mui_cut_top(right, 50, &slider_rect);
            slider_rect = mui_shrink(slider_rect, padding);
            mui_simple_slider(&slider_state, false, slider_rect);

            right = mui_shrink(right, padding);


            size_t length = infos.items[selected].freq.count;
            double *fs = infos.items[selected].freq.items;
            Complex *s21 = infos.items[selected].s21.items;
            Complex *s12 = infos.items[selected].s12.items;
            Complex *s11 = infos.items[selected].s11.items;
            Complex *s22 = infos.items[selected].s22.items;
            double max_y = slider_state_2.value * 100; 
            double max_f = slider_state.value * 2e11;
            
            if (show_s11_checkbox_state.checked) {
                gra_xy_plot(fs, s11, mag, length, "frequency [Hz]", "mag(S11)", 0, max_f, 0, max_y, 2e9, 1, MUI_RED, right);
            }

            if (show_s21_checkbox_state.checked) {
                gra_xy_plot(fs, s21, mag, length, "frequency [Hz]", "mag(S21)", 0, max_f, 0, max_y, 2e9, 1, MUI_GREEN, right);
            }

            if (show_s12_checkbox_state.checked) {
                gra_xy_plot(fs, s12, mag, length, "frequency [Hz]", "mag(S12)", 0, max_f, 0, max_y, 2e9, 1, MUI_YELLOW, right);
            }

            if (show_s22_checkbox_state.checked) {
                gra_xy_plot(fs, s22, mag, length, "frequency [Hz]", "mag(S22)", 0, max_f, 0, max_y, 2e9, 1, MUI_BLUE, right);
            }

        mui_end_drawing();
        nob_temp_reset();
    }

    mui_close_window();

    return 0;
}