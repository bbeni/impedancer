#ifndef CIRCUIT_VIEWS_H_
#define CIRCUIT_VIEWS_H_

#include "circuit.h"
#include "mui.h"

#define CIRCUIT_VIEW_SYMBOL_AREA_HEIGHT 150

//
// circuit Views
//

struct Stage_View;
struct Stage_View {

    struct Circuit_Component_Stage *stage;
    size_t active_setting;

    Mui_Checkbox_State show_s11_checkbox_state;
    Mui_Checkbox_State show_s21_checkbox_state;
    Mui_Checkbox_State show_s12_checkbox_state;
    Mui_Checkbox_State show_s22_checkbox_state;
    Mui_Checkbox_State show_Gopt_checkbox_state;
    Mui_Checkbox_State show_no_idea_what_state;
    Mui_Slider_State slider_state_1;
    Mui_Slider_State slider_state_2;
    Mui_Collapsable_Section_State collapsable_section_state_1;
    Mui_Collapsable_Section_State collapsable_section_state_2;
    Mui_Collapsable_Section_State collapsable_section_state_3;
    Mui_Collapsable_Section_State collapsable_section_state_4;
    Mui_Collapsable_Section_State collapsable_section_state_5;

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

struct Resistor_Ideal_View {
    struct Circuit_Component_Resistor_Ideal *resistor;
    Mui_Collapsable_Section_State collapsable_section_1;
};

struct Resistor_Ideal_Parallel_View {
    struct Circuit_Component_Resistor_Ideal_Parallel *resistor;
    Mui_Collapsable_Section_State collapsable_section_1;
};

struct Capacitor_Ideal_View {
    struct Circuit_Component_Capacitor_Ideal *capacitor;
    Mui_Collapsable_Section_State collapsable_section_1;
};

struct Capacitor_Ideal_Parallel_View {
    struct Circuit_Component_Capacitor_Ideal_Parallel *capacitor;
    Mui_Collapsable_Section_State collapsable_section_1;
};

struct Inductor_Ideal_View {
    struct Circuit_Component_Inductor_Ideal *inductor;
    Mui_Collapsable_Section_State collapsable_section_1;
};

struct Inductor_Ideal_Parallel_View {
    struct Circuit_Component_Inductor_Ideal_Parallel *inductor;
    Mui_Collapsable_Section_State collapsable_section_1;
};

struct Circuit_Component_View {
    union {
        struct Stage_View stage_view;
        struct Resistor_Ideal_View resistor_ideal_view;
        struct Resistor_Ideal_Parallel_View resistor_ideal_parallel_view;
        struct Capacitor_Ideal_View capacitor_ideal_view;
        struct Capacitor_Ideal_Parallel_View capacitor_ideal_parallel_view;
        struct Inductor_Ideal_View inductor_ideal_view;
        struct Inductor_Ideal_Parallel_View inductor_ideal_parallel_view;
    } as;
    CIRCUIT_COMPONENT_KIND kind;
};

void circuit_component_view_init(struct Circuit_Component_View* component_view, struct Circuit_Component* component);
void circuit_component_view_draw(struct Circuit_Component_View* component_view, Mui_Rectangle widget_area, bool is_selected);


// component the stage and stage_view and initialize state.
void stage_view_init(struct Stage_View* stage_view, struct Circuit_Component_Stage* stage);
void stage_view_update_active_setting(struct Stage_View* stage_view, size_t new_setting);
void stage_view_update_data(struct Stage_View* stage_view);
void stage_symbol_draw(Mui_Rectangle symbol_area, bool should_highlight);
void stage_view_settings_draw(struct Stage_View* stage_view, Mui_Rectangle symbol_area);
void stage_view_draw(struct Stage_View* stage_view, Mui_Rectangle widget_area, bool is_selected);

// resistor
void resistor_view_init(struct Resistor_Ideal_View* resistor_view, struct Circuit_Component_Resistor_Ideal* resistor);
void resistor_view_draw(struct Resistor_Ideal_View* resistor_view, Mui_Rectangle widget_area, bool is_selected);
// resistor parallel
void resistor_parallel_view_init(struct Resistor_Ideal_Parallel_View* resistor_parallel_view, struct Circuit_Component_Resistor_Ideal_Parallel* resistor);
void resistor_parallel_view_draw(struct Resistor_Ideal_Parallel_View* resistor_parallel_view, Mui_Rectangle widget_area, bool is_selected);

// capacitor
void capacitor_view_init(struct Capacitor_Ideal_View* capacitor_view, struct Circuit_Component_Capacitor_Ideal* capacitor);
void capacitor_view_draw(struct Capacitor_Ideal_View* capacitor_view, Mui_Rectangle widget_area, bool is_selected);
// capacitor parallel
void capacitor_parallel_view_init(struct Capacitor_Ideal_Parallel_View* capacitor_parallel_view, struct Circuit_Component_Capacitor_Ideal_Parallel* capacitor);
void capacitor_parallel_view_draw(struct Capacitor_Ideal_Parallel_View* resistor_parallel_view, Mui_Rectangle widget_area, bool is_selected);

// inductor
void inductor_view_init(struct Inductor_Ideal_View* inductor_view, struct Circuit_Component_Inductor_Ideal* inductor);
void inductor_view_draw(struct Inductor_Ideal_View* inductor_view, Mui_Rectangle widget_area, bool is_selected);
// inductor parallel
void inductor_parallel_view_init(struct Inductor_Ideal_Parallel_View* inductor_parallel_view, struct Circuit_Component_Inductor_Ideal_Parallel* inductor);
void inductor_parallel_view_draw(struct Inductor_Ideal_Parallel_View* inductor_parallel_view, Mui_Rectangle widget_area, bool is_selected);


#endif //CIRCUIT_VIEWS_H_
