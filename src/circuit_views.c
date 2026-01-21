#include "circuit_views.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "gra.h"
#include "uti.h"

double mag(size_t i, void* x) {
    struct Complex *s = (struct Complex *)x;
    struct Complex si = s[i];
    return sqrtf(si.i * si.i + si.r* si.r);
}

double dB_from_squared(double x) {
    return 10 * log10f(x);
}

double dB(size_t i, void* x) {
    struct Complex *s = (struct Complex *)x;
    struct Complex si = s[i];
    return dB_from_squared(si.i * si.i + si.r* si.r);
}


// stage

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


// compone the stage and stage_view and initialize state.
void stage_view_init(struct Stage_View* stage_view, struct Circuit_Component_Stage* stage) {
    memset(stage_view, 0, sizeof(*stage_view));

    stage_view->stage = stage;

    stage_view->active_setting = 0;
    stage_view->show_s11_checkbox_state.checked = true;
    stage_view->show_s21_checkbox_state.checked = true;
    stage_view->show_s12_checkbox_state.checked = true;
    stage_view->show_s22_checkbox_state.checked = true;
    stage_view->show_Gopt_checkbox_state.checked = true;
    stage_view->show_no_idea_what_state.checked = false;
    stage_view->slider_state_1.value = 0.5f;
    stage_view->slider_state_2.value = 0.2f;
    stage_view->selector_start = 0;
    stage_view->selector_end = 0;

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

    stage_view_update_active_setting(stage_view, 0);
    stage_view_update_data(stage_view);
};

void stage_view_update_active_setting(struct Stage_View* stage_view, size_t new_setting) {

    stage_view->active_setting = new_setting;
    stage_view->stage->selected_setting = new_setting;

    size_t active_setting = stage_view->active_setting;
    struct S2P_Info* info = &stage_view->stage->s2p_infos[active_setting];

    stage_view->length = info->data_length;
    stage_view->fs = info->freq;
    stage_view->s_params[0] = info->s11;
    stage_view->s_params[1] = info->s21;
    stage_view->s_params[2] = info->s12;
    stage_view->s_params[3] = info->s22;
    stage_view->z_params[0] = info->z11.items;
    stage_view->z_params[1] = info->z21.items;
    stage_view->z_params[2] = info->z12.items;
    stage_view->z_params[3] = info->z22.items;
    stage_view->Gopt = info->noise.GammaOpt;
    stage_view->zGopt = info->zGopt.items;
    stage_view->noise_length = info->noise.length;
    stage_view->NFmins = info->noise.NFmin;
    stage_view->noise_fs = info->noise.freq;

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

void stage_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);


    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;
    float dy = lower_y - upper_y;
    Mui_Rectangle body_rect;
    body_rect.height = 8.0f / 16.0f * r_inset.width;
    body_rect.width = 5.0f / 16.0f * r_inset.width;
    body_rect.y = upper_y + (dy - body_rect.height) * 0.5f;
    body_rect.x = center.x - body_rect.width * 0.5f;


    float line_thickness = 3;
    float hemt_thickness = line_thickness * 2;


    float f1 = 0.35f;
    float f2 = 2 * f1;
    float l_x = body_rect.x;
    float r_x = body_rect.x + body_rect.width;
    float m_x = body_rect.x + body_rect.width * 0.5f;

    // hemt
    mui_draw_line(l_x, upper_y + dy * f1, m_x, upper_y + dy * f1, hemt_thickness, col);
    mui_draw_line(r_x, upper_y, m_x, upper_y, hemt_thickness, col);
    mui_draw_line(r_x, upper_y + dy * f2, m_x, upper_y + dy * f2, hemt_thickness, col);
    mui_draw_line(m_x, upper_y - body_rect.height * 0.1f, m_x,  upper_y + dy * f2 + body_rect.height * 0.1f, hemt_thickness, col);


    // special connections
    mui_draw_line(r.x, upper_y, l_x, upper_y + dy * f1, line_thickness, col);
    mui_draw_line(r_x, upper_y + dy * f2, r_x, lower_y, line_thickness, col);

    // normal connections
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
    mui_draw_line(symbol_area.x, upper_y, r.x, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x + symbol_area.width, upper_y, r_x, upper_y, line_thickness, col);

}



