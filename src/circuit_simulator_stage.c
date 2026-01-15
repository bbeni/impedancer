#include "circuit_simulator.h"
#include "uti.h"
#include "s2p.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

bool create_stage_archetype(char* device_settings_csv_file_name, char* dir, struct Stage *stage_out) {
    // #model, file_name, drain_voltage(V), drain_current(A), temperature(K)

    char *content;
    size_t content_size;
    char full_path[2048]; // TODO: check this
    sprintf(full_path, "%s/%s", dir, device_settings_csv_file_name); // TODO: leaks
    if (!uti_read_entire_file(full_path, &content, &content_size)) {
        return false;
    }

    struct Uti_String_View content_sv = uti_sv_from_parts(content, content_size);

    // first pass dry run to get memory size
    size_t string_data_block_models_length = 0;
    size_t string_data_block_filenames_length = 0;
    size_t length = 0;
    struct Uti_String_View line = uti_sv_trim(uti_sv_chop_by_delim(&content_sv, '\n'));
    while (line.length > 0) {
        struct Uti_String_View model_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        struct Uti_String_View file_name_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        //struct Uti_String_View v_ds_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        //struct Uti_String_View i_ds_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        //struct Uti_String_View temp_sv = uti_sv_trim(uti_sv_chop_by_delim(&line, ','));
        string_data_block_filenames_length += file_name_sv.length + 1;
        string_data_block_models_length += model_sv.length + 1;

        line = uti_sv_trim(uti_sv_chop_by_delim(&content_sv, '\n'));
        length++;
    }

    //
    // now malloc and set the data in second pass
    //
    stage_out->s2p_infos = malloc(sizeof(*stage_out->s2p_infos) * length);
    //char* data_block_filenames = malloc(sizeof(char) * string_data_block_filenames_length);
    char* data_block_models = malloc(sizeof(char) * string_data_block_models_length);
    stage_out->models = malloc(sizeof(char *) * length);
    stage_out->current_ds_array = malloc(sizeof(char *) * length);
    stage_out->voltage_ds_array = malloc(sizeof(char *) * length);
    stage_out->temperatures = malloc(sizeof(char *) * length);
    stage_out->n_settings = length;
    stage_out->selected_setting = 0;

    content_sv = uti_sv_from_parts(content, content_size);
    for (size_t i = 0; i < length; i ++) {
        line = uti_sv_trim(uti_sv_chop_by_delim(&content_sv, '\n'));
        if (line.length == 0) {
            continue;
        }
        if (line.text[0] == '#') {
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

        char* line_cstr = uti_temp_strndup(line.text, line.length);

        stage_out->temperatures[i] = strtod(temp_cstr, NULL);
        stage_out->current_ds_array[i] = strtod(i_ds_cstr, NULL);
        stage_out->voltage_ds_array[i] = strtod(v_ds_cstr, NULL);

        stage_out->models[i] = data_block_models;
        memcpy(stage_out->models[i], model_sv.text, model_sv.length);
        data_block_models[model_sv.length] = '\0';
        data_block_models += model_sv.length + 1;

        // load the s2p_info
        char* file_dir = dir;
        struct S2P_Info *info = &stage_out->s2p_infos[i];
        if (!read_s2p_file(file_name_cstr, file_dir, info))
            return false;
        if (!parse_s2p_file(info, true))
            return false;

    }

    for (size_t i = 0; i < length; i ++) {
        printf("%s\n", stage_out->models[i]);
        printf("%f\n", stage_out->voltage_ds_array[i]);
        printf("%f\n", stage_out->current_ds_array[i]);
        printf("%f\n", stage_out->temperatures[i]);
    }

    return true;
}

