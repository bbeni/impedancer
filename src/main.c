#include "s2p.h"

#include "stdio.h"
#include "math.h"

#define MUI_IMPLEMENTATION
#include "mui.h"

#include "raylib.h"
#include "rlgl.h"

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

void draw_graph(S2P_Info info, int posx, int posy, int width, int height, Complex_Array (* selector)(S2P_Info* info), Color color) {
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
        DrawCircle(x, y, 2, color);
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
    h = 1100;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(w, h, "Impedancer (s2p stats for impedance matching) - by bbeni");

    size_t selected = 0;
    Mui_Checkbox_State show_s11_checkbox_state = {0};
    Mui_Checkbox_State show_s21_checkbox_state = {0};
    Mui_Checkbox_State show_s12_checkbox_state = {0};
    Mui_Checkbox_State show_s22_checkbox_state = {0};

    mui_load_ttf_font("resources/font/NimbusSans-Regular.ttf");

    while (!WindowShouldClose())
    {
        w = GetScreenWidth();
        h = GetScreenHeight();
        mui_time_now = GetTime();
        mui_mouse_pos = GetMousePosition();

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
            selected = (selected + 1) % infos.count;
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
            selected = (selected - 1) % infos.count;
        }

        BeginDrawing();
            ClearBackground(mui_protos_theme.global_background_color);

            Rectangle screen = {0, 0, w, h};
            Rectangle t_r = {0};
            Rectangle rest = mui_cut_top(screen, 100, &t_r);
            
            float padding = 5;

            t_r = mui_shrink(t_r, padding);
            mui_label(&mui_protos_theme, nob_temp_sprintf("%lld: %s", selected, infos.items[selected].file_name), t_r);

            Rectangle left;
            Rectangle right = mui_cut_left(rest, 450, &left);

            Rectangle sg_r;
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

            right = mui_shrink(right, padding);

            if (show_s11_checkbox_state.checked) {
                draw_graph(infos.items[selected], right.x, right.y, right.width, right.height, sel_s11, RED);
            }

            if (show_s21_checkbox_state.checked) {
                draw_graph(infos.items[selected], right.x, right.y, right.width, right.height, sel_s21, GREEN);
            }

            if (show_s12_checkbox_state.checked) {
                draw_graph(infos.items[selected], right.x, right.y, right.width, right.height, sel_s12, YELLOW);
            }

            if (show_s22_checkbox_state.checked) {
                draw_graph(infos.items[selected], right.x, right.y, right.width, right.height, sel_s22, BROWN);
            }


        EndDrawing();
    }

    CloseWindow();

    return 0;
}