void stage_view_settings_draw(struct Stage_View* stage_view, Mui_Rectangle symbol_area) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 10);

    char* model_text = stage_view->stage->models[stage_view->active_setting];
    Mui_Vector2 text_size = mui_measure_text(font, model_text, font_size, 0.2f, 0, strlen(model_text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, model_text, col, 0, strlen(model_text));

    char v_ds_text[40];
    snprintf(v_ds_text, 40, "%.2fV", stage_view->stage->voltage_ds_array[stage_view->active_setting]);
    text_size = mui_measure_text(font, v_ds_text, font_size, 0.2f, 0, strlen(v_ds_text));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, v_ds_text, col, 0, strlen(v_ds_text));

    char i_ds_text[40];
    snprintf(i_ds_text, 40, "%.4fmA", stage_view->stage->current_ds_array[stage_view->active_setting]*1000);
    text_size = mui_measure_text(font, i_ds_text, font_size, 0.2f, 0, strlen(i_ds_text));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f + text_size.y;
    mui_draw_text_line(font, pos, 0.2f, font_size, i_ds_text, col, 0, strlen(i_ds_text));

    char temp_text[40];
    snprintf(temp_text, 40, "%.0fK", stage_view->stage->temperatures[stage_view->active_setting]);
    text_size = mui_measure_text(font, temp_text, font_size, 0.2f, 0, strlen(temp_text));
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f + text_size.y;
    mui_draw_text_line(font, pos, 0.2f, font_size, temp_text, col, 0, strlen(temp_text));

}

void stage_view_draw(struct Stage_View* stage_view, Mui_Rectangle widget_area, bool is_selected) {

    stage_view_update_data(stage_view);

    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    stage_symbol_draw(symbol_area, is_selected);
    stage_view_settings_draw(stage_view, symbol_area);

    float padding = 5;

    Mui_Rectangle rest = widget_area;

    const float checkbox_s = 36;


    double step_f = 2e9;
    double step_y = 1;
    double min_nfmin = 0;
    double max_nfmin = 0.1;
    double step_nfmin = 0.01;
    double min_y = stage_view->slider_state_2.value * (-30);
    double max_y = stage_view->slider_state_2.value * 60;
    double min_f = stage_view->min_f;
    double max_f = stage_view->max_f;

    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_1, "Stage Settings", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        mui_draw_rectangle(sg_r, mui_protos_theme_g.bg_dark);
        sg_r = mui_shrink(sg_r, padding);
        mui_checkbox(&stage_view->show_no_idea_what_state, "show no idea what", sg_r);
    }


    Mui_Rectangle collabsable_area_2;
    rest = mui_cut_top(rest, 36, &collabsable_area_2);
    //collabsable_area_2 = mui_cut_top(collabsable_area_2, 2, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_2, "S-parameter plot", collabsable_area_2)) {
        //
        // s paremter plot
        //
        Mui_Rectangle s_param_plot_area;
        rest = mui_cut_top(rest, 300, &s_param_plot_area);
        mui_draw_rectangle(s_param_plot_area, mui_protos_theme_g.bg_dark);

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
        mui_draw_rectangle(s_param_plot_area, mui_protos_theme_g.bg_dark);
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
    //collabsable_area_3 = mui_cut_top(collabsable_area_3, 2, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_3, "Smith chart", collabsable_area_3)) {
        Mui_Rectangle smith_plot_area;
        rest = mui_cut_top(rest, 400, &smith_plot_area);

        //
        // smith chart draw
        //
        mui_draw_rectangle(smith_plot_area, mui_protos_theme_g.bg_dark);
        draw_smith_grid(true, true, NULL, 0, smith_plot_area);
        for (int i = 0; i < 4; i++) {
            if (stage_view->mask[i]) {
                gra_smith_plot_data(stage_view->fs_interpolated, stage_view->z_params_interpolated[i], N_INTERPOL, min_f, max_f, stage_view->colors[i], '-', 2, smith_plot_area);
            }
        }
        if (stage_view->show_Gopt_checkbox_state.checked) {
            gra_smith_plot_data(stage_view->fs_interpolated, stage_view->zGopt_interpolated, N_INTERPOL-1, min_f, max_f, MUI_BEIGE, '-', 2, smith_plot_area);
        }
    }

    Mui_Rectangle collabsable_area_4;
    rest = mui_cut_top(rest, 36, &collabsable_area_4);
    //collabsable_area_4 = mui_cut_top(collabsable_area_4, 2, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_4, "NFmin plot", collabsable_area_4)) {
        Mui_Rectangle noise_plot_area;
        rest = mui_cut_top(rest, 200, &noise_plot_area);
        //
        // noise plot draw
        //
        mui_draw_rectangle(noise_plot_area, mui_protos_theme_g.bg_dark);
        Mui_Rectangle plot_area2 = gra_xy_plot_labels_and_grid("frequency [Hz]", "NFmin", min_f, max_f, min_nfmin, max_nfmin, step_f, step_nfmin, true, noise_plot_area);
        gra_xy_plot_data_points(stage_view->fs_interpolated, stage_view->NFmins_interpolated, NULL, N_INTERPOL, min_f, max_f, min_nfmin, max_nfmin, MUI_GREEN, 1.0f, plot_area2);
        gra_xy_plot_data_points(stage_view->noise_fs, stage_view->NFmins, NULL, stage_view->noise_length, min_f, max_f, min_nfmin, max_nfmin, MUI_BLUE, 3.0f, plot_area2);
    }

    Mui_Rectangle collabsable_area_5;
    rest = mui_cut_top(rest, 36, &collabsable_area_5);
    //collabsable_area_5 = mui_cut_top(collabsable_area_5, 0, NULL);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_5, "Plot Settings", collabsable_area_5)) {
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
    Mui_Rectangle collabsable_area_6;
    rest = mui_cut_top(rest, 36, &collabsable_area_6);
    if (mui_collapsable_section(&stage_view->collapsable_section_state_6, "Impedances at min(f)", collabsable_area_6)) {
        //
        // Text data view
        //
        Mui_Rectangle text_data_view;
        rest = mui_cut_top(rest, 440, &text_data_view);
        mui_text_selectable(stage_view->selectable_text, &stage_view->selector_start, &stage_view->selector_end, text_data_view);
    }

}

