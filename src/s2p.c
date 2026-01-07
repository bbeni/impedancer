#include "s2p.h"
#include "uti.h"

#include "math.h"
#include "stdbool.h"
#include "stdio.h"
#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "mma.h"

#define NOB_NO_MINIRENT
#include "nob.h"

typedef enum { FMT_RI, FMT_MA, FMT_DB } S_Format;


int read_s2p_files(const char* dir, struct S2P_Info_Array *infos) {

    const char* suffix = ".s2p";

    char** file_names;
    size_t file_names_count;
    if (!read_entire_dir(dir, &file_names, &file_names_count)) return 1;

    // naively allocate enough space
    infos->capacity = file_names_count;
    infos->count = 0;
    infos->items = malloc(sizeof(infos->items[0])*infos->capacity);
    memset(infos->items, 0xCD, sizeof(infos->items[0])*infos->capacity);

    for (size_t i = 0; i < file_names_count; i++) {       
        if (strlen(file_names[i]) < 4 || strcmp(file_names[i], ".") == 0 || strcmp(file_names[i], "..") == 0)
            continue;
        if (strncmp(file_names[i] + strlen(file_names[i]) - strlen(suffix), suffix, strlen(suffix)) != 0)
            continue;

        struct S2P_Info * info = &infos->items[infos->count++];
        
        sprintf(info->full_path, "%s%s", dir, file_names[i]);
        sprintf(info->file_name, "%s", file_names[i]);

        if (!read_entire_file(info->full_path, &info->file__content, &info->file__content_size)) {
            infos->count--;
            exit(3);
            continue;
        }
        printf("INFO: read file %s\n", info->full_path);
    }

    if (infos->count == 0) {
        printf("ERROR: No .s2p files found in directory %s\n", dir);
        return 1;
    }

    return 0;
}

struct Complex parse_complex(double v1, double v2, S_Format fmt) {
    struct Complex c;
    switch (fmt) {
        case FMT_MA: // Magnitude / Angle (degrees)
            c.r = v1 * cos(v2 * M_PI / 180.0);
            c.i = v1 * sin(v2 * M_PI / 180.0);
            break;
        case FMT_DB: // Decibels / Angle (degrees)
            double mag = pow(10.0, v1 / 20.0);
            c.r = mag * cos(v2 * M_PI / 180.0);
            c.i = mag * sin(v2 * M_PI / 180.0);
            break;
        case FMT_RI: // Real / Imaginary
        default:
            c.r = v1;
            c.i = v2;
            break;
    }
    return c;
}

