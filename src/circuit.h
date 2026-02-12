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
};


void calc_s_from_t_array(struct Complex_2x2_SoA *t, struct Complex_2x2_SoA *s_out, size_t length);
void calc_t_from_s_array(struct Complex_2x2_SoA *s, struct Complex_2x2_SoA *t_out, size_t length);
void calc_mu_and_mu_prime(struct Complex s11, struct Complex s12, struct Complex s21, struct Complex s22, double* mu_out, double* mu_prime_out);


struct Simulation_Settings {
    double f_min;
    double f_max;
    double z0_in;
    double z0_out;
    size_t n_frequencies;
};

typedef enum {
    OPTIMIZATION_LESS_THAN,
    OPTIMIZATION_MORE_THAN
} OPTIMIZATION_TYPE;

struct Optimization_Goal {
    OPTIMIZATION_TYPE type;
    double target;
    double f_min;
    double f_max;
    double weight;
    bool active;
};

bool simulation_interpolate_sparams_circuit_component(struct Circuit_Component *component, double *frequencies, struct Complex_2x2_SoA *s_out, size_t n_frequencies);
bool circuit_simulation_setup(struct Circuit_Component *component_cascade, size_t n_components, struct Simulation_State *sim_state, struct Simulation_Settings *settings);
bool circuit_simulation_destroy(struct Simulation_State *sim_state);
bool circuit_simulation_do(struct Simulation_State *sim_state);

#endif //CIRCUIT_H_