//
// resistor
//

void resistor_view_init(struct Resistor_Ideal_View* resistor_view, struct Circuit_Component_Resistor_Ideal* resistor) {
    resistor_view->resistor = resistor;
    resistor_view->collapsable_section_1.open = false;
}

void resistor_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);

    float line_thickness = 3;
    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;
    Mui_Rectangle resistor_body_rect;
    resistor_body_rect.width = 4.0f / 8.0f * r_inset.width;
    resistor_body_rect.height = 3.0f / 16.0f* r_inset.width;
    resistor_body_rect.y = upper_y - resistor_body_rect.height * 0.5f;
    resistor_body_rect.x = center.x - resistor_body_rect.width * 0.5f;
    mui_draw_rectangle_lines(resistor_body_rect, col, line_thickness);

    mui_draw_line(symbol_area.x, upper_y, resistor_body_rect.x, upper_y, line_thickness, col);
    mui_draw_line(resistor_body_rect.x + resistor_body_rect.width, upper_y, symbol_area.x + symbol_area.width, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
}

void resistor_view_settings_draw(struct Resistor_Ideal_View* resistor_view, Mui_Rectangle symbol_area) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 5);


    double r = resistor_view->resistor->R;
    char rendered_resistance[40];
    if (r < 1000) {
        snprintf(rendered_resistance, 40, "%.2f", r);
    } else if (r < 1e6) {
        snprintf(rendered_resistance, 40, "%.2fk", r/1000);
    } else if (r < 1e9) {
        snprintf(rendered_resistance, 40, "%.2fM", r/1e6);
    } else {
        snprintf(rendered_resistance, 40, "%.2fG", r/1e9);
    }

    // top left
    const char text[] = " R ";
    Mui_Vector2 text_size = mui_measure_text(font, text, font_size, 0.2f, 0, strlen(text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, text, col, 0, strlen(text));


    // bot right
    text_size = mui_measure_text(font, rendered_resistance, font_size, 0.2f, 0, strlen(rendered_resistance));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, rendered_resistance, col, 0, strlen(rendered_resistance));
}

