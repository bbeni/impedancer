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

    int grid_pixels = 40;

    int w, h;
    w = 1900;
    h = 1120;

    mui_open_window(w, h, 10, 10, "Impedancer (s2p stats for impedance matching) - by bbeni", 1.0f, MUI_WINDOW_RESIZEABLE | MUI_WINDOW_UNDECORATED, NULL);
    mui_init_themes(0, 0, true, "resources/font/NimbusSans-Regular.ttf");

    #define MAX_CIRCUIT_COMPONENTS 20
    struct Circuit_Component *component_array;
    component_array = malloc(sizeof(*component_array) * MAX_CIRCUIT_COMPONENTS);
    struct Circuit_Component_View *component_view_array;
    component_view_array = malloc(sizeof(*component_view_array) * MAX_CIRCUIT_COMPONENTS);


    size_t n_comps = 8;
    size_t i = 0;


    // R 35k Ohm parallel
    circuit_create_resistor_ideal_parallel(35e3, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    // C 120 pF
    circuit_create_capacitor_ideal(120e-12, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    // stage 1
    circuit_create_stage(&stage_archetype, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    // R 5
    circuit_create_resistor_ideal(5, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    // L 1.7 uH parallel
    circuit_create_inductor_ideal_parallel(1.7e-6, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    // stage 2
    circuit_create_stage(&stage_archetype, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    // L 5.5 nH
    circuit_create_inductor_ideal(5.5e-9, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;
    // C 95 fF
    circuit_create_capacitor_ideal_parallel(95e-15, &component_array[i]);
    circuit_component_view_init(&component_view_array[i], &component_array[i]);
    i++;

    assert(i == n_comps && "update n_comps please");

    size_t selected_comp = 0;

    Mui_Button_State dark_mode_btn = mui_button_state();
    mui_protos_theme_g = mui_protos_theme_light_g;
    bool dark_mode = false;

    Mui_Slider_State chroma_slider = mui_slider_state();
    Mui_Slider_State hue_slider = mui_slider_state();


    struct Simulation_State simulation_state;
    bool todo_first_sim = true;

    while (!mui_window_should_close())
    {
        mui_update_core();

        w = mui_screen_width();
        h = mui_screen_height();

        //
        // input handling and simulation
        //
        // change setting values of components (stage excluded)
        if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_DOWN) ||
            mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
            double* vptr = NULL;
            switch(component_array[selected_comp].kind) {
                case CIRCUIT_COMPONENT_RESISTOR_IDEAL:
                    vptr = &component_array[selected_comp].as.resistor_ideal.R;
                break;
                case CIRCUIT_COMPONENT_CAPACITOR_IDEAL:
                    vptr = &component_array[selected_comp].as.capacitor_ideal.C ;
                break;
                case CIRCUIT_COMPONENT_INDUCTOR_IDEAL:
                    vptr = &component_array[selected_comp].as.inductor_ideal.L;
                break;
                case CIRCUIT_COMPONENT_STAGE:
                    // dont do anything handle it later down
                break;
                case CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL:
                    vptr = &component_array[selected_comp].as.resistor_ideal_parallel.R;
                break;
                case CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL:
                    vptr = &component_array[selected_comp].as.capacitor_ideal_parallel.C;
                break;
                case CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL:
                    vptr = &component_array[selected_comp].as.inductor_ideal_parallel.L;
                break;
                case CIRCUIT_COMPONENT_KIND_COUNT:
                    assert(false && "TODO: implement UP/DOWN for me");
                break;
                default:
                    assert(false && "TODO: implement UP/DOWN for me (2+ cases)");
                break;
            }

            if (vptr) {
                if (mui_is_key_pressed(MUI_KEY_DOWN) || mui_is_key_pressed_repeat(MUI_KEY_DOWN)) {
                    *vptr *= 0.5f;
                }

                if (mui_is_key_pressed(MUI_KEY_UP) || mui_is_key_pressed_repeat(MUI_KEY_UP)) {
                    *vptr *= 2;
                }
            }
        }

        // change setting values of stage
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

        }

        // select component
        if (mui_is_key_pressed(MUI_KEY_RIGHT) || mui_is_key_pressed_repeat(MUI_KEY_RIGHT)) {
            selected_comp = (selected_comp + 1) % n_comps;
        }

        // select component
        if (mui_is_key_pressed(MUI_KEY_LEFT) || mui_is_key_pressed_repeat(MUI_KEY_LEFT)) {
            if (selected_comp == 0) selected_comp = n_comps;
            selected_comp = (selected_comp - 1) % n_comps;
        }

        // simulate
        if (mui_is_key_pressed(MUI_KEY_S)) {
            if (!todo_first_sim) circuit_simulation_destroy(&simulation_state);
            circuit_simulation_setup(component_array, n_comps, &simulation_state);
            circuit_simulation_do(&simulation_state);

            if (todo_first_sim) todo_first_sim = false;
        }

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

        // update themes
        if (hue_slider_last_value != hue_slider.value || chroma_slider_last_value != chroma_slider.value) {
            mui_init_themes(chroma_slider.value * 0.1f, hue_slider.value*360, dark_mode, NULL);
        }

        Mui_Rectangle screen_inset = mui_shrink(screen, 5);

        Mui_Rectangle component_view_rect;
        Mui_Rectangle rest = screen_inset;


        //
        // simulation results draw
        //
        Mui_Rectangle bottom_place = screen_inset;
        Mui_Rectangle simulation_plot_rect;
        bottom_place.height *= 0.5f;
        bottom_place.y += bottom_place.height;
        if (!todo_first_sim) {

            double fmi = simulation_state.frequencies[0];
            double fma = simulation_state.frequencies[simulation_state.n_frequencies - 1];
            double ymi = -30;
            double yma = 60;
            double ystep = 10;
            double fstep = 10e9;

            bool should_plot_mask[4] = {true, true, true, true};
            char* x_label = "f [Hz]";
            char* y_label = "dB(S)";
            char* labels[4] = {"dB(S11)", "dB(S21)", "dB(S12)", "dB(S22)"};
            Mui_Color colors[4] = {MUI_RED, MUI_ORANGE, MUI_GREEN, MUI_BLUE};
            struct Complex* s[4] = {
                simulation_state.s11_result_plottable,
                simulation_state.s21_result_plottable,
                simulation_state.s12_result_plottable,
                simulation_state.s22_result_plottable
            };

            // TODO: gra: change signature
            //simulation_plot_rect = gra_xy_plot_labels_and_grid(
            //    x_label, "real(S)",
            //    fmi, fma, ymi, yma, fstep, ystep, true, simulation_plot_rect
            //);

            struct Gra_Gridded_Base_Arguments sim_plot_args;
            sim_plot_args.grid_unit_pixels = grid_pixels;
            sim_plot_args.grid_w = 22;
            sim_plot_args.grid_h = 14;
            sim_plot_args.grid_left_axis_off = 2;
            sim_plot_args.grid_bot_axis_off = 2;
            sim_plot_args.grid_skip_x = 3;
            sim_plot_args.grid_skip_y = 3;
            sim_plot_args.x_left = fmi;
            sim_plot_args.x_right = fma;
            sim_plot_args.y_bot = ymi;
            sim_plot_args.y_top = yma;
            sim_plot_args.x_label = x_label;
            sim_plot_args.y_label = y_label;
            sim_plot_args.thick_y_zero = true;
            sim_plot_args.tick_x_label_fmt = "%.0f";
            sim_plot_args.tick_y_label_fmt = "%.0f";

            Mui_Rectangle stability_plot_rect = mui_cut_left(bottom_place, (sim_plot_args.grid_w + 1) * grid_pixels, &simulation_plot_rect);
            simulation_plot_rect = gra_gridded_xy_base(&sim_plot_args, simulation_plot_rect);

            for (size_t i = 0; i < 4; i++) {
                gra_xy_plot_data_points(
                    simulation_state.frequencies,
                    s[i], dB, simulation_state.n_frequencies,
                    fmi, fma, ymi, yma, colors[i], 2, simulation_plot_rect
                );
            }
            gra_xy_legend(labels, colors, should_plot_mask, 4, simulation_plot_rect);

            // stability plot
            double mu_min = -1;
            double mu_max = 5;


            struct Gra_Gridded_Base_Arguments stability_plot_args;
            stability_plot_args.grid_unit_pixels = grid_pixels;
            stability_plot_args.grid_w = 22;
            stability_plot_args.grid_h = 14;
            stability_plot_args.grid_left_axis_off = 2;
            stability_plot_args.grid_bot_axis_off = 2;
            stability_plot_args.grid_skip_x = 3;
            stability_plot_args.grid_skip_y = 0;
            stability_plot_args.x_left = fmi;
            stability_plot_args.x_right = fma;
            stability_plot_args.y_bot = mu_min;
            stability_plot_args.y_top = mu_max;
            stability_plot_args.x_label = "f [Hz]";
            stability_plot_args.y_label = "Stabilty Factor mu, mu'";
            stability_plot_args.thick_y_zero = true;
            stability_plot_args.tick_x_label_fmt = "%.0f";
            stability_plot_args.tick_y_label_fmt = "%.1f";

            stability_plot_rect = gra_gridded_xy_base(&stability_plot_args, stability_plot_rect);

            gra_xy_plot_data_points(
                simulation_state.frequencies,
                simulation_state.stab_mu, NULL, simulation_state.n_frequencies,
                fmi, fma, mu_min, mu_max, MUI_GREEN, 2, stability_plot_rect
            );

            gra_xy_plot_data_points(
                simulation_state.frequencies,
                simulation_state.stab_mu_prime, NULL, simulation_state.n_frequencies,
                fmi, fma, mu_min, mu_max, MUI_DARKGREEN, 2, stability_plot_rect
            );

            char* mu_labels[2] = {"mu", "mu'"};
            Mui_Color mu_colors[2] = {MUI_GREEN, MUI_DARKGREEN};
            bool mu_mask[2] = {true, true};
            gra_xy_legend(mu_labels, mu_colors, mu_mask, 2, stability_plot_rect);


        } else {
            mui_label(&mui_protos_theme_g, "Move LEFT-RIGHT/UP-DOWN to select/change component. Press S to simulate circuit :)", MUI_TEXT_ALIGN_CENTER, bottom_place);
        }


        //
        // circuit elements draw
        //
        for (size_t i = 0; i < n_comps; i ++) {
            float width = 180.0f;
            if (component_view_array[i].kind == CIRCUIT_COMPONENT_STAGE)
                width = 360.0f;
            rest = mui_cut_left(rest, width, &component_view_rect);
            circuit_component_view_draw(&component_view_array[i], component_view_rect, selected_comp == i);
        }


        mui_end_drawing();
        uti_temp_reset();
    }

    mui_close_window();

    return 0;
}