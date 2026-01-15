// Copyright (C) 2026 Benjamin Froelich
// This file is part of https://github.com/bbeni/impedancer
// For conditions of distribution and use, see copyright notice in project root.
/* Let's have some abstrations
*/
#include "s2p.h"

struct Resitor_Ideal {
    double R;
};

struct Inductor_Ideal {
    double L;
};

struct Capacitor_Ideal {
    double C;
};

struct Stage {
    struct S2P_Info* s2p_infos;
    char **models;
    double *voltage_ds_array;
    double *current_ds_array;
    double *temperatures;
    size_t selected_setting;
    size_t n_settings;
};

bool create_stage_archetype(char* device_settings_csv_file_name, char* dir, struct Stage *stage_out);