void resistor_view_draw(struct Resistor_Ideal_View* resistor_view, Mui_Rectangle widget_area, bool is_selected) {

    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    resistor_symbol_draw(symbol_area, is_selected);
    resistor_view_settings_draw(resistor_view, symbol_area);

    float padding = 5;

    Mui_Rectangle rest = widget_area;

    const float checkbox_s = 36;

    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&resistor_view->collapsable_section_1, "R Series", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_draw_rectangle(sg_r, MUI_RED);
    }

}

//
// resistor parallel
//

void resistor_parallel_view_init(struct Resistor_Ideal_Parallel_View* resistor_parallel_view, struct Circuit_Component_Resistor_Ideal_Parallel* resistor) {
    resistor_parallel_view->resistor = resistor;
    resistor_parallel_view->collapsable_section_1.open = false;
}

void resistor_parallel_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);

    float line_thickness = 3;
    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;
    float dy = lower_y - upper_y;
    Mui_Rectangle resistor_body_rect;
    resistor_body_rect.height = 8.0f / 16.0f * r_inset.width;
    resistor_body_rect.width = 3.0f / 16.0f * r_inset.width;
    resistor_body_rect.y = upper_y + (dy - resistor_body_rect.height) * 0.5f;
    resistor_body_rect.x = center.x - resistor_body_rect.width * 0.5f;

    mui_draw_rectangle_lines(resistor_body_rect, col, line_thickness);
    mui_draw_line(resistor_body_rect.x + resistor_body_rect.width * 0.5f, upper_y, resistor_body_rect.x + resistor_body_rect.width * 0.5f, resistor_body_rect.y, line_thickness, col);
    mui_draw_line(resistor_body_rect.x + resistor_body_rect.width * 0.5f, lower_y, resistor_body_rect.x + resistor_body_rect.width * 0.5f, resistor_body_rect.y + resistor_body_rect.height, line_thickness, col);

    mui_draw_line(symbol_area.x, upper_y, symbol_area.x + symbol_area.width, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
}

void resistor_parallel_view_settings_draw(struct Resistor_Ideal_Parallel_View* resistor_parallel_view, Mui_Rectangle symbol_area) {

    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 5);


    double r = resistor_parallel_view->resistor->R;
    char rendered_resistance[40];
    if (r < 1000) {
        snprintf(rendered_resistance, 40, "%.2f", r);
    } else if (r < 1e6) {
        snprintf(rendered_resistance, 40, "%.2fk", r/1000);
    } else if (r < 1e9) {
        snprintf(rendered_resistance, 40, "%.2fM", r/1e6);
    } else {
        snprintf(rendered_resistance, 40, "%.2fG", r/1e9);
    }

    // top left
    const char text[] = " R ";
    Mui_Vector2 text_size = mui_measure_text(font, text, font_size, 0.2f, 0, strlen(text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, text, col, 0, strlen(text));


    // bot right
    text_size = mui_measure_text(font, rendered_resistance, font_size, 0.2f, 0, strlen(rendered_resistance));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, rendered_resistance, col, 0, strlen(rendered_resistance));

}

void resistor_parallel_view_draw(struct Resistor_Ideal_Parallel_View* resistor_parallel_view, Mui_Rectangle widget_area, bool is_selected) {

    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    resistor_parallel_symbol_draw(symbol_area, is_selected);
    resistor_parallel_view_settings_draw(resistor_parallel_view, symbol_area);

    float padding = 5;

    Mui_Rectangle rest = widget_area;

    const float checkbox_s = 36;

    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&resistor_parallel_view->collapsable_section_1, "R Parallel", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_draw_rectangle(sg_r, MUI_RED);
    }

}

// capacitor

void capacitor_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);

    float line_thickness = 3;
    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;
    Mui_Rectangle body_rect;
    body_rect.width = 2.0f / 16.0f * r_inset.width;
    body_rect.height = 5.0f / 16.0f * r_inset.width;
    body_rect.y = upper_y - body_rect.height * 0.5f;
    body_rect.x = center.x - body_rect.width * 0.5f;
    mui_draw_line(body_rect.x, body_rect.y, body_rect.x, body_rect.y + body_rect.height,  line_thickness, col);
    mui_draw_line(body_rect.x + body_rect.width, body_rect.y, body_rect.x + body_rect.width, body_rect.y + body_rect.height, line_thickness, col);

    mui_draw_line(symbol_area.x, upper_y, body_rect.x, upper_y, line_thickness, col);
    mui_draw_line(body_rect.x + body_rect.width, upper_y, symbol_area.x + symbol_area.width, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
}