int parse_s2p_files(struct S2P_Info_Array *infos, bool calc_z) {

    for (size_t i = 0; i < infos->count; ++i) {
        struct S2P_Info *info = &infos->items[i];
        info->s11.count = 0;
        info->s12.count = 0;
        info->s21.count = 0;
        info->s22.count = 0;
        info->noise.count = 0;
        info->freq.count = 0;

        #define INITIAL_CAP 512
        info->s11.items = malloc(sizeof(*info->s11.items)*INITIAL_CAP);
        info->s11.capacity = INITIAL_CAP;
        info->s21.items = malloc(sizeof(*info->s21.items)*INITIAL_CAP);
        info->s21.capacity = INITIAL_CAP;
        info->s12.items = malloc(sizeof(*info->s12.items)*INITIAL_CAP);
        info->s12.capacity = INITIAL_CAP;
        info->s22.items = malloc(sizeof(*info->s22.items)*INITIAL_CAP);
        info->s22.capacity = INITIAL_CAP;
        info->noise.items = malloc(sizeof(*info->noise.items)*INITIAL_CAP);
        info->noise.capacity = INITIAL_CAP;
        info->freq.items = malloc(sizeof(*info->freq.items)*INITIAL_CAP);
        info->freq.capacity = INITIAL_CAP;


        Nob_String_View content = { .data = info->file__content, .count = info->file__content_size};

        double freq_multiplier = 1.0;
        S_Format format = FMT_RI;

        while (content.count > 0) {
            Nob_String_View line = nob_sv_trim(nob_sv_chop_by_delim(&content, '\n'));
            if (line.count == 0 || *line.data == '!') continue;

            char *line_cstr = nob_temp_sprintf("%.*s", (int)line.count, line.data);

            // 1. Parse Option Line: # <Hz/kHz/MHz/GHz> <S/Y/Z/G/H> <DB/MA/RI> R <val>
            if (*line.data == '#') {
                if (strstr(line_cstr, "GHz")) freq_multiplier = 1e9;
                else if (strstr(line_cstr, "MHz")) freq_multiplier = 1e6;
                else if (strstr(line_cstr, "KHz")) freq_multiplier = 1e3;

                if (strstr(line_cstr, "MA"))      format = FMT_MA;
                else if (strstr(line_cstr, "DB")) format = FMT_DB;
                else if (strstr(line_cstr, "RI")) format = FMT_RI;
                continue;
            }

            double val[9];
            int scanned = sscanf(line_cstr, "%lf %lf %lf %lf %lf %lf %lf %lf %lf",
                                 &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7], &val[8]);

            if (scanned == 9) {
                // S-Parameter Line: Freq S11.1 S11.2 S21.1 S21.2 S12.1 S12.2 S22.1 S22.2
                nob_da_append(&info->freq, val[0] * freq_multiplier);
                nob_da_append(&info->s11, parse_complex(val[1], val[2], format));
                nob_da_append(&info->s21, parse_complex(val[3], val[4], format));
                nob_da_append(&info->s12, parse_complex(val[5], val[6], format));
                nob_da_append(&info->s22, parse_complex(val[7], val[8], format));
            } 
            else if (scanned == 5) {
                // Noise Data Line: Freq Fmin Gamma_Mag Gamma_Ang Rn
                // Freq       Fmin(dB)  Mag(Gopt) Ang(Gopt) Rn/50
                struct Noise_Data nd = {
                    .Fmin = val[1],
                    .GammaOptdB = val[2], // Usually stored as Mag/Ang, can be converted if needed
                    .GammaOptAngle = val[3],
                    .Rn = val[4]
                };
                nob_da_append(&info->noise, nd);
            }
        }

        assert(info->s11.count == info->s12.count);
        assert(info->s11.count == info->s21.count);
        assert(info->s11.count == info->s22.count);

        uti_temp_reset();
        nob_temp_reset();
        printf("INFO: Parsed %zu frequency points from %s\n", info->freq.count, info->file_name);
    }


    if (calc_z) {
        for (size_t i = 0; i < infos->count; ++i) {
            struct S2P_Info *info = &infos->items[i];
            info->z11.count = 0;
            info->z12.count = 0;
            info->z21.count = 0;
            info->z22.count = 0;
            size_t n = info->s11.count;
            info->z11.items = malloc(sizeof(*info->z11.items)*n);
            info->z11.capacity = n;
            info->z21.items = malloc(sizeof(*info->z21.items)*n);
            info->z21.capacity = n;
            info->z12.items = malloc(sizeof(*info->z12.items)*n);
            info->z12.capacity = n;
            info->z22.items = malloc(sizeof(*info->z22.items)*n);
            info->z22.capacity = n;

            for (size_t j = 0; j < n; j++) {
                // no z0 calculated
                struct Complex one_m_s11 = {1 - info->s11.items[j].r, -info->s11.items[j].i};
                struct Complex one_m_s22 = {1 - info->s22.items[j].r, -info->s22.items[j].i};
                struct Complex one_p_s11 = {1 + info->s11.items[j].r,  info->s11.items[j].i};
                struct Complex one_p_s22 = {1 + info->s22.items[j].r,  info->s22.items[j].i};
                struct Complex s12s21 = mma_complex_mult(info->s12.items[j], info->s21.items[j]);
                struct Complex delta_s = mma_complex_subtract(mma_complex_mult(one_m_s11, one_m_s22), s12s21);
                info->z11.items[j] = mma_complex_divide_or_zero(mma_complex_add(mma_complex_mult(one_p_s11, one_m_s22), s12s21), delta_s);
                info->z22.items[j] = mma_complex_divide_or_zero(mma_complex_add(mma_complex_mult(one_m_s11, one_p_s22), s12s21), delta_s);
                info->z12.items[j] = mma_complex_divide_or_zero(info->s12.items[j], delta_s);
                info->z12.items[j].r *= 2; info->z12.items[j].i *= 2;
                info->z21.items[j] = mma_complex_divide_or_zero(info->s21.items[j], delta_s);
                info->z21.items[j].r *= 2; info->z21.items[j].i *= 2;
            }
        }
    }

    return 0;
}

