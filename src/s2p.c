#include "s2p.h"
#include "math.h"

typedef enum { FMT_RI, FMT_MA, FMT_DB } S_Format;

int read_s2p_files(const char* dir, S2P_Infos *infos) {
    Nob_File_Paths files = {0};
    if (!nob_read_entire_dir(dir, &files)) return 1;

    for (size_t i = 0; i < files.count; i++) {
        if (i < 10) continue;
        if (i > 20) break;
        
        if (strlen(files.items[i]) < 4 || strcmp(files.items[i], ".") == 0 || strcmp(files.items[i], "..") == 0)
            continue;

        const char* suffix = ".s2p";
        if (strncmp(files.items[i] + strlen(files.items[i]) - strlen(suffix), suffix, strlen(suffix)) != 0)
            continue;

        S2P_Info info = {0};
        
        char* full_path = nob_temp_sprintf("%s/%s", dir, files.items[i]);
        
        strncpy(info.file_name, files.items[i], strlen(files.items[i]));

        if (!nob_read_entire_file(full_path, &info.file_content)) {
            printf("ERROR: Could not read file %s\n", full_path);
            continue;
        }

        printf("INFO: read file %s\n", full_path);
        nob_da_append(infos, info);
        
        nob_temp_reset();
    }

    if (infos->count == 0) {
        printf("ERROR: No .s2p files found in directory %s\n", dir);
        return 1;
    }

    return 0;
}

Complex parse_complex(double v1, double v2, S_Format fmt) {
    Complex c;
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

int parse_s2p_files(S2P_Infos *infos) {
    for (size_t i = 0; i < infos->count; ++i) {
        S2P_Info *info = &infos->items[i];
        Nob_String_View content = { .data = info->file_content.items, .count = info->file_content.count };

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
                Noise_Data nd = {
                    .Fmin = val[1],
                    .GammaOptdB = val[2], // Usually stored as Mag/Ang, can be converted if needed
                    .GammaOptAngle = val[3],
                    .Rn = val[4]
                };
                nob_da_append(&info->noise, nd);
            }
        }



        nob_temp_reset();
        printf("INFO: Parsed %zu frequency points from %s\n", info->freq.count, info->file_name);
    }
    return 0;
}