void capacitor_view_settings_draw(struct Capacitor_Ideal_View* capacitor_view, Mui_Rectangle symbol_area) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 5);

    double c = capacitor_view->capacitor->C;
    char rendered_capacitance[40];
    if (c < 1e-12) {
        snprintf(rendered_capacitance, 40, "%.2ff", c * 1e15);
    } else if (c < 1e-9) {
        snprintf(rendered_capacitance, 40, "%.2fp", c * 1e12);
    } else if (c < 1e-6) {
        snprintf(rendered_capacitance, 40, "%.2fn", c * 1e9);
    } else if (c < 1e-3) {
        snprintf(rendered_capacitance, 40, "%.2fu", c * 1e6);
    } else {
        snprintf(rendered_capacitance, 40, "%.2fm", c * 1e3);
    }

    // left
    const char text[] = " C ";
    Mui_Vector2 text_size = mui_measure_text(font, text, font_size, 0.2f, 0, strlen(text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, text, col, 0, strlen(text));

    // right
    text_size = mui_measure_text(font, rendered_capacitance, font_size, 0.2f, 0, strlen(rendered_capacitance));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, rendered_capacitance, col, 0, strlen(rendered_capacitance));
}

void capacitor_view_init(struct Capacitor_Ideal_View* capacitor_view, struct Circuit_Component_Capacitor_Ideal* capacitor) {
    capacitor_view->capacitor = capacitor;
    capacitor_view->collapsable_section_1.open = false;
}

void capacitor_view_draw(struct Capacitor_Ideal_View* capacitor_view, Mui_Rectangle widget_area, bool is_selected) {
    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    capacitor_symbol_draw(symbol_area, is_selected);
    capacitor_view_settings_draw(capacitor_view, symbol_area);

    float padding = 5;
    Mui_Rectangle rest = widget_area;
    const float checkbox_s = 36;
    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&capacitor_view->collapsable_section_1, "C Series", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_draw_rectangle(sg_r, MUI_RED);
    }
}

// capacitor parallel

void capacitor_parallel_view_init(struct Capacitor_Ideal_Parallel_View* capacitor_parallel_view, struct Circuit_Component_Capacitor_Ideal_Parallel* capacitor) {
    capacitor_parallel_view->capacitor = capacitor;
    capacitor_parallel_view->collapsable_section_1.open = false;
}

void capacitor_parallel_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);

    float line_thickness = 3;
    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;
    float dy = lower_y - upper_y;
    Mui_Rectangle body_rect;
    body_rect.height = 2.0f / 16.0f * r_inset.width;
    body_rect.width = 5.0f / 16.0f * r_inset.width;
    body_rect.y = upper_y + (dy - body_rect.height) * 0.5f;
    body_rect.x = center.x - body_rect.width * 0.5f;

    float r_x = body_rect.x + body_rect.width;
    float l_x = body_rect.x;
    float y1 = body_rect.y;
    float y2 = body_rect.y + body_rect.height;

    mui_draw_line(l_x, y1, r_x, y1, line_thickness, col);
    mui_draw_line(l_x, y2, r_x, y2, line_thickness, col);

    mui_draw_line(body_rect.x + body_rect.width * 0.5f, upper_y, body_rect.x + body_rect.width * 0.5f, body_rect.y, line_thickness, col);
    mui_draw_line(body_rect.x + body_rect.width * 0.5f, lower_y, body_rect.x + body_rect.width * 0.5f, body_rect.y + body_rect.height, line_thickness, col);

    mui_draw_line(symbol_area.x, upper_y, symbol_area.x + symbol_area.width, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
}

