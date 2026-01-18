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
    CIRCUIT_COMPONENT_KIND_COUNT
} CIRCUIT_COMPONENT_KIND;

struct Circuit_Component {
    union {
        struct Circuit_Component_Stage stage;
        struct Circuit_Component_Resistor_Ideal resistor_ideal;
        struct Circuit_Component_Capacitor_Ideal capacitor_ideal;
        struct Circuit_Component_Inductor_Ideal inductor_ideal;
    } as;
    CIRCUIT_COMPONENT_KIND kind;
};

bool circuit_create_stage_archetype(char* device_settings_csv_file_name, char* dir, struct Circuit_Component_Stage *component_out);
bool circuit_create_stage( struct Circuit_Component_Stage *stage_archetype, struct Circuit_Component *component_out);
bool circuit_create_resistor_ideal(double resistance, struct Circuit_Component *component_out);
bool circuit_create_capacitor_ideal(double capacitance, struct Circuit_Component *component_out);
bool circuit_create_inductor_ideal(double inductance, struct Circuit_Component *component_out);


#endif //CIRCUIT_H_
