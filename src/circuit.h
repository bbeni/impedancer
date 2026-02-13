// Copyright (C) 2026 Benjamin Froelich
// This file is part of https://github.com/bbeni/impedancer
// For conditions of distribution and use, see copyright notice in project root.
/* Let's have some abstrations
*/
#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include "s2p.h"


struct Circuit_Component_Resistor_Ideal {
    double R;
};

struct Circuit_Component_Inductor_Ideal {
    double L;
};

struct Circuit_Component_Capacitor_Ideal {
    double C;
};

struct Circuit_Component_Resistor_Ideal_Parallel {
    double R;
};

struct Circuit_Component_Inductor_Ideal_Parallel {
    double L;
};

struct Circuit_Component_Capacitor_Ideal_Parallel {
    double C;
};

struct Circuit_Component_Stage {
    struct S2P_Info* s2p_infos;
    char **models;
    double *voltage_ds_array;
    double *current_ds_array;
    double *temperatures;
    size_t selected_setting;
    size_t n_settings;
};

typedef enum {
    CIRCUIT_COMPONENT_RESISTOR_IDEAL,
    CIRCUIT_COMPONENT_CAPACITOR_IDEAL,
    CIRCUIT_COMPONENT_INDUCTOR_IDEAL,
    CIRCUIT_COMPONENT_STAGE,
    CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL,
    CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL,
    CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL,
    CIRCUIT_COMPONENT_KIND_COUNT
} CIRCUIT_COMPONENT_KIND;

struct Circuit_Component {
    union {
        struct Circuit_Component_Stage stage;
        struct Circuit_Component_Resistor_Ideal resistor_ideal;
        struct Circuit_Component_Capacitor_Ideal capacitor_ideal;
        struct Circuit_Component_Inductor_Ideal inductor_ideal;
        struct Circuit_Component_Resistor_Ideal_Parallel resistor_ideal_parallel;
        struct Circuit_Component_Capacitor_Ideal_Parallel capacitor_ideal_parallel;
        struct Circuit_Component_Inductor_Ideal_Parallel inductor_ideal_parallel;
    } as;
    CIRCUIT_COMPONENT_KIND kind;
};

bool circuit_create_stage_archetype(char* device_settings_csv_file_name, char* dir, struct Circuit_Component_Stage *component_out);
bool circuit_create_stage( struct Circuit_Component_Stage *stage_archetype, struct Circuit_Component *component_out);
bool circuit_create_resistor_ideal(double resistance, struct Circuit_Component *component_out);
bool circuit_create_capacitor_ideal(double capacitance, struct Circuit_Component *component_out);
bool circuit_create_inductor_ideal(double inductance, struct Circuit_Component *component_out);
bool circuit_create_resistor_ideal_parallel(double resistance, struct Circuit_Component *component_out);
bool circuit_create_capacitor_ideal_parallel(double capacitance, struct Circuit_Component *component_out);
bool circuit_create_inductor_ideal_parallel(double inductance, struct Circuit_Component *component_out);


struct Complex_2x2_SoA {
    double* r11;
    double* r12;
    double* r21;
    double* r22;
    double* i11;
    double* i12;
    double* i21;
    double* i22;
    // count is implicit by the caller (i.e. n_frequencies)
};

struct Simulation_Component_Intermediate_State {
    struct Complex_2x2_SoA t;
    struct Complex_2x2_SoA s;
};

struct Simulation_State {

    size_t n_components;
    struct Circuit_Component *components_cascade;
    struct Simulation_Component_Intermediate_State *intermediate_states;

    double z0_in;  // Source Impedance
    double z0_out; // Load Impedance

    size_t n_frequencies;
    double *frequencies;
    struct Complex_2x2_SoA s_result;
    struct Complex_2x2_SoA t_result;
    // plottable / derived results
    struct Complex* s11_result_plottable;
    struct Complex* s12_result_plottable;
    struct Complex* s21_result_plottable;
    struct Complex* s22_result_plottable;
    double* stab_mu;
    double* stab_mu_prime;

    bool memory_initalized;
};

struct Simulation_Settings {
    double f_min;
    double f_max;
    double z0_in;
    double z0_out;
    size_t n_frequencies;
};

bool circuit_simulation_setup(struct Circuit_Component *component_cascade, size_t n_components, struct Simulation_State *sim_state, const struct Simulation_Settings *settings);
bool circuit_simulation_do(struct Simulation_State *sim_state, bool print_stdout);
bool circuit_simulation_destroy(struct Simulation_State *sim_state);

void circuit_update_s_and_t_paramas_of_component(struct Simulation_State* sim_state, size_t component_index);
bool circuit_interpolate_sparams_circuit_component(struct Circuit_Component *component, double *frequencies, struct Complex_2x2_SoA *s_out, size_t n_frequencies);

void calc_s_from_t_array(struct Complex_2x2_SoA *t, struct Complex_2x2_SoA *s_out, size_t length);
void calc_t_from_s_array(struct Complex_2x2_SoA *s, struct Complex_2x2_SoA *t_out, size_t length);
void calc_mu_and_mu_prime(struct Complex s11, struct Complex s12, struct Complex s21, struct Complex s22, double* mu_out, double* mu_prime_out);

// less than the target is meant
typedef enum {
    OPTIMIZATION_TYPE_LESS_THAN,
    OPTIMIZATION_TYPE_MORE_THAN
} OPTIMIZATION_TYPE;

typedef enum {
    OPTIMIZATION_TARGET_S11,
    OPTIMIZATION_TARGET_S21,
    OPTIMIZATION_TARGET_S12,
    OPTIMIZATION_TARGET_S22,
    OPTIMIZATION_TARGET_MU,
    OPTIMIZATION_TARGET_MU_PRIME,
} OPTIMIZATION_TARGET;

struct Optimization_Goal {
    OPTIMIZATION_TYPE type;
    OPTIMIZATION_TARGET target;
    double goal_value;
    double f_min;
    double f_max;
    double weight;
    bool active;
    double (*value_map)(size_t, void*); // for example dB(...), mag(...), ...
};

struct Optimizer_State {
    size_t iteration;
    size_t max_iterations;
    double best_total_loss_value;
    struct Circuit_Component *initial_component_cascade;
    struct Circuit_Component *best_component_cascade;
    struct Circuit_Component *temporary_component_cascade;
    size_t n_components;
};

double circuit_optimizer_calculate_loss_hinge_lt(void* values, size_t value_count, double (*value_map)(size_t, void*), double goal);
double circuit_optimizer_calculate_loss_hinge_gt(void* values, size_t value_count, double (*value_map)(size_t, void*), double goal);
double circuit_optimizer_evaluate_goal(const struct Optimization_Goal* goal, struct Simulation_State* sim_state);

bool circuit_optimizer_setup(struct Optimizer_State* state, size_t max_iterations, struct Circuit_Component *intial_component_cascade, size_t n_components);
bool circuit_optimizer_update_one_round(struct Optimizer_State* opt_state, struct Simulation_State* sim_state, const struct Simulation_Settings* sim_settings, const struct Optimization_Goal* goals, size_t goal_count, struct Circuit_Component *component_cascade, size_t n_components);

#endif //CIRCUIT_H_