void capacitor_parallel_view_settings_draw(struct Capacitor_Ideal_Parallel_View* capacitor_parallel_view, Mui_Rectangle symbol_area) {

    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 5);

    double c = capacitor_parallel_view->capacitor->C;
    char rendered_capacitance[40];
    if (c < 1e-12) {
        snprintf(rendered_capacitance, 40, "%.2ff", c * 1e15);
    } else if (c < 1e-9) {
        snprintf(rendered_capacitance, 40, "%.2fp", c * 1e12);
    } else if (c < 1e-6) {
        snprintf(rendered_capacitance, 40, "%.2fn", c * 1e9);
    } else if (c < 1e-3) {
        snprintf(rendered_capacitance, 40, "%.2fu", c * 1e6);
    } else {
        snprintf(rendered_capacitance, 40, "%.2fm", c * 1e3);
    }

    // top left
    const char text[] = " C ";
    Mui_Vector2 text_size = mui_measure_text(font, text, font_size, 0.2f, 0, strlen(text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, text, col, 0, strlen(text));


    // bot right
    text_size = mui_measure_text(font, rendered_capacitance, font_size, 0.2f, 0, strlen(rendered_capacitance));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, rendered_capacitance, col, 0, strlen(rendered_capacitance));

}


void capacitor_parallel_view_draw(struct Capacitor_Ideal_Parallel_View* resistor_parallel_view, Mui_Rectangle widget_area, bool is_selected) {
    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    capacitor_parallel_symbol_draw(symbol_area, is_selected);
    capacitor_parallel_view_settings_draw(resistor_parallel_view, symbol_area);

    float padding = 5;
    Mui_Rectangle rest = widget_area;
    const float checkbox_s = 36;
    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&resistor_parallel_view->collapsable_section_1, "C Parallel", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_draw_rectangle(sg_r, MUI_RED);
    }
}


// inductor
void inductor_view_init(struct Inductor_Ideal_View* inductor_view, struct Circuit_Component_Inductor_Ideal* inductor) {
    inductor_view->inductor = inductor;
    inductor_view->collapsable_section_1.open = false;
}

void inductor_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);

    float line_thickness = 3;
    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;

    float rad = 1.0f / 8.0f * r_inset.width;
    Mui_Vector2 pos;
    pos.y = upper_y;
    pos.x = r_inset.x + rad * 2;
    mui_draw_arc_lines(pos, rad, 0, 180, col, line_thickness);
    pos.x = r_inset.x + rad * 4;
    mui_draw_arc_lines(pos, rad, 0, 180, col, line_thickness);
    pos.x = r_inset.x + rad * 6;
    mui_draw_arc_lines(pos, rad, 0, 180, col, line_thickness);

    mui_draw_line(symbol_area.x, upper_y, r_inset.x + rad, upper_y, line_thickness, col);
    mui_draw_line(r_inset.x + r_inset.width - rad, upper_y, symbol_area.x + symbol_area.width, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
}

void inductor_view_settings_draw(struct Inductor_Ideal_View* inductor_view, Mui_Rectangle symbol_area) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 5);

    double l = inductor_view->inductor->L;
    char rendered_capacitance[40];
    if (l < 1e-12) {
        snprintf(rendered_capacitance, 40, "%.2ff", l * 1e15);
    } else if (l < 1e-9) {
        snprintf(rendered_capacitance, 40, "%.2fp", l * 1e12);
    } else if (l < 1e-6) {
        snprintf(rendered_capacitance, 40, "%.2fn", l * 1e9);
    } else if (l < 1e-3) {
        snprintf(rendered_capacitance, 40, "%.2fu", l * 1e6);
    } else {
        snprintf(rendered_capacitance, 40, "%.2fm", l * 1e3);
    }

    // left
    const char text[] = " L ";
    Mui_Vector2 text_size = mui_measure_text(font, text, font_size, 0.2f, 0, strlen(text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, text, col, 0, strlen(text));


    // right
    text_size = mui_measure_text(font, rendered_capacitance, font_size, 0.2f, 0, strlen(rendered_capacitance));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, rendered_capacitance, col, 0, strlen(rendered_capacitance));
}


void inductor_view_draw(struct Inductor_Ideal_View* inductor_view, Mui_Rectangle widget_area, bool is_selected) {
    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    inductor_symbol_draw(symbol_area, is_selected);
    inductor_view_settings_draw(inductor_view, symbol_area);

    float padding = 5;
    Mui_Rectangle rest = widget_area;
    const float checkbox_s = 36;
    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&inductor_view->collapsable_section_1, "L Series", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_draw_rectangle(sg_r, MUI_RED);
    }

}



