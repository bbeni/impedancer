// Copyright (C) 2026 Benjamin Froelich
// This file is part of https://github.com/bbeni/impedancer
// For conditions of distribution and use, see copyright notice in project root.
#include "s2p.h"

#include "stdio.h"
#include "math.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"

#include "mui.h"
#include "gra.h"
#include "uti.h"

#include "circuit.h"
#include "circuit_views.h"



char* next(int* count, char*** argv) {
    (*count)--;
    return *((*argv)++);
}


int main(int argc, char** argv) {

    char* prog_name = next(&argc, &argv);

    if (argc == 0) {
        printf("ERROR: Need 's2p_dir'\n");
        printf("Usage: %s 's2p_dir'\n", prog_name);
        exit(1);
    }

    char* directory = next(&argc, &argv);

    struct Circuit_Component_Stage stage_archetype;
    if (!circuit_create_stage_archetype("000_device_settings.csv", directory, &stage_archetype))
        return 2;

    //struct S2P_Info_Array infos = {0};
    //if(!read_s2p_files_from_dir(directory, &infos) != 0)
    //    return 1;

    //if(!parse_s2p_files(&infos, true) != 0)
    //    return 1;


    int w, h;
    w = 1900;
    h = 1100;

    mui_open_window(w, h, 10, 10, "Impedancer (s2p stats for impedance matching) - by bbeni", 1.0f, MUI_WINDOW_RESIZEABLE | MUI_WINDOW_UNDECORATED, NULL);
    mui_init_themes(0, 0, true, "resources/font/NimbusSans-Regular.ttf");

    #define MAX_CIRCUIT_COMPONENTS 20
    struct Circuit_Component *component_array;
    component_array = malloc(sizeof(*component_array) * MAX_CIRCUIT_COMPONENTS);
    struct Circuit_Component_View *component_view_array;
    component_view_array = malloc(sizeof(*component_view_array) * MAX_CIRCUIT_COMPONENTS);


    // stage resistor resistor resistor
    size_t n_comps = 8;
    size_t i = 0;
    // C 12 pF
    circuit_create_capacitor_ideal(12e-12, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // R 35k Ohm parallel
    circuit_create_resistor_ideal_parallel(35e3, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // stage 1
    circuit_create_stage(&stage_archetype, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // R 2M Ohm
    circuit_create_resistor_ideal(2e6, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // L 6.8 uH parallel
    circuit_create_inductor_ideal_parallel(6.8e-6, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // stage 2
    circuit_create_stage(&stage_archetype, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // L 360 nH
    circuit_create_inductor_ideal(360e-9, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // C 1.5 nF
    circuit_create_capacitor_ideal_parallel(1.5e-9, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    assert(i == n_comps && "update n_comps please");

    size_t selected_comp = 0;

    Mui_Button_State disco_mode_btn = mui_button_state();
    bool disco_mode = false;

    Mui_Button_State dark_mode_btn = mui_button_state();
    mui_protos_theme_g = mui_protos_theme_dark_g;
    bool dark_mode = true;

    Mui_Slider_State chroma_slider = mui_slider_state();
    Mui_Slider_State hue_slider = mui_slider_state();


    while (!mui_window_should_close())
    {
        mui_update_core();

        w = mui_screen_width();
        h = mui_screen_height();

        //
        // input handling
        //

        if (component_array[selected_comp].kind == CIRCUIT_COMPONENT_STAGE) {
            struct Stage_View* stage_view = &(component_view_array[selected_comp].as.stage_view);
            size_t selected_before = stage_view->active_setting;
            size_t active_setting = selected_before;
            if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_DOWN)) {
                active_setting = (active_setting + 1) % stage_view->stage->n_settings;
            }
            if (mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
                if (active_setting == 0) active_setting = stage_view->stage->n_settings - 1;
                active_setting = (active_setting - 1) % stage_view->stage->n_settings;
            }
            if (selected_before != active_setting) {
                stage_view_update_active_setting(stage_view, active_setting);
            }


            if (mui_is_key_down(MUI_KEY_LEFT_CONTROL) || mui_is_key_down(MUI_KEY_RIGHT_CONTROL)) {
                if (mui_is_key_pressed(MUI_KEY_C)) {
                    if (stage_view->selector_start < stage_view->selector_end) {
                        char* temp_str = uti_temp_strndup(&stage_view->selectable_text[stage_view->selector_start], stage_view->selector_end - stage_view->selector_start);
                        mui_set_clipboard_text(temp_str);
                    }
                }
            }

        }

        if (mui_is_key_pressed(MUI_KEY_RIGHT) || mui_is_key_pressed_repeat(MUI_KEY_RIGHT)) {
            selected_comp = (selected_comp + 1) % n_comps;
        }
        if (mui_is_key_pressed(MUI_KEY_LEFT) || mui_is_key_pressed_repeat(MUI_KEY_LEFT)) {
            if (selected_comp == 0) selected_comp = n_comps;
            selected_comp = (selected_comp - 1) % n_comps;
        }

        //
        // data loading
        //


        //
        // drawing
        //
        mui_begin_drawing();
        mui_clear_background(mui_protos_theme_g.bg_dark, NULL);

        Mui_Rectangle whole_screen = mui_rectangle(0, 0, w, h);
        const float decoration_height = 36.0f;
        Mui_Rectangle menu_bar_area = mui_window_decoration(decoration_height, true, true, true, true, true, whole_screen);
        Mui_Rectangle screen = mui_cut_top(whole_screen, decoration_height, NULL);

        Mui_Rectangle dark_mode_btn_area;
        menu_bar_area = mui_cut_left(menu_bar_area, 140, &dark_mode_btn_area);
        dark_mode_btn_area = mui_shrink(dark_mode_btn_area, 5.0f);
        char* dm_text = dark_mode ? "light mode" : "dark mode";
        if (mui_button(&dark_mode_btn, dm_text, dark_mode_btn_area)) {
            dark_mode = !dark_mode;
            if (dark_mode) {
                mui_protos_theme_g = mui_protos_theme_dark_g;
            } else {
                mui_protos_theme_g = mui_protos_theme_light_g;
            }
        }

        Mui_Rectangle chroma_slider_area;
        menu_bar_area = mui_cut_left(menu_bar_area, 140,  &chroma_slider_area);
        chroma_slider_area = mui_shrink(chroma_slider_area, 5.0f);
        float chroma_slider_last_value = chroma_slider.value;
        mui_simple_slider(&chroma_slider, false, chroma_slider_area);

        Mui_Rectangle hue_slider_area;
        menu_bar_area = mui_cut_left(menu_bar_area, 140,  &hue_slider_area);
        hue_slider_area = mui_shrink(hue_slider_area, 5.0f);
        float hue_slider_last_value = hue_slider.value;
        mui_simple_slider(&hue_slider, false, hue_slider_area);

        Mui_Rectangle disco_mode_btn_area;
        menu_bar_area = mui_cut_left(menu_bar_area, 140, &disco_mode_btn_area);
        disco_mode_btn_area = mui_shrink(disco_mode_btn_area, 5.0f);
        char* disco_mode_text = disco_mode ? "disco mode" : "disco mode";
        if (mui_button(&disco_mode_btn, disco_mode_text, disco_mode_btn_area)) {
            disco_mode = !disco_mode;
        }
        // update themes
        if (disco_mode) {
            hue_slider.value = fmodf(disco_mode_btn.last_time * 0.1f, 1.0f);
        }
        if (hue_slider_last_value != hue_slider.value || chroma_slider_last_value != chroma_slider.value) {
            mui_init_themes(chroma_slider.value * 0.1f, hue_slider.value*360, dark_mode, NULL);
        }

        Mui_Rectangle screen_inset = mui_shrink(screen, 5);

        Mui_Rectangle stage_view_rect;
        Mui_Rectangle rest = screen_inset;

        for (size_t i = 0; i < n_comps; i ++) {
            float width = 160.0f;
            if (component_view_array[i].kind == CIRCUIT_COMPONENT_STAGE)
                width = 420.0f;
            rest = mui_cut_left(rest, width, &stage_view_rect);
            circuit_component_view_draw(&component_view_array[i], stage_view_rect, selected_comp == i);
        }

        mui_end_drawing();
        uti_temp_reset();
    }

    mui_close_window();

    return 0;
}