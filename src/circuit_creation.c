#include "circuit.h"

#include "uti.h"
#include "s2p.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"

bool circuit_create_stage_archetype(char* device_settings_csv_file_name, char* dir, struct Circuit_Component_Stage *component_out) {
    // #model, file_name, drain_voltage(V), drain_current(A), temperature(K)

    char *content;
    size_t content_size;
    char full_path[2048];
    if (snprintf(full_path, 2048, "%s/%s", dir, device_settings_csv_file_name) > 2046) {
        printf("ERROR: snprintf overflow\n");
        return false;
    }
    if (!uti_read_entire_file(full_path, &content, &content_size)) {
        return false;
    }

    struct Uti_String_View dry_run_sv = uti_sv_from_parts(content, content_size);

    // first pass dry run to get memory size
    size_t string_data_block_models_length = 0;
    size_t string_data_block_filenames_length = 0;
    size_t length = 0;
    while (dry_run_sv.length > 0) {
        struct Uti_String_View line = uti_sv_trim(uti_sv_chop_by_delim(&dry_run_sv, '\n'));
        if (line.length == 0 || line.text[0] == '#') {
            continue;
        }

        struct Uti_String_View model_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        struct Uti_String_View file_name_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        string_data_block_filenames_length += file_name_sv.length + 1;
        string_data_block_models_length += model_sv.length + 1;

        length++;
    }

    //
    // now malloc and set the data in second pass
    //
    component_out->s2p_infos = malloc(sizeof(*component_out->s2p_infos) * length);
    //char* data_block_filenames = malloc(sizeof(char) * string_data_block_filenames_length);
    char* data_block_models = malloc(sizeof(char) * string_data_block_models_length);
    char* cursor = data_block_models; // Use this to move through the memory
    component_out->models = malloc(sizeof(*component_out->models) * length);
    component_out->current_ds_array = malloc(sizeof(*component_out->current_ds_array) * length);
    component_out->voltage_ds_array = malloc(sizeof(*component_out->voltage_ds_array) * length);
    component_out->temperatures = malloc(sizeof(*component_out->temperatures) * length);
    component_out->n_settings = length;
    component_out->selected_setting = 0;

    struct Uti_String_View content_sv = uti_sv_from_parts(content, content_size);
    size_t i = 0;
    while (content_sv.length > 0) {
        struct Uti_String_View line = uti_sv_trim(uti_sv_chop_by_delim(&content_sv, '\n'));
        if (line.length == 0 || line.text[0] == '#') {
            continue;
        }

        struct Uti_String_View model_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        //char *model_cstr = uti_temp_strndup(model_sv.text, model_sv.length);
        struct Uti_String_View file_name_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        char *file_name_cstr = uti_temp_strndup(file_name_sv.text, file_name_sv.length);
        struct Uti_String_View v_ds_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        char *v_ds_cstr = uti_temp_strndup(v_ds_sv.text, v_ds_sv.length);
        struct Uti_String_View i_ds_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        char *i_ds_cstr = uti_temp_strndup(i_ds_sv.text, i_ds_sv.length);
        struct Uti_String_View temp_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        char *temp_cstr = uti_temp_strndup(temp_sv.text, temp_sv.length);

        component_out->temperatures[i] = strtod(temp_cstr, NULL);
        component_out->current_ds_array[i] = strtod(i_ds_cstr, NULL);
        component_out->voltage_ds_array[i] = strtod(v_ds_cstr, NULL);

        component_out->models[i] = cursor;
        memcpy(component_out->models[i], model_sv.text, model_sv.length);
        cursor[model_sv.length] = '\0';
        cursor += model_sv.length + 1;

        // load the s2p_info
        char* file_dir = dir;
        struct S2P_Info *info = &component_out->s2p_infos[i];
        if (!read_s2p_file(file_name_cstr, file_dir, info))
            return false;
        if (!parse_s2p_file(info, true))
            return false;
        i++;
    }

    assert(i == length);

    return true;
}


bool circuit_create_stage(struct Circuit_Component_Stage *stage_archetype, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_STAGE;
    // shallow copy archetype
    component_out->as.stage = *stage_archetype;
    return true;
}

bool circuit_create_resistor_ideal(double resistance, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_RESISTOR_IDEAL;
    component_out->as.resistor_ideal.R = resistance;
    return true;
}

bool circuit_create_capacitor_ideal(double capacitance, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_CAPACITOR_IDEAL;
    component_out->as.capacitor_ideal.C = capacitance;
    return true;
}

bool circuit_create_inductor_ideal(double inductance, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_INDUCTOR_IDEAL;
    component_out->as.inductor_ideal.L = inductance;
    return true;
}

bool circuit_create_resistor_ideal_parallel(double resistance, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL;
    component_out->as.resistor_ideal_parallel.R = resistance;
    return true;
}

bool circuit_create_capacitor_ideal_parallel(double capacitance, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL;
    component_out->as.capacitor_ideal_parallel.C = capacitance;
    return true;
}

bool circuit_create_inductor_ideal_parallel(double inductance, struct Circuit_Component *component_out) {
    component_out->kind = CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL;
    component_out->as.inductor_ideal_parallel.L = inductance;
    return true;
}