// inductor parallel

void inductor_parallel_view_init(struct Inductor_Ideal_Parallel_View* inductor_parallel_view, struct Circuit_Component_Inductor_Ideal_Parallel* inductor) {
    inductor_parallel_view->inductor = inductor;
    inductor_parallel_view->collapsable_section_1.open = false;
}


void inductor_parallel_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg;
    Mui_Color hl_color = mui_protos_theme_g.primary;

    mui_draw_rectangle(symbol_area, bg);
    if (should_highlight) {
        float border = 4.0f;
        mui_draw_rectangle_rounded_lines(mui_shrink(symbol_area, border), 10.0f, hl_color, border);
    }

    float w = min(symbol_area.width, symbol_area.height);
    Mui_Rectangle r;
    r.width = w;
    r.height = w;
    mui_center_rectangle_inside_rectangle(&r, symbol_area);

    float line_thickness = 3;
    float f = w * 0.05f;
    Mui_Rectangle r_inset = mui_shrink(r, f);
    Mui_Vector2 center = mui_center_of_rectangle(r_inset);

    float upper_y = r.y + 3.0f / 16.0f * r.height;
    float lower_y = r.y + 13.0f / 16.0f * r.height;
    float dy = lower_y - upper_y;
    float mid = r_inset.x + r_inset.width * 0.5f;

    float rad = 1.0f / 8.0f * dy;
    Mui_Vector2 pos;
    pos.x = mid;
    pos.y = upper_y + rad * 2;
    mui_draw_arc_lines(pos, rad, 90, 270, col, line_thickness);
    pos.y = upper_y + rad * 4;
    mui_draw_arc_lines(pos, rad, 90, 270, col, line_thickness);
    pos.y = upper_y + rad * 6;
    mui_draw_arc_lines(pos, rad, 90, 270, col, line_thickness);


    mui_draw_line(mid, upper_y, mid, upper_y + rad, line_thickness, col);
    mui_draw_line(mid, lower_y, mid, lower_y - rad, line_thickness, col);

    mui_draw_line(symbol_area.x, upper_y, symbol_area.x + symbol_area.width, upper_y, line_thickness, col);
    mui_draw_line(symbol_area.x, lower_y, symbol_area.x + symbol_area.width, lower_y, line_thickness, col);
}

void inductor_parallel_view_settings_draw(struct Inductor_Ideal_Parallel_View* inductor_view, Mui_Rectangle symbol_area) {
    Mui_Color col = mui_protos_theme_g.text;
    Mui_Color bg = mui_protos_theme_g.bg_dark;
    struct Mui_Font *font = mui_protos_theme_g.font;
    float font_size = mui_protos_theme_g.font_size;

    Mui_Rectangle inset_area = mui_shrink(symbol_area, 5);

    double l = inductor_view->inductor->L;
    char rendered_capacitance[40];
    if (l < 1e-12) {
        snprintf(rendered_capacitance, 40, "%.2ff", l * 1e15);
    } else if (l < 1e-9) {
        snprintf(rendered_capacitance, 40, "%.2fp", l * 1e12);
    } else if (l < 1e-6) {
        snprintf(rendered_capacitance, 40, "%.2fn", l * 1e9);
    } else if (l < 1e-3) {
        snprintf(rendered_capacitance, 40, "%.2fu", l * 1e6);
    } else {
        snprintf(rendered_capacitance, 40, "%.2fm", l * 1e3);
    }

    // left
    const char text[] = " L ";
    Mui_Vector2 text_size = mui_measure_text(font, text, font_size, 0.2f, 0, strlen(text));
    Mui_Vector2 pos;
    pos.x = inset_area.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    Mui_Rectangle bg_rect = mui_rectangle(pos.x, pos.y, text_size.x, text_size.y);
    mui_draw_rectangle_rounded(bg_rect, 4.0f, bg);
    mui_draw_text_line(font, pos, 0.2f, font_size, text, col, 0, strlen(text));


    // right
    text_size = mui_measure_text(font, rendered_capacitance, font_size, 0.2f, 0, strlen(rendered_capacitance));
    pos.x = inset_area.x + inset_area.width - text_size.x;
    pos.y = inset_area.y + (inset_area.height - text_size.y) * 0.5f;
    mui_draw_text_line(font, pos, 0.2f, font_size, rendered_capacitance, col, 0, strlen(rendered_capacitance));
}


