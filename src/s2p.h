// Copyright (C) 2026 Benjamin Froelich
// This file is part of https://github.com/bbeni/impedancer
// For conditions of distribution and use, see copyright notice in project root.
#ifndef S2P_H_
#define S2P_H_

#include "stddef.h"
#include "stdbool.h"
#include "mma.h"

struct Double_Array {
    double *items;
    size_t length;
    size_t capacity;
};

struct Complex_Array {
    struct Complex *items;
    size_t length;
    size_t capacity;
};

struct Noise_Data {
    double* freq; // TODO check if it is needed or the same as for s params
    double* Rn;
    double* NFmin;
    struct Complex* GammaOpt;
    size_t length;
    size_t capacity;
};

struct S2P_Info {
    // must be here values
    double R_ref;

    double* freq;
    struct Complex* s11;
    struct Complex* s12;
    struct Complex* s21;
    struct Complex* s22;
    size_t data_length;
    size_t data_capacity;

    char file_name[512];
    char full_path[512];
    char* file_content;
    size_t file_content_size;

    // maybe here values
    struct Noise_Data noise;

    // derived / calculated data
    struct Complex_Array z11;
    struct Complex_Array z12;
    struct Complex_Array z21;
    struct Complex_Array z22;
    struct Complex_Array zGopt;
    struct Double_Array NFmin;
};

struct S2P_Info_Array {
    struct S2P_Info* items;
    size_t length;
    size_t capacity;
};


bool read_s2p_files_from_dir(const char* dir, struct S2P_Info_Array *infos);
bool read_s2p_file(const char* file_name, const char* dir, struct S2P_Info *info);
bool parse_s2p_files(struct S2P_Info_Array *infos, bool calc_z);
bool parse_s2p_file(struct S2P_Info *info, bool calc_z);


#endif // S2P_H_