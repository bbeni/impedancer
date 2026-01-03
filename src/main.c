#include "stdio.h"
#include "math.h"
#include "raylib.h"

#include "s2p.h"
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"


char* next(int* count, char*** argv) {
    (*count)--;
    return *((*argv)++);
}

void draw_graph(S2P_Info info, int width, int height) {
    double max_f = info.freq.items[info.freq.count-1];
    double min_f = info.freq.items[0];

    double mag_min = INFINITY;
    double mag_max = -INFINITY;

    for (int i=0; i < info.freq.count; i++) {
        Complex s21 = info.s21.items[i];
        double mag_s21 = s21.r*s21.r + s21.i * s21.i;
        if (mag_s21 < mag_min) mag_min = mag_s21;
        if (mag_s21 > mag_max) mag_max = mag_s21;
    }


    for (int i=0; i < info.freq.count; i++) {
        Complex s21 = info.s21.items[i];
        double mag_s21 = s21.r*s21.r + s21.i * s21.i;
        double f = info.freq.items[i];
        int x = (f-min_f)/(max_f - min_f) * width;
        int y = height - (mag_s21-mag_min)/(mag_max - mag_min)*height;
        DrawCircle(x, y, 1, GREEN);
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
    w = 1400;
    h = 900;
    InitWindow(1400, 900, "Impedancer (s2p stats for impedance matching) - by bbeni");

    size_t selected = 0;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
            selected = (selected + 1) % infos.count;
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
            selected = (selected - 1) % infos.count;
        }

        BeginDrawing();
            ClearBackground(BLACK);
            draw_graph(infos.items[selected], w, h);
            DrawText(nob_temp_sprintf("%d: %s", selected, infos.items[selected].file_name), 200, 0, 32, RED); 
        EndDrawing();
    }

    CloseWindow();

    return 0;
}