void inductor_parallel_view_draw(struct Inductor_Ideal_Parallel_View* inductor_parallel_view, Mui_Rectangle widget_area, bool is_selected) {
    Mui_Rectangle symbol_area;
    widget_area = mui_cut_top(widget_area, CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT, &symbol_area);

    inductor_parallel_symbol_draw(symbol_area, is_selected);
    inductor_parallel_view_settings_draw(inductor_parallel_view, symbol_area);

    float padding = 5;
    Mui_Rectangle rest = widget_area;
    const float checkbox_s = 36;
    Mui_Rectangle collabsable_area_1;
    rest = mui_cut_top(rest, 36, &collabsable_area_1);
    //collabsable_area_1 = mui_cut_top(collabsable_area_1, 0, NULL);
    if (mui_collapsable_section(&inductor_parallel_view->collapsable_section_1, "L Parallel", collabsable_area_1)) {
        Mui_Rectangle sg_r;
        rest = mui_cut_top(rest, checkbox_s, &sg_r);
        sg_r = mui_shrink(sg_r, padding);
        mui_draw_rectangle(sg_r, MUI_RED);
    }

}



//
// polymorphism stuff
//

void circuit_component_view_init(struct Circuit_Component_View* component_view, struct Circuit_Component* component) {
    switch (component->kind) {
    case CIRCUIT_COMPONENT_RESISTOR_IDEAL:
        resistor_view_init(&component_view->as.resistor_ideal_view, &component->as.resistor_ideal);
    break;
    case CIRCUIT_COMPONENT_CAPACITOR_IDEAL:
        capacitor_view_init(&component_view->as.capacitor_ideal_view, &component->as.capacitor_ideal);
    break;
    case CIRCUIT_COMPONENT_INDUCTOR_IDEAL:
        inductor_view_init(&component_view->as.inductor_ideal_view, &component->as.inductor_ideal);
    break;
    case CIRCUIT_COMPONENT_STAGE:
        stage_view_init(&component_view->as.stage_view, &component->as.stage);
    break;
    case CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL:
        resistor_parallel_view_init(&component_view->as.resistor_ideal_parallel_view, &component->as.resistor_ideal_parallel);
    break;
    case CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL:
        capacitor_parallel_view_init(&component_view->as.capacitor_ideal_parallel_view, &component->as.capacitor_ideal_parallel);
    break;
    case CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL:
        inductor_parallel_view_init(&component_view->as.inductor_ideal_parallel_view, &component->as.inductor_ideal_parallel);
    break;
    case CIRCUIT_COMPONENT_KIND_COUNT:
        assert(false && "TODO: implement the next kind here!");
    break;
    default:
        assert(false);
    break;
    }

    component_view->kind = component->kind;
}


void circuit_component_view_draw(struct Circuit_Component_View* component_view, Mui_Rectangle widget_area, bool is_selected) {
    switch (component_view->kind) {
    case CIRCUIT_COMPONENT_RESISTOR_IDEAL:
        resistor_view_draw(&component_view->as.resistor_ideal_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_CAPACITOR_IDEAL:
        capacitor_view_draw(&component_view->as.capacitor_ideal_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_INDUCTOR_IDEAL:
        inductor_view_draw(&component_view->as.inductor_ideal_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_STAGE:
        stage_view_draw(&component_view->as.stage_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL:
        resistor_parallel_view_draw(&component_view->as.resistor_ideal_parallel_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL:
        capacitor_parallel_view_draw(&component_view->as.capacitor_ideal_parallel_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL:
        inductor_parallel_view_draw(&component_view->as.inductor_ideal_parallel_view, widget_area, is_selected);
    break;
    case CIRCUIT_COMPONENT_KIND_COUNT:
        assert(false && "TODO: implement the next kind, add case ...: above !");
    break;
    default:
        assert(false);
    break;
    }

}
